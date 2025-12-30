#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <cmath>
#include <sstream>
#include <locale>

using namespace std;

namespace Utils {
    /**
     * @brief 去除字符串首尾空白字符
     * @param str 输入字符串
     * @return 修剪后的字符串
     */
    string trim(const string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == string::npos) return "";
        
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }
    
    /**
     * @brief 按分隔符分割字符串
     * @param str 输入字符串
     * @param delimiter 分隔符
     * @return 分割后的字符串向量
     */
    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        
        while (getline(ss, token, delimiter)) {
            tokens.push_back(trim(token));
        }
        
        return tokens;
    }
    
    /**
     * @brief 检查文件是否存在
     * @param filename 文件名
     * @return 存在返回true，否则false
     */
    bool fileExists(const string& filename) {
        ifstream file(filename);
        return file.good();
    }
    
    /**
     * @brief 读取文件所有行
     * @param filename 文件名
     * @return 行字符串向量
     */
    vector<string> readLines(const string& filename) {
        vector<string> lines;
        ifstream file(filename);
        string line;
        
        if (file.is_open()) {
            while (getline(file, line)) {
                lines.push_back(line);
            }
            file.close();
        }
        
        return lines;
    }
    
    /**
     * @brief 角度转弧度
     * @param degrees 角度
     * @return 弧度
     */
    double toRadians(double degrees) {
        return degrees * PI / 180.0;
    }
    
    /**
     * @brief 使用Haversine公式计算球面距离
     * @param lat1 起点纬度
     * @param lon1 起点经度
     * @param lat2 终点纬度
     * @param lon2 终点经度
     * @return 距离（公里）
     */
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        double dLat = toRadians(lat2 - lat1);
        double dLon = toRadians(lon2 - lon1);
        
        double a = sin(dLat/2) * sin(dLat/2) +
                   cos(toRadians(lat1)) * cos(toRadians(lat2)) *
                   sin(dLon/2) * sin(dLon/2);
        
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        double R = 6371.0; // 地球半径（公里）
        
        return R * c;
    }
    
    /**
     * @brief 格式化距离输出
     * @param distance 距离（公里）
     * @return 格式化字符串
     */
    string formatDistance(double distance) {
        if (distance >= 1000) {
            return to_string(static_cast<int>(distance / 1000)) + "千公里";
        } else {
            return to_string(static_cast<int>(distance)) + "公里";
        }
    }
    
    /**
     * @brief 格式化时间输出
     * @param hours 小时数
     * @return 格式化字符串
     */
    string formatTime(double hours) {
        int h = static_cast<int>(hours);
        int m = static_cast<int>((hours - h) * 60);
        return to_string(h) + "小时" + to_string(m) + "分钟";
    }
    
    /**
     * @brief 验证城市名是否有效
     * @param name 城市名
     * @return 有效返回true，否则false
     */
    bool isValidCityName(const string& name) {
        if (name.empty()) return false;
        
        for (char c : name) {
            if (!isprint(c) && c != '\t' && c != '\n' && c != '\r') {
                return false;
            }
        }
        return true;
    }
    
    /**
     * @brief 判断城市是否为省会
     * @param city 城市名
     * @return 是省会返回true，否则false
     */
    bool isProvincialCapital(const string& city) {
        vector<string> capitals = {
            "北京", "天津", "上海", "重庆", "哈尔滨", "长春", "沈阳",
            "呼和浩特", "石家庄", "太原", "西安", "济南", "郑州", "南京",
            "合肥", "杭州", "南昌", "福州", "长沙", "武汉", "广州", "南宁",
            "成都", "贵阳", "昆明", "兰州", "西宁", "银川", "乌鲁木齐"
        };
        
        return find(capitals.begin(), capitals.end(), city) != capitals.end();
    }
}   