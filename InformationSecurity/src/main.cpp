#include "chaos.h"
#include "cipher.h"
#include "metrics.h"
#include "pgm.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

isec::LorenzKey defaultKey() {
    isec::LorenzKey key;
    key.params.a = 10.0;
    key.params.b = 8.0 / 3.0;
    key.params.c = 28.0;
    key.params.r = -1.0;
    key.params.step = 0.001;
    key.params.discard = 2000;
    key.params.sample_gap = 7;
    key.seed.x = 1.0;
    key.seed.y = 1.0;
    key.seed.z = 1.0;
    key.seed.w = 1.0;
    return key;
}

void printUsage(std::ostream& out) {
    out << "Usage:\n"
        << "  image_cipher encrypt <input.pgm> <cipher.pgm> "
           "[--preview <preview.pgm>] [--x0 v --y0 v --z0 v --w0 v]\n"
        << "  image_cipher decrypt <cipher.pgm> <output.pgm> "
           "[--x0 v --y0 v --z0 v --w0 v]\n"
        << "  image_cipher test <input.pgm> <cipher.pgm> <output.pgm> "
           "[--preview <preview.pgm>] [--x0 v --y0 v --z0 v --w0 v]\n";
}

void printKey(std::ostream& out, const isec::LorenzKey& key) {
    out << "Lorenz parameters: a=" << key.params.a
        << " b=" << key.params.b
        << " c=" << key.params.c
        << " r=" << key.params.r
        << " h=" << key.params.step
        << " discard=" << key.params.discard
        << " gap=" << key.params.sample_gap << '\n';
    out << "Initial state: x0=" << key.seed.x
        << " y0=" << key.seed.y
        << " z0=" << key.seed.z
        << " w0=" << key.seed.w << '\n';
}

std::string defaultPreviewPath(const std::string& cipherPath) {
    const std::size_t dot = cipherPath.find_last_of('.');
    if (dot == std::string::npos) {
        return cipherPath + "_preview.pgm";
    }
    return cipherPath.substr(0, dot) + "_preview.pgm";
}

void parseOptions(int argc, char** argv, int firstOption, isec::LorenzKey& key,
                  std::string& preview) {
    for (int i = firstOption; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--preview") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--preview requires a file path");
            }
            preview = argv[++i];
        } else if (option == "--x0") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--x0 requires a value");
            }
            key.seed.x = std::stod(argv[++i]);
        } else if (option == "--y0") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--y0 requires a value");
            }
            key.seed.y = std::stod(argv[++i]);
        } else if (option == "--z0") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--z0 requires a value");
            }
            key.seed.z = std::stod(argv[++i]);
        } else if (option == "--w0") {
            if (i + 1 >= argc) {
                throw std::invalid_argument("--w0 requires a value");
            }
            key.seed.w = std::stod(argv[++i]);
        } else {
            throw std::invalid_argument("unknown option: " + option);
        }
    }
}

}  // namespace

int main(int argc, char** argv) {
    try {
        if (argc < 2) {
            printUsage(std::cerr);
            return 1;
        }

        const std::string command = argv[1];
        if (command == "encrypt" && argc >= 4) {
            const std::string inputPath = argv[2];
            const std::string cipherPath = argv[3];
            std::string previewPath = defaultPreviewPath(cipherPath);
            isec::LorenzKey key = defaultKey();
            parseOptions(argc, argv, 4, key, previewPath);

            isec::PGMImage plain;
            isec::readPGM(inputPath, plain);
            const isec::PGMImage cipher =
                isec::encryptToCoefficientImage(plain, key);
            isec::writePGM(cipherPath, cipher);
            isec::writePGM(previewPath, isec::makePreviewImage(cipher));

            std::cout << "Encrypted " << inputPath << " -> "
                      << cipherPath << '\n';
            std::cout << "Preview written to " << previewPath << '\n';
            printKey(std::cout, key);
            return 0;
        }

        if (command == "decrypt" && argc >= 4) {
            const std::string cipherPath = argv[2];
            const std::string outputPath = argv[3];
            isec::LorenzKey key = defaultKey();
            std::string unusedPreview;
            parseOptions(argc, argv, 4, key, unusedPreview);

            isec::PGMImage cipher;
            isec::readPGM(cipherPath, cipher);
            const isec::PGMImage recovered =
                isec::decryptFromCoefficientImage(cipher, key);
            isec::writePGM(outputPath, recovered);

            std::cout << "Decrypted " << cipherPath << " -> "
                      << outputPath << '\n';
            printKey(std::cout, key);
            return 0;
        }

        if (command == "test" && argc >= 5) {
            const std::string inputPath = argv[2];
            const std::string cipherPath = argv[3];
            const std::string outputPath = argv[4];
            std::string previewPath = defaultPreviewPath(cipherPath);
            isec::LorenzKey key = defaultKey();
            parseOptions(argc, argv, 5, key, previewPath);

            isec::PGMImage plain;
            isec::readPGM(inputPath, plain);
            const isec::PGMImage cipher =
                isec::encryptToCoefficientImage(plain, key);
            const isec::PGMImage preview = isec::makePreviewImage(cipher);
            isec::writePGM(cipherPath, cipher);
            isec::writePGM(previewPath, preview);

            isec::PGMImage cipherFromDisk;
            isec::readPGM(cipherPath, cipherFromDisk);
            const isec::PGMImage recovered =
                isec::decryptFromCoefficientImage(cipherFromDisk, key);
            isec::writePGM(outputPath, recovered);

            std::cout << std::fixed << std::setprecision(6);
            std::cout << "Files:\n"
                      << "  plain    : " << inputPath << '\n'
                      << "  cipher   : " << cipherPath << '\n'
                      << "  preview  : " << previewPath << '\n'
                      << "  recovered: " << outputPath << "\n\n";

            std::cout << "Key:\n";
            printKey(std::cout, key);

            std::cout << "\nMetrics:\n";
            std::cout << "  plain entropy              : "
                      << isec::entropy(plain) << " bit\n";
            std::cout << "  cipher preview entropy     : "
                      << isec::entropy(preview) << " bit\n";
            std::cout << "  cipher horizontal corr     : "
                      << isec::adjacentCorrelation(preview, true) << '\n';
            std::cout << "  cipher vertical corr       : "
                      << isec::adjacentCorrelation(preview, false) << '\n';
            std::cout << "  decrypted PSNR             : "
                      << isec::psnr(plain, recovered) << " dB\n";
            std::cout << "  decrypted MSE              : "
                      << isec::mse(plain, recovered) << '\n';
            return 0;
        }

        printUsage(std::cerr);
        return 1;
    } catch (const std::exception& error) {
        std::cerr << "error: " << error.what() << '\n';
        return 1;
    }
}
