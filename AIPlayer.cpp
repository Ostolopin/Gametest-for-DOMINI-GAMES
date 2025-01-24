#include "AIPlayer.hpp"
#include "Board.hpp"
#include "IPiece.hpp"
#include <algorithm>
#include <queue>
#include <map>

AIPlayer::~AIPlayer() noexcept = default;

AIPlayer::AIPlayer(PlayerSide side)
    : side_(side), myTurnCount_(0)
{
    std::random_device rd;
    gen_ = std::mt19937(rd());

    // Случайный выбор
    isAggressive_ = (rd() % 2 == 0);
    if (isAggressive_) {
        std::cout << "[AIPlayer] Behavior = Aggressive\n";
    }
    else {
        std::cout << "[AIPlayer] Behavior = Capture Territory\n";
    }
}

PlayerSide AIPlayer::getSide() const {
    return side_;
}

bool AIPlayer::isAggressive() const {
    return isAggressive_;
}

void AIPlayer::makeMove(Board& board) {
    myTurnCount_++;

    auto myPieces = board.getPiecesBySide(side_);
    if (myPieces.empty()) return;

    if (tryImmediateCapture(board, myPieces)) {
        return;
    }

    auto candidate = findBestCandidateByBFS(board, myPieces);
    if (candidate) {
        if (candidate->path.size() > 1) {
            int sx = candidate->path[0].first;
            int sy = candidate->path[0].second;
            int nx = candidate->path[1].first;
            int ny = candidate->path[1].second;
            if (board.movePiece(candidate->piece, nx, ny)) {
                std::cout << "AI(" << (isAggressive_ ? "Agg" : "Capt") << ") BFS: ("
                    << sx << "," << sy << ")->("
                    << nx << "," << ny << ")\n";
                lastMoveTurn_[candidate->piece.get()] = myTurnCount_;
                return;
            }
        }
    }

    randomMove(board);
}

bool AIPlayer::tryImmediateCapture(Board& board,
    const std::vector<std::shared_ptr<IPiece>>& myPieces)
{
    auto shuffled = myPieces;
    std::shuffle(shuffled.begin(), shuffled.end(), gen_);

    for (auto& piece : shuffled) {
        // игнор кулдаун
        int x = piece->getX();
        int y = piece->getY();
        int dx[4] = { 1,-1,0,0 };
        int dy[4] = { 0,0,1,-1 };
        std::vector<int> dirs = { 0,1,2,3 };
        std::shuffle(dirs.begin(), dirs.end(), gen_);

        for (int i = 0; i < 4; i++) {
            int idx = dirs[i];
            int nx = x + dx[idx];
            int ny = y + dy[idx];
            if (!board.isValidPosition(nx, ny)) continue;
            auto occupant = board.getPieceAt(nx, ny);
            if (occupant && occupant->getSide() != piece->getSide()) {
                if (board.movePiece(piece, nx, ny)) {
                    std::cout << "AI immediate capture: ("
                        << x << "," << y << ")->("
                        << nx << "," << ny << ")\n";
                    lastMoveTurn_[piece.get()] = myTurnCount_;
                    return true;
                }
            }
        }
    }
    return false;
}

std::optional<AIPlayer::MoveCandidate> AIPlayer::findBestCandidateByBFS(
    Board& board,
    const std::vector<std::shared_ptr<IPiece>>& myPieces)
{
    auto targets = gatherTargets(board);
    if (targets.empty()) return {};

    auto shuffled = myPieces;
    std::shuffle(shuffled.begin(), shuffled.end(), gen_);

    std::optional<MoveCandidate> best;

    for (auto& p : shuffled) {
        if (!isPieceAllowedToMove(p)) continue;
        auto path = bfsFindPath(board, p, targets);
        if (!path.empty()) {
            int dist = (int)path.size() - 1;
            if (!best || dist < best->dist) {
                best = MoveCandidate{ p, path, dist };
            }
            else if (best && dist == best->dist) {
                if (rand() % 2 == 0) {
                    best = MoveCandidate{ p,path,dist };
                }
            }
        }
    }
    return best;
}

std::vector<std::pair<int, int>> AIPlayer::gatherTargets(Board& board) {
    std::vector<std::pair<int, int>> res;
    PlayerSide enemy = (side_ == PlayerSide::HUMAN) ? PlayerSide::AI : PlayerSide::HUMAN;
    if (isAggressive_) {
        auto en = board.getPiecesBySide(enemy);
        for (auto& ep : en) {
            res.push_back({ ep->getX(), ep->getY() });
        }
    }
    else {
        if (enemy == PlayerSide::AI) {
            auto& zone = board.getInitAI();
            for (auto& c : zone) res.push_back(c);
        }
        else {
            auto& zone = board.getInitHuman();
            for (auto& c : zone) res.push_back(c);
        }
    }
    return res;
}

std::vector<std::pair<int, int>> AIPlayer::bfsFindPath(
    Board& board,
    std::shared_ptr<IPiece> piece,
    const std::vector<std::pair<int, int>>& targets)
{
    if (targets.empty()) return {};
    int sx = piece->getX();
    int sy = piece->getY();

    for (auto& t : targets) {
        if (sx == t.first && sy == t.second) {
            return { {sx,sy} };
        }
    }

    PlayerSide me = piece->getSide();
    PlayerSide enemy = (me == PlayerSide::HUMAN) ? PlayerSide::AI : PlayerSide::HUMAN;
    auto threatened = gatherThreatenedCells(board, enemy);

    std::queue<std::pair<int, int>> q;
    std::map<std::pair<int, int>, bool> visited;
    std::map<std::pair<int, int>, std::pair<int, int>> parent;

    q.push({ sx,sy });
    visited[{sx, sy}] = true;

    int DX[4] = { 1,-1,0,0 };
    int DY[4] = { 0,0,1,-1 };

    bool found = false;
    std::pair<int, int> foundCell;

    while (!q.empty() && !found) {
        auto [cx, cy] = q.front();
        q.pop();
        for (auto& tg : targets) {
            if (cx == tg.first && cy == tg.second) {
                found = true;
                foundCell = { cx,cy };
                break;
            }
        }
        if (found) break;

        for (int i = 0; i < 4; i++) {
            int nx = cx + DX[i];
            int ny = cy + DY[i];
            if (!board.isValidPosition(nx, ny)) continue;
            auto occupant = board.getPieceAt(nx, ny);
            if (occupant && occupant->getSide() == me) {
                continue;
            }
            if (!occupant) {
                if (threatened.count({ nx,ny })) {
                    continue;
                }
            }
            if (!visited[{nx, ny}]) {
                visited[{nx, ny}] = true;
                parent[{nx, ny}] = { cx,cy };
                q.push({ nx,ny });
            }
        }
    }

    if (!found) return {};
    std::vector<std::pair<int, int>> path;
    auto cur = foundCell;
    while (true) {
        path.push_back(cur);
        if (cur.first == sx && cur.second == sy) break;
        cur = parent[cur];
    }
    std::reverse(path.begin(), path.end());
    return path;
}

std::set<std::pair<int, int>> AIPlayer::gatherThreatenedCells(Board& board, PlayerSide enemy) {
    std::set<std::pair<int, int>> result;
    auto en = board.getPiecesBySide(enemy);
    int dx[4] = { 1,-1,0,0 };
    int dy[4] = { 0,0,1,-1 };
    for (auto& ep : en) {
        int x = ep->getX();
        int y = ep->getY();
        for (int i = 0; i < 4; i++) {
            int nx = x + dx[i];
            int ny = y + dy[i];
            if (board.isValidPosition(nx, ny)) {
                result.insert({ nx,ny });
            }
        }
    }
    return result;
}

void AIPlayer::randomMove(Board& board) {
    auto myPieces = board.getPiecesBySide(side_);
    std::shuffle(myPieces.begin(), myPieces.end(), gen_);

    bool done = false;
    for (auto& piece : myPieces) {
        if (!isPieceAllowedToMove(piece)) continue;
        int x = piece->getX();
        int y = piece->getY();

        int dx[4] = { 1,-1,0,0 };
        int dy[4] = { 0,0,1,-1 };
        std::vector<int> dirs = { 0,1,2,3 };
        std::shuffle(dirs.begin(), dirs.end(), gen_);

        for (int i = 0; i < 4 && !done; i++) {
            int idx = dirs[i];
            int nx = x + dx[idx];
            int ny = y + dy[idx];
            if (board.movePiece(piece, nx, ny)) {
                std::cout << "AI fallback: (" << x << "," << y << ")->("
                    << nx << "," << ny << ")\n";
                lastMoveTurn_[piece.get()] = myTurnCount_;
                done = true;
            }
        }
        if (done) break;
    }
}

bool AIPlayer::isPieceAllowedToMove(std::shared_ptr<IPiece> piece) {
    auto it = lastMoveTurn_.find(piece.get());
    if (it == lastMoveTurn_.end()) {
        return true;
    }
    int last = it->second;
    if ((myTurnCount_ - last) >= 1) {
        return true;
    }
    return false;
}