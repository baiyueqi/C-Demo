#ifndef PCB_H
#define PCB_H

#include <string>

struct PCB
{
    std::string name;

    int need;          // 剩余运行时间
    int originalNeed;  // 初始运行时间

    int wait;          // 已等待时间

    int turn;          // 周转时间

    int rq1Count;      // 在RQ1中已轮转次数
};

#endif