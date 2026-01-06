#pragma once
#include <string>
#include <vector>

class Resp {
public:
    static std::vector<std::string> parse(const std::string& input);
    static std::string simple(const std::string& s);
    static std::string bulk(const std::string& s);
    static std::string nullBulk();
};
