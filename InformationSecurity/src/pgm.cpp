#include "pgm.h"

#include <cctype>
#include <cstdint>
#include <fstream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace isec {
namespace {

std::string readToken(std::ifstream& in,
                      std::vector<std::string>& comments) {
    std::string token;
    bool started = false;

    while (in) {
        const int ch = in.get();
        if (ch == std::char_traits<char>::eof()) {
            break;
        }

        if (ch == '#') {
            std::string comment;
            std::getline(in, comment);
            comments.push_back(comment);
            continue;
        }

        if (!started) {
            if (std::isspace(static_cast<unsigned char>(ch))) {
                continue;
            }
            started = true;
            token.push_back(static_cast<char>(ch));
        } else {
            if (std::isspace(static_cast<unsigned char>(ch))) {
                if (ch == '\r' && in.peek() == '\n') {
                    in.get();
                }
                break;
            }
            token.push_back(static_cast<char>(ch));
        }
    }

    if (token.empty()) {
        throw std::runtime_error("unexpected end of PGM header");
    }
    return token;
}

int parseHeaderInt(const std::string& token, const std::string& field) {
    std::size_t used = 0;
    const long long value = std::stoll(token, &used);
    if (used != token.size() || value <= 0 ||
        value > std::numeric_limits<int>::max()) {
        throw std::runtime_error("invalid " + field + " in PGM header");
    }
    return static_cast<int>(value);
}

void parseOriginalSize(const std::vector<std::string>& comments,
                       PGMImage& image) {
    for (const std::string& comment : comments) {
        std::istringstream stream(comment);
        std::string key;
        while (stream >> key) {
            if (key == "orig_width") {
                int value = 0;
                if (stream >> value) {
                    image.original_width = value;
                }
            } else if (key == "orig_height") {
                int value = 0;
                if (stream >> value) {
                    image.original_height = value;
                }
            } else {
                // Unknown comments are intentionally ignored.
                break;
            }
        }
    }
}

}  // namespace

void readPGM(const std::string& path, PGMImage& image) {
    std::ifstream in(path, std::ios::binary);
    if (!in) {
        throw std::runtime_error("cannot open PGM file: " + path);
    }

    std::vector<std::string> comments;
    const std::string magic = readToken(in, comments);
    if (magic != "P5") {
        throw std::runtime_error("only binary PGM (P5) is supported");
    }

    const int width = parseHeaderInt(readToken(in, comments), "width");
    const int height = parseHeaderInt(readToken(in, comments), "height");
    const int maxval = parseHeaderInt(readToken(in, comments), "maxval");
    if (maxval > 65535) {
        throw std::runtime_error("PGM maxval is too large");
    }

    const std::size_t pixelCount =
        static_cast<std::size_t>(width) * static_cast<std::size_t>(height);
    if (pixelCount == 0 || pixelCount > std::numeric_limits<std::size_t>::max() / 2) {
        throw std::runtime_error("PGM image dimensions are invalid");
    }

    image.width = width;
    image.height = height;
    image.maxval = maxval;
    image.original_width = 0;
    image.original_height = 0;
    image.pixels.assign(pixelCount, 0);
    parseOriginalSize(comments, image);

    if (maxval < 256) {
        std::vector<unsigned char> bytes(pixelCount);
        in.read(reinterpret_cast<char*>(bytes.data()),
                static_cast<std::streamsize>(pixelCount));
        if (in.gcount() != static_cast<std::streamsize>(pixelCount)) {
            throw std::runtime_error("PGM raster data is truncated");
        }
        for (std::size_t i = 0; i < pixelCount; ++i) {
            image.pixels[i] = static_cast<std::uint16_t>(bytes[i]);
        }
    } else {
        for (std::size_t i = 0; i < pixelCount; ++i) {
            const int high = in.get();
            const int low = in.get();
            if (high == std::char_traits<char>::eof() ||
                low == std::char_traits<char>::eof()) {
                throw std::runtime_error("PGM raster data is truncated");
            }
            image.pixels[i] = static_cast<std::uint16_t>((high << 8) | low);
        }
    }
}

void writePGM(const std::string& path, const PGMImage& image) {
    if (image.width <= 0 || image.height <= 0 || image.maxval <= 0 ||
        image.maxval > 65535 ||
        image.pixels.size() !=
            static_cast<std::size_t>(image.width) * image.height) {
        throw std::runtime_error("cannot write invalid PGM image");
    }

    std::ofstream out(path, std::ios::binary);
    if (!out) {
        throw std::runtime_error("cannot create PGM file: " + path);
    }

    out << "P5\n";
    if (image.original_width > 0 && image.original_height > 0) {
        out << "# orig_width " << image.original_width
            << " orig_height " << image.original_height << "\n";
    }
    out << image.width << ' ' << image.height << '\n';
    out << image.maxval << '\n';

    if (image.maxval < 256) {
        for (const std::uint16_t pixel : image.pixels) {
            const auto value = static_cast<unsigned char>(
                std::min<std::uint16_t>(255, pixel));
            out.put(static_cast<char>(value));
        }
    } else {
        for (const std::uint16_t pixel : image.pixels) {
            const auto value = static_cast<std::uint16_t>(
                std::min<std::uint16_t>(image.maxval, pixel));
            out.put(static_cast<char>((value >> 8) & 0xff));
            out.put(static_cast<char>(value & 0xff));
        }
    }

    if (!out) {
        throw std::runtime_error("failed while writing PGM file: " + path);
    }
}

}  // namespace isec
