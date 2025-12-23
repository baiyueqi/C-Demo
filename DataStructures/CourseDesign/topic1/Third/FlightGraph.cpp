#include "FlightGraph.h"
#include <queue>
#include <algorithm>

// 添加航线到图中
void FlightGraph::addEdge(const string& from,
                          const string& to,
                          int price) {
    adjList[from].push_back({to, price});
}

// 使用类似 Dijkstra 的思想，按价格推荐路径
vector<string> FlightGraph::recommendBestPath(const string& from,
                                              const string& to) {
    std::queue<string> q;
    map<string, int> cost;
    map<string, string> prev;

    q.push(from);
    cost[from] = 0;

    while (!q.empty()) {
        string cur = q.front();
        q.pop();

        for (auto& e : adjList[cur]) {
            int newCost = cost[cur] + e.price;
            if (!cost.count(e.to) || newCost < cost[e.to]) {
                cost[e.to] = newCost;
                prev[e.to] = cur;
                q.push(e.to);
            }
        }
    }

    vector<string> path;
    if (!prev.count(to)) return path;

    for (string at = to; at != from; at = prev[at])
        path.push_back(at);
    path.push_back(from);

    std::reverse(path.begin(), path.end());
    return path;
}
