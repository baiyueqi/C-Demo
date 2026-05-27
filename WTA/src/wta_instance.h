#ifndef WTA_INSTANCE_H
#define WTA_INSTANCE_H

#include <string>
#include <vector>

// 目标信息：id 用于标识目标，threatValue 表示目标威胁值 v_j。
struct Target {
    int id;
    double threatValue;
};

// WTA 问题实例：包含武器数量、目标数量、目标威胁值和毁伤概率矩阵 p_ij。
struct WtaInstance {
    int weaponCount;
    int targetCount;
    std::vector<Target> targets;
    std::vector<std::vector<double>> killProbability;
};

// 评价结果：remainingThreat 越小越好，damageValue 越大越好。
struct EvaluationResult {
    double remainingThreat;
    double damageValue;
};

// 求解器统一返回结构，便于贪心算法、遗传算法和后续实验模块做横向对比。
struct SolverResult {
    std::string solverName;
    // assignment[weaponIndex] = targetIndex，表示某个武器分配给哪个目标。
    std::vector<int> assignment;
    EvaluationResult evaluation;
    double runtimeMs;
    // 遗传算法使用的收敛历史；贪心算法不需要时保持为空。
    std::vector<double> bestFitnessHistory;
    std::vector<double> averageFitnessHistory;
};

#endif
