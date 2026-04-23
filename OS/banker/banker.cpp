// banker.cpp————核心实现
#include "banker.h"

// 构造函数
Banker::Banker(int procNum, int resNum)
    : n(procNum), m(resNum) {
    Available.resize(m);
    Allocation.resize(n, Vec(m));
    Need.resize(n, Vec(m));
    Max.resize(n, Vec(m));
}

// 初始化系统
void Banker::initSystem(const Vec& avail, const Mat& alloc, const Mat& need) {
    Available = avail;
    Allocation = alloc;
    Need = need;
}

// 显示状态
void Banker::displayState() {
    cout << "\n===== 当前系统状态 =====\n";

    cout << "Available: ";
    for (int x : Available) cout << x << " ";
    cout << endl;

    cout << "\nAllocation:\n";
    for (int i = 0; i < n; i++) {
        cout << "P" << i << ": ";
        for (int j = 0; j < m; j++) {
            cout << Allocation[i][j] << " ";
        }
        cout << endl;
    }

    cout << "\nNeed:\n";
    for (int i = 0; i < n; i++) {
        cout << "P" << i << ": ";
        for (int j = 0; j < m; j++) {
            cout << Need[i][j] << " ";
        }
        cout << endl;
    }
}

// 安全性检测
bool Banker::isSafe(Vec& work, vector<bool>& finish, vector<int>& safeSeq) {
    for (int k = 0; k < n; k++) {
        bool found = false;

        for (int i = 0; i < n; i++) {
            if (!finish[i]) {
                bool ok = true;

                for (int j = 0; j < m; j++) {
                    if (Need[i][j] > work[j]) {
                        ok = false;
                        break;
                    }
                }

                if (ok) {
                    for (int j = 0; j < m; j++) {
                        work[j] += Allocation[i][j];
                    }
                    finish[i] = true;
                    safeSeq.push_back(i);
                    found = true;
                }
            }
        }

        if (!found) break;
    }

    for (bool f : finish) {
        if (!f) return false;
    }
    return true;
}

// 显示安全序列
void Banker::displaySafeSequence(const vector<int>& seq) {
    cout << "安全序列: ";
    for (int i = 0; i < seq.size(); i++) {
        cout << "P" << seq[i];
        if (i != seq.size() - 1) cout << " -> ";
    }
    cout << endl;
}

// 请求资源
bool Banker::requestResource(int pid, const Vec& request) {
    cout << "\n>>> P" << pid << " 请求: ";
    for (int x : request) cout << x << " ";
    cout << endl;

    // 1. request <= need ?
    for (int i = 0; i < m; i++) {
        if (request[i] > Need[pid][i]) {
            cout << "请求超过需求，非法\n";
            return false;
        }
    }

    // 2. request <= available ?
    for (int i = 0; i < m; i++) {
        if (request[i] > Available[i]) {
            cout << "资源不足，阻塞\n";
            return false;
        }
    }

    // 3. 试探分配
    for (int i = 0; i < m; i++) {
        Available[i] -= request[i];
        Allocation[pid][i] += request[i];
        Need[pid][i] -= request[i];
    }

    // 4. 安全性检查
    Vec work = Available;
    vector<bool> finish(n, false);
    vector<int> safeSeq;

    if (isSafe(work, finish, safeSeq)) {
        cout << "分配成功\n";
        displaySafeSequence(safeSeq);
        return true;
    } else {
        // 回滚
        for (int i = 0; i < m; i++) {
            Available[i] += request[i];
            Allocation[pid][i] -= request[i];
            Need[pid][i] += request[i];
        }
        cout << "不安全，回滚\n";
        return false;
    }
}