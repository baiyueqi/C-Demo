#include "resp.h"
#include <vector>
#include <string>
#include <cstring>

std::vector<std::string> Resp::parse(const std::string& input) {
    std::vector<std::string> result;

    size_t i = 0;
    size_t n = input.size();

    // 必须以 * 开头（RESP Array）
    if (n == 0 || input[i] != '*') return result;

    // 跳过 "*<num>\r\n"
    i = input.find("\r\n", i);
    if (i == std::string::npos) return result;
    i += 2;

    while (i < n) {
        if (input[i] != '$') break;

        // 跳过 "$<len>\r\n"
        i = input.find("\r\n", i);
        if (i == std::string::npos) break;
        i += 2;

        // 读取真正的数据
        size_t end = input.find("\r\n", i);
        if (end == std::string::npos) break;

        result.push_back(input.substr(i, end - i));
        i = end + 2;
    }

    return result;
}

std::string Resp::simple(const std::string& s) {
    return "+" + s + "\r\n";
}

std::string Resp::bulk(const std::string& s) {
    return "$" + std::to_string(s.size()) + "\r\n" + s + "\r\n";
}

std::string Resp::nullBulk() {
    return "$-1\r\n";
}
