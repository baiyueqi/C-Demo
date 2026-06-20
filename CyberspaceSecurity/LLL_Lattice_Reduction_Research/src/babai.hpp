#pragma once

#include "lattice.hpp"

namespace lll {

struct BabaiResult {
    Vector latticeVector;
    std::vector<Integer> coefficients;
    Real distance = 0.0L;
    double runtimeMs = 0.0;
    long long candidateCount = 0;
};

struct RandomizedBabaiConfig {
    int rounds = 32;
    std::uint64_t seed = 2026;
};

struct TailEnumerationConfig {
    int tailBlockSize = 4;
    int windowRadius = 1;
    long long maxCandidates = 10000;
};

struct LocalSearchConfig {
    int searchVectorCount = 4;
    int windowRadius = 1;
    long long maxCandidates = 10000;
};

BabaiResult babaiNearestPlane(const Matrix& basis, const RealVector& target);
BabaiResult randomizedBabaiNearestPlane(const Matrix& basis, const RealVector& target,
                                        const RandomizedBabaiConfig& config);
BabaiResult tailEnumerationBabai(const Matrix& basis, const RealVector& target,
                                 const TailEnumerationConfig& config);
BabaiResult localSearchBabai(const Matrix& basis, const RealVector& target,
                             const LocalSearchConfig& config);
BabaiResult exactClosestByEnumeration(const Matrix& basis, const RealVector& target,
                                      int coefficientRadius);

}  // namespace lll
