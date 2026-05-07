#include <iostream>
#include <fstream>
#include <algorithm>

#include "../include/scheduler.h"

using namespace std;

Scheduler::Scheduler()
{
    clockTime = 0;
}

void Scheduler::loadData(const string& filename)
{
    ifstream fin(filename);

    if (!fin.is_open())
    {
        cout << "文件打开失败" << endl;
        return;
    }

    int type;

    PCB temp;

    while (fin >> type)
    {
        // 输入格式：队列类型 进程名 运行时间 已等待时间
        fin >> temp.name;
        fin >> temp.need;
        fin >> temp.wait;

        temp.originalNeed = temp.need;

        temp.turn = 0;

        temp.rq1Count = 0;

        if (type == 1)
        {
            // type为1的进程进入高优先级队列
            RQ1.push(temp);
        }
        else
        {
            // 其他进程直接进入第二级队列
            RQ2.push(temp);
        }
    }

    fin.close();
}

void Scheduler::runRQ1()
{
    cout << endl;
    cout << "========== RQ1 开始 ==========" << endl;

    while (!RQ1.empty())
    {
        PCB current = RQ1.front();

        RQ1.pop();

        // RQ1每次最多运行4个时间单位
        int t = min(4, current.need);

        current.need -= t;

        clockTime += t;

        cout << current.name
             << " 运行 "
             << t
             << " 剩余 "
             << current.need
             << " clock="
             << clockTime
             << endl;

        if (current.need == 0)
        {
            // 进程完成，记录周转时间
            current.turn = clockTime + current.wait;

            Finish.push_back(current);

            cout << current.name
                 << " 完成"
                 << endl;
        }
        else
        {
            current.rq1Count++;

            if (current.rq1Count < 3)
            {
                // 未达到轮转次数上限，继续留在RQ1
                RQ1.push(current);

                cout << current.name
                     << " 返回RQ1"
                     << endl;
            }
            else
            {
                // 在RQ1运行3次仍未完成，降级到RQ2
                RQ2.push(current);

                cout << current.name
                     << " 转入RQ2"
                     << endl;
            }
        }
    }
}

void Scheduler::runRQ2()
{
    cout << endl;
    cout << "========== RQ2 开始 ==========" << endl;

    while (!RQ2.empty())
    {
        PCB current = RQ2.front();

        RQ2.pop();

        // RQ2每次最多运行8个时间单位
        int t = min(8, current.need);

        current.need -= t;

        clockTime += t;

        cout << current.name
             << " 运行 "
             << t
             << " 剩余 "
             << current.need
             << " clock="
             << clockTime
             << endl;

        if (current.need == 0)
        {
            // 进程完成，记录周转时间
            current.turn = clockTime + current.wait;

            Finish.push_back(current);

            cout << current.name
                 << " 完成"
                 << endl;
        }
        else
        {
            // RQ2仍未完成的进程降级到RQ3
            RQ3.push_back(current);

            cout << current.name
                 << " 转入RQ3"
                 << endl;
        }
    }
}

void Scheduler::runRQ3()
{
    cout << endl;
    cout << "========== RQ3 开始 ==========" << endl;

    while (!RQ3.empty())
    {
        int pos = 0;

        // 在RQ3中选择剩余时间最短的进程
        for (int i = 1; i < (int)RQ3.size(); i++)
        {
            if (RQ3[i].need < RQ3[pos].need)
            {
                pos = i;
            }
        }

        PCB current = RQ3[pos];

        RQ3.erase(RQ3.begin() + pos);

        // RQ3中选中的进程一次运行到完成
        int t = current.need;

        current.need = 0;

        clockTime += t;

        cout << current.name
             << " 运行 "
             << t
             << " 完成 "
             << " clock="
             << clockTime
             << endl;

        current.turn = clockTime + current.wait;

        Finish.push_back(current);
    }
}

void Scheduler::printResult() const
{
    cout << endl;
    cout << "========== 最终结果 ==========" << endl;

    if (Finish.empty())
    {
        cout << "没有完成的进程，无法计算平均周转时间" << endl;
        return;
    }

    double sum = 0;

    for (const auto& p : Finish)
    {
        cout << p.name
             << " 周转时间: "
             << p.turn
             << endl;

        sum += p.turn;
    }

    cout << endl;

    cout << "平均周转时间: "
         << sum / Finish.size()
         << endl;
}
