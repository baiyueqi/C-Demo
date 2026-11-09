// 自定义 ls 命令的入口，负责解析命令行选项并调用目录列表逻辑。
#include "../include/myls.h"

#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    // 使用结构体集中保存四个选项，避免多个全局布尔变量。
    MyLsOptions options;
    std::vector<std::string> paths;

    // 解析命令行参数；选项可以合并，例如 -la。
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg.size() > 1 && arg[0] == '-') {
            for (std::string::size_type j = 1; j < arg.size(); ++j) {
                switch (arg[j]) {
                    case 'a':
                        options.show_all = true;
                        break;
                    case 'l':
                        options.long_format = true;
                        break;
                    case 'R':
                        options.recursive = true;
                        break;
                    case 'i':
                        options.show_inode = true;
                        break;
                    default:
                        std::fprintf(stderr,
                                     "myls: unknown option '-%c'\n", arg[j]);
                        return 1;
                }
            }
        } else {
            // 非选项参数作为要列出的目录或文件路径。
            paths.push_back(arg);
        }
    }

    if (paths.empty()) {
        // 未指定路径时默认列出当前目录。
        paths.emplace_back(".");
    }

    // 指定多个路径时打印目录标题，方便区分输出。
    bool header = paths.size() > 1;
    for (const auto& path : paths) {
        list_dir(path, options, header);
    }

    return 0;
}
