// 生成文件系统审计报告，并输出程序用法。
#include "../include/report.h"

#include "../include/fs_auditor.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

void write_report(const std::string& report_path,
                  const std::string& target,
                  const std::string& extension_filter,
                  const AuditStats& stats) {
    // 以只写方式创建报告文件；创建失败按题目要求返回退出码 3。
    FILE* f = std::fopen(report_path.c_str(), "w");
    if (f == nullptr) {
        std::fprintf(stderr, "fs_auditor: cannot create report '%s': %s\n",
                     report_path.c_str(), std::strerror(errno));
        std::exit(3);
    }

    // 写入报告头、目标目录和扩展名过滤信息。
    std::fprintf(f, "=== File System Audit Report ===\n");
    std::fprintf(f, "Target directory: %s\n", target.c_str());
    std::fprintf(f, "Extension filter: %s\n\n",
                 extension_filter.empty() ? "(none)" : extension_filter.c_str());

    // 输出普通文件、目录、链接、权限、最近修改和总大小等统计信息。
    std::fprintf(f, "Statistics:\n");
    std::fprintf(f, "  %-17s %10lld\n", "Regular files:", stats.regular_files);
    std::fprintf(f, "  %-17s %10lld\n", "Directories:", stats.directories);
    std::fprintf(f, "  %-17s %10lld\n", "Symbolic links:", stats.symlinks);
    std::fprintf(f, "  %-17s %10lld\n", "Hard links:", stats.hard_links);
    std::fprintf(f, "  %-17s %10lld\n", "Unreadable files:", stats.unreadable_files);
    std::fprintf(f, "  %-17s %10lld\n", "Executable files:", stats.executable_files);
    std::fprintf(f, "  %-17s %10lld\n", "Recent files:", stats.recent_files);
    std::fprintf(f, "  %-17s %10llu bytes\n", "Total size:", stats.total_size);

    // 输出扫描到的最大和最小普通文件。
    std::fprintf(f, "\n");
    if (stats.has_largest) {
        std::fprintf(f, "Largest file:  %s (%lld bytes)\n",
                     stats.largest_path.c_str(), stats.largest_size);
    } else {
        std::fprintf(f, "Largest file:  (none)\n");
    }
    if (stats.has_smallest) {
        std::fprintf(f, "Smallest file: %s (%lld bytes)\n",
                     stats.smallest_path.c_str(), stats.smallest_size);
    } else {
        std::fprintf(f, "Smallest file: (none)\n");
    }

    // 输出每个扫描对象的权限字符串；排序后输出可保证报告顺序稳定。
    std::fprintf(f, "\nFiles (permission strings):\n");
    if (stats.permission_lines.empty()) {
        std::fprintf(f, "  (none)\n");
    } else {
        std::vector<std::string> permissions = stats.permission_lines;
        std::sort(permissions.begin(), permissions.end());
        for (const auto& line : permissions) {
            std::fprintf(f, "  %s\n", line.c_str());
        }
    }

    // 输出扫描过程中记录的错误，包括无法访问或无法读取的路径。
    std::fprintf(f, "\nErrors:\n");
    if (stats.errors.empty()) {
        std::fprintf(f, "  (none)\n");
    } else {
        for (const auto& err : stats.errors) {
            std::fprintf(f, "  %s\n", err.c_str());
        }
    }

    if (std::fclose(f) != 0) {
        std::fprintf(stderr, "fs_auditor: failed to write report '%s': %s\n",
                     report_path.c_str(), std::strerror(errno));
        std::exit(3);
    }
}

void print_usage(const char* prog) {
    // 输出命令行用法到标准错误，帮助用户了解所有参数和选项。
    std::fprintf(stderr,
                 "usage: %s [options] <directory> <report-file>\n"
                 "options:\n"
                 "  -e EXT   only count regular files with extension EXT\n"
                 "  -j N     use N worker threads for directory scanning\n"
                 "  -h       show this help\n",
                 prog);
}
