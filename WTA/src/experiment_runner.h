#ifndef EXPERIMENT_RUNNER_H
#define EXPERIMENT_RUNNER_H

#include "wta_instance.h"

#include <string>
#include <vector>

// 单个实验实例配置，例如 WTA2 对应 10 x 10。
struct ExperimentCase {
    std::string instanceName; // 实例编号，如 WTA2。
    int weaponCount;          // 武器数量。
    int targetCount;          // 目标数量。
};

// 批量实验运行器：负责生成数据、运行算法、统计结果并导出 CSV。
class ExperimentRunner {
public:
    explicit ExperimentRunner(std::string outputDirectory);

    void run(const std::vector<ExperimentCase>& cases);

private:
    struct GeneticStatistics {
        double bestDamageValue;
        double averageDamageValue;
        double worstDamageValue;
        double standardDeviation;
        double averageRemainingThreat;
        double averageRuntimeMs;
    };

    GeneticStatistics calculateGeneticStatistics(
        const std::vector<SolverResult>& results
    ) const;
    void writeConvergenceCsv(
        const std::string& instanceName,
        const SolverResult& result
    ) const;
    void writeVisualizationJson(
        const std::string& instanceName,
        const WtaInstance& instance,
        const SolverResult& greedyResult,
        const SolverResult& geneticResult
    ) const;

    std::string outputDirectory_; // 结果输出目录，例如 WTA/results。
};

#endif
