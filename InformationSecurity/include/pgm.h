#pragma once

#include "common.h"

#include <cstdint>
#include <string>
#include <vector>

namespace isec {

struct PGMImage {
    int width = 0;
    int height = 0;
    int maxval = 255;

    // Optional dimensions stored in PGM comments. These describe the original
    // image size before padding and are used by the decryption step.
    int original_width = 0;
    int original_height = 0;

    std::vector<std::uint16_t> pixels;

    std::uint16_t at(int x, int y) const {
        return pixels[static_cast<std::size_t>(y) * width + x];
    }

    void set(int x, int y, std::uint16_t value) {
        pixels[static_cast<std::size_t>(y) * width + x] = value;
    }
};

void readPGM(const std::string& path, PGMImage& image);
void writePGM(const std::string& path, const PGMImage& image);

}  // namespace isec
