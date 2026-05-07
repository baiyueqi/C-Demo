#ifndef SCHEDULER_H
#define SCHEDULER_H

#include <queue>
#include <vector>

#include "pcb.h"

class Scheduler
{
private:

    std::queue<PCB> RQ1;      // 第一级队列，时间片为4
    std::queue<PCB> RQ2;      // 第二级队列，时间片为8

    std::vector<PCB> RQ3;     // 第三级队列，按短作业优先

    std::vector<PCB> Finish;  // 已完成进程

    int clockTime;            // 模拟系统时钟

public:

    Scheduler();

    void loadData(const std::string& filename);

    void runRQ1();

    void runRQ2();

    void runRQ3();

    void printResult() const;
};

#endif
