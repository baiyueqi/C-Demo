#ifndef EVALUATOR_H
#define EVALUATOR_H

#include "wta_instance.h"

#include <vector>

// 统一评价函数：所有求解器都通过它计算剩余威胁和毁伤收益。
EvaluationResult evaluateAssignment(
    const WtaInstance& instance,
    // assignment[weaponIndex] = targetIndex。
    const std::vector<int>& assignment
);

#endif
