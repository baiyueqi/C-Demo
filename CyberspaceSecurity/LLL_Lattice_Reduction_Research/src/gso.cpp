#include "gso.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <stdexcept>

namespace lll {
namespace {

constexpr Real kRankTolerance = 1e-24L;

void ensureAllocated(const Matrix& basis, GsoData& data) {
    const std::size_t n = basis.size();
    const std::size_t m = basis.front().size();
    data.orthogonal.assign(n, RealVector(m, 0.0L));
    data.mu.assign(n, std::vector<Real>(n, 0.0L));
    data.squaredNorms.assign(n, 0.0L);
    data.fullRank = true;
}

}  // namespace

GsoData computeGso(const Matrix& basis) {
    validateBasisShape(basis);

    GsoData data;
    ensureAllocated(basis, data);
    recomputeGsoSuffix(basis, data, 0);
    return data;
}

std::size_t recomputeGsoSuffix(const Matrix& basis, GsoData& data, std::size_t startIndex) {
    validateBasisShape(basis);
    const std::size_t n = basis.size();
    const std::size_t m = basis.front().size();
    if (startIndex > n) {
        throw std::invalid_argument("GSO suffix start is out of range");
    }
    if (data.orthogonal.size() != n || data.mu.size() != n || data.squaredNorms.size() != n) {
        ensureAllocated(basis, data);
        startIndex = 0;
    }

    data.fullRank = true;
    for (std::size_t i = startIndex; i < n; ++i) {
        const RealVector basisVector = toRealVector(basis[i]);
        data.orthogonal[i] = basisVector;
        std::fill(data.mu[i].begin(), data.mu[i].end(), 0.0L);

        for (std::size_t j = 0; j < i; ++j) {
            const Real denom = data.squaredNorms[j];
            if (!(denom > kRankTolerance) || !std::isfinite(denom)) {
                data.fullRank = false;
                continue;
            }

            data.mu[i][j] = dot(basisVector, data.orthogonal[j]) / denom;
            for (std::size_t col = 0; col < m; ++col) {
                data.orthogonal[i][col] -= data.mu[i][j] * data.orthogonal[j][col];
            }
        }

        data.squaredNorms[i] = squaredNorm(data.orthogonal[i]);
        if (!(data.squaredNorms[i] > kRankTolerance) || !std::isfinite(data.squaredNorms[i])) {
            data.fullRank = false;
        }
    }

    for (std::size_t i = 0; i < std::min(startIndex, n); ++i) {
        if (!(data.squaredNorms[i] > kRankTolerance) || !std::isfinite(data.squaredNorms[i])) {
            data.fullRank = false;
        }
    }

    return n - startIndex;
}

Real determinantEstimate(const GsoData& data) {
    if (data.squaredNorms.empty()) {
        return 0.0L;
    }

    Real product = 1.0L;
    for (const Real squaredNorm : data.squaredNorms) {
        if (!(squaredNorm > 0.0L) || !std::isfinite(squaredNorm)) {
            return 0.0L;
        }
        product *= std::sqrt(squaredNorm);
    }
    return product;
}

bool hasFiniteGso(const GsoData& data) {
    if (!data.fullRank) {
        return false;
    }

    for (const auto& row : data.mu) {
        for (const Real value : row) {
            if (!std::isfinite(value)) {
                return false;
            }
        }
    }
    for (const Real value : data.squaredNorms) {
        if (!std::isfinite(value)) {
            return false;
        }
    }
    return true;
}

}  // namespace lll
