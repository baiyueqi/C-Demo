#pragma once

#include "common.h"

#include <array>

namespace isec {

using DctBlock = std::array<std::array<double, kBlockSize>, kBlockSize>;

void dct2(DctBlock& block);
void idct2(DctBlock& block);

}  // namespace isec
