#pragma once
#include "Player.hpp"
#include <memory>
#include <unordered_map>
#include <vector>
#include <optional>
#include <set>
#include <random>

class IPiece;
class Board;

class AIPlayer : public Player {
public:
    ~AIPlayer() noexcept override;
    AIPlayer(PlayerSide side);

    PlayerSide getSide() const override;
    void makeMove(Board& board) override;
    bool isAggressive() const;

private:
    PlayerSide side_;
    bool isAggressive_;
    int  myTurnCount_;
    std::mt19937 gen_;

    // Запоминаем последний ход каждой фигуры (кулдаун)
    std::unordered_map<IPiece*, int> lastMoveTurn_;

    // Структура для BFS
    struct MoveCandidate {
        std::shared_ptr<IPiece> piece;
        std::vector<std::pair<int, int>> path;
        int dist;
    };

    bool tryImmediateCapture(Board& board,
        const std::vector<std::shared_ptr<IPiece>>& myPieces);
    std::optional<MoveCandidate> findBestCandidateByBFS(
        Board& board,
        const std::vector<std::shared_ptr<IPiece>>& myPieces
    );

    std::vector<std::pair<int, int>> gatherTargets(Board& board);
    std::vector<std::pair<int, int>> bfsFindPath(
        Board& board,
        std::shared_ptr<IPiece> piece,
        const std::vector<std::pair<int, int>>& targets
    );
    std::set<std::pair<int, int>> gatherThreatenedCells(Board& board, PlayerSide enemy);
    void randomMove(Board& board);
    bool isPieceAllowedToMove(std::shared_ptr<IPiece> piece);
};