#pragma once

#include "chaos.h"
#include "pgm.h"

#include <array>
#include <cstddef>

namespace isec {

PGMImage encryptToCoefficientImage(const PGMImage& plain,
                                   const LorenzKey& key);

PGMImage decryptFromCoefficientImage(const PGMImage& cipher,
                                     const LorenzKey& key);

PGMImage makePreviewImage(const PGMImage& coefficientImage);

}  // namespace isec
