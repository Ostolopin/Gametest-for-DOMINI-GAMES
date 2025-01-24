#include "ChessPiece.hpp"

ChessPiece::ChessPiece(int x, int y, PlayerSide s)
    : x_(x), y_(y), side_(s)
{}

int ChessPiece::getX() const { return x_; }
int ChessPiece::getY() const { return y_; }

void ChessPiece::setPosition(int x, int y) {
    x_ = x;
    y_ = y;
}

PlayerSide ChessPiece::getSide() const {
    return side_;
}