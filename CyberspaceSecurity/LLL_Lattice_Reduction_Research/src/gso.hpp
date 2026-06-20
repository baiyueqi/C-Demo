#pragma once

#include "lattice.hpp"

#include <cstddef>
#include <vector>

namespace lll {

struct GsoData {
    std::vector<RealVector> orthogonal;
    std::vector<std::vector<Real>> mu;
    std::vector<Real> squaredNorms;
    bool fullRank = true;
};

GsoData computeGso(const Matrix& basis);
std::size_t recomputeGsoSuffix(const Matrix& basis, GsoData& data, std::size_t startIndex);
Real determinantEstimate(const GsoData& data);
bool hasFiniteGso(const GsoData& data);

}  // namespace lll
