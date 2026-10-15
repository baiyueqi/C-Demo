#pragma once

#include "pgm.h"

#include <cstddef>
#include <vector>

namespace isec {

double mse(const PGMImage& a, const PGMImage& b);
double psnr(const PGMImage& a, const PGMImage& b);
double entropy(const PGMImage& image);
double adjacentCorrelation(const PGMImage& image, bool horizontal = true);
std::vector<std::size_t> histogram(const PGMImage& image, int maxval);

}  // namespace isec
