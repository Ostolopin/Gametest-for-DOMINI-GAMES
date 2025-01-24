#include "Player.hpp"
#include "Board.hpp" // чтобы makeMove(...) мог использовать Board

HumanPlayer::HumanPlayer(PlayerSide s)
    : side_(s)
{}

PlayerSide HumanPlayer::getSide() const {
    return side_;
}

void HumanPlayer::makeMove(Board& /*board*/) {
    // Для человека логика - в колбэках мыши
    std::cout << "[HumanPlayer] (mouse-based)\n";
}