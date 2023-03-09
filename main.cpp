#include <iostream>
#include <random>
#include <unistd.h>
#include <termios.h>
#include <fcntl.h>

using namespace std;

// linux 用户 不调 win api

#define HEIGHT 40
#define WIDTH 80

// 不要 magic number，尽量声明为常量或宏定义
#define DIRECTION_TOP 1
#define DIRECTION_BOTTOM 2
#define DIRECTION_LEFT 3
#define DIRECTION_RIGHT 4

int move_direction;
int food_xy[2] = { 0, 0 };
int canvas[HEIGHT][WIDTH] = { 0 };

// linux上 的 kbhit
int kbhit() {
    struct termios oldt, newt;
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

void handle_key_hit() {
    if (kbhit()) {
        char input = (char) getchar();
        // 不能转向当前方向的反方向
        switch (input) {
            case 'w': {
                if (move_direction != DIRECTION_BOTTOM)
                    move_direction = DIRECTION_TOP;
                break;
            }
            case 'a': {
                if (move_direction != DIRECTION_RIGHT)
                    move_direction = DIRECTION_LEFT;
                break;
            }
            case 's': {
                if (move_direction != DIRECTION_TOP)
                    move_direction = DIRECTION_BOTTOM;
                break;
            }
            case 'd': {
                if (move_direction != DIRECTION_LEFT)
                    move_direction = DIRECTION_RIGHT;
                break;
            }
            default: {
                break;
            }
        }
        handle_key_hit();
    }
}

// 渲染一帧画面
void render() {
    for (auto & canva : canvas) {
        for (int code : canva) {
            switch (code) {
                case 0: {
                    cout << " ";
                    break;
                }
                case 1: {
                    cout << "\033[1;36m█\033[0m";
                    break;
                }
                case -1: {
                    cout << "\033[1;34m█\033[0m";
                    break;
                }
                case -2: {
                    cout << "\033[1;35m█\033[0m";
                    break;
                }
                default: {
                    cout << "\033[1;33m█\033[0m";
                    break;
                }
            }
        }
        cout << endl;
    }
}

// 生成下一个食物
void next_food() {

    food_xy[0] = rand() % (HEIGHT - 5) + 2;
    food_xy[1] = rand() % (WIDTH - 5) + 2;

    canvas[food_xy[0]][food_xy[1]] = -2;
}

bool move_by_direction() {
    // 全部+1
    for (auto & canva : canvas) {
        for (int & j : canva) {
            int n = j;
            if (n > 0) {
                j += 1;
            }
        }
    }
    // 找到最大值，置为0
    // 并找到值为2的点位
    int max = 0, mPosX, mPosY, pos2X, pos2Y;
    for (int i = 0; i < HEIGHT; ++i) {
        for (int j = 0; j < WIDTH; ++j) {
            int n = canvas[i][j];
            if (n > max) {
                max = n;
                mPosX = i;
                mPosY = j;
            }
            if (n == 2) {
                pos2X = i;
                pos2Y = j;
            }
        }
    }
    // 根据方向在值为2的点位的旁边找一个点位=1
    switch (move_direction) {
        case DIRECTION_TOP: {
            pos2X--;
            break;
        }
        case DIRECTION_BOTTOM: {
            pos2X++;
            break;
        }
        case DIRECTION_LEFT: {
            pos2Y--;
            break;
        }
        case DIRECTION_RIGHT: {
            pos2Y++;
            break;
        }
    }
    if (canvas[pos2X][pos2Y] == 0) {
        canvas[mPosX][mPosY] = 0;
        canvas[pos2X][pos2Y] = 1;
    } else if (canvas[pos2X][pos2Y] == -2) {
        // 吃到食物
        canvas[pos2X][pos2Y] = 1;
        next_food();
    } else if (canvas[pos2X][pos2Y] == -1) {
        canvas[mPosX][mPosY] = 0;
        // 进入边界 到另外一侧
        if (pos2X == HEIGHT - 1) {
            canvas[1][pos2Y] = 1;
        } else if (pos2X == 0) {
            canvas[HEIGHT - 2][pos2Y] = 1;
        } else if (pos2Y == WIDTH - 1) {
            canvas[pos2X][1] = 1;
        } else {
            canvas[pos2X][WIDTH - 1] = 1;
        }
    } else if (canvas[pos2X][pos2Y] > 0) {
        // 咬到自己了
        return false;
    }
    return true;
}

// 每一游戏刻发生的操作
bool tick() {
    render();
    handle_key_hit();
    return move_by_direction();
}

void init() {

    system("stty -echo");

    // 初始化蛇
    canvas[HEIGHT / 2][WIDTH / 2] = 1;
    for (int i = 0; i < 4; ++i) {
        canvas[HEIGHT / 2][WIDTH / 2 - i - 1] = i + 2;
    }
    move_direction = DIRECTION_RIGHT;

    // 初始化边界
    for (int i = 0; i < WIDTH; ++i) {
        canvas[0][i] = -1;
        canvas[HEIGHT - 1][i] = -1;
    }
    for (auto & canva : canvas) {
        canva[0] = -1;
        canva[WIDTH - 1] = -1;
    }

    next_food();
}

int main() {
    init();
    while (tick()) {
        // 0.1 秒为一刻
        usleep(0.1 * 1000 * 1000);
        system("clear");
    }
}