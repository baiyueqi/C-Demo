#include "common.h"
#include "simulator.h"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>

namespace {

int parseInt(const std::string& value, const std::string& option)
{
    std::size_t pos = 0;
    const int parsed = std::stoi(value, &pos);
    if (pos != value.size()) {
        throw std::invalid_argument("选项 " + option + " 需要整数：" + value);
    }
    return parsed;
}

long long parseLongLong(const std::string& value, const std::string& option)
{
    std::size_t pos = 0;
    const long long parsed = std::stoll(value, &pos);
    if (pos != value.size()) {
        throw std::invalid_argument("选项 " + option + " 需要整数：" + value);
    }
    return parsed;
}

std::string readValue(int& index, int argc, char** argv, const std::string& option)
{
    if (index + 1 >= argc) {
        throw std::invalid_argument("选项 " + option + " 缺少参数值");
    }
    ++index;
    return argv[index];
}

void printUsage(const char* program)
{
    std::cout
        << "用法: " << program << " [选项]\n"
        << "  --readers N        读者线程数量，默认 8\n"
        << "  --writers N        写者线程数量，默认 3\n"
        << "  --cache-size N     有界缓存容量，默认 5\n"
        << "  --rounds N         每个读者/写者的操作轮数，默认 5\n"
        << "  --student-no N     学号随机种子，默认 20241001655\n"
        << "  --delay-min-ms N   模拟延时下限（毫秒），默认 1000\n"
        << "  --delay-max-ms N   模拟延时上限（毫秒），默认 5000\n"
        << "  --seed N           随机选书种子，默认使用学号\n";
}

Config parseArgs(int argc, char** argv)
{
    Config config;
    bool seed_provided = false;

    // 参数解析保持简单：每个选项都只读取一个值
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--help" || option == "-h") {
            printUsage(argv[0]);
            std::exit(0);
        } else if (option == "--readers") {
            config.readers = parseInt(readValue(i, argc, argv, option), option);
        } else if (option == "--writers") {
            config.writers = parseInt(readValue(i, argc, argv, option), option);
        } else if (option == "--cache-size") {
            config.cache_size = parseInt(readValue(i, argc, argv, option), option);
        } else if (option == "--rounds") {
            config.rounds = parseInt(readValue(i, argc, argv, option), option);
        } else if (option == "--student-no") {
            config.student_no = parseLongLong(readValue(i, argc, argv, option), option);
        } else if (option == "--delay-min-ms") {
            config.delay_min_ms = parseInt(readValue(i, argc, argv, option), option);
        } else if (option == "--delay-max-ms") {
            config.delay_max_ms = parseInt(readValue(i, argc, argv, option), option);
        } else if (option == "--seed") {
            config.random_seed = static_cast<unsigned int>(
                parseInt(readValue(i, argc, argv, option), option));
            seed_provided = true;
        } else {
            throw std::invalid_argument("未知选项：" + option);
        }
    }

    if (!seed_provided) {
        config.random_seed = static_cast<unsigned int>(config.student_no);
    }
    if (config.student_no < 0) {
        throw std::invalid_argument("student-no 必须为非负数");
    }
    if (config.readers < 0 || config.writers < 0 || config.rounds < 0) {
        throw std::invalid_argument("readers、writers 和 rounds 必须为非负数");
    }
    if (config.cache_size <= 0) {
        throw std::invalid_argument("cache-size 必须为正数");
    }
    if (config.delay_min_ms < 0 || config.delay_max_ms < config.delay_min_ms) {
        throw std::invalid_argument("延时范围必须满足 0 <= min <= max");
    }

    return config;
}

} // 匿名命名空间

int main(int argc, char** argv)
{
    try {
        // main 只负责构造配置；所有线程和同步逻辑都交给 Simulator
        Config config = parseArgs(argc, argv);
        Simulator simulator(config);
        return simulator.run();
    } catch (const std::exception& ex) {
        std::cerr << "library_cache_sync：" << ex.what() << '\n';
        return 1;
    }
}
