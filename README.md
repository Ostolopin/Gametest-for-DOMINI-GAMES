
Игра на C++ с использованием OpenGL (FreeGLUT).

Описание:
Gametest-for-DOMINI-GAMES — это настольная игра, где игрок и искусственный интеллект соревнуются, перемещая фигуры на доске.

Цель игры —:
Переместить все свои фигуры в зону начальных позиций противника.
Либо съесть все фигуры противника.

Особенности:
Искусственный интеллект с двумя режимами поведения:
  1. Агрессивный (стремится съесть фигуры игрока).
  2. Захват территории (стремится занять начальную зону противника).
Графический интерфейс с подсветкой возможных ходов.
Поддержка взаимодействия через мышь.
Информационная панель с текущим состоянием игры и статистикой.

Управление:
ЛКМ: Выбор фигуры и клетки для хода.
Кнопка Restart: Начать игру заново.
Кнопка Exit: Выход из игры.

Структура проекта:
Game.cpp / Game.hpp: Управление игровой логикой.
Board.cpp / Board.hpp: Реализация доски и её состояний.
AIPlayer.cpp / AIPlayer.hpp: Логика ИИ (агрессивный и захват территории).
Player.cpp / Player.hpp: Логика управления человеком.
ChessPiece.cpp / ChessPiece.hpp: Реализация шахматных фигур.
Gametest.cpp: Основной файл с рендерингом и обработкой пользовательского ввода.

Правила игры:
Каждый игрок начинает с 9 фигур, расположенных на своих стартовых позициях (3x3).
Игроки поочередно делают ходы. Искусственный интеллект ходит автоматически.
Фигура может двигаться на одну клетку по вертикали или горизонтали.
Если фигура попадает на клетку с фигурой противника, она "съедает" её.
Игра заканчивается, если:
  Все фигуры противника съедены.
  Все фигуры игрока (или ИИ) достигли начальной зоны противника.
