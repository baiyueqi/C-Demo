// 实现文件系统审计扫描、统计和遍历逻辑。
#include "../include/fs_auditor.h"

#include "../include/thread_pool.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <memory>

#include <sys/stat.h>
#include <unistd.h>

static std::string join_path(const std::string& dir, const std::string& name) {
    // 拼接父目录和子项名称；父目录为空或已经以斜杠结尾时直接拼接。
    if (dir.empty() || dir.back() == '/') {
        return dir + name;
    }
    return dir + "/" + name;
}

static std::string base_name(const std::string& path) {
    // 提取路径最后一个斜杠之后的内容作为文件名。
    std::string::size_type pos = path.find_last_of('/');
    if (pos == std::string::npos) {
        return path;
    }
    return path.substr(pos + 1);
}

// 线程安全地向统计结果中添加一条错误信息。
static void add_error(AuditStats& stats, std::mutex& stats_mutex,
                      const std::string& message) {
    // 所有错误信息都通过同一把锁保护，供多线程扫描时安全写入。
    std::lock_guard<std::mutex> lock(stats_mutex);
    stats.errors.push_back(message);
}

void DirCloser::operator()(DIR* d) const {
    // 供 unique_ptr 使用，在目录句柄离开作用域时自动关闭。
    if (d != nullptr) {
        closedir(d);
    }
}

// 根据文件类型和权限位生成类似 ls -l 的 10 位权限字符串。
static std::string mode_string(mode_t mode) {
    // 先根据文件类型设置首字符，再按用户/组/其他三个权限段填充 rwx。
    char buf[11];
    std::strcpy(buf, "----------");

    if (S_ISDIR(mode)) {
        buf[0] = 'd';
    } else if (S_ISLNK(mode)) {
        buf[0] = 'l';
    } else if (S_ISCHR(mode)) {
        buf[0] = 'c';
    } else if (S_ISBLK(mode)) {
        buf[0] = 'b';
    } else if (S_ISFIFO(mode)) {
        buf[0] = 'p';
    } else if (S_ISSOCK(mode)) {
        buf[0] = 's';
    }

    if (mode & S_IRUSR) buf[1] = 'r';
    if (mode & S_IWUSR) buf[2] = 'w';
    if (mode & S_IXUSR) buf[3] = 'x';
    if (mode & S_IRGRP) buf[4] = 'r';
    if (mode & S_IWGRP) buf[5] = 'w';
    if (mode & S_IXGRP) buf[6] = 'x';
    if (mode & S_IROTH) buf[7] = 'r';
    if (mode & S_IWOTH) buf[8] = 'w';
    if (mode & S_IXOTH) buf[9] = 'x';

    if (mode & S_ISUID) buf[3] = (buf[3] == 'x') ? 's' : 'S';
    if (mode & S_ISGID) buf[6] = (buf[6] == 'x') ? 's' : 'S';
    if (mode & S_ISVTX) buf[9] = (buf[9] == 'x') ? 't' : 'T';

    return std::string(buf);
}

// 判断文件名是否符合已经去掉前导点的扩展名过滤规则。
static bool has_extension(const std::string& filename,
                          const std::string& normalized_extension) {
    if (normalized_extension.empty()) {
        return true;
    }

    // 只比较最后一个点号后面的扩展名。
    std::string::size_type dot = filename.find_last_of('.');
    if (dot == std::string::npos) {
        return false;
    }
    return filename.compare(dot + 1, std::string::npos,
                            normalized_extension) == 0;
}

void process_entry(const std::string& path,
                   AuditStats& stats,
                   std::set<std::pair<dev_t, ino_t>>& seen_inodes,
                   std::mutex& stats_mutex,
                   ThreadPool& pool,
                   const std::string& normalized_extension,
                   std::time_t now) {
    // 每个路径都会先 lstat，再按类型分派到符号链接、目录或普通文件处理。
    struct stat st;
    // 使用 lstat 获取文件自身信息，不跟随符号链接。
    if (lstat(path.c_str(), &st) != 0) {
        add_error(stats, stats_mutex, path + ": " + std::strerror(errno));
        std::fprintf(stderr, "fs_auditor: warning: %s: %s\n",
                     path.c_str(), std::strerror(errno));
        return;
    }

    // 记录每个扫描对象的权限字符串，方便报告显示。
    {
        std::lock_guard<std::mutex> lock(stats_mutex);
        stats.permission_lines.push_back(mode_string(st.st_mode) + " " + path);
    }

    if (S_ISLNK(st.st_mode)) {
        // 符号链接只统计链接本身，不跟随目标。
        std::lock_guard<std::mutex> lock(stats_mutex);
        stats.symlinks++;
        return;
    }

    if (S_ISDIR(st.st_mode)) {
        // 目录统计只需要记录一次，随后读取并提交其子项。
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            stats.directories++;
        }

        std::unique_ptr<DIR, DirCloser> d(opendir(path.c_str()));
        if (!d) {
            add_error(stats, stats_mutex,
                      path + ": " + std::strerror(errno));
            std::fprintf(stderr,
                         "fs_auditor: warning: cannot open directory %s: %s\n",
                         path.c_str(), std::strerror(errno));
            return;
        }

        std::vector<std::string> names;
        struct dirent* entry;
        errno = 0;
        // 先收集目录项，避免在递归扫描期间继续持有目录流。
        while ((entry = readdir(d.get())) != nullptr) {
            std::string name = entry->d_name;
            // 跳过 . 和 ..，避免无限递归。
            if (name == "." || name == "..") {
                continue;
            }
            names.push_back(name);
        }
        int read_errno = errno;

        if (read_errno != 0) {
            // readdir 返回 nullptr 后，需要检查 errno 区分正常结束和读取错误。
            add_error(stats, stats_mutex,
                      path + ": " + std::strerror(read_errno));
            std::fprintf(stderr,
                         "fs_auditor: warning: cannot read directory %s: %s\n",
                         path.c_str(), std::strerror(read_errno));
        }

        // 排序后提交子项扫描任务，保证任务生成顺序稳定。
        std::sort(names.begin(), names.end());
        for (const auto& name : names) {
            std::string full = join_path(path, name);
            pool.submit([&stats, &seen_inodes, &stats_mutex, &pool,
                         &normalized_extension, now, full] {
                process_entry(full, stats, seen_inodes, stats_mutex, pool,
                              normalized_extension, now);
            });
        }
        return;
    }

    if (S_ISREG(st.st_mode)) {
        // 普通文件按扩展名过滤后再参与统计。
        if (!has_extension(base_name(path), normalized_extension)) {
            return;
        }

        long long size = static_cast<long long>(st.st_size);
        // 读权限检查和执行位判断可以先在线程局部完成，减少持锁时间。
        bool unreadable = (access(path.c_str(), R_OK) != 0);
        bool executable =
            (st.st_mode & (S_IXUSR | S_IXGRP | S_IXOTH)) != 0;
        double age_seconds = std::difftime(now, st.st_mtime);
        bool recent =
            age_seconds >= 0.0 && age_seconds <= 7.0 * 24.0 * 60.0 * 60.0;

        // 设备号 + inode 号组成唯一键，用于识别硬链接。
        auto key = std::make_pair(st.st_dev, st.st_ino);
        {
            std::lock_guard<std::mutex> lock(stats_mutex);
            // 使用设备号 + inode 号去重，同一个文件的多个硬链接只统计一次。
            if (seen_inodes.count(key) != 0) {
                return;
            }
            seen_inodes.insert(key);

            stats.regular_files++;
            stats.total_size += static_cast<unsigned long long>(size);
            if (st.st_nlink > 1) {
                stats.hard_links++;
            }
            if (unreadable) {
                stats.unreadable_files++;
            }
            if (executable) {
                stats.executable_files++;
            }
            if (recent) {
                stats.recent_files++;
            }

            if (!stats.has_largest || size > stats.largest_size) {
                stats.has_largest = true;
                stats.largest_size = size;
                stats.largest_path = path;
            }
            if (!stats.has_smallest || size < stats.smallest_size) {
                stats.has_smallest = true;
                stats.smallest_size = size;
                stats.smallest_path = path;
            }
        }
        return;
    }

    // 其他类型暂不计入常规统计，但仍保留权限字符串。
}
