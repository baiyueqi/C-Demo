#include "chaos.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <numeric>
#include <stdexcept>

namespace isec {
namespace {

LorenzState derivative(const LorenzState& s, const LorenzParams& p) {
    LorenzState out{};
    out.x = p.a * (s.y - s.x) + s.w;
    out.y = p.c * s.x - s.y - s.x * s.z;
    out.z = s.x * s.y - p.b * s.z;
    out.w = -s.y * s.z + p.r * s.w;
    return out;
}

LorenzState add(const LorenzState& a, const LorenzState& b, double scaleB) {
    LorenzState out{};
    out.x = a.x + scaleB * b.x;
    out.y = a.y + scaleB * b.y;
    out.z = a.z + scaleB * b.z;
    out.w = a.w + scaleB * b.w;
    return out;
}

}  // namespace

void rk4Step(LorenzState& state, const LorenzParams& params) {
    if (params.step <= 0.0) {
        throw std::invalid_argument("Lorenz integration step must be positive");
    }

    const LorenzState s = state;
    const LorenzState k1 = derivative(s, params);
    const LorenzState k2 =
        derivative(add(s, k1, params.step * 0.5), params);
    const LorenzState k3 =
        derivative(add(s, k2, params.step * 0.5), params);
    const LorenzState k4 = derivative(add(s, k3, params.step), params);

    state.x = s.x + params.step / 6.0 *
                         (k1.x + 2.0 * k2.x + 2.0 * k3.x + k4.x);
    state.y = s.y + params.step / 6.0 *
                         (k1.y + 2.0 * k2.y + 2.0 * k3.y + k4.y);
    state.z = s.z + params.step / 6.0 *
                         (k1.z + 2.0 * k2.z + 2.0 * k3.z + k4.z);
    state.w = s.w + params.step / 6.0 *
                         (k1.w + 2.0 * k2.w + 2.0 * k3.w + k4.w);
}

LorenzSequences generateSequences(const LorenzKey& key, std::size_t count) {
    if (key.params.discard < 0 || key.params.sample_gap <= 0) {
        throw std::invalid_argument(
            "Lorenz discard must be non-negative and sample_gap must be positive");
    }

    LorenzSequences sequences;
    if (count == 0) {
        return sequences;
    }

    sequences.x.reserve(count);
    sequences.y.reserve(count);
    sequences.z.reserve(count);
    sequences.w.reserve(count);

    LorenzState state = key.seed;
    for (int i = 0; i < key.params.discard; ++i) {
        rk4Step(state, key.params);
    }

    for (std::size_t k = 0; k < count; ++k) {
        for (int gap = 0; gap < key.params.sample_gap; ++gap) {
            rk4Step(state, key.params);
        }
        sequences.x.push_back(state.x);
        sequences.y.push_back(state.y);
        sequences.z.push_back(state.z);
        sequences.w.push_back(state.w);
    }

    return sequences;
}

std::vector<std::size_t> sortPermutation(
    const std::vector<double>& sequence) {
    std::vector<std::size_t> indices(sequence.size());
    std::iota(indices.begin(), indices.end(), std::size_t{0});
    std::stable_sort(indices.begin(), indices.end(),
                     [&sequence](std::size_t left, std::size_t right) {
                         return sequence[left] < sequence[right];
                     });
    return indices;
}

std::vector<std::size_t> inversePermutation(
    const std::vector<std::size_t>& permutation) {
    std::vector<std::size_t> inverse(permutation.size());
    for (std::size_t i = 0; i < permutation.size(); ++i) {
        inverse[permutation[i]] = i;
    }
    return inverse;
}

std::vector<std::uint16_t> diffusionMask(const std::vector<double>& sequence,
                                         int modulus) {
    if (modulus <= 0) {
        throw std::invalid_argument("diffusion modulus must be positive");
    }

    std::vector<std::uint16_t> mask(sequence.size());
    for (std::size_t i = 0; i < sequence.size(); ++i) {
        const long long scaled = std::llround(std::abs(sequence[i]) * 1000000.0);
        const long long reduced = scaled % modulus;
        mask[i] = static_cast<std::uint16_t>(reduced);
    }
    return mask;
}

}  // namespace isec
