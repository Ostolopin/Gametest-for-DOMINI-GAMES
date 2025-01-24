#include "Game.hpp"
#include "Player.hpp"
#include "AIPlayer.hpp"
#include <iostream>

Game::Game()
    : human_(std::make_unique<HumanPlayer>(PlayerSide::HUMAN)),
    ai_(std::make_unique<AIPlayer>(PlayerSide::AI)),
    currentTurn_(PlayerSide::HUMAN),
    gameOver_(false)
{
    board_.initDefaultPositions();
}

Board& Game::getBoardMutable() {
    return board_;
}

const Board& Game::getBoard() const {
    return board_;
}

PlayerSide Game::getCurrentTurn() const {
    return currentTurn_;
}

bool Game::isGameOver() const {
    return gameOver_;
}

void Game::update() {
    if (gameOver_) return;
    if (currentTurn_ == PlayerSide::AI) {
        ai_->makeMove(board_);
        checkAfterMove();
    }
}

bool Game::isAiAggressive() const {
    // Преобразуем unique_ptr<Player> -> AIPlayer*
    AIPlayer* aiPtr = dynamic_cast<AIPlayer*>(ai_.get());
    if (aiPtr) {
        return aiPtr->isAggressive();
    }
    // на всякий случай, если dynamic_cast не сработал, вернём false
    return false;
}


void Game::endHumanTurn() {
    if (currentTurn_ == PlayerSide::HUMAN) {
        checkAfterMove();
    }
}

void Game::restart() {
    board_.initDefaultPositions();
    currentTurn_ = PlayerSide::HUMAN;
    gameOver_ = false;
}

void Game::checkAfterMove() {
    if (board_.checkWin(currentTurn_)) {
        std::cout << (currentTurn_ == PlayerSide::HUMAN ? "Human" : "AI") << " wins!\n";
        gameOver_ = true;
        return;
    }
    currentTurn_ = (currentTurn_ == PlayerSide::HUMAN) ? PlayerSide::AI : PlayerSide::HUMAN;
}