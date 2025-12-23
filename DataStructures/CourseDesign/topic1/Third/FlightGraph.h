#ifndef FLIGHT_GRAPH_H
#define FLIGHT_GRAPH_H

#include <map>
#include <vector>
#include <string>

using std::map;
using std::vector;
using std::string;

// 带权边（价格作为权值）
struct Edge {
    string to;   // 目的城市
    int price;   // 航班价格
};

class FlightGraph {
public:
    // 添加航线
    void addEdge(const string& from,
                 const string& to,
                 int price);

    // 推荐最优中转路径（价格最小）
    vector<string> recommendBestPath(const string& from,
                                     const string& to);

private:
    map<string, vector<Edge>> adjList; // 邻接表
};

#endif
