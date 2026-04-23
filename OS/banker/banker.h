// banker.h————头文件
#ifndef BANKER_H
#define BANKER_H

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;

// 资源向量类型
using Vec = vector<int>;
using Mat = vector<vector<int>>;

// 银行家算法类
class Banker {
private:
    int n;              // 进程数
    int m;              // 资源种类数
    Vec Available;      // 可用资源
    Mat Allocation;     // 已分配资源
    Mat Need;           // 需求资源
    Mat Max;            // 最大需求

    // 安全性检查
    bool isSafe(Vec& work, vector<bool>& finish, vector<int>& safeSeq);

public:
    Banker(int procNum, int resNum);

    // 初始化系统状态
    void initSystem(const Vec& avail, const Mat& alloc, const Mat& need);

    // 处理资源请求
    bool requestResource(int procId, const Vec& request);

    // 显示当前状态
    void displayState();

    // 显示安全序列
    void displaySafeSequence(const vector<int>& seq);

    // 获取当前状态
    Vec getAvailable() const { return Available; }
    Mat getAllocation() const { return Allocation; }
    Mat getNeed() const { return Need; }
};

#endif
