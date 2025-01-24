#include <GL/freeglut.h>
#include <cstdlib>      
#include <ctime>       
#include <iostream>     
#include <string>       
#include <cstdio>       

// Подключаем основные заголовки:
#include "Common.hpp"   
#include "Game.hpp"     
#include "IPiece.hpp"   

// ========== Глобальные переменные для окна/рендеринга ==========

static int   g_windowWidth = 800;
static int   g_windowHeight = 600;
static const int MIN_WINDOW_DIM = 300;
static const int INFO_PANEL_WIDTH = 250;

// Размеры нарисованной доски
static float g_side = 0.f;
static float g_offsetX = 0.f;
static float g_offsetY = 0.f;
static float g_boardMargin = 30.f;
static float g_boardSide = 0.f;
static float g_cellSize = 0.f;

// Кнопки
static const int BTN_X = 10;
static const int BTN_Y_RESTART = 100;
static const int BTN_Y_EXIT = 70;
static const int BTN_W = 100;
static const int BTN_H = 20;

// Глобальный объект Game
Game g_game;

// Для выделения фигуры игроком
std::shared_ptr<IPiece> g_selectedPiece = nullptr;
std::vector<std::pair<int, int>> g_possibleMoves;

// Вспомогательные функции
void renderText(float x, float y, const char* text);
void drawBoard();
void drawInfoPanel();
void handleMouseClickAt(float mx, float my);
void RestartGame();
void ExitGame();

// ========== CALLBACKS OpenGL/FreeGLUT ==========

// (mouseCallback) - вызывается при клике
void mouseCallback(int button, int state, int x, int y) {
    if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
        // переворачиваем Y
        float invY = g_windowHeight - y;
        handleMouseClickAt((float)x, invY);
    }
}

// (displayCallback) - рисуем
void displayCallback() {
    glClear(GL_COLOR_BUFFER_BIT);

    // Информационная панель слева
    drawInfoPanel();

    // Доска
    drawBoard();

    glutSwapBuffers();
}

// (timerCallback) - периодически вызываем update()
void timerCallback(int) {
    if (!g_game.isGameOver()) {
        g_game.update();
        glutPostRedisplay();
        glutTimerFunc(500, timerCallback, 0);
    }
}

// (reshapeCallback) - окно меняет размер
void reshapeCallback(int w, int h) {
    if (w < MIN_WINDOW_DIM || h < MIN_WINDOW_DIM) {
        int newW = (w < MIN_WINDOW_DIM) ? MIN_WINDOW_DIM : w;
        int newH = (h < MIN_WINDOW_DIM) ? MIN_WINDOW_DIM : h;
        glutReshapeWindow(newW, newH);
        return;
    }

    g_windowWidth = w;
    g_windowHeight = h;

    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, w, 0, h);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    float boardAreaWidth = (float)w - INFO_PANEL_WIDTH;
    float boardAreaHeight = (float)h;

    float side = (boardAreaWidth < boardAreaHeight) ? boardAreaWidth : boardAreaHeight;
    g_side = side;

    // Центрируем
    g_offsetX = INFO_PANEL_WIDTH + (boardAreaWidth - side) * 0.5f;
    g_offsetY = (boardAreaHeight - side) * 0.5f;

    // Учитываем boardMargin
    float bs = side - 2 * g_boardMargin;
    if (bs < 0) bs = 0;
    g_boardSide = bs;

    if (g_boardSide > 0) {
        g_cellSize = g_boardSide / 8;
    }
    else {
        g_cellSize = 0;
    }
}

// ========== Функции логики ==========

void RestartGame() {
    g_game.restart();
    g_selectedPiece = nullptr;
    g_possibleMoves.clear();
    std::cout << "Game restarted.\n";

    // перерисуем
    glutPostRedisplay();
}
void ExitGame() {
    std::exit(0);
}

std::vector<std::pair<int, int>> calcPossibleMoves(const Board& board, std::shared_ptr<IPiece> piece) {
    std::vector<std::pair<int, int>> moves;
    int x = piece->getX();
    int y = piece->getY();
    int dx[4] = { 1, -1, 0, 0 };
    int dy[4] = { 0, 0, 1, -1 };

    for (int i = 0; i < 4; ++i) {
        int nx = x + dx[i];
        int ny = y + dy[i];
        if (board.isValidPosition(nx, ny)) {
            auto occupant = board.getPieceAt(nx, ny);
            // Если свободно или чужая фигура - можем ходить
            if (!occupant || occupant->getSide() != piece->getSide()) {
                moves.push_back({ nx, ny });
            }
        }
    }
    return moves;
}

void handleMouseClickAt(float mx, float my) {
    // Проверка кнопок
    // "Restart"
    if (mx >= BTN_X && mx < (BTN_X + BTN_W) &&
        my >= BTN_Y_RESTART && my < (BTN_Y_RESTART + BTN_H)) {
        RestartGame();
        return;
    }
    // "Exit"
    if (mx >= BTN_X && mx < (BTN_X + BTN_W) &&
        my >= BTN_Y_EXIT && my < (BTN_Y_EXIT + BTN_H)) {
        ExitGame();
        return;
    }

    // Если игра окончена или сейчас ход AI - игнорируем
    if (g_game.isGameOver() || g_game.getCurrentTurn() == PlayerSide::AI) {
        return;
    }

    // Проверяем, попали ли на доску
    if (mx < g_offsetX + g_boardMargin ||
        mx > g_offsetX + g_boardMargin + g_boardSide ||
        my < g_offsetY + g_boardMargin ||
        my > g_offsetY + g_boardMargin + g_boardSide)
    {
        // вне доски
        g_selectedPiece = nullptr;
        g_possibleMoves.clear();
        return;
    }

    // Иначе вычисляем клетку
    float localX = mx - (g_offsetX + g_boardMargin);
    float localY = my - (g_offsetY + g_boardMargin);
    int cx = (int)(localX / g_cellSize);
    int cy = (int)(localY / g_cellSize);

    // Если нет выбранной фигуры - пытаемся выбрать
    if (!g_selectedPiece) {
        // ищем HUMAN-фигуру
        auto& allPieces = g_game.getBoard().getPieces();

        // 1) объявляем piece
        std::shared_ptr<IPiece> piece = nullptr;

        // 2) ищем фигуру в координатах (cx, cy)
        for (auto& p : allPieces) {
            if (p->getSide() == PlayerSide::HUMAN &&
                p->getX() == cx &&
                p->getY() == cy)
            {
                piece = p;
                break;
            }
        }

        // 3) если нашли - делаем её g_selectedPiece и вычисляем ходы
        if (piece) {
            g_selectedPiece = piece;
            g_possibleMoves = calcPossibleMoves(g_game.getBoard(), piece);
        }
    }
    else {
        // проверяем, можно ли ходить на (cx, cy)
        bool valid = false;
        for (auto& pm : g_possibleMoves) {
            if (pm.first == cx && pm.second == cy) {
                valid = true;
                break;
            }
        }
        if (valid) {
            g_game.getBoardMutable().movePiece(g_selectedPiece, cx, cy);
            g_selectedPiece = nullptr;
            g_possibleMoves.clear();
            // Завершаем ход
            g_game.endHumanTurn();
        }
        else {
            // сброс
            g_selectedPiece = nullptr;
            g_possibleMoves.clear();
        }
    }

    glutPostRedisplay();
}

// ========== Рендеринг ==========
void drawInfoPanel() {
    glColor3f(0.9f, 0.9f, 0.9f);
    glBegin(GL_QUADS);
    glVertex2f(0, 0);
    glVertex2f(INFO_PANEL_WIDTH, 0);
    glVertex2f(INFO_PANEL_WIDTH, g_windowHeight);
    glVertex2f(0, g_windowHeight);
    glEnd();

    glColor3f(0, 0, 0);
    renderText(10, g_windowHeight - 30, "Controls:");
    renderText(10, g_windowHeight - 50, "- LMB: select piece");
    renderText(10, g_windowHeight - 70, "- LMB: select cell");

    // Кнопка "Restart"
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(BTN_X, BTN_Y_RESTART);
    glVertex2f(BTN_X + BTN_W, BTN_Y_RESTART);
    glVertex2f(BTN_X + BTN_W, BTN_Y_RESTART + BTN_H);
    glVertex2f(BTN_X, BTN_Y_RESTART + BTN_H);
    glEnd();
    glColor3f(0, 0, 0);
    renderText(BTN_X + 10, BTN_Y_RESTART + 5, "Restart");

    // Кнопка "Exit"
    glColor3f(0.7f, 0.7f, 0.7f);
    glBegin(GL_QUADS);
    glVertex2f(BTN_X, BTN_Y_EXIT);
    glVertex2f(BTN_X + BTN_W, BTN_Y_EXIT);
    glVertex2f(BTN_X + BTN_W, BTN_Y_EXIT + BTN_H);
    glVertex2f(BTN_X, BTN_Y_EXIT + BTN_H);
    glEnd();
    glColor3f(0, 0, 0);
    renderText(BTN_X + 30, BTN_Y_EXIT + 5, "Exit");

    if (g_game.isAiAggressive()) {
        renderText(10, g_windowHeight - 100, "AI is Aggressive");
    }
    else {
        renderText(10, g_windowHeight - 100, "AI is Capture Territory");
    }

    // Текущий ход / статус
    if (g_game.isGameOver()) {
        renderText(10, g_windowHeight - 130, "Game Over");
    }
    else {
        auto turn = g_game.getCurrentTurn();
        if (turn == PlayerSide::HUMAN)
            renderText(10, g_windowHeight - 130, "Turn: Human");
        else
            renderText(10, g_windowHeight - 130, "Turn: AI");
    }

    // Счётчики
    int eH = g_game.getBoard().getEatenByHuman();
    int eA = g_game.getBoard().getEatenByAI();
    char buf[64];
    sprintf(buf, "Human eaten: %d", eH);
    renderText(10, g_windowHeight - 160, buf);
    sprintf(buf, "AI eaten: %d", eA);
    renderText(10, g_windowHeight - 180, buf);
}

static void drawLabels() {
    glColor3f(0.f, 0.f, 0.f);

    static const char* rowLabels[8] = { "a","b","c","d","e","f","g","h" };
    static const char* colLabels[8] = { "1","2","3","4","5","6","7","8" };

    int w = g_windowWidth;
    int h = g_windowHeight;

    float L = g_offsetX + g_boardMargin;
    float B = g_offsetY + g_boardMargin;
    float R = L + g_boardSide;
    float T = B + g_boardSide;

    // Ряды (a..h) = y=0..7
    for (int row = 0; row < BOARD_SIZE; ++row) {
        // Центр клетки по Y
        float cy = B + (row + 0.5f) * g_cellSize;

        // слева - немного левее L
        float cxLeft = L - 15;
        // справа - немного правее R
        float cxRight = R + 5;

        // Зажмём в границах окна
        if (cxLeft < 0) cxLeft = 0;
        if (cxRight > w - 20) cxRight = w - 20;
        if (cy < 0) cy = 0;
        if (cy > h - 20) cy = h - 20;

        renderText(cxLeft, cy, rowLabels[row]); // слева
        renderText(cxRight, cy, rowLabels[row]); // справа
    }

    // Столбцы (1..8) = x=0..7
    for (int col = 0; col < BOARD_SIZE; ++col) {
        // центр клетки по X
        float cx = L + (col + 0.5f) * g_cellSize;

        // снизу/сверху
        float cyBottom = B - 15;
        float cyTop = T + 5;

        // clamp
        if (cx < 0) cx = 0;
        if (cx > w - 20) cx = w - 20;
        if (cyBottom < 0) cyBottom = 0;
        if (cyTop > h - 20) cyTop = h - 20;

        renderText(cx, cyBottom, colLabels[col]);
        renderText(cx, cyTop, colLabels[col]);
    }
}

// drawBoard, drawLabels
void drawBoard() {
    // Рисуем клетки
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if ((x + y) % 2 == 0) glColor3f(0.8f, 0.8f, 0.8f);
            else           glColor3f(0.3f, 0.3f, 0.3f);

            float bx = g_offsetX + g_boardMargin + x * g_cellSize;
            float by = g_offsetY + g_boardMargin + y * g_cellSize;
            glBegin(GL_QUADS);
            glVertex2f(bx, by);
            glVertex2f(bx + g_cellSize, by);
            glVertex2f(bx + g_cellSize, by + g_cellSize);
            glVertex2f(bx, by + g_cellSize);
            glEnd();
        }
    }

    // Рисуем фигуры
    const auto& pcs = g_game.getBoard().getPieces();
    for (auto& p : pcs) {
        if (p->getSide() == PlayerSide::HUMAN) glColor3f(0.f, 0.f, 1.f);
        else                                glColor3f(1.f, 0.f, 0.f);

        int cx = p->getX();
        int cy = p->getY();

        float figureScale = 0.65f;
        float figureSize = figureScale * g_cellSize;
        float offset = (g_cellSize - figureSize) * 0.5f;
        float px = g_offsetX + g_boardMargin + cx * g_cellSize + offset;
        float py = g_offsetY + g_boardMargin + cy * g_cellSize + offset;

        glBegin(GL_QUADS);
        glVertex2f(px, py);
        glVertex2f(px + figureSize, py);
        glVertex2f(px + figureSize, py + figureSize);
        glVertex2f(px, py + figureSize);
        glEnd();
    }

    // Рамка
    glColor3f(0.f, 0.f, 0.f);
    glLineWidth(2.f);
    float L = g_offsetX + g_boardMargin;
    float B = g_offsetY + g_boardMargin;
    float R = L + g_boardSide;
    float T = B + g_boardSide;
    glBegin(GL_LINE_LOOP);
    glVertex2f(L, B);
    glVertex2f(R, B);
    glVertex2f(R, T);
    glVertex2f(L, T);
    glEnd();

    drawLabels();

    // Подсветка
    if (g_selectedPiece) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glColor4f(0.f, 1.f, 0.f, 0.4f);

        for (auto& pm : g_possibleMoves) {
            int x = pm.first;
            int y = pm.second;
            float bx = g_offsetX + g_boardMargin + x * g_cellSize;
            float by = g_offsetY + g_boardMargin + y * g_cellSize;
            glBegin(GL_QUADS);
            glVertex2f(bx, by);
            glVertex2f(bx + g_cellSize, by);
            glVertex2f(bx + g_cellSize, by + g_cellSize);
            glVertex2f(bx, by + g_cellSize);
            glEnd();
        }

        glDisable(GL_BLEND);
    }
}

// Рисуем текст
void renderText(float x, float y, const char* text) {
    glRasterPos2f(x, y);
    while (*text) {
        glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, *text);
        ++text;
    }
}

// ========== main() ==========

int main(int argc, char** argv) {
    srand((unsigned)time(nullptr));

    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(g_windowWidth, g_windowHeight);
    glutCreateWindow("Chess-Like Splitted Example");

    glutDisplayFunc(displayCallback);
    glutReshapeFunc(reshapeCallback);
    glutMouseFunc(mouseCallback);
    glutTimerFunc(500, timerCallback, 0);

    glClearColor(1.f, 1.f, 1.f, 1.f);

    glutMainLoop();
    return 0;
}
