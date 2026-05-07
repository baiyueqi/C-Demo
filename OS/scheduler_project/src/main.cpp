#include "../include/scheduler.h"

int main()
{
    Scheduler s;

    // 读取进程数据并初始化各级队列
    s.loadData("data/input.txt");

    // 依次执行三级反馈队列调度
    s.runRQ1();

    s.runRQ2();

    s.runRQ3();

    // 输出周转时间统计结果
    s.printResult();

    return 0;
}
