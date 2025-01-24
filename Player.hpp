#pragma once
#include "Common.hpp"
#include <iostream>

class Board; // forward declaration

class Player {
public:
    virtual ~Player() noexcept = default;
    virtual void makeMove(Board& board) = 0;
    virtual PlayerSide getSide() const = 0;
};

class HumanPlayer : public Player {
private:
    PlayerSide side_;
public:
    HumanPlayer(PlayerSide side);
    ~HumanPlayer() noexcept override = default;

    PlayerSide getSide() const override;
    void makeMove(Board& board) override;
};