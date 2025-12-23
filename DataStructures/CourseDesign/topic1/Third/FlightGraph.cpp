#include "FlightGraph.h"
#include <queue>
#include <algorithm>
#include <iostream>
#include <climits>  // 添加这行

using namespace std;

// 添加航线到图中
void FlightGraph::addEdge(const string& from,
                         const string& to,
                         int price,
                         const string& flightId,
                         const string& departTime,
                         const string& arriveTime) {
    Edge edge;
    edge.to = to;
    edge.price = price;
    edge.flightId = flightId;
    edge.departTime = departTime;
    edge.arriveTime = arriveTime;
    
    adjList[from].push_back(edge);
}

// 使用类似 Dijkstra 的思想，按价格推荐路径
vector<string> FlightGraph::recommendBestPath(const string& from,
                                             const string& to) {
    if (adjList.empty()) {
        cout << "航班图为空，请先加载航班数据\n";
        return {};
    }

    // 使用优先队列实现 Dijkstra 算法
    priority_queue<pair<int, string>, 
                   vector<pair<int, string>>, 
                   greater<pair<int, string>>> pq;
    
    map<string, int> dist;      // 最短距离
    map<string, string> prev;   // 前驱节点
    map<string, bool> visited;  // 访问标记

    // 初始化
    for (const auto& pair : adjList) {
        dist[pair.first] = INT_MAX;
    }
    
    dist[from] = 0;
    pq.push({0, from});

    while (!pq.empty()) {
        string u = pq.top().second;
        pq.pop();

        if (visited[u]) continue;
        visited[u] = true;

        // 如果找到了目标城市，提前结束
        if (u == to) break;

        // 遍历所有邻接边
        if (adjList.find(u) != adjList.end()) {
            for (const auto& edge : adjList[u]) {
                string v = edge.to;
                int weight = edge.price;

                if (!visited[v] && dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
    }

    // 如果无法到达目标城市
    if (dist.find(to) == dist.end() || dist[to] == INT_MAX) {
        cout << "无法从 " << from << " 到达 " << to << endl;
        return {};
    }

    // 重构路径
    vector<string> path;
    for (string at = to; at != from; at = prev[at]) {
        path.push_back(at);
    }
    path.push_back(from);
    
    reverse(path.begin(), path.end());

    cout << "最低价格: " << dist[to] << " 元" << endl;
    return path;
}