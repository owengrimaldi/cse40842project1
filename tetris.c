/*
tetris.c - Tetris in C (ogrimald)

Tetris in C's main and only file.
Uses ncurses to recreate Tetris on the command-line.
*/

#include <stdlib.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <string.h>

#define WIDTH 40
#define HEIGHT 80
#define MULTI 4
#define BOARD_WIDTH 10
#define BOARD_HEIGHT 20

#define SPEED_VALUE 0.97

// block falling speed
int FALL_DELAY = 250000;
int fastDropSpeed = 50000;
int isFastDropping = 0;

typedef struct {
    int shape[4][4];
    int x, y;
} Tetromino;

// 4x4 matrix shapes for all 7 Tetrominos
// you can edit them however you want
int shapeI[4][4] = {
    {1, 1, 1, 1},
    {0, 0, 0, 0},
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
    {0, 0, 1, 1},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

int shapeZ[4][4] = {
    {1, 1, 0, 0},
    {0, 1, 1, 0},
    {0, 0, 0, 0},
    {0, 0, 0, 0}
};

int (*shapes[7])[4][4] = {&shapeI, &shapeO, &shapeT, &shapeL, &shapeJ, &shapeS, &shapeZ};
int colors[7] = {1, 2, 3, 4, 5, 6, 7};

int color;
WINDOW *window;

int well[BOARD_HEIGHT][BOARD_WIDTH] = {0};
int rows = BOARD_HEIGHT;
int cols = BOARD_WIDTH;

int score = 0; 
int speed = 1;         

// increases speed
void increaseSpeed() {
    static int speedIncrement = 1; // amount to increase speed
    static int minDelay = 50000;   // minimum delay (faster speed limit)

    speed++;
    if (FALL_DELAY > minDelay) {
        FALL_DELAY -= speedIncrement * 5000;  // decrease delay
        if (FALL_DELAY < minDelay) {
            FALL_DELAY = minDelay;  // ensure delay doesn't go below minimum
        }
    }
}

// clears tetris rows and lowers above
void clearFullRows(int well[BOARD_HEIGHT][BOARD_WIDTH]) {
    int row, col;
    int fullRowCount = 0;

    for (row = rows - 1; row >= 0; row--) {
        int isFull = 1;
        for (col = 0; col < cols; col++) {
            if (well[row][col] == 0) {
                isFull = 0;
                break;
            }
        }
        if (isFull) {
            fullRowCount++;
            // clear the row
            for (int i = row; i >= 1; i--) {
                memcpy(well[i], well[i - 1], cols * sizeof(int));
            }
            memset(well[0], 0, cols * sizeof(int)); // clear the top row
            row++; // recheck this row as new blocks may now fill it
        }
    }

    // update the score based on the number of rows cleared
    if (fullRowCount > 0) {
        int points = 0;
        switch (fullRowCount) {
            case 1: points = 100; break;
            case 2: points = 300; break;
            case 3: points = 500; break;
            case 4: points = 800; break;
        }
        score += points * speed;
    }
}

// draw the board every game frame
void drawBoard(int well[BOARD_HEIGHT][BOARD_WIDTH], int gameOver) {
    clear();

    // draw title
    mvprintw(0, 0, "Tetris in C - Owen Grimaldi");

    // draw the game board
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            if (well[y][x] == 1) {
                attron(COLOR_PAIR(color));  // set color for the block
                mvprintw(y + 2, 2 * x + 1, "  ");  // print two spaces for each block
                attroff(COLOR_PAIR(color));  // turn off color
            }
        }
    }

    // draw borders around the game board
    for (int x = 0; x < 2 * cols + 2; x++) {
        mvprintw(1, x, "-");
        mvprintw(rows + 2, x, "-");
    }

    for (int y = 2; y < rows + 2; y++) {
        mvprintw(y, 0, "|");
        mvprintw(y, 2 * cols + 1, "|");
    }

    // draw the side information
    for (int y = 0; y < rows; y++) {
        mvprintw(y + 2, 2 * cols + 3, "  ");
    }
    mvprintw(2, 2 * cols + 3, "Controls:");
    mvprintw(3, 2 * cols + 3, "  a: Move Left");
    mvprintw(4, 2 * cols + 3, "  d: Move Right");
    mvprintw(5, 2 * cols + 3, "  w: Rotate");
    mvprintw(6, 2 * cols + 3, "  s: Accelerate Down (hold)");
    mvprintw(7, 2 * cols + 3, "  q: Quit");

    mvprintw(9, 2 * cols + 3, "Tips:");
    mvprintw(10, 2 * cols + 3, "  1. Inputs are buffered, every input will occur");
    mvprintw(11, 2 * cols + 3, "  2. Tetrominos are in random order");
    mvprintw(12, 2 * cols + 3, "  3. Don't stack the middle");

    mvprintw(14, 2 * cols + 3, "Speed: %d", speed);
    mvprintw(15, 2 * cols + 3, "Score: %d", score);

    if (gameOver) {
        mvprintw(17, 2 * cols + 3, "Game over! Your score is: %d", score);
    }

    refresh();
}

//clears tetrominos off board after theyve been placed in the well
void clearTetromino(int well[BOARD_HEIGHT][BOARD_WIDTH], int shape[4][4], int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j] == 1) {
                if (y + i >= 0 && y + i < rows && x + j >= 0 && x + j < cols) {
                    well[y + i][x + j] = 0;
                    mvprintw(y + i + 1, 2 * (x + j) + 1, "  ");  // clear two spaces per block
                }
            }
        }
    }
}

// draw current moving tetromino
void drawTetromino(int well[BOARD_HEIGHT][BOARD_WIDTH], int shape[4][4], int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j] == 1) {
                if (y + i >= 0 && y + i < rows && x + j >= 0 && x + j < cols) {
                    well[y + i][x + j] = 1;
                    attron(COLOR_PAIR(color));
                    mvprintw(y + i + 1, 2 * (x + j) + 1, "  ");
                    attroff(COLOR_PAIR(color));
                }
            }
        }
    }
}

// checks if a tetromino can move down
int canMoveDown(int well[BOARD_HEIGHT][BOARD_WIDTH], int shape[4][4], int x, int y) {
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

// checks if tetromino can move left
int canMoveLeft(int well[BOARD_HEIGHT][BOARD_WIDTH], int shape[4][4], int x, int y) {
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

// checks if tetromino can move right
int canMoveRight(int well[BOARD_HEIGHT][BOARD_WIDTH], int shape[4][4], int x, int y) {
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

// rotates current tetromino shape
void rotateTetromino(int shape[4][4]) {
    int temp[4][4];
    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            temp[x][3 - y] = shape[y][x];
        }
    }
    memcpy(shape, temp, sizeof(temp));
}

// init ncurses, colors
void init() {
    window = initscr();
    keypad(window, TRUE);
    nodelay(window, TRUE);
    curs_set(0);
    noecho();

    if (has_colors()) {
        start_color();
        init_pair(1, COLOR_CYAN, COLOR_CYAN);   // Cyan for shape I
        init_pair(2, COLOR_YELLOW, COLOR_YELLOW); // Yellow for shape O
        init_pair(3, COLOR_MAGENTA, COLOR_MAGENTA); // Magenta for shape T
        init_pair(4, COLOR_BLUE, COLOR_BLUE);   // Blue for shape L
        init_pair(5, COLOR_GREEN, COLOR_GREEN);   // Green for shape J
        init_pair(6, COLOR_WHITE, COLOR_WHITE); // White for shape S
        init_pair(7, COLOR_RED, COLOR_RED);     // Red for shape Z
    }

    color = colors[(rand() % 7)];
}

// gets random shape for next tetromino
int (*getRandomTetromino())[4][4] {
    int index = rand() % 7;
    return shapes[index];
}

// checks if tetromino can be placed at the spawn point, game over if false
int canPlaceTetromino(int well[BOARD_HEIGHT][BOARD_WIDTH], int shape[4][4], int x, int y) {
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            if (shape[i][j] == 1) {
                if (x + j < 0 || x + j >= cols || y + i >= rows || well[y + i][x + j] == 1) {
                    return 0;
                }
            }
        }
    }
    return 1;
}

// drops y value of current tetromino
void dropTetromino(int well[BOARD_HEIGHT][BOARD_WIDTH], Tetromino *tetromino) {
    while (canMoveDown(well, tetromino->shape, tetromino->x, tetromino->y)) {
        tetromino->y++;
    }
}

// main
int main() {
    srand(time(NULL));

    init();

    int gameOver = 0;
    int delay = FALL_DELAY; // initialize delay with default value

    while (!gameOver) {
        // picks random color every new tetromino
        color = colors[(rand() % 7)];
        Tetromino currentTetromino = { .shape = {0}, .x = cols/2 - 2, .y = 0};
        memcpy(currentTetromino.shape, getRandomTetromino(), sizeof(currentTetromino.shape));

        // checks if game is over
        if (!canPlaceTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y)) {
            gameOver = 1;
            drawBoard(well, gameOver);
            while (1) {
                int ch = getch();
                if (ch == 'q' || ch == 'Q') {
                    endwin();
                    return 0;
                }
            }
        }

        while (1) {
            clearTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);

            if (canMoveDown(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y)) {
                currentTetromino.y++;
            } else {
                drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                drawBoard(well, 0);

                // handle cleared rows and update score
                clearFullRows(well);

                // increase speed after placing a new tetromino
                speed++;
                delay = delay * SPEED_VALUE; // calculate new speed

                break;
            }

            drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
            drawBoard(well, 0);

            // adjust speed based on fast drop state
            if (isFastDropping) {
                usleep(fastDropSpeed);
            } else {
                usleep(delay); // adjust speed
            }

            int ch = getch();
            // quit
            if (ch == 'q' || ch == 'Q') {
                endwin();
                return 0;
            }
            // move left
            if (ch == 'a') {
                clearTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                if (canMoveLeft(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y)) {
                    currentTetromino.x--;
                }
                drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                drawBoard(well, 0);
            }
            // move right
            if (ch == 'd') {
                clearTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                if (canMoveRight(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y)) {
                    currentTetromino.x++;
                }
                drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                drawBoard(well, 0);
            }
            // accelerate down
            if (ch == 's') {
                isFastDropping = 1;
            }
            // if no key is pressed, reset fast drop
            if (ch == ERR) {
                isFastDropping = 0;
            }
            // rotate tetromino
            if (ch == 'w') { 
                clearTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                
                rotateTetromino(currentTetromino.shape);

                // check if the new rotation is valid
                if (!canPlaceTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y)) {
                    // if not valid, rotate back
                    rotateTetromino(currentTetromino.shape);
                }

                drawTetromino(well, currentTetromino.shape, currentTetromino.x, currentTetromino.y);
                drawBoard(well, 0);
            }
        }

        currentTetromino.x = cols/2 - 2;
        currentTetromino.y = 0;
        memcpy(currentTetromino.shape, getRandomTetromino(), sizeof(currentTetromino.shape));
    }

    endwin();
    return 0;
}