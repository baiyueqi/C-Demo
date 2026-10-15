#include "cipher.h"

#include "dct.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace isec {
namespace {

using ZigzagOrder = std::array<std::pair<int, int>, kBlockSize * kBlockSize>;

const ZigzagOrder& zigzagOrder() {
    static const ZigzagOrder order = []() {
        ZigzagOrder generated{};
        std::size_t index = 0;
        for (int sum = 0; sum <= 2 * (kBlockSize - 1); ++sum) {
            if (sum % 2 == 0) {
                for (int u = sum; u >= 0; --u) {
                    const int v = sum - u;
                    if (u < kBlockSize && v < kBlockSize) {
                        generated[index++] = {u, v};
                    }
                }
            } else {
                for (int u = 0; u <= sum; ++u) {
                    const int v = sum - u;
                    if (u < kBlockSize && v < kBlockSize) {
                        generated[index++] = {u, v};
                    }
                }
            }
        }
        return generated;
    }();
    return order;
}

std::uint16_t paddedPixel(const PGMImage& image, int x, int y) {
    x = std::max(0, std::min(x, image.width - 1));
    y = std::max(0, std::min(y, image.height - 1));
    return image.at(x, y);
}

std::size_t blockCount(int width, int height) {
    const int horizontalBlocks = width / kBlockSize;
    const int verticalBlocks = height / kBlockSize;
    return static_cast<std::size_t>(horizontalBlocks) * verticalBlocks;
}

std::vector<int> transformToCoefficients(const PGMImage& plain,
                                         int paddedWidth,
                                         int paddedHeight) {
    const std::size_t totalBlocks = blockCount(paddedWidth, paddedHeight);
    std::vector<int> coefficients(totalBlocks * kBlockSize * kBlockSize);

    std::size_t blockIndex = 0;
    for (int by = 0; by < paddedHeight / kBlockSize; ++by) {
        for (int bx = 0; bx < paddedWidth / kBlockSize; ++bx) {
            DctBlock block{};
            for (int y = 0; y < kBlockSize; ++y) {
                for (int x = 0; x < kBlockSize; ++x) {
                    const int sourceX = bx * kBlockSize + x;
                    const int sourceY = by * kBlockSize + y;
                    block[y][x] =
                        static_cast<double>(paddedPixel(plain, sourceX, sourceY)) -
                        128.0;
                }
            }

            dct2(block);

            const ZigzagOrder& zigzag = zigzagOrder();
            const std::size_t base = blockIndex * kBlockSize * kBlockSize;
            for (std::size_t k = 0; k < zigzag.size(); ++k) {
                const int u = zigzag[k].first;
                const int v = zigzag[k].second;
                coefficients[base + k] = clampCoefficient(block[v][u]);
            }

            ++blockIndex;
        }
    }

    return coefficients;
}

PGMImage coefficientsToSpatial(const std::vector<int>& coefficients,
                               int paddedWidth,
                               int paddedHeight) {
    PGMImage image;
    image.width = paddedWidth;
    image.height = paddedHeight;
    image.maxval = 255;
    image.original_width = 0;
    image.original_height = 0;
    image.pixels.assign(
        static_cast<std::size_t>(paddedWidth) * paddedHeight, 0);

    const ZigzagOrder& zigzag = zigzagOrder();
    std::size_t blockIndex = 0;
    for (int by = 0; by < paddedHeight / kBlockSize; ++by) {
        for (int bx = 0; bx < paddedWidth / kBlockSize; ++bx) {
            DctBlock block{};
            const std::size_t base = blockIndex * kBlockSize * kBlockSize;
            for (std::size_t k = 0; k < zigzag.size(); ++k) {
                const int u = zigzag[k].first;
                const int v = zigzag[k].second;
                block[v][u] = static_cast<double>(coefficients[base + k]);
            }

            idct2(block);

            for (int y = 0; y < kBlockSize; ++y) {
                for (int x = 0; x < kBlockSize; ++x) {
                    const int targetX = bx * kBlockSize + x;
                    const int targetY = by * kBlockSize + y;
                    image.set(targetX, targetY,
                              clampByte(block[y][x] + 128.0));
                }
            }

            ++blockIndex;
        }
    }

    return image;
}

}  // namespace

PGMImage encryptToCoefficientImage(const PGMImage& plain,
                                   const LorenzKey& key) {
    if (plain.width <= 0 || plain.height <= 0 || plain.maxval > 255) {
        throw std::invalid_argument(
            "plain image must be a valid 8-bit grayscale PGM");
    }

    const int paddedWidth = padToBlock(plain.width);
    const int paddedHeight = padToBlock(plain.height);
    std::vector<int> coefficients =
        transformToCoefficients(plain, paddedWidth, paddedHeight);

    const LorenzSequences sequences =
        generateSequences(key, coefficients.size());
    const std::vector<std::size_t> permutation =
        sortPermutation(sequences.x);
    const std::vector<std::uint16_t> mask =
        diffusionMask(sequences.y, kCoeffModulus);

    PGMImage cipher;
    cipher.width = paddedWidth;
    cipher.height = paddedHeight;
    cipher.maxval = kCoeffModulus - 1;
    cipher.original_width = plain.width;
    cipher.original_height = plain.height;
    cipher.pixels.resize(coefficients.size());

    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        const int signedValue = coefficients[permutation[i]];
        const int shifted = signedValue + kCoeffOffset;
        const int encrypted = (shifted + mask[i]) % kCoeffModulus;
        cipher.pixels[i] = static_cast<std::uint16_t>(encrypted);
    }

    return cipher;
}

PGMImage decryptFromCoefficientImage(const PGMImage& cipher,
                                     const LorenzKey& key) {
    if (cipher.width <= 0 || cipher.height <= 0 ||
        cipher.pixels.size() !=
            static_cast<std::size_t>(cipher.width) * cipher.height) {
        throw std::invalid_argument("cipher image has invalid dimensions");
    }

    const int paddedWidth = cipher.width;
    const int paddedHeight = cipher.height;
    const std::size_t coefficientCount = cipher.pixels.size();

    const LorenzSequences sequences =
        generateSequences(key, coefficientCount);
    const std::vector<std::size_t> permutation =
        sortPermutation(sequences.x);
    const std::vector<std::uint16_t> mask =
        diffusionMask(sequences.y, kCoeffModulus);

    std::vector<int> coefficients(coefficientCount);
    for (std::size_t i = 0; i < coefficientCount; ++i) {
        const int encrypted = cipher.pixels[i];
        const int shifted =
            (encrypted + kCoeffModulus - mask[i]) % kCoeffModulus;
        coefficients[permutation[i]] = shifted - kCoeffOffset;
    }

    PGMImage padded = coefficientsToSpatial(coefficients, paddedWidth,
                                            paddedHeight);

    const int originalWidth = std::max(
        1, std::min(cipher.original_width > 0 ? cipher.original_width
                                               : paddedWidth,
                     paddedWidth));
    const int originalHeight = std::max(
        1, std::min(cipher.original_height > 0 ? cipher.original_height
                                                : paddedHeight,
                     paddedHeight));

    PGMImage result;
    result.width = originalWidth;
    result.height = originalHeight;
    result.maxval = 255;
    result.original_width = 0;
    result.original_height = 0;
    result.pixels.resize(
        static_cast<std::size_t>(originalWidth) * originalHeight);

    for (int y = 0; y < originalHeight; ++y) {
        for (int x = 0; x < originalWidth; ++x) {
            result.set(x, y, padded.at(x, y));
        }
    }

    return result;
}

PGMImage makePreviewImage(const PGMImage& coefficientImage) {
    if (coefficientImage.width <= 0 || coefficientImage.height <= 0 ||
        coefficientImage.maxval <= 0) {
        throw std::invalid_argument(
            "cannot create preview from an invalid coefficient image");
    }

    PGMImage preview;
    preview.width = coefficientImage.width;
    preview.height = coefficientImage.height;
    preview.maxval = 255;
    preview.original_width = coefficientImage.original_width;
    preview.original_height = coefficientImage.original_height;
    preview.pixels.resize(coefficientImage.pixels.size());

    for (std::size_t i = 0; i < coefficientImage.pixels.size(); ++i) {
        const double normalized =
            static_cast<double>(coefficientImage.pixels[i]) /
            static_cast<double>(coefficientImage.maxval) * 255.0;
        preview.pixels[i] =
            static_cast<std::uint16_t>(std::llround(normalized));
    }

    return preview;
}

}  // namespace isec
