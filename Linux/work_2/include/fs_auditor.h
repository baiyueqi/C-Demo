// 声明文件系统审计所需的数据结构、扫描接口和目录 RAII 清理器。
#ifndef FS_AUDITOR_H
#define FS_AUDITOR_H

#include <ctime>
#include <dirent.h>
#include <mutex>
#include <set>
#include <string>
#include <sys/types.h>
#include <utility>
#include <vector>

// 前向声明线程池类型，避免在本头文件中引入线程池实现。
class ThreadPool;

// 用于 std::unique_ptr 自动关闭目录句柄。
struct DirCloser {
    void operator()(DIR* d) const;
};

// 汇总文件系统审计结果，保存各类统计数据和最大/最小文件信息。
struct AuditStats {
    long long regular_files = 0;    // 普通文件数量
    long long directories = 0;      // 目录数量
    long long symlinks = 0;         // 符号链接数量
    long long hard_links = 0;       // 硬链接文件数量
    long long unreadable_files = 0; // 当前进程不可读的普通文件数量
    long long executable_files = 0; // 可执行普通文件数量
    long long recent_files = 0;     // 最近 7 天修改过的普通文件数量
    // 普通文件总大小，单位：字节。
    unsigned long long total_size = 0;

    // 最大/最小普通文件信息，尚未遇到普通文件时 has_* 为 false。
    bool has_largest = false;
    long long largest_size = 0;
    std::string largest_path;

    bool has_smallest = false;
    long long smallest_size = 0;
    std::string smallest_path;

    // 每个扫描对象的权限字符串和路径，例如 "drwxr-xr-x /tmp/demo"。
    std::vector<std::string> permission_lines;

    // 记录遍历过程中遇到的错误，供报告集中输出。
    std::vector<std::string> errors;
};

/*
 * 扫描指定路径，递归处理普通文件、目录和符号链接。
 *
 * path:                 当前待扫描路径
 * stats:                汇总统计结果
 * seen_inodes:          用于硬链接去重的设备号 + inode 集合
 * stats_mutex:          保护 stats 和 seen_inodes 的互斥锁
 * pool:                 并行处理子目录的线程池
 * normalized_extension: 去掉前导点后的扩展名过滤条件
 * now:                  扫描开始时间，用于最近 7 天判断
 */
void process_entry(const std::string& path,
                   AuditStats& stats,
                   std::set<std::pair<dev_t, ino_t>>& seen_inodes,
                   std::mutex& stats_mutex,
                   ThreadPool& pool,
                   const std::string& normalized_extension,
                   std::time_t now);

#endif
