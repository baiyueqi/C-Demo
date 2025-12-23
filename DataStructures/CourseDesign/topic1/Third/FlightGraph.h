// FlightGraph.h
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
    string to;         // 目的城市
    int price;         // 航班价格
    string flightId;   // 航班号
    string departTime; // 出发时间
    string arriveTime; // 到达时间
};

class FlightGraph {
public:
    // 添加航线 - 更新为完整版本
    void addEdge(const string& from,
                 const string& to,
                 int price,
                 const string& flightId = "",
                 const string& departTime = "",
                 const string& arriveTime = "");

    // 推荐最优中转路径（价格最小）
    vector<string> recommendBestPath(const string& from,
                                     const string& to);

private:
    map<string, vector<Edge>> adjList; // 邻接表
};

#endif