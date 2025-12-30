//城市结构体定义
#ifndef CITY_H
#define CITY_H

#include <string>
#include <vector>
#include <utility>

// 城市结构体
struct City {
    std::string name;           // 城市名称
    std::string province;       // 省份
    double latitude;           // 纬度
    double longitude;          // 经度
    std::vector<std::pair<int, double>> neighbors; // 邻接表：城市索引, 距离
};

// 路径结构体
struct Path {
    std::vector<int> nodes;     // 路径经过的城市索引
    double totalDistance;      // 总距离
    int nodeCount;            // 节点数量
    
    // 用于优先队列的比较运算符
    bool operator>(const Path& other) const {
        return totalDistance > other.totalDistance;
    }
};

#endif // CITY_H