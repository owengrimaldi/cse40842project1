#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <string.h>

#define WIDTH 40
#define HEIGHT 80
#define MULTI 4
#define FALL_DELAY 250000

typedef struct {
    int shape[4][4];
    int x, y;
} Tetromino;

int shapeI[4][4] = {
    {0, 0, 0, 0},
    {1, 1, 1, 1},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

int shapeO[4][4] = {
    {0, 1, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

int shapeT[4][4] = {
    {0, 1, 1, 1},
    {0, 0, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

int shapeL[4][4] = {
    {0, 1, 0, 0},
    {0, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

int shapeJ[4][4] = {
    {0, 0, 1, 0},
    {0, 0, 1, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

int shapeS[4][4] = {
    {0, 0, 0, 0},
    {0, 0, 1, 1},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

int shapeZ[4][4] = {
    {0, 0, 0, 0},
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0}
};

int (*shapes[7])[4][4] = {&shapeI, &shapeO, &shapeT, &shapeL, &shapeJ, &shapeS, &shapeZ};

WINDOW *window;

int well[20][10] = {0};
int rows = 20;
int cols = 10;

void drawBoard(int well[20][10]) {
    clear();

    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (well[y][x] == 1) {
                mvprintw(y + 1, x + 1, "@");
            }
        }
    }

    for (int x = 0; x < cols + 2; x++) {
        mvprintw(0, x, "-");
        mvprintw(rows + 1, x, "-");
    }

    for (int y = 1; y < rows + 1; y++) {
        mvprintw(y, 0, "|");
        mvprintw(y, cols + 1, "|");
    }

    refresh();
}

void clearTetromino(int well[20][10], int shape[4][4], int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j] == 1) {
                if (y + i >= 0 && y + i < rows && x + j >= 0 && x + j < cols) {
                    well[y + i][x + j] = 0;
                }
            }
        }
    }
}

void drawTetromino(int well[20][10], int shape[4][4], int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j] == 1) {
                if (y + i >= 0 && y + i < rows && x + j >= 0 && x + j < cols) {
                    well[y + i][x + j] = 1;
                }
            }
        }
    }
}

int canMoveDown(int well[20][10], int shape[4][4], int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j] == 1) {
                if (y + i + 1 >= rows || well[y + i + 1][x + j] == 1) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int canMoveLeft(int well[20][10], int shape[4][4], int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j] == 1) {
                if (x + j - 1 < 0 || well[y + i][x + j - 1] == 1) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

int canMoveRight(int well[20][10], int shape[4][4], int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j] == 1) {
                if (x + j + 1 >= cols || well[y + i][x + j + 1] == 1) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

void init() {
    window = initscr();
    keypad(window, TRUE);
    nodelay(window, TRUE);
    curs_set(0);
    noecho();
}

int* getRandomTetromino() {
    int index = rand() % 7;
    return shapes[index];
}

int main() {
    srand(time(NULL));

    init();

    Tetromino currentTetromino = { .shape = {0}, .x = cols/2 - 2, .y = 0 };
    memcpy(currentTetromino.shape, getRandomTetromino(), sizeof(currentTetromino.shape));

    while (1) {
        drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
        drawBoard(well);

        while (1) {
            clearTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);

            if (canMoveDown(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y)) {
                currentTetromino.y++;
            } else {
                drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                drawBoard(well);
                break;
            }

            drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
            drawBoard(well);

            usleep(FALL_DELAY);

            int ch = getch();
            if (ch == 'q' || ch == 'Q') {
                endwin();
                return 0;
            }
            if (ch == 'a') {
                clearTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                if (currentTetromino.x > 0 && canMoveDown(well, currentTetromino.shape, currentTetromino.x - 1, currentTetromino.y)) {
                    currentTetromino.x--;
                }
                drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                drawBoard(well);
            }
            if (ch == 'd') {
                clearTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                if (currentTetromino.x + 4 < cols && canMoveDown(well, currentTetromino.shape, currentTetromino.x + 1, currentTetromino.y)) {
                    currentTetromino.x++;
                }
                drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                drawBoard(well);
            }
        }

        currentTetromino.x = cols/2 - 2;
        currentTetromino.y = 0;
        memcpy(currentTetromino.shape, getRandomTetromino(), sizeof(currentTetromino.shape));
    }

    getch();
    endwin();
    return 0;
}