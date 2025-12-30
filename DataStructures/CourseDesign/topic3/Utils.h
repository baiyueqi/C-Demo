#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

using namespace std;

namespace Utils {
    const double PI = 3.14159265358979323846;
    
    // 字符串处理
    string trim(const string& str);                              // 去除空白字符
    vector<string> split(const string& str, char delimiter);     // 分割字符串
    
    // 文件操作
    bool fileExists(const string& filename);                     // 检查文件是否存在
    vector<string> readLines(const string& filename);            // 读取文件所有行
    
    // 数学计算
    double calculateDistance(double lat1, double lon1,           // 计算球面距离
                           double lat2, double lon2);
    double toRadians(double degrees);                            // 角度转弧度
    
    // 格式化输出
    string formatDistance(double distance);                      // 格式化距离
    string formatTime(double hours);                             // 格式化时间
    
    // 验证函数
    bool isValidCityName(const string& name);                    // 验证城市名
    bool isProvincialCapital(const string& city);                // 判断是否为省会
};

#endif