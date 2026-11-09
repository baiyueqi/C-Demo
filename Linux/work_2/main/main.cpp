// fs_auditor 的程序入口，负责参数解析、顶层检查、调度扫描和生成报告。
#include "../include/fs_auditor.h"
#include "../include/report.h"
#include "../include/thread_pool.h"

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <thread>

int main(int argc, char** argv) {
    // 保存两个位置参数：扫描目录和报告文件，以及可选的扩展名过滤条件。
    std::string target;
    std::string report;
    std::string extension_filter;

    // 默认线程数取硬件并发数，并限制在 32 以内，避免线程过多。
    unsigned int hw = std::thread::hardware_concurrency();
    unsigned int thread_count = (hw == 0 ? 1u : hw);
    if (thread_count > 32) {
        thread_count = 32;
    }

    // 逐项解析命令行参数，支持 -e、-j、-h 和位置参数。
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-e") {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
                return 1;
            }
            extension_filter = argv[++i];
        } else if (arg == "-j") {
            if (i + 1 >= argc) {
                print_usage(argv[0]);
                return 1;
            }
            char* end = nullptr;
            unsigned long n = std::strtoul(argv[i + 1], &end, 10);
            if (end == argv[i + 1] || *end != '\0' || n == 0 || n > 256) {
                std::fprintf(stderr,
                             "fs_auditor: invalid thread count '%s'\n",
                             argv[i + 1]);
                return 1;
            }
            thread_count = static_cast<unsigned int>(n);
            ++i;
        } else if (arg == "-h" || arg == "--help") {
            print_usage(argv[0]);
            return 0;
        } else if (arg.size() > 1 && arg[0] == '-') {
            std::fprintf(stderr, "fs_auditor: unknown option '%s'\n",
                         arg.c_str());
            print_usage(argv[0]);
            return 1;
        } else if (target.empty()) {
            target = arg;
        } else if (report.empty()) {
            report = arg;
        } else {
            print_usage(argv[0]);
            return 1;
        }
    }

    if (target.empty() || report.empty()) {
        print_usage(argv[0]);
        return 1;
    }

    // 扫描前统一去掉扩展名前导点，避免每个文件都重复处理。
    std::string normalized_extension = extension_filter;
    if (!normalized_extension.empty() &&
        normalized_extension.front() == '.') {
        normalized_extension.erase(0, 1);
    }

    // 顶层目录无法访问时，按题意返回退出码 2。
    std::unique_ptr<DIR, DirCloser> root(opendir(target.c_str()));
    if (!root) {
        std::fprintf(stderr, "fs_auditor: cannot access directory '%s': %s\n",
                     target.c_str(), std::strerror(errno));
        return 2;
    }

    // 创建统计对象、inode 去重集合和共享互斥锁。
    AuditStats stats;
    std::set<std::pair<dev_t, ino_t>> seen_inodes;
    std::mutex stats_mutex;
    // 线程池生命周期覆盖整个扫描过程，pool.wait() 结束后结果才完整。
    ThreadPool pool(thread_count);
    std::time_t now = std::time(nullptr);

    // 从顶层目录开始扫描，子目录由线程池并行处理。
    process_entry(target, stats, seen_inodes, stats_mutex, pool,
                  normalized_extension, now);
    pool.wait();

    // 所有扫描任务完成后，一次性生成审计报告。
    write_report(report, target, extension_filter, stats);
    return 0;
}
