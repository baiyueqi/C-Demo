#include <algorithm>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <numeric>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

using Bits = std::vector<int>;
using Carrier = std::vector<unsigned char>;

struct DecodeStats {
    int totalCodewords = 0;
    int cleanCodewords = 0;
    int correctedCodewords = 0;
    int failedCodewords = 0;
};

struct DecodeResult {
    Bits bits;
    DecodeStats stats;
};

struct ExperimentResult {
    int noisePercent = 0;
    std::string recoveredText;
    double charAccuracy = 0.0;
    DecodeStats stats;
};

Bits textToBits(const std::string& text) {
    Bits bits;
    bits.reserve(text.size() * 8);

    for (unsigned char ch : text) {
        for (int bit = 7; bit >= 0; --bit) {
            bits.push_back((ch >> bit) & 1);
        }
    }

    return bits;
}

std::string bitsToText(const Bits& bits, std::size_t textLength) {
    std::string text;
    text.reserve(textLength);

    for (std::size_t i = 0; i + 7 < bits.size() && text.size() < textLength; i += 8) {
        unsigned char ch = 0;
        for (int j = 0; j < 8; ++j) {
            ch = static_cast<unsigned char>((ch << 1) | bits[i + j]);
        }
        text.push_back(static_cast<char>(ch));
    }

    return text;
}

void padToBlockSize(Bits& bits, std::size_t blockSize) {
    while (bits.size() % blockSize != 0) {
        bits.push_back(0);
    }
}

Bits hammingEncodeBlock(int d1, int d2, int d3, int d4) {
    const int p1 = d1 ^ d2 ^ d4;
    const int p2 = d1 ^ d3 ^ d4;
    const int p3 = d2 ^ d3 ^ d4;
    return {d1, d2, d3, d4, p1, p2, p3};
}

Bits hammingEncode(const Bits& dataBits) {
    if (dataBits.size() % 4 != 0) {
        throw std::invalid_argument("data bit count must be a multiple of 4");
    }

    Bits encoded;
    encoded.reserve(dataBits.size() / 4 * 7);

    for (std::size_t i = 0; i < dataBits.size(); i += 4) {
        Bits codeword = hammingEncodeBlock(
            dataBits[i],
            dataBits[i + 1],
            dataBits[i + 2],
            dataBits[i + 3]);
        encoded.insert(encoded.end(), codeword.begin(), codeword.end());
    }

    return encoded;
}

int syndromeToErrorIndex(int s1, int s2, int s3) {
    static const int columns[7][3] = {
        {1, 1, 0},
        {1, 0, 1},
        {0, 1, 1},
        {1, 1, 1},
        {1, 0, 0},
        {0, 1, 0},
        {0, 0, 1},
    };

    for (int i = 0; i < 7; ++i) {
        if (columns[i][0] == s1 && columns[i][1] == s2 && columns[i][2] == s3) {
            return i;
        }
    }

    return -1;
}

DecodeResult hammingDecode(const Bits& encodedBits) {
    if (encodedBits.size() % 7 != 0) {
        throw std::invalid_argument("encoded bit count must be a multiple of 7");
    }

    DecodeResult result;
    result.bits.reserve(encodedBits.size() / 7 * 4);

    for (std::size_t i = 0; i < encodedBits.size(); i += 7) {
        Bits r(encodedBits.begin() + i, encodedBits.begin() + i + 7);
        ++result.stats.totalCodewords;

        const int s1 = r[0] ^ r[1] ^ r[3] ^ r[4];
        const int s2 = r[0] ^ r[2] ^ r[3] ^ r[5];
        const int s3 = r[1] ^ r[2] ^ r[3] ^ r[6];

        if (s1 == 0 && s2 == 0 && s3 == 0) {
            ++result.stats.cleanCodewords;
        } else {
            const int errorIndex = syndromeToErrorIndex(s1, s2, s3);
            if (errorIndex >= 0) {
                r[errorIndex] ^= 1;
                ++result.stats.correctedCodewords;
            } else {
                ++result.stats.failedCodewords;
            }
        }

        result.bits.insert(result.bits.end(), r.begin(), r.begin() + 4);
    }

    return result;
}

Carrier generateCarrier(std::size_t size, unsigned int seed) {
    Carrier carrier(size);
    std::mt19937 rng(seed);
    std::uniform_int_distribution<int> byteDist(0, 255);

    for (unsigned char& value : carrier) {
        value = static_cast<unsigned char>(byteDist(rng));
    }

    return carrier;
}

std::vector<std::size_t> generateRandomPositions(
    std::size_t carrierSize,
    std::size_t count,
    unsigned int seed) {
    if (count > carrierSize) {
        throw std::invalid_argument("carrier capacity is smaller than required bit count");
    }

    std::vector<std::size_t> positions(carrierSize);
    std::iota(positions.begin(), positions.end(), 0);

    std::mt19937 rng(seed);
    std::shuffle(positions.begin(), positions.end(), rng);
    positions.resize(count);
    return positions;
}

Carrier embedBits(const Carrier& carrier, const Bits& encodedBits, const std::vector<std::size_t>& positions) {
    if (encodedBits.size() != positions.size()) {
        throw std::invalid_argument("encoded bit count and position count do not match");
    }

    Carrier stegoCarrier = carrier;
    for (std::size_t i = 0; i < encodedBits.size(); ++i) {
        unsigned char& value = stegoCarrier[positions[i]];
        value = static_cast<unsigned char>((value & 0xFE) | encodedBits[i]);
    }

    return stegoCarrier;
}

Bits extractBits(const Carrier& carrier, const std::vector<std::size_t>& positions) {
    Bits bits;
    bits.reserve(positions.size());

    for (std::size_t position : positions) {
        bits.push_back(carrier[position] & 1);
    }

    return bits;
}

void applyNoise(Carrier& carrier, int noisePercent, unsigned int seed) {
    const std::size_t attackCount = carrier.size() * static_cast<std::size_t>(noisePercent) / 100;
    std::vector<std::size_t> positions = generateRandomPositions(carrier.size(), attackCount, seed);

    for (std::size_t position : positions) {
        carrier[position] ^= 1;
    }
}

double calculateCharAccuracy(const std::string& expected, const std::string& actual) {
    if (expected.empty()) {
        return actual.empty() ? 100.0 : 0.0;
    }

    std::size_t correct = 0;
    for (std::size_t i = 0; i < expected.size(); ++i) {
        if (i < actual.size() && expected[i] == actual[i]) {
            ++correct;
        }
    }

    return static_cast<double>(correct) * 100.0 / static_cast<double>(expected.size());
}

ExperimentResult runExperiment(
    const Carrier& stegoCarrier,
    const std::string& message,
    std::size_t encodedBitCount,
    unsigned int embedSeed,
    int noisePercent) {
    Carrier attackedCarrier = stegoCarrier;
    if (noisePercent > 0) {
        applyNoise(attackedCarrier, noisePercent, embedSeed + 1009U * static_cast<unsigned int>(noisePercent));
    }

    const std::vector<std::size_t> positions =
        generateRandomPositions(attackedCarrier.size(), encodedBitCount, embedSeed);
    const Bits extractedBits = extractBits(attackedCarrier, positions);
    const DecodeResult decoded = hammingDecode(extractedBits);
    const std::string recoveredText = bitsToText(decoded.bits, message.size());

    return {
        noisePercent,
        recoveredText,
        calculateCharAccuracy(message, recoveredText),
        decoded.stats,
    };
}

void printUsage(const char* programName) {
    std::cout << "用法: " << programName << " [秘密信息] [随机种子] [载体长度]\n"
              << "示例: " << programName << " \"Hello Information Theory\" 202405 100000\n";
}

void printStats(const DecodeStats& stats) {
    std::cout << "无错误码字数: " << stats.cleanCodewords << '\n'
              << "纠正码字数: " << stats.correctedCodewords << '\n'
              << "无法纠正码字数: " << stats.failedCodewords << '\n';
}

int main(int argc, char* argv[]) {
    const std::string message = argc > 1 ? argv[1] : "Hello Information Theory";
    const unsigned int seed = argc > 2 ? static_cast<unsigned int>(std::stoul(argv[2])) : 202405U;
    const std::size_t carrierSize = argc > 3 ? static_cast<std::size_t>(std::stoull(argv[3])) : 100000U;

    if (argc > 4) {
        printUsage(argv[0]);
        return 1;
    }

    try {
        Bits dataBits = textToBits(message);
        const std::size_t originalBitCount = dataBits.size();
        padToBlockSize(dataBits, 4);
        const Bits encodedBits = hammingEncode(dataBits);

        if (encodedBits.size() > carrierSize) {
            std::cerr << "错误: 载体容量不足。需要 " << encodedBits.size()
                      << " bit，当前载体容量为 " << carrierSize << " bit。\n";
            return 1;
        }

        const Carrier carrier = generateCarrier(carrierSize, seed);
        const std::vector<std::size_t> positions =
            generateRandomPositions(carrier.size(), encodedBits.size(), seed);
        const Carrier stegoCarrier = embedBits(carrier, encodedBits, positions);

        std::cout << "原始信息: " << message << '\n'
                  << "随机种子: " << seed << '\n'
                  << "载体长度: " << carrierSize << " byte\n"
                  << "编码前 bit 数: " << originalBitCount << '\n'
                  << "补齐后 bit 数: " << dataBits.size() << '\n'
                  << "编码后 bit 数: " << encodedBits.size() << '\n'
                  << "载体容量: " << carrierSize << " bit\n\n";

        std::cout << std::fixed << std::setprecision(2);

        for (int noisePercent : {0, 1, 2, 3, 4, 5}) {
            const ExperimentResult result =
                runExperiment(stegoCarrier, message, encodedBits.size(), seed, noisePercent);

            std::cout << "噪声比例: " << result.noisePercent << "%\n"
                      << "恢复信息: " << result.recoveredText << '\n'
                      << "字符正确率: " << result.charAccuracy << "%\n";
            printStats(result.stats);
            std::cout << '\n';
        }
    } catch (const std::exception& ex) {
        std::cerr << "错误: " << ex.what() << '\n';
        printUsage(argv[0]);
        return 1;
    }

    return 0;
}
