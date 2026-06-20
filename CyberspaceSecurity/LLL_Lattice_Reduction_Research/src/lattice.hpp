#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace lll {

using Integer = long long;
using Real = long double;
using Vector = std::vector<Integer>;
using Matrix = std::vector<Vector>;
using RealVector = std::vector<Real>;

int dimension(const Matrix& basis);
int ambientDimension(const Matrix& basis);
void validateBasisShape(const Matrix& basis);

Real dot(const Vector& lhs, const Vector& rhs);
Real dot(const RealVector& lhs, const RealVector& rhs);
Real dot(const RealVector& lhs, const Vector& rhs);
Real squaredNorm(const Vector& vector);
Real squaredNorm(const RealVector& vector);
Real norm(const Vector& vector);
Real distance(const RealVector& lhs, const Vector& rhs);

RealVector toRealVector(const Vector& vector);
Vector integerCombination(const Matrix& basis, const std::vector<Integer>& coefficients);
void subtractMultiple(Vector& target, const Vector& source, Integer multiple);

Matrix randomIntegerBasis(int dimension, int coefficientBits, std::uint64_t seed);
Real orthogonalityDefect(const Matrix& basis, Real determinantEstimate);
Real rootHermiteFactor(Real firstVectorNorm, Real determinantEstimate, int dimension);

std::string formatVector(const Vector& vector);

}  // namespace lll
