#pragma once
#include <memory>
#include "Board.hpp"
#include "Player.hpp"

class Game {
private:
    Board board_;
    std::unique_ptr<Player> human_;
    std::unique_ptr<Player> ai_;
    PlayerSide currentTurn_;
    bool gameOver_;

public:
    Game();

    Board& getBoardMutable();
    const Board& getBoard() const;

    PlayerSide getCurrentTurn() const;
    bool isGameOver() const;

    void update();        // ����������, ����� ����� ��� AI
    void endHumanTurn();  // ����������, ����� ������� �������� ���

    void restart();       // �������� ����

    bool isAiAggressive() const;


private:
    void checkAfterMove();
};