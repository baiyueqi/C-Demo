// reverse 程序的入口，负责解析参数、读取输入并控制逆序处理流程。
#include "../include/reverse.h"

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char** argv) {
    // 分别记录是否按行逆序、是否按词逆序，以及待处理的文件路径。
    bool reverse_lines = false;
    bool reverse_words_mode = false;
    std::string file;

    // 解析 -l、-w 以及合并形式 -lw/-wl。
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "-l") {
            reverse_lines = true;
        } else if (arg == "-w") {
            reverse_words_mode = true;
        } else if (arg == "-lw" || arg == "-wl") {
            reverse_lines = true;
            reverse_words_mode = true;
        } else if (!arg.empty() && arg[0] == '-') {
            std::fprintf(stderr, "reverse: unknown option '%s'\n", arg.c_str());
            return 1;
        } else {
            file = arg;
        }
    }

    if (!reverse_lines && !reverse_words_mode) {
        std::fprintf(stderr, "usage: reverse [-l] [-w] <file>\n");
        return 1;
    }

    // 指定文件时打开文件流；否则使用标准输入。
    std::ifstream in;
    if (!file.empty()) {
        in.open(file);
        if (!in) {
            std::fprintf(stderr, "reverse: cannot open '%s'\n", file.c_str());
            return 1;
        }
    }

    // 没有给出文件时从标准输入读取。
    std::istream& input = file.empty() ? static_cast<std::istream&>(std::cin)
                                       : static_cast<std::istream&>(in);

    // 从文件或标准输入逐行读取内容。
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(line);
    }

    // 若指定 -w，先反转每行内的单词顺序。
    if (reverse_words_mode) {
        for (auto& l : lines) {
            l = reverse_words(l);
        }
    }

    // 若指定 -l，再反转整行顺序。
    if (reverse_lines) {
        std::reverse(lines.begin(), lines.end());
    }

    // 输出处理结果。
    for (const auto& l : lines) {
        std::cout << l << '\n';
    }
    return 0;
}
