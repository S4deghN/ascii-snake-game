/*
 * Created on December 21 2022
 * Free to copy for your homework assignment.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
// For input handling
#include <fcntl.h>
#include <termios.h>

#define W       (32)
#define H       (16)
#define MAX_INX (H * W)
#define BODY_C  's'
#define HEAD_C  'Q'
#define APPLE_C '*'

// Set this to 1 for passing through walls and comming out from the other side
#define PASS_WALL 1

// `CSI 0 ; 0 H` puts the cursor at 0, 0 (defaul position).
// `CSI 0 J` clears from cursor to end of screen.
// `CSI 3 J` clears entire screen and deletes all lines saved in the scrollback
// buffer.
#define cls()            printf("\033[H\033[J\033[3J")
#define hide_cursor()    printf("\033[?25l")
#define show_cursor()    printf("\033[?25h")
#define set_cursor(x, y) printf("\033[%d;%dH", x, y)

typedef struct Point {
    char c;
    struct Point* next;
} Point;

// We have to store the index of the snake's head even though we already have
// a pointer to it. because the `Point` does not store coordinates. We could
// alos opt to include fields like `x` and `y` in the `Point` struct but they
// would have no use except for moving the head and maybe passing walls which
// are already solved quite easily.
typedef struct Snake {
    int len;
    int x;
    int y;
    int vx;   // x axis velocity
    int vy;   // y axis velocity
    Point* head;
    Point* tail;
} Snake;

void drop_apple(Point table[][W]) {
    while (1) {
        int inx = rand();
        int y = inx % H;
        int x = inx % W;

        if (table[y][x].c == ' ') {
            table[y][x].c = APPLE_C;
            break;
        }
    }
}

void init_game(Point table[][W], Snake* snake) {
    // Fill table with spaces
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            table[i][j].c = ' ';
            table[i][j].next = NULL;
        }
    }

    // Fill vertical table borders with '|'
    for (int j = 0; j < W; j += (W - 1)) {
        for (int i = 0; i < H; ++i) {
            table[i][j].c = '|';
        }
    }

    // Fill horizontal table borders with '-'
    for (int i = 0; i < H; i += (H - 1)) {
        for (int j = 0; j < W; ++j) {
            table[i][j].c = '-';
        }
    }

    // We use a fixed position and body direction for now.
    int x = 1;
    int y = 1;

    // Tail of the snake
    table[y][x].c = BODY_C;
    table[y][x].next = &table[y][x + 1];
    snake->tail = &table[y][x];

    // Body of the snake
    for (int j = 1; j < snake->len; ++j) {
        table[y][j + 1].c = BODY_C;
        table[y][j + 1].next = &table[y][x + j +1];
    }

    // Head of the snake
    table[y][x + snake->len].c = HEAD_C;
    table[y][x + snake->len].next = NULL;
    snake->head = &table[y][x + snake->len];
    snake->y = y;
    snake->x = x + snake->len;

    srand(time(NULL));  // Initialization, should only be called once.
    drop_apple(table);
}

void get_input_vxy(int* vx, int* vy) {
    int kbhit(void); // Defined at the end of the file.

    char c = ' ';
    if (kbhit()) {
        // Skip aditional characters sent by arrow keys
        c = getchar(); if (c != 27) return;
        c = getchar(); if (c != 91) return;
        c = getchar();
    }
    switch (c) {
        case 68:      // LEFT
            *vx = -1;
            *vy = 0;
            break;
        case 66:      // DOWN
            *vy = 1;
            *vx = 0;
            break;
        case 65:      // UP
            *vy = -1;
            *vx = 0;
            break;
        case 67:      // RIGHT
            *vx = 1;
            *vy = 0;
            break;
    }
}

void update_table(Point table[][W], Snake* snake) {
    int vx = 0;
    int vy = 0;
    get_input_vxy(&vx, &vy);

    // Ignore changing the velocity if trying to move backwards.
    if (!(snake->vx == -vx || snake->vy == -vy)) {
        snake->vx = vx;
        snake->vy = vy;
    }
    int y = snake->y + snake->vy;
    int x = snake->x + snake->vx;

    // We are opting to move the tail before the head. So that the player could
    // run in a circle. Therefore we check for invalid index after moving the
    // tail.

    if (table[y][x].c != APPLE_C) {
        // Move the tail one up in linked list
        Point* temp = snake->tail->next;
        snake->tail->next = NULL;
        snake->tail->c = ' ';
        snake->tail = temp;
    } else {
        // Don't move the tail and increase score (len) drop a new apple
        snake->len += 1;
        drop_apple(table);
    }

#if PASS_WALL == 1
    // Validate the new head position.
    if (table[y][x].c == BODY_C) {
        printf("You lost!\n");
        exit(1);
    }

    // Move to the other side if hitting walls.
    if (table[y][x].c == '|') {
        x = x == 0 ? x + (W -2) : x - (W - 2);
    } else if (table[y][x].c == '-') {
        y = y == 0 ? y + (H - 2) : y - (H - 2);
    }
#else
    // Validate the new head position.
    if (table[inx].c != ' ' && table[inx].c != APPLE_C) {
        printf("You lost!\n");
        exit(1);
    }
#endif

    // Set the new head position.
    table[y][x].next = NULL;
    table[y][x].c = HEAD_C;
    snake->head->next = &table[y][x];
    snake->head->c = BODY_C;
    snake->head = &table[y][x];
    snake->y = y;
    snake->x = x;
}

void draw_table(const Point table[][W], const Snake* snake) {
    cls();
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            printf("%c", table[i][j].c);
        }
        printf("\r\n");
    }
    printf("score: %d\n", snake->len - 3);
}


int main(void) {
    Point table[H][W];
    Snake snake = {.len = 3, .vx = 1, .vy = 0};

    init_game(table, &snake);

    while (1) {
        update_table(table, &snake);
        draw_table(table, &snake);

        // loop's delay
        usleep(150000);
    }
}

// Copied from the internet for non-blocking, non-line-bufferd input read Thanks
// to Thantos for his answer in this thread:
// https://cboard.cprogramming.com/c-programming/63166-kbhit-linux.html
int kbhit(void) {
    struct termios oldt;
    struct termios newt;
    int ch;
    int oldf;

    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
    oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
    fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

    ch = getchar();

    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
    fcntl(STDIN_FILENO, F_SETFL, oldf);

    if (ch != EOF) {
        ungetc(ch, stdin);
        return 1;
    }

    return 0;
}
