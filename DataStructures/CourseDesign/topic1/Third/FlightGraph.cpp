//中转推荐实现（图 + BFS）
#include "FlightGraph.h"
#include<iostream>
#include <algorithm>   
#include <queue>

// 添加航线
void FlightGraph::addEdge(const string& from, const string& to) {
    adjList[from].push_back(to);
}

// 查找中转路径（BFS）
vector<string> FlightGraph::findTransferPath(const string& from, const string& to) {
    queue<string> q;
    map<string, string> prev;
    q.push(from);
    prev[from] = "";

    while (!q.empty()) {
        string cur = q.front(); q.pop();
        if (cur == to) break;

        for (auto& next : adjList[cur]) {
            if (!prev.count(next)) {
                prev[next] = cur;
                q.push(next);
            }
        }
    }

    vector<string> path;
    if (!prev.count(to)) return path;

    for (string at = to; at != ""; at = prev[at])
        path.push_back(at);

    reverse(path.begin(), path.end());
    return path;
}
