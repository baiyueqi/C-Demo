#include "evaluator.h"

#include <stdexcept>
#include <vector>

EvaluationResult evaluateAssignment(
    const WtaInstance& instance,
    const std::vector<int>& assignment
) {
    if (instance.weaponCount <= 0 || instance.targetCount <= 0) {
        throw std::invalid_argument("instance weapon and target counts must be positive");
    }
    if (instance.targets.size() != static_cast<std::size_t>(instance.targetCount)) {
        throw std::invalid_argument("target vector size must match target count");
    }
    if (instance.killProbability.size() != static_cast<std::size_t>(instance.weaponCount)) {
        throw std::invalid_argument("kill probability row count must match weapon count");
    }

    if (assignment.size() != static_cast<std::size_t>(instance.weaponCount)) {
        throw std::invalid_argument("assignment size must match weapon count");
    }

    // survivalProbability[j] 表示目标 j 在所有已分配武器攻击后的生存概率 s_j。
    std::vector<double> survivalProbability(instance.targetCount, 1.0);
    double totalThreat = 0.0; // 所有目标初始威胁值总和。

    for (const Target& target : instance.targets) {
        totalThreat += target.threatValue;
    }

    for (int weaponIndex = 0; weaponIndex < instance.weaponCount; ++weaponIndex) {
        if (instance.killProbability[weaponIndex].size() !=
            static_cast<std::size_t>(instance.targetCount)) {
            throw std::invalid_argument("kill probability column count must match target count");
        }

        int targetIndex = assignment[weaponIndex]; // 当前武器被分配攻击的目标编号。
        if (targetIndex < 0 || targetIndex >= instance.targetCount) {
            throw std::out_of_range("assignment target index is out of range");
        }

        // 多个武器攻击同一目标时，目标生存概率按乘法累积。
        survivalProbability[targetIndex] *=
            (1.0 - instance.killProbability[weaponIndex][targetIndex]);
    }

    double remainingThreat = 0.0; // 所有目标被攻击后的剩余威胁总和。
    for (int targetIndex = 0; targetIndex < instance.targetCount; ++targetIndex) {
        remainingThreat +=
            instance.targets[targetIndex].threatValue *
            survivalProbability[targetIndex];
    }

    return {remainingThreat, totalThreat - remainingThreat};
}
