#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "IPiece.hpp"
#include "Common.hpp"

/**
 * Класс Board отвечает за:
 *  - Хранение списка фигур
 *  - Зоны начальных позиций (чтобы знать «вражескую» зону)
 *  - Логику перемещения (включая поедание фигур)
 *  - Проверку условия победы
 */

static const int BOARD_SIZE = 8;

class Board {
private:
    std::vector<std::shared_ptr<IPiece>> pieces_;
    // Начальные позиции
    std::vector<std::pair<int, int>> initHuman_;
    std::vector<std::pair<int, int>> initAI_;

    int eatenByHuman_;
    int eatenByAI_;

public:
    Board();

    // Инициализировать стандартные позиции (3×3 для AI, 3×3 для HUMAN)
    void initDefaultPositions();

    // Геттеры зон
    const std::vector<std::pair<int, int>>& getInitHuman() const;
    const std::vector<std::pair<int, int>>& getInitAI()    const;

    // Счётчики
    int getEatenByHuman() const;
    int getEatenByAI()    const;

    // Проверка клетки
    bool isValidPosition(int x, int y) const;
    std::shared_ptr<IPiece> getPieceAt(int x, int y) const;
    bool isCellFree(int x, int y) const;

    // Основная логика перемещения
    bool movePiece(std::shared_ptr<IPiece> piece, int nx, int ny);
    void capturePiece(PlayerSide attackerSide, std::shared_ptr<IPiece> occupant);

    // Доступ к вектору фигур
    const std::vector<std::shared_ptr<IPiece>>& getPieces() const;
    std::vector<std::shared_ptr<IPiece>> getPiecesBySide(PlayerSide side) const;

    // Проверка победы
    bool checkWin(PlayerSide side) const;
};
