#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace Utils {
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delim);
}

#endif