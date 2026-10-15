#include "metrics.h"

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <vector>

namespace isec {

double mse(const PGMImage& a, const PGMImage& b) {
    if (a.width != b.width || a.height != b.height ||
        a.pixels.size() != b.pixels.size()) {
        throw std::invalid_argument("MSE requires images of the same size");
    }

    double sum = 0.0;
    for (std::size_t i = 0; i < a.pixels.size(); ++i) {
        const double diff =
            static_cast<double>(a.pixels[i]) - static_cast<double>(b.pixels[i]);
        sum += diff * diff;
    }
    return sum / static_cast<double>(a.pixels.size());
}

double psnr(const PGMImage& a, const PGMImage& b) {
    const double error = mse(a, b);
    if (error == 0.0) {
        return std::numeric_limits<double>::infinity();
    }
    return 10.0 * std::log10((255.0 * 255.0) / error);
}

double entropy(const PGMImage& image) {
    const std::vector<std::size_t> counts = histogram(image, image.maxval);
    const double pixelCount = static_cast<double>(image.pixels.size());
    double result = 0.0;

    for (const std::size_t count : counts) {
        if (count == 0) {
            continue;
        }
        const double probability =
            static_cast<double>(count) / pixelCount;
        result -= probability * std::log2(probability);
    }

    return result;
}

double adjacentCorrelation(const PGMImage& image, bool horizontal) {
    if (image.width <= 0 || image.height <= 0) {
        throw std::invalid_argument("cannot correlate an empty image");
    }

    const int pairsPerRow = horizontal ? image.width - 1 : image.width;
    const int pairsPerCol = horizontal ? image.height : image.height - 1;
    if (pairsPerRow <= 0 || pairsPerCol <= 0) {
        return 0.0;
    }

    const std::size_t pairCount =
        static_cast<std::size_t>(pairsPerRow) * pairsPerCol;
    double sumX = 0.0;
    double sumY = 0.0;

    for (int y = 0; y < pairsPerCol; ++y) {
        for (int x = 0; x < pairsPerRow; ++x) {
            const int x2 = horizontal ? x + 1 : x;
            const int y2 = horizontal ? y : y + 1;
            sumX += static_cast<double>(image.at(x, y));
            sumY += static_cast<double>(image.at(x2, y2));
        }
    }

    const double meanX = sumX / static_cast<double>(pairCount);
    const double meanY = sumY / static_cast<double>(pairCount);
    double covariance = 0.0;
    double varianceX = 0.0;
    double varianceY = 0.0;

    for (int y = 0; y < pairsPerCol; ++y) {
        for (int x = 0; x < pairsPerRow; ++x) {
            const int x2 = horizontal ? x + 1 : x;
            const int y2 = horizontal ? y : y + 1;
            const double dx =
                static_cast<double>(image.at(x, y)) - meanX;
            const double dy =
                static_cast<double>(image.at(x2, y2)) - meanY;
            covariance += dx * dy;
            varianceX += dx * dx;
            varianceY += dy * dy;
        }
    }

    if (varianceX == 0.0 || varianceY == 0.0) {
        return 0.0;
    }
    return covariance / std::sqrt(varianceX * varianceY);
}

std::vector<std::size_t> histogram(const PGMImage& image, int maxval) {
    if (maxval < 0 || maxval > 65535) {
        throw std::invalid_argument("histogram maxval is out of range");
    }

    std::vector<std::size_t> counts(static_cast<std::size_t>(maxval) + 1, 0);
    for (const std::uint16_t pixel : image.pixels) {
        if (pixel <= maxval) {
            ++counts[pixel];
        }
    }
    return counts;
}

}  // namespace isec
