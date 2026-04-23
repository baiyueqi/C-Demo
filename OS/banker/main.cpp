// main.cpp———测试入口
#include "banker.h"

int main() {
    int n = 5, m = 3;

    Banker banker(n, m);

    Vec Available = {2, 3, 3};

    Mat Allocation = {
        {2, 1, 2},
        {4, 0, 2},
        {3, 0, 5},
        {2, 0, 4},
        {3, 1, 4}
    };

    Mat Need = {
        {3, 4, 7},
        {1, 3, 4},
        {0, 0, 3},
        {2, 2, 1},
        {1, 1, 0}
    };

    banker.initSystem(Available, Allocation, Need);

    banker.displayState();

    // 测试请求
    banker.requestResource(1, {0, 3, 4}); // P1
    banker.displayState();

    banker.requestResource(3, {1, 0, 1}); // P3
    banker.displayState();

    banker.requestResource(0, {2, 0, 1}); // P0
    banker.displayState();

    banker.requestResource(2, {0, 0, 2}); // P2
    banker.displayState();

    return 0;
}