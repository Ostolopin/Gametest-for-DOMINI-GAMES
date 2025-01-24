#pragma once
#include "Common.hpp"

class IPiece {
public:
    virtual ~IPiece() = default;
    virtual int getX() const = 0;
    virtual int getY() const = 0;
    virtual void setPosition(int x, int y) = 0;
    virtual PlayerSide getSide() const = 0;
};

