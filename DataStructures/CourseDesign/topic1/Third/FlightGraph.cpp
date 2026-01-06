#include "FlightGraph.h"
#include <queue>
#include <algorithm>
#include <iostream>
#include <climits>  // 添加这行

using namespace std;

// 添加航线到图中,功能：在图中加入一条航线（边）
void FlightGraph::addEdge(const string& from,
                         const string& to,
                         int price,
                         const string& flightId,
                         const string& departTime,
                         const string& arriveTime) {
    Edge edge;// 创建边对象
    edge.to = to;//终点城市名称
    edge.price = price;//航班票价
    edge.flightId = flightId;//航班号
    edge.departTime = departTime;//起飞时间
    edge.arriveTime = arriveTime;//到达时间
    
    adjList[from].push_back(edge);
}

// 使用类似 Dijkstra 的思想，按价格推荐路径
vector<string> FlightGraph::recommendBestPath(const string& from,
                                             const string& to) {
    if (adjList.empty()) {
        cout << "航班图为空，请先加载航班数据\n";
        return {};
    }

    // 使用优先队列实现 Dijkstra 算法,建立最小优先队列（Min Heap），用来每次选择当前价格最小的城市
    priority_queue<pair<int, string>,//价格，城市名称 
                   vector<pair<int, string>>, 
                   greater<pair<int, string>>> pq;//表示“越小越优先” → 最小堆
    
    map<string, int> dist;      // 最短距离,dist存储从起点到每个城市的最低累计票价
    map<string, string> prev;   // 前驱节点，prev存储路径中每个城市的前一个城市，用来回溯最终路线
    map<string, bool> visited;  // 访问标记，visited记录城市是否已经处理过，避免重复处理

    // 初始化(起点还没处理，所以先设为无穷大)
    for (const auto& pair : adjList) {
        dist[pair.first] = INT_MAX;//遍历所有城市，把价格初始化为 INT_MAX（表示“无限大”）
    }
    
    dist[from] = 0;//起点价格为0
    pq.push({0, from});//将起点加入优先队列

    //Dijkstra 核心循环
    while (!pq.empty()) {
        //每次从队列里取累计价格最小的城市 u
        string u = pq.top().second;//pq.top().second 是城市名称，pq.top().first 是累计票价
        pq.pop();

        if (visited[u]) continue;//已经处理过的城市跳过
        visited[u] = true;

        // 如果找到了目标城市，提前结束(如果当前城市就是终点，就可以提前结束循环，提高效率)
        if (u == to) break;

        // 遍历所有邻接边(遍历 u 出发的所有航班,v 是邻接城市,weight 是价格)
        if (adjList.find(u) != adjList.end()) {
            for (const auto& edge : adjList[u]) {
                string v = edge.to;//终点城市
                int weight = edge.price;//航班价格

                if (!visited[v] && dist[u] + weight < dist[v]) {
                    dist[v] = dist[u] + weight;//更新最低价格
                    prev[v] = u;//更新前驱节点
                    pq.push({dist[v], v});//将更新后的城市加入队列
                }
            }
        }
    }

    // 如果无法到达目标城市
    if (dist.find(to) == dist.end() || dist[to] == INT_MAX) {
        cout << "无法从 " << from << " 到达 " << to << endl;
        return {};
    }

    // 重构路径(回溯路径)
    vector<string> path;//存储最终路径
    //从目标城市 to 开始，沿 prev 回溯到起点 from(将路径反转，得到从起点到终点的顺序)
    for (string at = to; at != from; at = prev[at]) {
        path.push_back(at);
    }
    path.push_back(from);
    
    reverse(path.begin(), path.end());//反转路径

    cout << "最低价格: " << dist[to] << " 元" << endl;//输出最低价格
    return path;//返回路径
}