// 实现自定义 ls 命令的权限格式化、长格式输出和递归列目录。
#include "../include/myls.h"

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <string>
#include <vector>

#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

static std::string mode_string(mode_t m) {
    // 先根据文件类型设置首字符，再按用户/组/其他三个权限段填充 rwx。
    char buf[11];
    std::strcpy(buf, "----------");

    if (S_ISDIR(m)) {
        buf[0] = 'd';
    } else if (S_ISLNK(m)) {
        buf[0] = 'l';
    } else if (S_ISCHR(m)) {
        buf[0] = 'c';
    } else if (S_ISBLK(m)) {
        buf[0] = 'b';
    } else if (S_ISFIFO(m)) {
        buf[0] = 'p';
    } else if (S_ISSOCK(m)) {
        buf[0] = 's';
    }

    if (m & S_IRUSR) buf[1] = 'r';
    if (m & S_IWUSR) buf[2] = 'w';
    if (m & S_IXUSR) buf[3] = 'x';
    if (m & S_IRGRP) buf[4] = 'r';
    if (m & S_IWGRP) buf[5] = 'w';
    if (m & S_IXGRP) buf[6] = 'x';
    if (m & S_IROTH) buf[7] = 'r';
    if (m & S_IWOTH) buf[8] = 'w';
    if (m & S_IXOTH) buf[9] = 'x';

    if (m & S_ISUID) buf[3] = (buf[3] == 'x') ? 's' : 'S';
    if (m & S_ISGID) buf[6] = (buf[6] == 'x') ? 's' : 'S';
    if (m & S_ISVTX) buf[9] = (buf[9] == 'x') ? 't' : 'T';

    return std::string(buf);
}

static std::string join_path(const std::string& dir, const std::string& name) {
    // 拼接目录和文件名；父目录为空或已经以斜杠结尾时直接拼接。
    if (dir.empty() || dir.back() == '/') {
        return dir + name;
    }
    return dir + "/" + name;
}

static void print_long(const std::string& path,
                       const std::string& name,
                       const struct stat& st) {
    // 输出长格式：权限、链接数、UID、GID、大小、修改时间和文件名。
    char timebuf[64];
    struct tm tmv;
    localtime_r(&st.st_mtime, &tmv);
    std::strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", &tmv);

    std::printf("%s %3ld %5u %5u %8lld %s %s",
                mode_string(st.st_mode).c_str(),
                static_cast<long>(st.st_nlink),
                static_cast<unsigned>(st.st_uid),
                static_cast<unsigned>(st.st_gid),
                static_cast<long long>(st.st_size),
                timebuf,
                name.c_str());

    // 符号链接额外读取并输出目标路径。
    if (S_ISLNK(st.st_mode)) {
        char link[4096];
        ssize_t n = readlink(path.c_str(), link, sizeof(link) - 1);
        if (n >= 0) {
            link[n] = '\0';
            std::printf(" -> %s", link);
        }
    }
    std::printf("\n");
}

void list_dir(const std::string& path,
              const MyLsOptions& options,
              bool print_header) {
    // 打开目录，读取所有条目，排序后按选项输出。
    DIR* d = opendir(path.c_str());
    if (d == nullptr) {
        std::fprintf(stderr, "myls: %s: %s\n",
                     path.c_str(), std::strerror(errno));
        return;
    }

    std::vector<std::string> names;
    struct dirent* e;
    // 先一次性读取目录项，关闭目录后再进行输出和递归处理。
    while ((e = readdir(d)) != nullptr) {
        names.emplace_back(e->d_name);
    }
    closedir(d);
    std::sort(names.begin(), names.end());

    if (print_header) {
        // 列出多个目录时，在每个目录前打印目录标题。
        std::printf("%s:\n", path.c_str());
    }

    for (const auto& name : names) {
        if (name == "." || name == "..") {
            continue;
        }
        // 未指定 -a 时跳过以点号开头的隐藏文件。
        if (!options.show_all && name[0] == '.') {
            continue;
        }

        std::string full = join_path(path, name);
        struct stat st;
        // 使用 lstat 获取符号链接自身信息，不跟随链接。
        if (lstat(full.c_str(), &st) != 0) {
            std::fprintf(stderr, "myls: %s: %s\n",
                         full.c_str(), std::strerror(errno));
            continue;
        }

        if (options.long_format) {
            // -l：打印完整长格式信息。
            print_long(full, name, st);
        } else if (options.show_inode) {
            // -i：只打印 inode 号和文件名。
            std::printf("%8llu %s\n",
                        static_cast<unsigned long long>(st.st_ino),
                        name.c_str());
        } else {
            // 默认只打印文件名。
            std::printf("%s\n", name.c_str());
        }
    }

    if (options.recursive) {
        // 指定 -R 时递归列出子目录；只对普通目录递归。
        for (const auto& name : names) {
            if (name == "." || name == "..") {
                continue;
            }
            if (!options.show_all && name[0] == '.') {
                continue;
            }

            std::string full = join_path(path, name);
            struct stat st;
            if (lstat(full.c_str(), &st) != 0) {
                continue;
            }
            if (S_ISDIR(st.st_mode)) {
                std::printf("\n");
                list_dir(full, options, true);
            }
        }
    }
}
