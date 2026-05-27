#ifndef GENETIC_SOLVER_H
#define GENETIC_SOLVER_H

#include "wta_instance.h"

#include <random>
#include <utility>
#include <vector>

// 遗传算法求解器：通过种群迭代搜索 damageValue 更高的分配方案。
class GeneticSolver {
public:
    // 参数默认值采用设计文档中的推荐配置。
    GeneticSolver(
        int populationSize = 50,
        int maxGeneration = 300,
        double crossoverRate = 0.8,
        double mutationRate = 0.03,
        int tournamentSize = 3,
        int eliteCount = 1,
        unsigned int randomSeed = 2026
    );

    SolverResult solve(
        const WtaInstance& instance,
        const std::vector<int>& seedAssignment = {}
    );

private:
    // 一个个体就是一个完整分配方案，fitness 当前直接等于 damageValue。
    struct Individual {
        std::vector<int> chromosome;
        EvaluationResult evaluation;
        double fitness;
    };

    Individual createRandomIndividual(const WtaInstance& instance);
    Individual createSeedIndividual(
        const WtaInstance& instance,
        const std::vector<int>& seedAssignment
    ) const;
    void evaluateIndividual(const WtaInstance& instance, Individual& individual) const;
    // 锦标赛选择：随机抽取若干个体，返回其中适应度最高者。
    const Individual& tournamentSelect(const std::vector<Individual>& population);
    // 单点交叉：交换两个父代在交叉点后的染色体片段。
    std::pair<Individual, Individual> crossover(
        const Individual& parentA,
        const Individual& parentB
    );
    // 变异：按概率随机改变某些武器的目标分配。
    void mutate(Individual& individual, int targetCount);

    int populationSize_;       // 种群规模。
    int maxGeneration_;        // 最大迭代代数。
    double crossoverRate_;     // 交叉概率。
    double mutationRate_;      // 变异概率。
    int tournamentSize_;       // 锦标赛选择规模。
    int eliteCount_;           // 精英保留数量。
    std::mt19937 randomEngine_; // 随机数引擎。
};

#endif
