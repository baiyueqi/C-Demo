// 实现按单词逆序处理文本的功能。
#include "../include/reverse.h"

#include <algorithm>
#include <sstream>
#include <string>
#include <vector>

std::string reverse_words(const std::string& line) {
    // 使用 stringstream 按空白切分单词。
    std::istringstream iss(line);
    std::vector<std::string> words;
    std::string word;
    while (iss >> word) {
        words.push_back(word);
    }

    // 反转 vector 中的单词顺序。
    std::reverse(words.begin(), words.end());

    // 用单个空格重新连接单词，恢复一行文本。
    std::string out;
    for (size_t i = 0; i < words.size(); ++i) {
        if (i != 0) {
            out += ' ';
        }
        out += words[i];
    }
    return out;
}
