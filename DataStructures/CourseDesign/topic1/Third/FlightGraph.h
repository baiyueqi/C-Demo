//航班图（中转推荐用）
#ifndef FLIGHT_GRAPH_H
#define FLIGHT_GRAPH_H

#include <map>
#include <vector>
#include <string>

using namespace std;

// 航班图：城市 -> 可达城市
class FlightGraph {
public:
    void addEdge(const string& from, const string& to);
    vector<string> findTransferPath(const string& from, const string& to);

private:
    map<string, vector<string>> adjList; // 邻接表
};

#endif
