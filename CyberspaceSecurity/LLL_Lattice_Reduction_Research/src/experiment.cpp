#include "babai.hpp"
#include "lll.hpp"
#include "reduction_variants.hpp"

#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {

struct Options {
    std::string algorithm = "scan";
    int dimension = 10;
    int bits = 8;
    int repeat = 3;
    std::uint64_t seed = 2026;
    lll::Real delta = 0.99L;
    lll::Real eta = 0.51L;
    std::string strategy = "all";
    std::string output = "results/lll_parameter_scan.csv";
    int rounds = 32;
    int tailBlockSize = 4;
    int windowRadius = 1;
    int localSearchVectors = 4;
    int exactRadius = 2;
    int blockSize = 4;
    int tours = 1;
    int maxInsertDepth = 4;
    int maxPasses = 1;
    long long maxCandidates = 10000;
};

struct BestRecord {
    bool present = false;
    lll::Real delta = 0.0L;
    lll::Real eta = 0.0L;
    std::string strategy;
    double runtimeMs = 0.0;
    lll::Real rootHermiteFactor = 0.0L;
};

std::string outputOrDefault(const Options& options, const std::string& fallback) {
    return options.output == "results/lll_parameter_scan.csv" ? fallback : options.output;
}

std::string csvEscape(const std::string& value) {
    if (value.find_first_of(",\"\n") == std::string::npos) {
        return value;
    }

    std::string escaped = "\"";
    for (const char ch : value) {
        if (ch == '"') {
            escaped += "\"\"";
        } else {
            escaped += ch;
        }
    }
    escaped += '"';
    return escaped;
}

std::string formatReal(lll::Real value) {
    std::ostringstream out;
    out << std::setprecision(12) << static_cast<long double>(value);
    return out.str();
}

void printUsage(std::ostream& out) {
    out << "Usage:\n"
        << "  ./build/lll_experiment --algorithm scan --dimension 20 --bits 8 --repeat 3\n"
        << "  ./build/lll_experiment --algorithm lll --dimension 20 --bits 8 --delta 0.99 --eta 0.51\n"
        << "  ./build/lll_experiment --algorithm babai --dimension 6 --bits 8\n"
        << "  ./build/lll_experiment --algorithm babai-compare --dimension 6 --bits 8 --repeat 5\n"
        << "  ./build/lll_experiment --algorithm advanced --dimension 12 --bits 8 --repeat 2\n"
        << "  ./build/lll_experiment --algorithm report --dimension 8 --bits 6 --repeat 1\n\n"
        << "Options:\n"
        << "  --algorithm <scan|lll|babai|babai-compare|advanced|report>\n"
        << "  --dimension <n>\n"
        << "  --bits <coefficient bits, 1..32>\n"
        << "  --repeat <count>\n"
        << "  --seed <uint64>\n"
        << "  --delta <value>\n"
        << "  --eta <value>\n"
        << "  --strategy <all|full|suffix>\n"
        << "  --rounds <randomized Babai rounds>\n"
        << "  --tail-block-size <Babai tail enumeration block size>\n"
        << "  --window-radius <enumeration/local-search radius>\n"
        << "  --local-search-vectors <local search vector count>\n"
        << "  --exact-radius <small CVP exact enumeration radius>\n"
        << "  --block-size <mini-BKZ block size>\n"
        << "  --tours <mini-BKZ tours>\n"
        << "  --max-insert-depth <DeepLLL/PotLLL post-pass depth>\n"
        << "  --max-passes <DeepLLL/PotLLL post-pass passes>\n"
        << "  --max-candidates <enumeration candidate cap>\n"
        << "  --output <csv path>\n";
}

Options parseOptions(int argc, char** argv) {
    Options options;
    for (int i = 1; i < argc; ++i) {
        const std::string arg = argv[i];
        if (arg == "--help" || arg == "-h") {
            printUsage(std::cout);
            std::exit(0);
        }

        if (i + 1 >= argc) {
            throw std::invalid_argument("missing value for option " + arg);
        }

        const std::string value = argv[++i];
        if (arg == "--algorithm") {
            options.algorithm = value;
        } else if (arg == "--dimension") {
            options.dimension = std::stoi(value);
        } else if (arg == "--bits") {
            options.bits = std::stoi(value);
        } else if (arg == "--repeat") {
            options.repeat = std::stoi(value);
        } else if (arg == "--seed") {
            options.seed = static_cast<std::uint64_t>(std::stoull(value));
        } else if (arg == "--delta") {
            options.delta = std::stold(value);
        } else if (arg == "--eta") {
            options.eta = std::stold(value);
        } else if (arg == "--strategy") {
            options.strategy = value;
        } else if (arg == "--rounds") {
            options.rounds = std::stoi(value);
        } else if (arg == "--tail-block-size") {
            options.tailBlockSize = std::stoi(value);
        } else if (arg == "--window-radius") {
            options.windowRadius = std::stoi(value);
        } else if (arg == "--local-search-vectors") {
            options.localSearchVectors = std::stoi(value);
        } else if (arg == "--exact-radius") {
            options.exactRadius = std::stoi(value);
        } else if (arg == "--block-size") {
            options.blockSize = std::stoi(value);
        } else if (arg == "--tours") {
            options.tours = std::stoi(value);
        } else if (arg == "--max-insert-depth") {
            options.maxInsertDepth = std::stoi(value);
        } else if (arg == "--max-passes") {
            options.maxPasses = std::stoi(value);
        } else if (arg == "--max-candidates") {
            options.maxCandidates = std::stoll(value);
        } else if (arg == "--output") {
            options.output = value;
        } else {
            throw std::invalid_argument("unknown option " + arg);
        }
    }

    if (options.algorithm != "scan" && options.algorithm != "lll" && options.algorithm != "babai" &&
        options.algorithm != "babai-compare" && options.algorithm != "advanced" &&
        options.algorithm != "report") {
        throw std::invalid_argument(
            "--algorithm must be scan, lll, babai, babai-compare, advanced, or report");
    }
    if (options.repeat <= 0) {
        throw std::invalid_argument("--repeat must be positive");
    }
    if (options.strategy != "all" && options.strategy != "full" && options.strategy != "suffix") {
        throw std::invalid_argument("--strategy must be all, full, or suffix");
    }
    if (options.rounds <= 0 || options.tailBlockSize <= 0 || options.windowRadius < 0 ||
        options.localSearchVectors <= 0 || options.exactRadius < 0 || options.blockSize < 2 ||
        options.tours <= 0 || options.maxInsertDepth <= 0 || options.maxPasses <= 0 ||
        options.maxCandidates <= 0) {
        throw std::invalid_argument("one or more numeric experiment options are out of range");
    }
    return options;
}

std::vector<lll::GsoUpdateStrategy> selectedStrategies(const Options& options) {
    if (options.strategy == "all") {
        return {lll::GsoUpdateStrategy::Full, lll::GsoUpdateStrategy::Suffix};
    }
    return {lll::parseGsoUpdateStrategy(options.strategy)};
}

lll::LllConfig makeLllConfig(const Options& options) {
    lll::LllConfig config;
    config.delta = options.delta;
    config.eta = options.eta;
    config.updateStrategy =
        options.strategy == "all" ? lll::GsoUpdateStrategy::Suffix
                                  : lll::parseGsoUpdateStrategy(options.strategy);
    return config;
}

lll::AdvancedReductionConfig makeAdvancedConfig(const Options& options) {
    lll::AdvancedReductionConfig config;
    config.lllConfig = makeLllConfig(options);
    config.tours = options.tours;
    config.blockSize = options.blockSize;
    config.enumerationRadius = options.windowRadius;
    config.maxInsertDepth = options.maxInsertDepth;
    config.maxPasses = options.maxPasses;
    config.maxCandidates = options.maxCandidates;
    return config;
}

void ensureOutputDirectory(const std::string& output) {
    const std::filesystem::path outputPath(output);
    if (outputPath.has_parent_path()) {
        std::filesystem::create_directories(outputPath.parent_path());
    }
}

void writeStatsCsvRow(std::ostream& out, int bits, std::uint64_t seed, int repeatIndex,
                      const lll::LllConfig& config, const lll::LllStats& stats,
                      const std::string& error) {
    out << "LLL," << stats.dimension << ',' << stats.ambientDimension << ',' << bits << ','
        << seed << ',' << repeatIndex << ',' << formatReal(config.delta) << ','
        << formatReal(config.eta) << ',' << lll::toString(config.updateStrategy) << ','
        << std::setprecision(12) << stats.runtimeMs << ',' << stats.swapCount << ','
        << stats.sizeReductionCount << ',' << stats.gsoUpdateCount << ','
        << formatReal(stats.firstVectorNorm) << ',' << formatReal(stats.determinantEstimate)
        << ',' << formatReal(stats.rootHermiteFactor) << ','
        << formatReal(stats.orthogonalityDefect) << ','
        << (stats.validReduced ? "true" : "false") << ',' << csvEscape(error) << '\n';
}

void runSingleLll(const Options& options) {
    const lll::LllConfig config = makeLllConfig(options);

    const lll::Matrix basis = lll::randomIntegerBasis(options.dimension, options.bits, options.seed);
    const lll::LllResult result = lll::reduceLll(basis, config);
    const auto& stats = result.stats;

    std::cout << "dimension=" << stats.dimension << '\n'
              << "ambientDimension=" << stats.ambientDimension << '\n'
              << "delta=" << formatReal(stats.delta) << '\n'
              << "eta=" << formatReal(stats.eta) << '\n'
              << "strategy=" << lll::toString(config.updateStrategy) << '\n'
              << "runtimeMs=" << std::setprecision(12) << stats.runtimeMs << '\n'
              << "swapCount=" << stats.swapCount << '\n'
              << "sizeReductionCount=" << stats.sizeReductionCount << '\n'
              << "gsoUpdateCount=" << stats.gsoUpdateCount << '\n'
              << "firstVectorNorm=" << formatReal(stats.firstVectorNorm) << '\n'
              << "rootHermiteFactor=" << formatReal(stats.rootHermiteFactor) << '\n'
              << "orthogonalityDefect=" << formatReal(stats.orthogonalityDefect) << '\n'
              << "validReduced=" << (stats.validReduced ? "true" : "false") << '\n';
}

void runBabaiDemo(const Options& options) {
    const lll::LllConfig config = makeLllConfig(options);

    lll::Matrix basis = lll::randomIntegerBasis(options.dimension, options.bits, options.seed);
    basis = lll::reduceLll(basis, config).reducedBasis;

    std::vector<lll::Integer> knownCoefficients(basis.size(), 0);
    for (std::size_t i = 0; i < knownCoefficients.size(); ++i) {
        knownCoefficients[i] = static_cast<lll::Integer>((i % 3) - 1);
    }

    const lll::Vector nearbyLatticePoint = lll::integerCombination(basis, knownCoefficients);
    lll::RealVector target = lll::toRealVector(nearbyLatticePoint);
    if (!target.empty()) {
        target.front() += 0.25L;
        target.back() -= 0.40L;
    }

    const lll::BabaiResult result = lll::babaiNearestPlane(basis, target);
    std::cout << "distance=" << formatReal(result.distance) << '\n'
              << "runtimeMs=" << std::setprecision(12) << result.runtimeMs << '\n'
              << "candidateCount=" << result.candidateCount << '\n'
              << "latticeVector=" << lll::formatVector(result.latticeVector) << '\n';
}

void runScan(const Options& options) {
    const std::vector<lll::Real> deltaValues = {0.75L, 0.90L, 0.95L, 0.99L, 0.999L};
    const std::vector<lll::Real> etaValues = {0.501L, 0.51L, 0.75L};
    const std::vector<lll::GsoUpdateStrategy> strategies = selectedStrategies(options);

    ensureOutputDirectory(options.output);

    std::ofstream out(options.output);
    if (!out) {
        throw std::runtime_error("failed to open output CSV: " + options.output);
    }

    out << "algorithm,dimension,ambientDimension,bits,seed,repeatIndex,delta,eta,"
        << "updateStrategy,runtimeMs,swapCount,sizeReductionCount,gsoUpdateCount,"
        << "firstVectorNorm,determinantEstimate,rootHermiteFactor,orthogonalityDefect,"
        << "validReduced,error\n";

    BestRecord bestQuality;
    BestRecord bestSpeed;
    int validRuns = 0;
    int totalRuns = 0;

    for (int repeatIndex = 0; repeatIndex < options.repeat; ++repeatIndex) {
        const std::uint64_t basisSeed = options.seed + static_cast<std::uint64_t>(repeatIndex) * 1009ULL;
        const lll::Matrix basis = lll::randomIntegerBasis(options.dimension, options.bits, basisSeed);

        for (const lll::Real delta : deltaValues) {
            for (const lll::Real eta : etaValues) {
                if (!(eta < std::sqrt(delta))) {
                    continue;
                }

                for (const lll::GsoUpdateStrategy strategy : strategies) {
                    ++totalRuns;
                    lll::LllConfig config;
                    config.delta = delta;
                    config.eta = eta;
                    config.updateStrategy = strategy;

                    try {
                        const lll::LllResult result = lll::reduceLll(basis, config);
                        writeStatsCsvRow(out, options.bits, basisSeed, repeatIndex, config,
                                         result.stats, "");

                        if (result.stats.validReduced) {
                            ++validRuns;
                            if (!bestQuality.present ||
                                result.stats.rootHermiteFactor < bestQuality.rootHermiteFactor) {
                                bestQuality.present = true;
                                bestQuality.delta = delta;
                                bestQuality.eta = eta;
                                bestQuality.strategy = lll::toString(strategy);
                                bestQuality.runtimeMs = result.stats.runtimeMs;
                                bestQuality.rootHermiteFactor = result.stats.rootHermiteFactor;
                            }
                            if (!bestSpeed.present || result.stats.runtimeMs < bestSpeed.runtimeMs) {
                                bestSpeed.present = true;
                                bestSpeed.delta = delta;
                                bestSpeed.eta = eta;
                                bestSpeed.strategy = lll::toString(strategy);
                                bestSpeed.runtimeMs = result.stats.runtimeMs;
                                bestSpeed.rootHermiteFactor = result.stats.rootHermiteFactor;
                            }
                        }
                    } catch (const std::exception& ex) {
                        lll::LllStats failedStats;
                        failedStats.dimension = options.dimension;
                        failedStats.ambientDimension = options.dimension;
                        failedStats.delta = delta;
                        failedStats.eta = eta;
                        writeStatsCsvRow(out, options.bits, basisSeed, repeatIndex, config,
                                         failedStats, ex.what());
                    }
                }
            }
        }
    }

    std::cout << "wrote " << options.output << '\n'
              << "validRuns=" << validRuns << "/" << totalRuns << '\n';
    if (bestQuality.present) {
        std::cout << "qualityRecommendation: delta=" << formatReal(bestQuality.delta)
                  << ", eta=" << formatReal(bestQuality.eta)
                  << ", strategy=" << bestQuality.strategy
                  << ", rootHermiteFactor=" << formatReal(bestQuality.rootHermiteFactor)
                  << ", runtimeMs=" << std::setprecision(12) << bestQuality.runtimeMs << '\n';
    }
    if (bestSpeed.present) {
        std::cout << "speedRecommendation: delta=" << formatReal(bestSpeed.delta)
                  << ", eta=" << formatReal(bestSpeed.eta)
                  << ", strategy=" << bestSpeed.strategy
                  << ", runtimeMs=" << std::setprecision(12) << bestSpeed.runtimeMs
                  << ", rootHermiteFactor=" << formatReal(bestSpeed.rootHermiteFactor) << '\n';
    }
}

lll::RealVector makeNearbyTarget(const lll::Matrix& basis) {
    std::vector<lll::Integer> coefficients(basis.size(), 0);
    for (std::size_t i = 0; i < coefficients.size(); ++i) {
        coefficients[i] = static_cast<lll::Integer>((i % 3) - 1);
    }

    const lll::Vector latticePoint = lll::integerCombination(basis, coefficients);
    lll::RealVector target = lll::toRealVector(latticePoint);
    for (std::size_t i = 0; i < target.size(); ++i) {
        const lll::Real sign = (i % 2 == 0) ? 1.0L : -1.0L;
        target[i] += sign * (0.35L / static_cast<lll::Real>(i + 1));
    }
    return target;
}

void writeBabaiComparisonRow(std::ostream& out, const std::string& basisKind,
                             const std::string& method, int dimension, int bits,
                             std::uint64_t seed, int repeatIndex,
                             const lll::BabaiResult& result, bool hasExact,
                             const lll::BabaiResult& exact, double preReductionTimeMs) {
    const bool exactHit = hasExact && result.latticeVector == exact.latticeVector;
    const lll::Real approxRatio =
        hasExact && exact.distance > 0.0L ? result.distance / exact.distance : 0.0L;

    out << basisKind << ',' << method << ',' << dimension << ',' << bits << ',' << seed << ','
        << repeatIndex << ',' << std::setprecision(12) << result.runtimeMs << ','
        << formatReal(result.distance) << ','
        << (hasExact ? formatReal(approxRatio) : "") << ','
        << (hasExact ? (exactHit ? "true" : "false") : "") << ','
        << result.candidateCount << ',' << preReductionTimeMs << '\n';
}

void runBabaiComparisonForBasis(std::ostream& out, const Options& options,
                                const std::string& basisKind, const lll::Matrix& basis,
                                const lll::RealVector& target, std::uint64_t seed,
                                int repeatIndex, bool hasExact, const lll::BabaiResult& exact,
                                double preReductionTimeMs) {
    const lll::BabaiResult base = lll::babaiNearestPlane(basis, target);
    writeBabaiComparisonRow(out, basisKind, "nearest-plane", options.dimension, options.bits, seed,
                            repeatIndex, base, hasExact, exact, preReductionTimeMs);

    lll::RandomizedBabaiConfig randomConfig;
    randomConfig.rounds = options.rounds;
    randomConfig.seed = seed + 17ULL;
    const lll::BabaiResult randomized =
        lll::randomizedBabaiNearestPlane(basis, target, randomConfig);
    writeBabaiComparisonRow(out, basisKind, "randomized", options.dimension, options.bits, seed,
                            repeatIndex, randomized, hasExact, exact, preReductionTimeMs);

    lll::TailEnumerationConfig tailConfig;
    tailConfig.tailBlockSize = options.tailBlockSize;
    tailConfig.windowRadius = options.windowRadius;
    tailConfig.maxCandidates = options.maxCandidates;
    const lll::BabaiResult tail = lll::tailEnumerationBabai(basis, target, tailConfig);
    writeBabaiComparisonRow(out, basisKind, "tail-enumeration", options.dimension, options.bits,
                            seed, repeatIndex, tail, hasExact, exact, preReductionTimeMs);

    lll::LocalSearchConfig localConfig;
    localConfig.searchVectorCount = options.localSearchVectors;
    localConfig.windowRadius = options.windowRadius;
    localConfig.maxCandidates = options.maxCandidates;
    const lll::BabaiResult local = lll::localSearchBabai(basis, target, localConfig);
    writeBabaiComparisonRow(out, basisKind, "local-search", options.dimension, options.bits, seed,
                            repeatIndex, local, hasExact, exact, preReductionTimeMs);
}

void runBabaiCompare(const Options& options) {
    const std::string output = outputOrDefault(options, "results/babai_comparison.csv");
    ensureOutputDirectory(output);

    std::ofstream out(output);
    if (!out) {
        throw std::runtime_error("failed to open output CSV: " + output);
    }
    out << "basisKind,method,dimension,bits,seed,repeatIndex,runtimeMs,distance,"
        << "approxRatio,exactHit,candidateCount,preReductionTimeMs\n";

    const lll::LllConfig config = makeLllConfig(options);
    for (int repeatIndex = 0; repeatIndex < options.repeat; ++repeatIndex) {
        const std::uint64_t basisSeed = options.seed + static_cast<std::uint64_t>(repeatIndex) * 1009ULL;
        const lll::Matrix rawBasis =
            lll::randomIntegerBasis(options.dimension, options.bits, basisSeed);
        const lll::RealVector target = makeNearbyTarget(rawBasis);
        const bool hasExact = rawBasis.size() <= 8;
        lll::BabaiResult exact;
        if (hasExact) {
            exact = lll::exactClosestByEnumeration(rawBasis, target, options.exactRadius);
        }

        runBabaiComparisonForBasis(out, options, "raw", rawBasis, target, basisSeed,
                                   repeatIndex, hasExact, exact, 0.0);

        const lll::LllResult reduced = lll::reduceLll(rawBasis, config);
        runBabaiComparisonForBasis(out, options, "lll-reduced", reduced.reducedBasis, target,
                                   basisSeed, repeatIndex, hasExact, exact,
                                   reduced.stats.runtimeMs);
    }

    std::cout << "wrote " << output << '\n';
}

void writeAdvancedRow(std::ostream& out, const std::string& algorithm, int dimension, int bits,
                      std::uint64_t seed, int repeatIndex, double runtimeMs,
                      long long candidateCount, long long insertionCount, int completedPasses,
                      const lll::LllStats& stats) {
    out << algorithm << ',' << dimension << ',' << bits << ',' << seed << ',' << repeatIndex << ','
        << std::setprecision(12) << runtimeMs << ',' << candidateCount << ','
        << insertionCount << ',' << completedPasses << ',' << stats.swapCount << ','
        << stats.sizeReductionCount << ',' << stats.gsoUpdateCount << ','
        << formatReal(stats.firstVectorNorm) << ',' << formatReal(stats.determinantEstimate)
        << ',' << formatReal(stats.rootHermiteFactor) << ','
        << formatReal(stats.orthogonalityDefect) << ','
        << (stats.validReduced ? "true" : "false") << '\n';
}

void runAdvancedCompare(const Options& options) {
    const std::string output = outputOrDefault(options, "results/advanced_comparison.csv");
    ensureOutputDirectory(output);

    std::ofstream out(output);
    if (!out) {
        throw std::runtime_error("failed to open output CSV: " + output);
    }
    out << "algorithm,dimension,bits,seed,repeatIndex,runtimeMs,candidateCount,"
        << "insertionCount,completedPasses,swapCount,sizeReductionCount,gsoUpdateCount,"
        << "firstVectorNorm,determinantEstimate,rootHermiteFactor,orthogonalityDefect,"
        << "validReduced\n";

    const lll::LllConfig lllConfig = makeLllConfig(options);
    const lll::AdvancedReductionConfig advancedConfig = makeAdvancedConfig(options);
    for (int repeatIndex = 0; repeatIndex < options.repeat; ++repeatIndex) {
        const std::uint64_t basisSeed = options.seed + static_cast<std::uint64_t>(repeatIndex) * 1009ULL;
        const lll::Matrix basis =
            lll::randomIntegerBasis(options.dimension, options.bits, basisSeed);

        const lll::LllResult baseline = lll::reduceLll(basis, lllConfig);
        writeAdvancedRow(out, "lll", options.dimension, options.bits, basisSeed, repeatIndex,
                         baseline.stats.runtimeMs, 0, 0, 0, baseline.stats);

        const lll::AdvancedReductionResult miniBkz = lll::reduceMiniBkz(basis, advancedConfig);
        writeAdvancedRow(out, miniBkz.stats.algorithm, options.dimension, options.bits, basisSeed,
                         repeatIndex, miniBkz.stats.runtimeMs, miniBkz.stats.candidateCount,
                         miniBkz.stats.insertionCount, miniBkz.stats.completedPasses,
                         miniBkz.stats.lllStats);

        const lll::AdvancedReductionResult deep =
            lll::reduceDeepLllPostPass(basis, advancedConfig);
        writeAdvancedRow(out, deep.stats.algorithm, options.dimension, options.bits, basisSeed,
                         repeatIndex, deep.stats.runtimeMs, deep.stats.candidateCount,
                         deep.stats.insertionCount, deep.stats.completedPasses,
                         deep.stats.lllStats);

        const lll::AdvancedReductionResult pot =
            lll::reducePotLllPostPass(basis, advancedConfig);
        writeAdvancedRow(out, pot.stats.algorithm, options.dimension, options.bits, basisSeed,
                         repeatIndex, pot.stats.runtimeMs, pot.stats.candidateCount,
                         pot.stats.insertionCount, pot.stats.completedPasses,
                         pot.stats.lllStats);
    }

    std::cout << "wrote " << output << '\n';
}

void runReport(Options options) {
    const std::string reportPath = outputOrDefault(options, "results/summary_report.md");
    ensureOutputDirectory(reportPath);

    const std::filesystem::path reportFile(reportPath);
    const std::filesystem::path reportDir =
        reportFile.has_parent_path() ? reportFile.parent_path() : std::filesystem::path(".");

    Options scanOptions = options;
    scanOptions.algorithm = "scan";
    scanOptions.output = (reportDir / "lll_parameter_scan.csv").string();
    runScan(scanOptions);

    Options babaiOptions = options;
    babaiOptions.algorithm = "babai-compare";
    babaiOptions.output = (reportDir / "babai_comparison.csv").string();
    runBabaiCompare(babaiOptions);

    Options advancedOptions = options;
    advancedOptions.algorithm = "advanced";
    advancedOptions.output = (reportDir / "advanced_comparison.csv").string();
    runAdvancedCompare(advancedOptions);

    std::ofstream report(reportPath);
    if (!report) {
        throw std::runtime_error("failed to open report: " + reportPath);
    }

    report << "# LLL 格基约化阶段实验报告\n\n"
           << "## 实验配置\n\n"
           << "- dimension: " << options.dimension << "\n"
           << "- coefficientBits: " << options.bits << "\n"
           << "- repeat: " << options.repeat << "\n"
           << "- seed: " << options.seed << "\n"
           << "- delta: " << formatReal(options.delta) << "\n"
           << "- eta: " << formatReal(options.eta) << "\n"
           << "- GSO strategy: "
           << (options.strategy == "all" ? "suffix for non-scan runs" : options.strategy)
           << "\n\n"
           << "## 输出文件\n\n"
           << "- `lll_parameter_scan.csv`: 阶段二参数扫描，包含运行时间、交换次数、RHF 和校验结果。\n"
           << "- `babai_comparison.csv`: 阶段三 Babai 基线、随机舍入、尾部枚举和局部搜索对比。\n"
           << "- `advanced_comparison.csv`: 阶段四 LLL、mini-BKZ、DeepLLL post-pass、PotLLL post-pass 对比。\n\n"
           << "## 结果解读\n\n"
           << "1. 参数扫描优先观察 `validReduced=true` 的记录，再比较 `rootHermiteFactor` 和 `runtimeMs`。\n"
           << "2. Babai 对比中，`lll-reduced` 行应作为预处理后效果，重点看 `approxRatio`、`exactHit` 和 `candidateCount`。\n"
           << "3. 高级约化对比中，`mini-bkz` 是小块枚举原型；`deep-lll-postpass` 和 `pot-lll-postpass` 是受限插入后处理，用于课程实验趋势对比。\n"
           << "4. 若某个高级算法的 RHF 更低但运行时间和候选数明显更高，应在报告中归为质量优先方案，而不是默认方案。\n\n"
           << "## 后续方向\n\n"
           << "1. 把当前 CSV 导入 notebook 绘制 `delta-runtime`、`delta-RHF`、Babai 命中率和高级算法 Pareto 图。\n"
           << "2. 高维或大系数实验前，将整数类型扩展到 `boost::multiprecision::cpp_int`，并增加精确行列式校验。\n"
           << "3. 若需要接近真实密码分析实验，应引入 fplll/fpylll 作为 BKZ 对照基准。\n";

    std::cout << "wrote " << reportPath << '\n';
}

}  // namespace

int main(int argc, char** argv) {
    try {
        const Options options = parseOptions(argc, argv);
        if (options.algorithm == "scan") {
            runScan(options);
        } else if (options.algorithm == "lll") {
            runSingleLll(options);
        } else if (options.algorithm == "babai") {
            runBabaiDemo(options);
        } else if (options.algorithm == "babai-compare") {
            runBabaiCompare(options);
        } else if (options.algorithm == "advanced") {
            runAdvancedCompare(options);
        } else {
            runReport(options);
        }
    } catch (const std::exception& ex) {
        std::cerr << "error: " << ex.what() << '\n';
        return 1;
    }
    return 0;
}
