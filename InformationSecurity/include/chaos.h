#pragma once

#include "common.h"

#include <cstddef>
#include <cstdint>
#include <vector>

namespace isec {

struct LorenzParams {
    double a = 10.0;
    double b = 8.0 / 3.0;
    double c = 28.0;
    double r = -1.0;
    double step = 0.001;
    int discard = 2000;
    int sample_gap = 7;
};

struct LorenzState {
    double x = 1.0;
    double y = 1.0;
    double z = 1.0;
    double w = 1.0;
};

struct LorenzKey {
    LorenzParams params;
    LorenzState seed;
};

struct LorenzSequences {
    std::vector<double> x;
    std::vector<double> y;
    std::vector<double> z;
    std::vector<double> w;
};

void rk4Step(LorenzState& state, const LorenzParams& params);

LorenzSequences generateSequences(const LorenzKey& key, std::size_t count);

std::vector<std::size_t> sortPermutation(const std::vector<double>& sequence);

std::vector<std::size_t> inversePermutation(
    const std::vector<std::size_t>& permutation);

std::vector<std::uint16_t> diffusionMask(
    const std::vector<double>& sequence,
    int modulus = kCoeffModulus);

}  // namespace isec
