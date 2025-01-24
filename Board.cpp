#include "Board.hpp"
#include "ChessPiece.hpp"
#include <iostream>

Board::Board()
    : eatenByHuman_(0), eatenByAI_(0)
{
    pieces_.reserve(16);
}

void Board::initDefaultPositions() {
    pieces_.clear();
    initHuman_.clear();
    initAI_.clear();
    eatenByHuman_ = 0;
    eatenByAI_ = 0;

    // AI (верхний левый 3?3)
    for (int row = (int)BOARD_SIZE - 3; row < (int)BOARD_SIZE; ++row) {
        for (int col = 0; col < 3; ++col) {
            pieces_.push_back(std::make_shared<ChessPiece>(col, row, PlayerSide::AI));
            initAI_.push_back({ col,row });
        }
    }

    // HUMAN (нижний правый 3?3)
    for (int row = 0; row < 3; ++row) {
        for (int col = (int)BOARD_SIZE - 3; col < (int)BOARD_SIZE; ++col) {
            pieces_.push_back(std::make_shared<ChessPiece>(col, row, PlayerSide::HUMAN));
            initHuman_.push_back({ col,row });
        }
    }
}

const std::vector<std::pair<int, int>>& Board::getInitHuman() const {
    return initHuman_;
}
const std::vector<std::pair<int, int>>& Board::getInitAI() const {
    return initAI_;
}

int Board::getEatenByHuman() const {
    return eatenByHuman_;
}
int Board::getEatenByAI() const {
    return eatenByAI_;
}

bool Board::isValidPosition(int x, int y) const {
    return (x >= 0 && x < (int)BOARD_SIZE && y >= 0 && y < (int)BOARD_SIZE);
}

std::shared_ptr<IPiece> Board::getPieceAt(int x, int y) const {
    for (auto& p : pieces_) {
        if (p->getX() == x && p->getY() == y) {
            return p;
        }
    }
    return nullptr;
}

bool Board::isCellFree(int x, int y) const {
    return (getPieceAt(x, y) == nullptr);
}

bool Board::movePiece(std::shared_ptr<IPiece> piece, int nx, int ny) {
    if (!isValidPosition(nx, ny)) return false;
    auto occupant = getPieceAt(nx, ny);
    if (occupant) {
        // сво€ фигура => нельз€
        if (occupant->getSide() == piece->getSide()) {
            return false;
        }
        // иначе поедаем
        capturePiece(piece->getSide(), occupant);
    }
    piece->setPosition(nx, ny);
    return true;
}

void Board::capturePiece(PlayerSide attackerSide, std::shared_ptr<IPiece> occupant) {
    auto it = std::find(pieces_.begin(), pieces_.end(), occupant);
    if (it != pieces_.end()) {
        pieces_.erase(it);
    }
    if (attackerSide == PlayerSide::HUMAN) {
        ++eatenByHuman_;
    }
    else {
        ++eatenByAI_;
    }
}

const std::vector<std::shared_ptr<IPiece>>& Board::getPieces() const {
    return pieces_;
}

std::vector<std::shared_ptr<IPiece>> Board::getPiecesBySide(PlayerSide side) const {
    std::vector<std::shared_ptr<IPiece>> result;
    for (auto& p : pieces_) {
        if (p->getSide() == side) {
            result.push_back(p);
        }
    }
    return result;
}

bool Board::checkWin(PlayerSide side) const {
    // ѕротивник
    PlayerSide enemy = (side == PlayerSide::HUMAN) ? PlayerSide::AI : PlayerSide::HUMAN;
    auto enemyPieces = getPiecesBySide(enemy);
    if (enemyPieces.empty()) {
        // все фигуры врага съедены
        return true;
    }

    // все мои в зоне противника?
    const auto& targetZone = (enemy == PlayerSide::AI) ? initAI_ : initHuman_;
    auto myPieces = getPiecesBySide(side);
    if (myPieces.empty()) {
        return false;
    }

    for (auto& mp : myPieces) {
        bool inZone = false;
        for (auto& tz : targetZone) {
            if (mp->getX() == tz.first && mp->getY() == tz.second) {
                inZone = true;
                break;
            }
        }
        if (!inZone) return false;
    }
    return true;
}
