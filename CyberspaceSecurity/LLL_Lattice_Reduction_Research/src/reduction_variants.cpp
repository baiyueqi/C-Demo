#include "reduction_variants.hpp"

#include "gso.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lll {
namespace {

struct LocalEnumerationState {
    Vector bestVector;
    std::vector<Integer> bestCoefficients;
    Real bestSquaredNorm = 0.0L;
    long long candidateCount = 0;
    bool improved = false;
};

void fillAdvancedStats(const std::string& algorithm, const LllResult& finalLll,
                       long long candidateCount, long long insertionCount,
                       int completedPasses,
                       const std::chrono::steady_clock::time_point& startedAt,
                       AdvancedReductionResult& result) {
    const auto finishedAt = std::chrono::steady_clock::now();
    result.stats.algorithm = algorithm;
    result.stats.lllStats = finalLll.stats;
    result.stats.candidateCount = candidateCount;
    result.stats.insertionCount = insertionCount;
    result.stats.completedPasses = completedPasses;
    result.stats.runtimeMs =
        std::chrono::duration<double, std::milli>(finishedAt - startedAt).count();
    result.stats.validReduced = finalLll.stats.validReduced;
}

Matrix insertRow(const Matrix& basis, std::size_t from, std::size_t to) {
    Matrix candidate = basis;
    const Vector moved = candidate[from];
    candidate.erase(candidate.begin() + static_cast<std::ptrdiff_t>(from));
    candidate.insert(candidate.begin() + static_cast<std::ptrdiff_t>(to), moved);
    return candidate;
}

Real logPotential(const Matrix& basis) {
    const GsoData gso = computeGso(basis);
    if (!hasFiniteGso(gso)) {
        return std::numeric_limits<Real>::infinity();
    }

    Real result = 0.0L;
    const std::size_t n = basis.size();
    for (std::size_t i = 0; i < n; ++i) {
        if (!(gso.squaredNorms[i] > 0.0L)) {
            return std::numeric_limits<Real>::infinity();
        }
        result += static_cast<Real>(n - i) * std::log(gso.squaredNorms[i]);
    }
    return result;
}

void enumerateMiniBkzBlock(const Matrix& block, int radius, long long maxCandidates,
                           std::size_t index, std::vector<Integer>& coefficients,
                           LocalEnumerationState& state) {
    if (state.candidateCount >= maxCandidates) {
        return;
    }

    if (index == coefficients.size()) {
        const Vector candidate = integerCombination(block, coefficients);
        const Real candidateNorm = squaredNorm(candidate);
        ++state.candidateCount;
        if (candidateNorm < state.bestSquaredNorm) {
            state.bestVector = candidate;
            state.bestCoefficients = coefficients;
            state.bestSquaredNorm = candidateNorm;
            state.improved = true;
        }
        return;
    }

    if (index == 0) {
        coefficients[0] = 1;
        enumerateMiniBkzBlock(block, radius, maxCandidates, index + 1, coefficients, state);
        coefficients[0] = -1;
        enumerateMiniBkzBlock(block, radius, maxCandidates, index + 1, coefficients, state);
        return;
    }

    for (int value = -radius; value <= radius; ++value) {
        if (state.candidateCount >= maxCandidates) {
            return;
        }
        coefficients[index] = static_cast<Integer>(value);
        enumerateMiniBkzBlock(block, radius, maxCandidates, index + 1, coefficients, state);
    }
}

LocalEnumerationState findMiniBkzReplacement(const Matrix& basis, std::size_t start,
                                             std::size_t end, int radius,
                                             long long maxCandidates,
                                             Real improvementRatio) {
    Matrix block;
    for (std::size_t i = start; i < end; ++i) {
        block.push_back(basis[i]);
    }

    LocalEnumerationState state;
    state.bestVector = basis[start];
    state.bestSquaredNorm = squaredNorm(basis[start]) * improvementRatio * improvementRatio;
    std::vector<Integer> coefficients(block.size(), 0);
    enumerateMiniBkzBlock(block, radius, maxCandidates, 0, coefficients, state);
    return state;
}

}  // namespace

void validateAdvancedReductionConfig(const AdvancedReductionConfig& config) {
    validateLllConfig(config.lllConfig);
    if (config.tours <= 0 || config.blockSize < 2 || config.enumerationRadius < 0 ||
        config.maxInsertDepth <= 0 || config.maxPasses <= 0 || config.maxCandidates <= 0) {
        throw std::invalid_argument("invalid advanced reduction configuration");
    }
    if (!(config.improvementRatio > 0.0L && config.improvementRatio <= 1.0L)) {
        throw std::invalid_argument("improvementRatio must be in (0, 1]");
    }
}

AdvancedReductionResult reduceMiniBkz(const Matrix& inputBasis,
                                      const AdvancedReductionConfig& config) {
    validateBasisShape(inputBasis);
    validateAdvancedReductionConfig(config);

    const auto startedAt = std::chrono::steady_clock::now();
    Matrix basis = reduceLll(inputBasis, config.lllConfig).reducedBasis;
    long long totalCandidates = 0;
    long long replacementCount = 0;
    int completedTours = 0;

    for (int tour = 0; tour < config.tours; ++tour) {
        bool improvedInTour = false;
        ++completedTours;

        for (std::size_t start = 0; start + 1 < basis.size(); ++start) {
            if (totalCandidates >= config.maxCandidates) {
                break;
            }

            const std::size_t end = std::min<std::size_t>(
                basis.size(), start + static_cast<std::size_t>(config.blockSize));
            const long long remaining = config.maxCandidates - totalCandidates;
            LocalEnumerationState state = findMiniBkzReplacement(
                basis, start, end, config.enumerationRadius, remaining, config.improvementRatio);
            totalCandidates += state.candidateCount;

            if (!state.improved) {
                continue;
            }

            basis[start] = state.bestVector;
            basis = reduceLll(basis, config.lllConfig).reducedBasis;
            ++replacementCount;
            improvedInTour = true;
        }

        if (!improvedInTour || totalCandidates >= config.maxCandidates) {
            break;
        }
    }

    LllResult finalLll = reduceLll(basis, config.lllConfig);
    AdvancedReductionResult result;
    result.reducedBasis = std::move(finalLll.reducedBasis);
    fillAdvancedStats("mini-bkz", finalLll, totalCandidates, replacementCount, completedTours,
                      startedAt, result);
    return result;
}

AdvancedReductionResult reduceDeepLllPostPass(const Matrix& inputBasis,
                                             const AdvancedReductionConfig& config) {
    validateBasisShape(inputBasis);
    validateAdvancedReductionConfig(config);

    const auto startedAt = std::chrono::steady_clock::now();
    LllResult current = reduceLll(inputBasis, config.lllConfig);
    Matrix basis = current.reducedBasis;
    Real currentMetric = current.stats.rootHermiteFactor;
    long long candidateCount = 0;
    long long insertionCount = 0;
    int completedPasses = 0;

    for (int pass = 0; pass < config.maxPasses; ++pass) {
        bool acceptedInPass = false;
        ++completedPasses;

        for (std::size_t k = 1; k < basis.size() && candidateCount < config.maxCandidates; ++k) {
            const std::size_t firstPosition =
                k > static_cast<std::size_t>(config.maxInsertDepth)
                    ? k - static_cast<std::size_t>(config.maxInsertDepth)
                    : 0;

            Matrix bestBasis;
            Real bestMetric = currentMetric;
            bool found = false;

            for (std::size_t i = firstPosition; i < k && candidateCount < config.maxCandidates; ++i) {
                ++candidateCount;
                LllResult candidate =
                    reduceLll(insertRow(basis, k, i), config.lllConfig);
                if (candidate.stats.rootHermiteFactor <
                    bestMetric * config.improvementRatio) {
                    bestMetric = candidate.stats.rootHermiteFactor;
                    bestBasis = std::move(candidate.reducedBasis);
                    found = true;
                }
            }

            if (found) {
                basis = std::move(bestBasis);
                currentMetric = bestMetric;
                ++insertionCount;
                acceptedInPass = true;
            }
        }

        if (!acceptedInPass || candidateCount >= config.maxCandidates) {
            break;
        }
    }

    LllResult finalLll = reduceLll(basis, config.lllConfig);
    AdvancedReductionResult result;
    result.reducedBasis = std::move(finalLll.reducedBasis);
    fillAdvancedStats("deep-lll-postpass", finalLll, candidateCount, insertionCount,
                      completedPasses, startedAt, result);
    return result;
}

AdvancedReductionResult reducePotLllPostPass(const Matrix& inputBasis,
                                            const AdvancedReductionConfig& config) {
    validateBasisShape(inputBasis);
    validateAdvancedReductionConfig(config);

    const auto startedAt = std::chrono::steady_clock::now();
    Matrix basis = reduceLll(inputBasis, config.lllConfig).reducedBasis;
    Real currentPotential = logPotential(basis);
    const Real acceptanceDrop = std::log(config.lllConfig.delta);
    long long candidateCount = 0;
    long long insertionCount = 0;
    int completedPasses = 0;

    for (int pass = 0; pass < config.maxPasses; ++pass) {
        bool acceptedInPass = false;
        ++completedPasses;

        for (std::size_t k = 1; k < basis.size() && candidateCount < config.maxCandidates; ++k) {
            const std::size_t firstPosition =
                k > static_cast<std::size_t>(config.maxInsertDepth)
                    ? k - static_cast<std::size_t>(config.maxInsertDepth)
                    : 0;

            Matrix bestBasis;
            Real bestPotential = currentPotential;
            bool found = false;

            for (std::size_t i = firstPosition; i < k && candidateCount < config.maxCandidates; ++i) {
                ++candidateCount;
                Matrix candidateBasis =
                    reduceLll(insertRow(basis, k, i), config.lllConfig).reducedBasis;
                const Real candidatePotential = logPotential(candidateBasis);
                if (candidatePotential < bestPotential + acceptanceDrop) {
                    bestPotential = candidatePotential;
                    bestBasis = std::move(candidateBasis);
                    found = true;
                }
            }

            if (found) {
                basis = std::move(bestBasis);
                currentPotential = bestPotential;
                ++insertionCount;
                acceptedInPass = true;
            }
        }

        if (!acceptedInPass || candidateCount >= config.maxCandidates) {
            break;
        }
    }

    LllResult finalLll = reduceLll(basis, config.lllConfig);
    AdvancedReductionResult result;
    result.reducedBasis = std::move(finalLll.reducedBasis);
    fillAdvancedStats("pot-lll-postpass", finalLll, candidateCount, insertionCount,
                      completedPasses, startedAt, result);
    return result;
}

}  // namespace lll
