//工具函数声明
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

namespace Utils {
    const double PI = 3.14159265358979323846;
    
    // 字符串处理
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    
    // 文件操作
    bool fileExists(const std::string& filename);
    std::vector<std::string> readLines(const std::string& filename);
    
    // 数学计算
    double calculateDistance(double lat1, double lon1, double lat2, double lon2);
    double toRadians(double degrees);
    
    // 格式化输出
    std::string formatDistance(double distance);
    std::string formatTime(double hours);
    
    // 验证函数
    bool isValidCityName(const std::string& name);
    bool isProvincialCapital(const std::string& city);
};

#endif // UTILS_H