#ifndef CITY_H
#define CITY_H

#include <string>
#include <vector>
#include <utility>

using namespace std;

/**
 * @brief 城市结构体，存储城市基本信息
 */
struct City {
    string name;           // 城市名称
    string province;       // 省份
    double latitude;       // 纬度
    double longitude;      // 经度
    vector<pair<int, double>> neighbors; // 邻接表：<城市索引, 距离>
};

/**
 * @brief 路径结构体，用于存储路径信息
 */
struct Path {
    vector<int> nodes;     // 路径经过的城市索引
    double totalDistance;  // 总距离
    int nodeCount;         // 节点数量
    
    // 用于优先队列的比较运算符（最小堆）
    bool operator>(const Path& other) const {
        return totalDistance > other.totalDistance;
    }
};

#endif