#include "lattice.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <random>
#include <sstream>
#include <stdexcept>

namespace lll {
namespace {

Integer checkedInteger(__int128 value) {
    if (value < static_cast<__int128>(std::numeric_limits<Integer>::min()) ||
        value > static_cast<__int128>(std::numeric_limits<Integer>::max())) {
        throw std::overflow_error("int64 lattice coefficient overflow");
    }
    return static_cast<Integer>(value);
}

Integer checkedAdd(Integer lhs, Integer rhs) {
    return checkedInteger(static_cast<__int128>(lhs) + static_cast<__int128>(rhs));
}

Integer checkedMultiply(Integer lhs, Integer rhs) {
    return checkedInteger(static_cast<__int128>(lhs) * static_cast<__int128>(rhs));
}

}  // namespace

int dimension(const Matrix& basis) {
    return static_cast<int>(basis.size());
}

int ambientDimension(const Matrix& basis) {
    return basis.empty() ? 0 : static_cast<int>(basis.front().size());
}

void validateBasisShape(const Matrix& basis) {
    if (basis.empty()) {
        throw std::invalid_argument("basis must contain at least one vector");
    }

    const std::size_t width = basis.front().size();
    if (width == 0) {
        throw std::invalid_argument("basis vectors must be non-empty");
    }

    for (const auto& row : basis) {
        if (row.size() != width) {
            throw std::invalid_argument("basis vectors must have the same ambient dimension");
        }
    }

    if (basis.size() > width) {
        throw std::invalid_argument("basis rank cannot exceed ambient dimension in this prototype");
    }
}

Real dot(const Vector& lhs, const Vector& rhs) {
    if (lhs.size() != rhs.size()) {
        throw std::invalid_argument("dot product dimension mismatch");
    }

    Real result = 0.0L;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        result += static_cast<Real>(lhs[i]) * static_cast<Real>(rhs[i]);
    }
    return result;
}

Real dot(const RealVector& lhs, const RealVector& rhs) {
    if (lhs.size() != rhs.size()) {
        throw std::invalid_argument("dot product dimension mismatch");
    }

    Real result = 0.0L;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        result += lhs[i] * rhs[i];
    }
    return result;
}

Real dot(const RealVector& lhs, const Vector& rhs) {
    if (lhs.size() != rhs.size()) {
        throw std::invalid_argument("dot product dimension mismatch");
    }

    Real result = 0.0L;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        result += lhs[i] * static_cast<Real>(rhs[i]);
    }
    return result;
}

Real squaredNorm(const Vector& vector) {
    return dot(vector, vector);
}

Real squaredNorm(const RealVector& vector) {
    return dot(vector, vector);
}

Real norm(const Vector& vector) {
    return std::sqrt(squaredNorm(vector));
}

Real distance(const RealVector& lhs, const Vector& rhs) {
    if (lhs.size() != rhs.size()) {
        throw std::invalid_argument("distance dimension mismatch");
    }

    Real sum = 0.0L;
    for (std::size_t i = 0; i < lhs.size(); ++i) {
        const Real diff = lhs[i] - static_cast<Real>(rhs[i]);
        sum += diff * diff;
    }
    return std::sqrt(sum);
}

RealVector toRealVector(const Vector& vector) {
    RealVector result(vector.size());
    for (std::size_t i = 0; i < vector.size(); ++i) {
        result[i] = static_cast<Real>(vector[i]);
    }
    return result;
}

Vector integerCombination(const Matrix& basis, const std::vector<Integer>& coefficients) {
    validateBasisShape(basis);
    if (basis.size() != coefficients.size()) {
        throw std::invalid_argument("coefficient count must match basis dimension");
    }

    Vector result(basis.front().size(), 0);
    for (std::size_t i = 0; i < basis.size(); ++i) {
        for (std::size_t j = 0; j < result.size(); ++j) {
            result[j] = checkedAdd(result[j], checkedMultiply(coefficients[i], basis[i][j]));
        }
    }
    return result;
}

void subtractMultiple(Vector& target, const Vector& source, Integer multiple) {
    if (target.size() != source.size()) {
        throw std::invalid_argument("basis vector dimension mismatch");
    }

    for (std::size_t i = 0; i < target.size(); ++i) {
        const __int128 value = static_cast<__int128>(target[i]) -
                               static_cast<__int128>(multiple) * source[i];
        target[i] = checkedInteger(value);
    }
}

Matrix randomIntegerBasis(int dim, int coefficientBits, std::uint64_t seed) {
    if (dim <= 0) {
        throw std::invalid_argument("dimension must be positive");
    }
    if (coefficientBits <= 0 || coefficientBits > 32) {
        throw std::invalid_argument("coefficientBits must be in [1, 32] for int64 experiments");
    }

    const Integer limit = coefficientBits == 1 ? 1LL : ((1LL << (coefficientBits - 1)) - 1LL);
    std::mt19937_64 rng(seed);
    std::uniform_int_distribution<Integer> entryDist(-limit, limit);
    std::uniform_int_distribution<Integer> diagDist(1, std::max<Integer>(1, limit));

    Matrix basis(static_cast<std::size_t>(dim), Vector(static_cast<std::size_t>(dim), 0));
    for (int i = 0; i < dim; ++i) {
        for (int j = 0; j <= i; ++j) {
            basis[static_cast<std::size_t>(i)][static_cast<std::size_t>(j)] = entryDist(rng);
        }

        const Integer diagonal = std::max<Integer>(1, limit + dim + diagDist(rng));
        basis[static_cast<std::size_t>(i)][static_cast<std::size_t>(i)] =
            (i % 2 == 0) ? diagonal : -diagonal;
    }

    // Keep the matrix full rank while making it less triangular.
    std::uniform_int_distribution<int> rowDist(0, dim - 1);
    std::uniform_int_distribution<int> mixDist(-2, 2);
    for (int step = 0; step < dim; ++step) {
        const int to = rowDist(rng);
        int from = rowDist(rng);
        if (from == to) {
            from = (from + 1) % dim;
        }

        const int mix = mixDist(rng);
        if (mix == 0) {
            continue;
        }

        for (int col = 0; col < dim; ++col) {
            const __int128 value =
                static_cast<__int128>(basis[static_cast<std::size_t>(to)][static_cast<std::size_t>(col)]) +
                static_cast<__int128>(mix) *
                    basis[static_cast<std::size_t>(from)][static_cast<std::size_t>(col)];
            basis[static_cast<std::size_t>(to)][static_cast<std::size_t>(col)] = checkedInteger(value);
        }
    }

    return basis;
}

Real orthogonalityDefect(const Matrix& basis, Real determinantEstimate) {
    validateBasisShape(basis);
    if (!(determinantEstimate > 0.0L)) {
        return std::numeric_limits<Real>::infinity();
    }

    Real product = 1.0L;
    for (const auto& vector : basis) {
        product *= norm(vector);
    }
    return product / determinantEstimate;
}

Real rootHermiteFactor(Real firstVectorNorm, Real determinantEstimate, int dim) {
    if (dim <= 0 || !(firstVectorNorm > 0.0L) || !(determinantEstimate > 0.0L)) {
        return 0.0L;
    }

    const Real detRoot = std::pow(determinantEstimate, 1.0L / static_cast<Real>(dim));
    if (!(detRoot > 0.0L)) {
        return 0.0L;
    }
    return std::pow(firstVectorNorm / detRoot, 1.0L / static_cast<Real>(dim));
}

std::string formatVector(const Vector& vector) {
    std::ostringstream out;
    out << '[';
    for (std::size_t i = 0; i < vector.size(); ++i) {
        if (i != 0) {
            out << ", ";
        }
        out << vector[i];
    }
    out << ']';
    return out.str();
}

}  // namespace lll
