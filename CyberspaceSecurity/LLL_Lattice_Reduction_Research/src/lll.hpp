#pragma once

#include "lattice.hpp"

#include <string>

namespace lll {

enum class GsoUpdateStrategy {
    Full,
    Suffix,
};

struct LllConfig {
    Real delta = 0.99L;
    Real eta = 0.51L;
    int maxIterations = 1000000;
    bool verifyOutput = true;
    GsoUpdateStrategy updateStrategy = GsoUpdateStrategy::Suffix;
};

struct LllStats {
    int dimension = 0;
    int ambientDimension = 0;
    Real delta = 0.0L;
    Real eta = 0.0L;
    long long swapCount = 0;
    long long sizeReductionCount = 0;
    long long gsoUpdateCount = 0;
    long long iterationCount = 0;
    double runtimeMs = 0.0;
    Real firstVectorNorm = 0.0L;
    Real determinantEstimate = 0.0L;
    Real rootHermiteFactor = 0.0L;
    Real orthogonalityDefect = 0.0L;
    bool validReduced = false;
    bool terminatedByIterationLimit = false;
};

struct LllResult {
    Matrix reducedBasis;
    LllStats stats;
};

GsoUpdateStrategy parseGsoUpdateStrategy(const std::string& value);
std::string toString(GsoUpdateStrategy strategy);

void validateLllConfig(const LllConfig& config);
bool isLllReduced(const Matrix& basis, const LllConfig& config, Real tolerance = 1e-8L);
LllResult reduceLll(const Matrix& basis, const LllConfig& config = LllConfig{});

}  // namespace lll
