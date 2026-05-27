#include "genetic_solver.h"

#include "evaluator.h"

#include <algorithm>
#include <chrono>
#include <numeric>
#include <stdexcept>
#include <utility>

GeneticSolver::GeneticSolver(
    int populationSize,        // 种群规模：每一代保留的候选分配方案数量。
    int maxGeneration,        // 最大迭代代数：遗传算法运行多少轮进化。
    double crossoverRate,     // 交叉概率：两个父代发生染色体交换的概率。
    double mutationRate,      // 变异概率：单个基因随机改变目标编号的概率。
    int tournamentSize,       // 锦标赛规模：每次选择时随机比较的个体数量。
    int eliteCount,           // 精英数量：每代直接保留的最优个体数量。
    unsigned int randomSeed   // 随机种子：保证实验结果可复现。
)
    : populationSize_(populationSize),
      maxGeneration_(maxGeneration),
      crossoverRate_(crossoverRate),
      mutationRate_(mutationRate),
      tournamentSize_(tournamentSize),
      eliteCount_(eliteCount),
      randomEngine_(randomSeed) {
    if (populationSize_ <= 0 || maxGeneration_ <= 0) {
        throw std::invalid_argument("population size and max generation must be positive");
    }
    if (crossoverRate_ < 0.0 || crossoverRate_ > 1.0 ||
        mutationRate_ < 0.0 || mutationRate_ > 1.0) {
        throw std::invalid_argument("crossover and mutation rates must be in [0, 1]");
    }
    if (tournamentSize_ <= 0 || eliteCount_ < 0) {
        throw std::invalid_argument("tournament size must be positive and elite count cannot be negative");
    }
}

SolverResult GeneticSolver::solve(
    const WtaInstance& instance,
    const std::vector<int>& seedAssignment
) {
    if (instance.weaponCount <= 0 || instance.targetCount <= 0) {
        throw std::invalid_argument("instance weapon and target counts must be positive");
    }

    const auto startTime = std::chrono::steady_clock::now();

    // 初始种群由随机分配方案组成，每个方案天然满足“每个武器分配一个目标”。
    std::vector<Individual> population; // 当前代种群。
    population.reserve(populationSize_);
    if (!seedAssignment.empty()) {
        population.push_back(createSeedIndividual(instance, seedAssignment));
    }
    for (int index = 0; index < populationSize_; ++index) {
        if (static_cast<int>(population.size()) >= populationSize_) {
            break;
        }
        population.push_back(createRandomIndividual(instance));
    }

    std::vector<double> bestFitnessHistory; // 每代最优适应度记录。
    std::vector<double> averageFitnessHistory; // 每代平均适应度记录。
    bestFitnessHistory.reserve(maxGeneration_);
    averageFitnessHistory.reserve(maxGeneration_);

    // bestIndividual 保存整个迭代过程中见过的最优个体。
    Individual bestIndividual = *std::max_element(
        population.begin(),
        population.end(),
        [](const Individual& left, const Individual& right) {
            return left.fitness < right.fitness;
        }
    );

    for (int generation = 0; generation < maxGeneration_; ++generation) {
        // 排序后，population.front() 就是当前代最优个体。
        std::sort(
            population.begin(),
            population.end(),
            [](const Individual& left, const Individual& right) {
                return left.fitness > right.fitness;
            }
        );

        if (population.front().fitness > bestIndividual.fitness) {
            bestIndividual = population.front();
        }

        double totalFitness = std::accumulate( // 当前代适应度总和。
            population.begin(),
            population.end(),
            0.0,
            [](double total, const Individual& individual) {
                return total + individual.fitness;
            }
        );
        bestFitnessHistory.push_back(bestIndividual.fitness);
        averageFitnessHistory.push_back(totalFitness / population.size());

        // 下一代先放入精英个体，避免当前最优解在交叉或变异中丢失。
        std::vector<Individual> nextPopulation; // 正在生成的下一代种群。
        nextPopulation.reserve(populationSize_);

        int actualEliteCount = std::min(eliteCount_, populationSize_); // 实际保留精英数。
        for (int index = 0; index < actualEliteCount; ++index) {
            nextPopulation.push_back(population[index]);
        }

        while (static_cast<int>(nextPopulation.size()) < populationSize_) {
            // 选择父代后执行交叉、变异，再重新计算子代适应度。
            const Individual& parentA = tournamentSelect(population); // 第一个父代。
            const Individual& parentB = tournamentSelect(population); // 第二个父代。
            auto children = crossover(parentA, parentB); // 交叉产生的两个子代。

            mutate(children.first, instance.targetCount);
            mutate(children.second, instance.targetCount);
            evaluateIndividual(instance, children.first);
            evaluateIndividual(instance, children.second);

            nextPopulation.push_back(children.first);
            if (static_cast<int>(nextPopulation.size()) < populationSize_) {
                nextPopulation.push_back(children.second);
            }
        }

        population = std::move(nextPopulation);
    }

    const Individual& finalBestIndividual = *std::max_element(
        population.begin(),
        population.end(),
        [](const Individual& left, const Individual& right) {
            return left.fitness < right.fitness;
        }
    );
    if (finalBestIndividual.fitness > bestIndividual.fitness) {
        bestIndividual = finalBestIndividual;
    }

    const auto endTime = std::chrono::steady_clock::now();
    const std::chrono::duration<double, std::milli> runtime = endTime - startTime;

    return {
        "GeneticSolver",
        bestIndividual.chromosome,
        bestIndividual.evaluation,
        runtime.count(),
        bestFitnessHistory,
        averageFitnessHistory
    };
}

GeneticSolver::Individual GeneticSolver::createRandomIndividual(
    const WtaInstance& instance
) {
    std::uniform_int_distribution<int> targetDistribution( // 随机目标编号分布。
        0,
        instance.targetCount - 1
    );

    Individual individual; // 随机生成的新个体。
    individual.chromosome.resize(instance.weaponCount);
    // chromosome[weaponIndex] = targetIndex。
    for (int weaponIndex = 0; weaponIndex < instance.weaponCount; ++weaponIndex) {
        individual.chromosome[weaponIndex] = targetDistribution(randomEngine_);
    }

    evaluateIndividual(instance, individual);
    return individual;
}

GeneticSolver::Individual GeneticSolver::createSeedIndividual(
    const WtaInstance& instance,
    const std::vector<int>& seedAssignment
) const {
    Individual individual;
    individual.chromosome = seedAssignment;
    evaluateIndividual(instance, individual);
    return individual;
}

void GeneticSolver::evaluateIndividual(
    const WtaInstance& instance,
    Individual& individual
) const {
    individual.evaluation = evaluateAssignment(instance, individual.chromosome);
    // 设计文档规定使用毁伤收益作为适应度，越大表示方案越优。
    individual.fitness = individual.evaluation.damageValue;
}

const GeneticSolver::Individual& GeneticSolver::tournamentSelect(
    const std::vector<Individual>& population
) {
    std::uniform_int_distribution<int> indexDistribution( // 随机个体下标分布。
        0,
        static_cast<int>(population.size()) - 1
    );

    const Individual* bestIndividual = &population[indexDistribution(randomEngine_)]; // 锦标赛当前最优。
    for (int index = 1; index < tournamentSize_; ++index) {
        const Individual& candidate = population[indexDistribution(randomEngine_)]; // 本次抽到的候选个体。
        if (candidate.fitness > bestIndividual->fitness) {
            bestIndividual = &candidate;
        }
    }

    return *bestIndividual;
}

std::pair<GeneticSolver::Individual, GeneticSolver::Individual>
GeneticSolver::crossover(
    const Individual& parentA,
    const Individual& parentB
) {
    Individual childA = parentA; // 第一个子代，初始为父代 A 的拷贝。
    Individual childB = parentB; // 第二个子代，初始为父代 B 的拷贝。

    // 未触发交叉时，直接返回父代拷贝作为子代。
    std::uniform_real_distribution<double> probabilityDistribution(0.0, 1.0);
    if (probabilityDistribution(randomEngine_) >= crossoverRate_ ||
        parentA.chromosome.size() < 2) {
        return {childA, childB};
    }

    std::uniform_int_distribution<int> pointDistribution(
        1,
        static_cast<int>(parentA.chromosome.size()) - 1
    );
    int crossoverPoint = pointDistribution(randomEngine_); // 单点交叉位置。

    // 交叉点之后的基因片段互换。
    for (int index = crossoverPoint; index < static_cast<int>(parentA.chromosome.size()); ++index) {
        childA.chromosome[index] = parentB.chromosome[index];
        childB.chromosome[index] = parentA.chromosome[index];
    }

    // 子代染色体已变化，旧评价结果失效，后续会重新 evaluateIndividual。
    childA.evaluation = {};
    childA.fitness = 0.0;
    childB.evaluation = {};
    childB.fitness = 0.0;

    return {childA, childB};
}

void GeneticSolver::mutate(Individual& individual, int targetCount) {
    std::uniform_real_distribution<double> probabilityDistribution(0.0, 1.0);
    std::uniform_int_distribution<int> targetDistribution(0, targetCount - 1);

    for (int& targetIndex : individual.chromosome) {
        if (probabilityDistribution(randomEngine_) < mutationRate_) {
            targetIndex = targetDistribution(randomEngine_);
        }
    }
}
