/*
 * Created on December 21 2022
 * Author: Sadegh Nobakhti
 *
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

#define W       32
#define H       16
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
    struct Point* n;  // next point
} Point;

// We have to store the index of the snake's head eventhough we already have
// a pointer to it. because the `Point` does not store coordinates. We could
// alos opt to include fields like `x` and `y` to the `Point` struct but they
// would have no use except for moving the head and maybe passing walls which
// are already solved quite easily.
typedef struct Snake {
    int len;
    int vx;   // x axis velocity
    int vy;   // y axis velocity
    int inx;  // Snake's head inx
    Point* head;
    Point* tail;
} Snake;

void drop_apple(Point* table) {
    while (1) {
        int a_inx = rand() % MAX_INX;

        if (table[a_inx].c == ' ') {
            table[a_inx].c = APPLE_C;
            break;
        }
    }
}

void init_game(Point* table, Snake* snake) {
    // Fill table with spaces
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            table[(i * W) + j].c = ' ';
            table[(i * W) + j].n = NULL;
        }
    }

    // Fill vertical table borders with '|'
    for (int j = 0; j < W; j += (W - 1)) {
        for (int i = 0; i < H; ++i) {
            table[(i * W) + j].c = '|';
        }
    }

    // Fill horizontal table borders with '-'
    for (int i = 0; i < H; i += (H - 1)) {
        for (int j = 0; j < W; ++j) {
            table[(i * W) + j].c = '-';
        }
    }

    // We use a fixed position and body direction for now.
    int x = 1;
    int y = 1;
    int inx = (y * W) + x;

    // Tail of the snake
    table[inx].c = BODY_C;
    table[inx].n = &table[inx + 1];
    snake->tail = &table[inx];

    // Body of the snake
    for (int i = 1; i < snake->len; ++i) {
        table[inx + i].c = BODY_C;
        table[inx + i].n = &table[inx + i + 1];
    }

    // Head of the snake
    table[inx + snake->len].c = HEAD_C;
    table[inx + snake->len].n = NULL;
    snake->head = &table[inx + snake->len];
    snake->inx = inx + snake->len;

    srand(time(NULL));  // Initialization, should only be called once.
    drop_apple(table);
}

void draw_table(const Point* table, const Snake* snake) {
    cls();
    for (int i = 0; i < H; ++i) {
        for (int j = 0; j < W; ++j) {
            printf("%c", table[(i * W) + j].c);
        }
        printf("\r\n");
    }
    printf("j: down, k: up, h: left, l: right\n");
    printf("score: %d\n", snake->len - 3);
}

// Defined at the end of the file.
int kbhit(void);
void get_input_vxy(int* vx, int* vy) {
    char c = ' ';
    if (kbhit()) {
        c = getchar();
    }

    switch (c) {
        case 'h':
            *vx = -1;
            *vy = 0;
            break;
        case 'j':
            *vy = 1;
            *vx = 0;
            break;
        case 'k':
            *vy = -1;
            *vx = 0;
            break;
        case 'l':
            *vx = 1;
            *vy = 0;
            break;
        default:; break;
    }
}

void update_table(Point* table, Snake* snake) {
    int vx = 0;
    int vy = 0;
    get_input_vxy(&vx, &vy);

    // Ignore changing the velocity if trying to move backwards.
    if (!(snake->vx == -vx || snake->vy == -vy)) {
        snake->vx = vx;
        snake->vy = vy;
    }
    int inx = snake->inx + snake->vx + (snake->vy * W);

    // We are opting to move the tail before the head. So that the player could
    // run in a circle. Therefore we check for invalid index after moving the
    // tail.

    if (table[inx].c != APPLE_C) {
        // Move the tail one up in linked list
        Point* temp = snake->tail->n;
        snake->tail->n = NULL;
        snake->tail->c = ' ';
        snake->tail = temp;
    } else {
        // Don't move the tail and increase score (len) drop a new apple
        snake->len += 1;
        drop_apple(table);
    }

#if PASS_WALL == 1
    // Validate the new head position.
    if (table[inx].c == BODY_C) {
        printf("You lost!\n");
        exit(1);
    }

    // Move to the other side if hitting walls.
    if (table[inx].c == '|') {
        inx = (inx % W) > (W / 2) ? inx - (W - 2) : inx + (W - 2);
    } else if (table[inx].c == '-') {
        inx = (inx / W) > (H / 2) ? inx - ((H - 2) * W) : inx + ((H - 2) * W);
    }
#else
    // Validate the new head position.
    if (table[inx].c != ' ' && table[inx].c != APPLE_C) {
        printf("You lost!\n");
        exit(1);
    }
#endif

    // Set the new head position.
    table[inx].n = NULL;
    table[inx].c = HEAD_C;
    snake->head->n = &table[inx];
    snake->head->c = BODY_C;
    snake->head = &table[inx];
    snake->inx = inx;
}

int main(int argc, char** argv) {
    Point table[W][H];
    Snake snake = {.len = 3, .vx = 1, .vy = 0};

    init_game(&table[0][0], &snake);

    while (1) {
        update_table(&table[0][0], &snake);
        draw_table(&table[0][0], &snake);

        // loop's delay
        usleep(150000);
    }
}

// Copied from the internet for non-blocking, non-line-bufferd input read Thanks
// to Thantos's answer in this thread:
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
