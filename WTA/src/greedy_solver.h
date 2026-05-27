#ifndef GREEDY_SOLVER_H
#define GREEDY_SOLVER_H

#include "wta_instance.h"

// 贪心求解器：每轮选择当前边际收益最大的“武器-目标”组合。
class GreedySolver {
public:
    // 返回完整分配方案、评价结果和运行时间。
    SolverResult solve(const WtaInstance& instance) const;
};

#endif
