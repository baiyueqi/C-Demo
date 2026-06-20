#pragma once

#include "lll.hpp"

#include <string>

namespace lll {

struct AdvancedReductionConfig {
    LllConfig lllConfig;
    int tours = 1;
    int blockSize = 4;
    int enumerationRadius = 1;
    int maxInsertDepth = 4;
    int maxPasses = 1;
    long long maxCandidates = 10000;
    Real improvementRatio = 0.999L;
};

struct AdvancedReductionStats {
    std::string algorithm;
    LllStats lllStats;
    long long candidateCount = 0;
    long long insertionCount = 0;
    int completedPasses = 0;
    double runtimeMs = 0.0;
    bool validReduced = false;
};

struct AdvancedReductionResult {
    Matrix reducedBasis;
    AdvancedReductionStats stats;
};

void validateAdvancedReductionConfig(const AdvancedReductionConfig& config);

AdvancedReductionResult reduceMiniBkz(const Matrix& basis,
                                      const AdvancedReductionConfig& config = {});
AdvancedReductionResult reduceDeepLllPostPass(const Matrix& basis,
                                             const AdvancedReductionConfig& config = {});
AdvancedReductionResult reducePotLllPostPass(const Matrix& basis,
                                            const AdvancedReductionConfig& config = {});

}  // namespace lll
