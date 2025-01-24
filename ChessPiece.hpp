#pragma once
#include "IPiece.hpp"

class ChessPiece : public IPiece {
private:
    int x_, y_;
    PlayerSide side_;

public:
    ChessPiece(int x, int y, PlayerSide side);
    int getX() const override;
    int getY() const override;
    void setPosition(int x, int y) override;
    PlayerSide getSide() const override;
};