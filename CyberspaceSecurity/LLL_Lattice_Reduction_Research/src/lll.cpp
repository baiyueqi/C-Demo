#include "lll.hpp"

#include "gso.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <limits>
#include <stdexcept>
#include <utility>

namespace lll {
namespace {

Integer roundToInteger(Real value) {
    const Real rounded = std::round(value);
    if (rounded < static_cast<Real>(std::numeric_limits<Integer>::min()) ||
        rounded > static_cast<Real>(std::numeric_limits<Integer>::max())) {
        throw std::overflow_error("rounded GSO coefficient does not fit int64");
    }
    return static_cast<Integer>(rounded);
}

bool lessOrNearlyEqual(Real lhs, Real rhs, Real tolerance) {
    const Real scale = std::max<Real>({1.0L, std::fabs(lhs), std::fabs(rhs)});
    return lhs <= rhs + tolerance * scale;
}

std::size_t refreshGso(const Matrix& basis, GsoData& gso, std::size_t startIndex,
                       GsoUpdateStrategy strategy) {
    if (strategy == GsoUpdateStrategy::Full) {
        gso = computeGso(basis);
        return basis.size();
    }
    return recomputeGsoSuffix(basis, gso, startIndex);
}

void fillQualityStats(const Matrix& basis, const GsoData& gso, LllStats& stats) {
    stats.firstVectorNorm = norm(basis.front());
    stats.determinantEstimate = determinantEstimate(gso);
    stats.rootHermiteFactor =
        rootHermiteFactor(stats.firstVectorNorm, stats.determinantEstimate, stats.dimension);
    stats.orthogonalityDefect = orthogonalityDefect(basis, stats.determinantEstimate);
}

}  // namespace

GsoUpdateStrategy parseGsoUpdateStrategy(const std::string& value) {
    if (value == "full") {
        return GsoUpdateStrategy::Full;
    }
    if (value == "suffix") {
        return GsoUpdateStrategy::Suffix;
    }
    throw std::invalid_argument("unknown GSO update strategy: " + value);
}

std::string toString(GsoUpdateStrategy strategy) {
    switch (strategy) {
        case GsoUpdateStrategy::Full:
            return "full";
        case GsoUpdateStrategy::Suffix:
            return "suffix";
    }
    return "unknown";
}

void validateLllConfig(const LllConfig& config) {
    if (!(config.delta > 0.25L && config.delta < 1.0L)) {
        throw std::invalid_argument("delta must be in (0.25, 1)");
    }
    if (!(config.eta >= 0.5L && config.eta < std::sqrt(config.delta))) {
        throw std::invalid_argument("eta must satisfy 0.5 <= eta < sqrt(delta)");
    }
    if (config.maxIterations <= 0) {
        throw std::invalid_argument("maxIterations must be positive");
    }
}

bool isLllReduced(const Matrix& basis, const LllConfig& config, Real tolerance) {
    validateBasisShape(basis);
    validateLllConfig(config);

    const GsoData gso = computeGso(basis);
    if (!hasFiniteGso(gso)) {
        return false;
    }

    const std::size_t n = basis.size();
    for (std::size_t i = 1; i < n; ++i) {
        for (std::size_t j = 0; j < i; ++j) {
            if (std::fabs(gso.mu[i][j]) > config.eta + tolerance) {
                return false;
            }
        }
    }

    for (std::size_t k = 1; k < n; ++k) {
        const Real lhs = config.delta * gso.squaredNorms[k - 1];
        const Real mu = gso.mu[k][k - 1];
        const Real rhs = gso.squaredNorms[k] + mu * mu * gso.squaredNorms[k - 1];
        if (!lessOrNearlyEqual(lhs, rhs, tolerance)) {
            return false;
        }
    }

    return true;
}

LllResult reduceLll(const Matrix& inputBasis, const LllConfig& config) {
    validateBasisShape(inputBasis);
    validateLllConfig(config);

    Matrix basis = inputBasis;
    LllStats stats;
    stats.dimension = dimension(basis);
    stats.ambientDimension = ambientDimension(basis);
    stats.delta = config.delta;
    stats.eta = config.eta;

    const auto startedAt = std::chrono::steady_clock::now();

    GsoData gso = computeGso(basis);
    stats.gsoUpdateCount += static_cast<long long>(basis.size());
    if (!hasFiniteGso(gso)) {
        throw std::invalid_argument("basis is rank deficient or produced invalid GSO data");
    }

    std::size_t k = 1;
    while (k < basis.size()) {
        ++stats.iterationCount;
        if (stats.iterationCount > config.maxIterations) {
            stats.terminatedByIterationLimit = true;
            break;
        }

        for (std::size_t offset = 0; offset < k; ++offset) {
            const std::size_t j = k - 1 - offset;
            if (std::fabs(gso.mu[k][j]) <= config.eta) {
                continue;
            }

            const Integer q = roundToInteger(gso.mu[k][j]);
            if (q == 0) {
                continue;
            }

            subtractMultiple(basis[k], basis[j], q);
            ++stats.sizeReductionCount;
            stats.gsoUpdateCount +=
                static_cast<long long>(refreshGso(basis, gso, k, config.updateStrategy));
            if (!hasFiniteGso(gso)) {
                throw std::invalid_argument("GSO became invalid during size reduction");
            }
        }

        const Real lhs = config.delta * gso.squaredNorms[k - 1];
        const Real mu = gso.mu[k][k - 1];
        const Real rhs = gso.squaredNorms[k] + mu * mu * gso.squaredNorms[k - 1];

        if (lessOrNearlyEqual(lhs, rhs, 1e-14L)) {
            ++k;
            continue;
        }

        std::swap(basis[k], basis[k - 1]);
        ++stats.swapCount;
        stats.gsoUpdateCount += static_cast<long long>(
            refreshGso(basis, gso, k - 1, config.updateStrategy));
        if (!hasFiniteGso(gso)) {
            throw std::invalid_argument("GSO became invalid after basis swap");
        }

        k = (k > 1) ? k - 1 : 1;
    }

    fillQualityStats(basis, gso, stats);
    stats.validReduced =
        !stats.terminatedByIterationLimit && (!config.verifyOutput || isLllReduced(basis, config));

    const auto finishedAt = std::chrono::steady_clock::now();
    stats.runtimeMs =
        std::chrono::duration<double, std::milli>(finishedAt - startedAt).count();

    return LllResult{std::move(basis), stats};
}

}  // namespace lll
