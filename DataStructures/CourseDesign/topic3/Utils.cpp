#include "Utils.h"
#include <algorithm>
#include <cctype>
#include <fstream>
#include <cmath>
#include <sstream>
#include <locale>

using namespace std;

namespace Utils {
    string trim(const string& str) {
        size_t start = str.find_first_not_of(" \t\n\r");
        if (start == string::npos) return "";
        
        size_t end = str.find_last_not_of(" \t\n\r");
        return str.substr(start, end - start + 1);
    }
    
    vector<string> split(const string& str, char delimiter) {
        vector<string> tokens;
        stringstream ss(str);
        string token;
        
        while (getline(ss, token, delimiter)) {
            tokens.push_back(trim(token));
        }
        
        return tokens;
    }
    
    bool fileExists(const string& filename) {
        ifstream file(filename);
        return file.good();
    }
    
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
    
    double toRadians(double degrees) {
        return degrees * PI / 180.0;
    }
    
    double calculateDistance(double lat1, double lon1, double lat2, double lon2) {
        // 使用Haversine公式计算球面距离
        double dLat = toRadians(lat2 - lat1);
        double dLon = toRadians(lon2 - lon1);
        
        double a = sin(dLat/2) * sin(dLat/2) +
                   cos(toRadians(lat1)) * cos(toRadians(lat2)) *
                   sin(dLon/2) * sin(dLon/2);
        
        double c = 2 * atan2(sqrt(a), sqrt(1-a));
        double R = 6371.0; // 地球半径，单位：公里
        
        return R * c;
    }
    
    string formatDistance(double distance) {
        if (distance >= 1000) {
            return to_string(static_cast<int>(distance / 1000)) + "千公里";
        } else {
            return to_string(static_cast<int>(distance)) + "公里";
        }
    }
    
    string formatTime(double hours) {
        int h = static_cast<int>(hours);
        int m = static_cast<int>((hours - h) * 60);
        return to_string(h) + "小时" + to_string(m) + "分钟";
    }
    
    bool isValidCityName(const string& name) {
        // 简化验证：检查字符串是否非空
        if (name.empty()) return false;
        
        // 检查是否包含非法字符（可选）
        for (char c : name) {
            if (!isprint(c) && c != '\t' && c != '\n' && c != '\r') {
                return false;
            }
        }
        return true;
    }
    
    bool isProvincialCapital(const string& city) {
        // 省会城市列表
        vector<string> capitals = {
            "北京", "天津", "上海", "重庆", "哈尔滨", "长春", "沈阳",
            "呼和浩特", "石家庄", "太原", "西安", "济南", "郑州", "南京",
            "合肥", "杭州", "南昌", "福州", "长沙", "武汉", "广州", "南宁",
            "成都", "贵阳", "昆明", "兰州", "西宁", "银川", "乌鲁木齐"
        };
        
        return find(capitals.begin(), capitals.end(), city) != capitals.end();
    }
}