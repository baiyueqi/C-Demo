#include "dct.h"

#include <array>
#include <cmath>

namespace isec {
namespace {

constexpr double kPi = 3.141592653589793238462643383279502884;

void dct1(const std::array<double, kBlockSize>& input,
          std::array<double, kBlockSize>& output) {
    for (int k = 0; k < kBlockSize; ++k) {
        double sum = 0.0;
        for (int n = 0; n < kBlockSize; ++n) {
            sum += input[n] *
                   std::cos(kPi * static_cast<double>(k) *
                            (2.0 * n + 1.0) / (2.0 * kBlockSize));
        }
        const double c = k == 0 ? (1.0 / std::sqrt(2.0)) : 1.0;
        output[k] = 0.5 * c * sum;
    }
}

void idct1(const std::array<double, kBlockSize>& input,
           std::array<double, kBlockSize>& output) {
    for (int n = 0; n < kBlockSize; ++n) {
        double sum = 0.0;
        for (int k = 0; k < kBlockSize; ++k) {
            const double c = k == 0 ? (1.0 / std::sqrt(2.0)) : 1.0;
            sum += c * input[k] *
                   std::cos(kPi * static_cast<double>(k) *
                            (2.0 * n + 1.0) / (2.0 * kBlockSize));
        }
        output[n] = 0.5 * sum;
    }
}

}  // namespace

void dct2(DctBlock& block) {
    std::array<double, kBlockSize> input{};
    std::array<double, kBlockSize> output{};

    for (int row = 0; row < kBlockSize; ++row) {
        input = block[row];
        dct1(input, output);
        block[row] = output;
    }

    for (int col = 0; col < kBlockSize; ++col) {
        for (int row = 0; row < kBlockSize; ++row) {
            input[row] = block[row][col];
        }
        dct1(input, output);
        for (int row = 0; row < kBlockSize; ++row) {
            block[row][col] = output[row];
        }
    }
}

void idct2(DctBlock& block) {
    std::array<double, kBlockSize> input{};
    std::array<double, kBlockSize> output{};

    for (int row = 0; row < kBlockSize; ++row) {
        input = block[row];
        idct1(input, output);
        block[row] = output;
    }

    for (int col = 0; col < kBlockSize; ++col) {
        for (int row = 0; row < kBlockSize; ++row) {
            input[row] = block[row][col];
        }
        idct1(input, output);
        for (int row = 0; row < kBlockSize; ++row) {
            block[row][col] = output[row];
        }
    }
}

}  // namespace isec
