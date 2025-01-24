#include "Player.hpp"
#include "Board.hpp" // ����� makeMove(...) ��� ������������ Board

HumanPlayer::HumanPlayer(PlayerSide s)
    : side_(s)
{}

PlayerSide HumanPlayer::getSide() const {
    return side_;
}

void HumanPlayer::makeMove(Board& /*board*/) {
    // ��� �������� ������ - � �������� ����
    std::cout << "[HumanPlayer] (mouse-based)\n";
}