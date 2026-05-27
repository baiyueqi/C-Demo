#include "greedy_solver.h"

#include "evaluator.h"

#include <chrono>
#include <limits>
#include <stdexcept>
#include <vector>

SolverResult GreedySolver::solve(const WtaInstance& instance) const {
    if (instance.weaponCount <= 0 || instance.targetCount <= 0) {
        throw std::invalid_argument("instance weapon and target counts must be positive");
    }

    const auto startTime = std::chrono::steady_clock::now();

    std::vector<int> assignment(instance.weaponCount, -1); // 最终分配方案。
    std::vector<bool> weaponAssigned(instance.weaponCount, false); // 标记武器是否已分配。
    // 当前各目标的生存概率，用于计算继续攻击该目标的边际收益。
    std::vector<double> survivalProbability(instance.targetCount, 1.0);

    for (int assignedCount = 0; assignedCount < instance.weaponCount; ++assignedCount) {
        double bestGain = -std::numeric_limits<double>::infinity(); // 当前轮最大边际收益。
        int bestWeaponIndex = -1; // 当前轮最优武器编号。
        int bestTargetIndex = -1; // 当前轮最优目标编号。

        for (int weaponIndex = 0; weaponIndex < instance.weaponCount; ++weaponIndex) {
            if (weaponAssigned[weaponIndex]) {
                continue;
            }

            for (int targetIndex = 0; targetIndex < instance.targetCount; ++targetIndex) {
                // gain = v_j * s_j * p_ij，表示本轮选择该组合带来的威胁削减量。
                double gain =
                    instance.targets[targetIndex].threatValue *
                    survivalProbability[targetIndex] *
                    instance.killProbability[weaponIndex][targetIndex];

                if (gain > bestGain) {
                    bestGain = gain;
                    bestWeaponIndex = weaponIndex;
                    bestTargetIndex = targetIndex;
                }
            }
        }

        // 固定本轮最优组合，并立即更新目标生存概率。
        assignment[bestWeaponIndex] = bestTargetIndex;
        weaponAssigned[bestWeaponIndex] = true;
        survivalProbability[bestTargetIndex] *=
            (1.0 - instance.killProbability[bestWeaponIndex][bestTargetIndex]);
    }

    EvaluationResult evaluation = evaluateAssignment(instance, assignment);

    const auto endTime = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> runtime = endTime - startTime;

    return {
        "GreedySolver",
        assignment,
        evaluation,
        runtime.count(),
        {},
        {}
    };
}
