#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <vector>

namespace isec {

constexpr int kBlockSize = 8;
constexpr int kCoeffOffset = 4096;
constexpr int kCoeffModulus = 8192;

inline int padToBlock(int value) {
    if (value <= 0) {
        throw std::invalid_argument("image dimension must be positive");
    }
    return ((value + kBlockSize - 1) / kBlockSize) * kBlockSize;
}

inline std::uint8_t clampByte(double value) {
    const int rounded = static_cast<int>(std::lround(value));
    return static_cast<std::uint8_t>(std::max(0, std::min(255, rounded)));
}

inline int clampCoefficient(double value) {
    const long long rounded = std::llround(value);
    if (rounded > kCoeffOffset - 1) {
        return kCoeffOffset - 1;
    }
    if (rounded < -kCoeffOffset) {
        return -kCoeffOffset;
    }
    return static_cast<int>(rounded);
}

}  // namespace isec
