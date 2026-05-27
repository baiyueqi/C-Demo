#include "experiment_runner.h"

#include "data_generator.h"
#include "genetic_solver.h"
#include "greedy_solver.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <utility>

ExperimentRunner::ExperimentRunner(std::string outputDirectory)
    : outputDirectory_(std::move(outputDirectory)) {}

void ExperimentRunner::run(const std::vector<ExperimentCase>& cases) {
    if (cases.empty()) {
        throw std::invalid_argument("experiment case list cannot be empty");
    }

    std::ofstream summaryFile(outputDirectory_ + "/summary.csv");
    if (!summaryFile) {
        throw std::runtime_error("failed to open summary output file");
    }

    summaryFile << "instance,solver,weaponCount,targetCount,"
                << "remainingThreat,damageValue,runtimeMs,"
                << "bestValue,averageValue,worstValue,standardDeviation,notes\n";
    summaryFile << std::fixed << std::setprecision(6);

    for (std::size_t caseIndex = 0; caseIndex < cases.size(); ++caseIndex) {
        const ExperimentCase& experimentCase = cases[caseIndex];
        unsigned int dataSeed = 2026 + static_cast<unsigned int>(caseIndex);

        DataGenerator generator(dataSeed);
        WtaInstance instance = generator.generateInstance(
            experimentCase.weaponCount,
            experimentCase.targetCount
        );

        std::cout << "Running " << experimentCase.instanceName
                  << " (" << experimentCase.weaponCount
                  << " x " << experimentCase.targetCount << ")\n";

        GreedySolver greedySolver;
        SolverResult greedyResult = greedySolver.solve(instance);
        summaryFile << experimentCase.instanceName << ','
                    << greedyResult.solverName << ','
                    << experimentCase.weaponCount << ','
                    << experimentCase.targetCount << ','
                    << greedyResult.evaluation.remainingThreat << ','
                    << greedyResult.evaluation.damageValue << ','
                    << greedyResult.runtimeMs << ','
                    << greedyResult.evaluation.damageValue << ','
                    << greedyResult.evaluation.damageValue << ','
                    << greedyResult.evaluation.damageValue << ','
                    << 0.0 << ','
                    << "single run\n";

        std::vector<SolverResult> geneticResults;
        geneticResults.reserve(10);
        for (int runIndex = 0; runIndex < 10; ++runIndex) {
            unsigned int solverSeed =
                2026 + static_cast<unsigned int>(caseIndex * 100 + runIndex);
            GeneticSolver geneticSolver(
                50,
                300,
                0.8,
                0.03,
                3,
                1,
                solverSeed
            );
            geneticResults.push_back(geneticSolver.solve(
                instance,
                greedyResult.assignment
            ));
        }

        writeConvergenceCsv(experimentCase.instanceName, geneticResults.front());

        GeneticStatistics statistics = calculateGeneticStatistics(geneticResults);
        const SolverResult& bestGeneticResult = *std::max_element(
            geneticResults.begin(),
            geneticResults.end(),
            [](const SolverResult& left, const SolverResult& right) {
                return left.evaluation.damageValue < right.evaluation.damageValue;
            }
        );
        writeVisualizationJson(
            experimentCase.instanceName,
            instance,
            greedyResult,
            bestGeneticResult
        );

        summaryFile << experimentCase.instanceName << ','
                    << "GeneticSolver" << ','
                    << experimentCase.weaponCount << ','
                    << experimentCase.targetCount << ','
                    << statistics.averageRemainingThreat << ','
                    << statistics.averageDamageValue << ','
                    << statistics.averageRuntimeMs << ','
                    << statistics.bestDamageValue << ','
                    << statistics.averageDamageValue << ','
                    << statistics.worstDamageValue << ','
                    << statistics.standardDeviation << ','
                    << "10 run average\n";
    }
}

ExperimentRunner::GeneticStatistics
ExperimentRunner::calculateGeneticStatistics(
    const std::vector<SolverResult>& results
) const {
    if (results.empty()) {
        throw std::invalid_argument("genetic result list cannot be empty");
    }

    double bestDamageValue = results.front().evaluation.damageValue;
    double worstDamageValue = results.front().evaluation.damageValue;
    double totalDamageValue = 0.0;
    double totalRemainingThreat = 0.0;
    double totalRuntimeMs = 0.0;

    for (const SolverResult& result : results) {
        double damageValue = result.evaluation.damageValue;
        bestDamageValue = std::max(bestDamageValue, damageValue);
        worstDamageValue = std::min(worstDamageValue, damageValue);
        totalDamageValue += damageValue;
        totalRemainingThreat += result.evaluation.remainingThreat;
        totalRuntimeMs += result.runtimeMs;
    }

    double averageDamageValue = totalDamageValue / results.size();
    double variance = 0.0;
    for (const SolverResult& result : results) {
        double difference = result.evaluation.damageValue - averageDamageValue;
        variance += difference * difference;
    }
    variance /= results.size();

    return {
        bestDamageValue,
        averageDamageValue,
        worstDamageValue,
        std::sqrt(variance),
        totalRemainingThreat / results.size(),
        totalRuntimeMs / results.size()
    };
}

void ExperimentRunner::writeConvergenceCsv(
    const std::string& instanceName,
    const SolverResult& result
) const {
    std::ofstream convergenceFile(
        outputDirectory_ + "/convergence_" + instanceName + ".csv"
    );
    if (!convergenceFile) {
        throw std::runtime_error("failed to open convergence output file");
    }

    convergenceFile << "generation,bestFitness,averageFitness\n";
    convergenceFile << std::fixed << std::setprecision(6);

    for (std::size_t index = 0; index < result.bestFitnessHistory.size(); ++index) {
        convergenceFile << index << ','
                        << result.bestFitnessHistory[index] << ','
                        << result.averageFitnessHistory[index] << '\n';
    }
}

void ExperimentRunner::writeVisualizationJson(
    const std::string& instanceName,
    const WtaInstance& instance,
    const SolverResult& greedyResult,
    const SolverResult& geneticResult
) const {
    std::ofstream jsonFile(
        outputDirectory_ + "/visualization_" + instanceName + ".json"
    );
    if (!jsonFile) {
        throw std::runtime_error("failed to open visualization output file");
    }

    jsonFile << std::fixed << std::setprecision(6);
    jsonFile << "{\n";
    jsonFile << "  \"instanceName\": \"" << instanceName << "\",\n";
    jsonFile << "  \"weaponCount\": " << instance.weaponCount << ",\n";
    jsonFile << "  \"targetCount\": " << instance.targetCount << ",\n";

    jsonFile << "  \"targets\": [\n";
    for (std::size_t index = 0; index < instance.targets.size(); ++index) {
        const Target& target = instance.targets[index];
        jsonFile << "    {\"id\": " << target.id
                 << ", \"threatValue\": " << target.threatValue << "}";
        jsonFile << (index + 1 == instance.targets.size() ? "\n" : ",\n");
    }
    jsonFile << "  ],\n";

    jsonFile << "  \"killProbability\": [\n";
    for (int weaponIndex = 0; weaponIndex < instance.weaponCount; ++weaponIndex) {
        jsonFile << "    [";
        for (int targetIndex = 0; targetIndex < instance.targetCount; ++targetIndex) {
            jsonFile << instance.killProbability[weaponIndex][targetIndex];
            if (targetIndex + 1 < instance.targetCount) {
                jsonFile << ", ";
            }
        }
        jsonFile << "]";
        jsonFile << (weaponIndex + 1 == instance.weaponCount ? "\n" : ",\n");
    }
    jsonFile << "  ],\n";

    auto writeSolverResult = [&jsonFile](const SolverResult& result) {
        jsonFile << "    \"solverName\": \"" << result.solverName << "\",\n";
        jsonFile << "    \"remainingThreat\": " << result.evaluation.remainingThreat << ",\n";
        jsonFile << "    \"damageValue\": " << result.evaluation.damageValue << ",\n";
        jsonFile << "    \"runtimeMs\": " << result.runtimeMs << ",\n";
        jsonFile << "    \"assignment\": [";
        for (std::size_t index = 0; index < result.assignment.size(); ++index) {
            jsonFile << result.assignment[index];
            if (index + 1 < result.assignment.size()) {
                jsonFile << ", ";
            }
        }
        jsonFile << "]\n";
    };

    jsonFile << "  \"greedyResult\": {\n";
    writeSolverResult(greedyResult);
    jsonFile << "  },\n";

    jsonFile << "  \"geneticResult\": {\n";
    writeSolverResult(geneticResult);
    jsonFile << "  }\n";
    jsonFile << "}\n";
}
