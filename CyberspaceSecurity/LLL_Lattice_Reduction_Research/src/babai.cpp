#include "babai.hpp"

#include "gso.hpp"

#include <chrono>
#include <cmath>
#include <limits>
#include <random>
#include <stdexcept>
#include <utility>

namespace lll {
namespace {

Integer roundRealToInteger(Real value) {
    const Real rounded = std::round(value);
    if (rounded < static_cast<Real>(std::numeric_limits<Integer>::min()) ||
        rounded > static_cast<Real>(std::numeric_limits<Integer>::max())) {
        throw std::overflow_error("Babai coefficient does not fit int64");
    }
    return static_cast<Integer>(rounded);
}

Integer floorRealToInteger(Real value) {
    const Real floored = std::floor(value);
    if (floored < static_cast<Real>(std::numeric_limits<Integer>::min()) ||
        floored > static_cast<Real>(std::numeric_limits<Integer>::max())) {
        throw std::overflow_error("Babai floor coefficient does not fit int64");
    }
    return static_cast<Integer>(floored);
}

Integer ceilRealToInteger(Real value) {
    const Real ceiled = std::ceil(value);
    if (ceiled < static_cast<Real>(std::numeric_limits<Integer>::min()) ||
        ceiled > static_cast<Real>(std::numeric_limits<Integer>::max())) {
        throw std::overflow_error("Babai ceil coefficient does not fit int64");
    }
    return static_cast<Integer>(ceiled);
}

void subtractCoefficient(RealVector& residual, const Vector& basisVector, Integer coefficient) {
    for (std::size_t col = 0; col < residual.size(); ++col) {
        residual[col] -= static_cast<Real>(coefficient) * basisVector[col];
    }
}

BabaiResult resultFromCoefficients(const Matrix& basis, const RealVector& target,
                                   const std::vector<Integer>& coefficients) {
    BabaiResult result;
    result.coefficients = coefficients;
    result.latticeVector = integerCombination(basis, result.coefficients);
    result.distance = distance(target, result.latticeVector);
    return result;
}

std::vector<Integer> nearestPlaneCoefficientsFromResidual(const Matrix& basis, const GsoData& gso,
                                                          RealVector residual,
                                                          std::size_t lastFreeIndex) {
    std::vector<Integer> coefficients(basis.size(), 0);
    for (std::size_t reverseIndex = lastFreeIndex + 1; reverseIndex > 0; --reverseIndex) {
        const std::size_t i = reverseIndex - 1;
        const Real alpha = dot(residual, gso.orthogonal[i]) / gso.squaredNorms[i];
        const Integer coefficient = roundRealToInteger(alpha);
        coefficients[i] = coefficient;
        subtractCoefficient(residual, basis[i], coefficient);
    }
    return coefficients;
}

void enumerateRecursive(const Matrix& basis, const RealVector& target, int radius,
                        std::size_t index, std::vector<Integer>& coefficients,
                        BabaiResult& best) {
    if (index == coefficients.size()) {
        const Vector candidate = integerCombination(basis, coefficients);
        const Real candidateDistance = distance(target, candidate);
        ++best.candidateCount;
        if (best.candidateCount == 1 || candidateDistance < best.distance) {
            best.latticeVector = candidate;
            best.coefficients = coefficients;
            best.distance = candidateDistance;
        }
        return;
    }

    for (int value = -radius; value <= radius; ++value) {
        coefficients[index] = static_cast<Integer>(value);
        enumerateRecursive(basis, target, radius, index + 1, coefficients, best);
    }
}

void enumerateTailRecursive(const Matrix& basis, const RealVector& target, const GsoData& gso,
                            const std::vector<Integer>& centers, std::size_t tailStart,
                            int radius, long long maxCandidates, std::size_t index,
                            std::vector<Integer>& coefficients, BabaiResult& best) {
    if (best.candidateCount >= maxCandidates) {
        return;
    }

    if (index == basis.size()) {
        RealVector residual = target;
        for (std::size_t i = tailStart; i < basis.size(); ++i) {
            subtractCoefficient(residual, basis[i], coefficients[i]);
        }

        for (std::size_t reverseIndex = tailStart; reverseIndex > 0; --reverseIndex) {
            const std::size_t i = reverseIndex - 1;
            const Real alpha = dot(residual, gso.orthogonal[i]) / gso.squaredNorms[i];
            coefficients[i] = roundRealToInteger(alpha);
            subtractCoefficient(residual, basis[i], coefficients[i]);
        }

        const BabaiResult candidate = resultFromCoefficients(basis, target, coefficients);
        ++best.candidateCount;
        if (best.candidateCount == 1 || candidate.distance < best.distance) {
            best.latticeVector = candidate.latticeVector;
            best.coefficients = candidate.coefficients;
            best.distance = candidate.distance;
        }
        return;
    }

    for (int offset = -radius; offset <= radius; ++offset) {
        if (best.candidateCount >= maxCandidates) {
            return;
        }
        coefficients[index] = centers[index] + static_cast<Integer>(offset);
        enumerateTailRecursive(basis, target, gso, centers, tailStart, radius, maxCandidates,
                               index + 1, coefficients, best);
    }
}

void enumerateLocalSearchRecursive(const Matrix& basis, const RealVector& target,
                                   const std::vector<Integer>& baseCoefficients,
                                   int selectedCount, int radius, long long maxCandidates,
                                   int index, std::vector<Integer>& coefficients,
                                   BabaiResult& best) {
    if (best.candidateCount >= maxCandidates) {
        return;
    }

    if (index == selectedCount) {
        const BabaiResult candidate = resultFromCoefficients(basis, target, coefficients);
        ++best.candidateCount;
        if (best.candidateCount == 1 || candidate.distance < best.distance) {
            best.latticeVector = candidate.latticeVector;
            best.coefficients = candidate.coefficients;
            best.distance = candidate.distance;
        }
        return;
    }

    for (int offset = -radius; offset <= radius; ++offset) {
        if (best.candidateCount >= maxCandidates) {
            return;
        }
        coefficients[static_cast<std::size_t>(index)] =
            baseCoefficients[static_cast<std::size_t>(index)] + static_cast<Integer>(offset);
        enumerateLocalSearchRecursive(basis, target, baseCoefficients, selectedCount, radius,
                                      maxCandidates, index + 1, coefficients, best);
    }
}

}  // namespace

BabaiResult babaiNearestPlane(const Matrix& basis, const RealVector& target) {
    validateBasisShape(basis);
    if (target.size() != basis.front().size()) {
        throw std::invalid_argument("target dimension must match basis ambient dimension");
    }

    const auto startedAt = std::chrono::steady_clock::now();
    const GsoData gso = computeGso(basis);
    if (!hasFiniteGso(gso)) {
        throw std::invalid_argument("basis is rank deficient or produced invalid GSO data");
    }

    std::vector<Integer> coefficients =
        nearestPlaneCoefficientsFromResidual(basis, gso, target, basis.size() - 1);
    BabaiResult result = resultFromCoefficients(basis, target, coefficients);
    result.candidateCount = 1;

    const auto finishedAt = std::chrono::steady_clock::now();
    result.runtimeMs =
        std::chrono::duration<double, std::milli>(finishedAt - startedAt).count();
    return result;
}

BabaiResult randomizedBabaiNearestPlane(const Matrix& basis, const RealVector& target,
                                        const RandomizedBabaiConfig& config) {
    validateBasisShape(basis);
    if (target.size() != basis.front().size()) {
        throw std::invalid_argument("target dimension must match basis ambient dimension");
    }
    if (config.rounds <= 0) {
        throw std::invalid_argument("randomized Babai rounds must be positive");
    }

    const auto startedAt = std::chrono::steady_clock::now();
    const GsoData gso = computeGso(basis);
    if (!hasFiniteGso(gso)) {
        throw std::invalid_argument("basis is rank deficient or produced invalid GSO data");
    }

    std::mt19937_64 rng(config.seed);
    std::uniform_real_distribution<Real> unit(0.0L, 1.0L);
    BabaiResult best;

    for (int round = 0; round < config.rounds; ++round) {
        RealVector residual = target;
        std::vector<Integer> coefficients(basis.size(), 0);
        for (std::size_t reverseIndex = basis.size(); reverseIndex > 0; --reverseIndex) {
            const std::size_t i = reverseIndex - 1;
            const Real alpha = dot(residual, gso.orthogonal[i]) / gso.squaredNorms[i];
            const Integer lower = floorRealToInteger(alpha);
            const Integer upper = ceilRealToInteger(alpha);
            const Real upperProbability = alpha - static_cast<Real>(lower);
            const Integer coefficient =
                (lower == upper || unit(rng) >= upperProbability) ? lower : upper;
            coefficients[i] = coefficient;
            subtractCoefficient(residual, basis[i], coefficient);
        }

        const BabaiResult candidate = resultFromCoefficients(basis, target, coefficients);
        ++best.candidateCount;
        if (best.candidateCount == 1 || candidate.distance < best.distance) {
            best.latticeVector = candidate.latticeVector;
            best.coefficients = candidate.coefficients;
            best.distance = candidate.distance;
        }
    }

    const auto finishedAt = std::chrono::steady_clock::now();
    best.runtimeMs =
        std::chrono::duration<double, std::milli>(finishedAt - startedAt).count();
    return best;
}

BabaiResult tailEnumerationBabai(const Matrix& basis, const RealVector& target,
                                 const TailEnumerationConfig& config) {
    validateBasisShape(basis);
    if (target.size() != basis.front().size()) {
        throw std::invalid_argument("target dimension must match basis ambient dimension");
    }
    if (config.tailBlockSize <= 0 || config.windowRadius < 0 || config.maxCandidates <= 0) {
        throw std::invalid_argument("invalid tail enumeration Babai configuration");
    }

    const auto startedAt = std::chrono::steady_clock::now();
    const GsoData gso = computeGso(basis);
    if (!hasFiniteGso(gso)) {
        throw std::invalid_argument("basis is rank deficient or produced invalid GSO data");
    }

    const BabaiResult center = babaiNearestPlane(basis, target);
    const std::size_t tailSize =
        std::min<std::size_t>(basis.size(), static_cast<std::size_t>(config.tailBlockSize));
    const std::size_t tailStart = basis.size() - tailSize;
    std::vector<Integer> coefficients = center.coefficients;
    BabaiResult best;

    enumerateTailRecursive(basis, target, gso, center.coefficients, tailStart,
                           config.windowRadius, config.maxCandidates, tailStart, coefficients,
                           best);

    const auto finishedAt = std::chrono::steady_clock::now();
    best.runtimeMs =
        std::chrono::duration<double, std::milli>(finishedAt - startedAt).count();
    return best;
}

BabaiResult localSearchBabai(const Matrix& basis, const RealVector& target,
                             const LocalSearchConfig& config) {
    validateBasisShape(basis);
    if (target.size() != basis.front().size()) {
        throw std::invalid_argument("target dimension must match basis ambient dimension");
    }
    if (config.searchVectorCount <= 0 || config.windowRadius < 0 || config.maxCandidates <= 0) {
        throw std::invalid_argument("invalid local search Babai configuration");
    }

    const auto startedAt = std::chrono::steady_clock::now();
    const BabaiResult center = babaiNearestPlane(basis, target);
    const int selectedCount = std::min<int>(
        static_cast<int>(basis.size()), config.searchVectorCount);
    std::vector<Integer> coefficients = center.coefficients;
    BabaiResult best;

    enumerateLocalSearchRecursive(basis, target, center.coefficients, selectedCount,
                                  config.windowRadius, config.maxCandidates, 0, coefficients,
                                  best);

    const auto finishedAt = std::chrono::steady_clock::now();
    best.runtimeMs =
        std::chrono::duration<double, std::milli>(finishedAt - startedAt).count();
    return best;
}

BabaiResult exactClosestByEnumeration(const Matrix& basis, const RealVector& target,
                                      int coefficientRadius) {
    validateBasisShape(basis);
    if (target.size() != basis.front().size()) {
        throw std::invalid_argument("target dimension must match basis ambient dimension");
    }
    if (coefficientRadius < 0) {
        throw std::invalid_argument("coefficientRadius must be non-negative");
    }
    if (basis.size() > 10) {
        throw std::invalid_argument("exact enumeration is intended only for small dimensions");
    }

    const auto startedAt = std::chrono::steady_clock::now();
    BabaiResult best;
    std::vector<Integer> coefficients(basis.size(), 0);
    enumerateRecursive(basis, target, coefficientRadius, 0, coefficients, best);

    const auto finishedAt = std::chrono::steady_clock::now();
    best.runtimeMs =
        std::chrono::duration<double, std::milli>(finishedAt - startedAt).count();
    return best;
}

}  // namespace lll
