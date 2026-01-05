#include "Graph.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stack>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;

Graph::Graph() : floydComputed(false) {}

bool Graph::loadFromCSV(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "无法打开文件: " << filename << endl;
        return false;
    }
    
    string line;
    // 读取表头
    if (getline(file, line)) {
        stringstream ss(line);
        string cityName;
        getline(ss, cityName, ',');
        
        while (getline(ss, cityName, ',')) {
            City city;
            city.name = cityName;
            cities.push_back(city);
            cityIndexMap[cityName] = cities.size() - 1;
        }
    }
    
    // 初始化邻接矩阵
    int n = cities.size();
    adjMatrix.resize(n, vector<double>(n, INF));
    next.resize(n, vector<int>(n, -1));
    
    for (int i = 0; i < n; i++) {
        adjMatrix[i][i] = 0;
        next[i][i] = i;
    }
    
    // 读取距离数据
    int row = 0;
    while (getline(file, line) && row < n) {
        stringstream ss(line);
        string cell;
        getline(ss, cell, ',');
        
        int col = 0;
        while (getline(ss, cell, ',') && col < n) {
            if (!cell.empty()) {
                try {
                    double dist = stod(cell);
                    if (dist > 0) {
                        adjMatrix[row][col] = dist;
                        next[row][col] = col;
                        cities[row].neighbors.push_back({col, dist});
                    }
                } catch (...) {
                    // 忽略转换错误
                }
            }
            col++;
        }
        row++;
    }
    
    floydComputed = false;
    cout << "成功加载 " << cities.size() << " 个城市" << endl;
    return true;
}

int Graph::getCityIndex(const string& name) {
    auto it = cityIndexMap.find(name);
    return it != cityIndexMap.end() ? it->second : -1;
}

string Graph::getCityName(int index) const {
    if (index >= 0 && index < cities.size()) {
        return cities[index].name;
    }
    return "";
}

double Graph::calculateDistance(const vector<int>& path) {
    if (path.size() < 2) return 0;
    
    double total = 0;
    for (size_t i = 0; i < path.size() - 1; i++) {
        int u = path[i];
        int v = path[i + 1];
        if (u >= 0 && v >= 0 && u < cities.size() && v < cities.size()) {
            if (adjMatrix[u][v] < INF) {
                total += adjMatrix[u][v];
            } else {
                return INF;
            }
        }
    }
    return total;
}

void Graph::printPath(const vector<int>& path) {
    for (size_t i = 0; i < path.size(); i++) {
        cout << getCityName(path[i]);
        if (i < path.size() - 1) {
            cout << " -> ";
        }
    }
    cout << endl;
}

vector<int> Graph::dijkstra(int src, int dest) {
    int n = cities.size();
    vector<double> dist(n, INF);
    vector<int> prev(n, -1);
    vector<bool> visited(n, false);
    
    using Pair = pair<double, int>;
    priority_queue<Pair, vector<Pair>, greater<Pair>> pq;
    
    dist[src] = 0;
    pq.push({0, src});
    
    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        
        if (visited[u]) continue;
        visited[u] = true;
        
        if (u == dest) break;
        
        for (const auto& [v, w] : cities[u].neighbors) {
            if (!visited[v] && dist[u] + w < dist[v]) {
                dist[v] = dist[u] + w;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    
    // 重建路径
    vector<int> path;
    if (dist[dest] >= INF) return path;
    
    for (int at = dest; at != -1; at = prev[at]) {
        path.push_back(at);
    }
    reverse(path.begin(), path.end());
    return path;
}

void Graph::computeFloyd() {
    if (floydComputed) return;
    
    int n = cities.size();
    vector<vector<double>> dist = adjMatrix;
    
    // Floyd算法核心
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < n; j++) {
                if (dist[k][j] == INF) continue;
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    next[i][j] = next[i][k];
                }
            }
        }
    }
    
    floydComputed = true;
}

vector<int> Graph::floydWarshall(int src, int dest) {
    computeFloyd();
    
    if (next[src][dest] == -1) {
        return {};
    }
    
    vector<int> path;
    int u = src;
    path.push_back(u);
    
    while (u != dest) {
        u = next[u][dest];
        path.push_back(u);
    }
    
    return path;
}

pair<vector<int>, double> Graph::getShortestPath(int src, int dest, int algorithm) {
    vector<int> path;
    double distance = 0;
    
    chrono::high_resolution_clock::time_point start, end;
    
    if (algorithm == 0) { // Dijkstra
        start = chrono::high_resolution_clock::now();
        path = dijkstra(src, dest);
        end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        distance = calculateDistance(path);
        cout << "Dijkstra算法时间: " << duration << " μs" << endl;
    } else { // Floyd
        start = chrono::high_resolution_clock::now();
        path = floydWarshall(src, dest);
        end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        distance = calculateDistance(path);
        cout << "Floyd算法时间: " << duration << " μs" << endl;
    }
    
    return {path, distance};
}

void Graph::compareAlgorithms(int src, int dest) {
    cout << "\n=== 算法比较 ===\n";
    
    // Dijkstra
    auto start = chrono::high_resolution_clock::now();
    auto dijkstraPath = dijkstra(src, dest);
    auto end = chrono::high_resolution_clock::now();
    auto dijkstraTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
    double dijkstraDist = calculateDistance(dijkstraPath);
    
    // Floyd
    start = chrono::high_resolution_clock::now();
    auto floydPath = floydWarshall(src, dest);
    end = chrono::high_resolution_clock::now();
    auto floydTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
    double floydDist = calculateDistance(floydPath);
    
    cout << "Dijkstra算法:\n";
    cout << "  路径: ";
    printPath(dijkstraPath);
    cout << "  距离: " << dijkstraDist << " km\n";
    cout << "  时间: " << dijkstraTime << " μs\n\n";
    
    cout << "Floyd算法:\n";
    cout << "  路径: ";
    printPath(floydPath);
    cout << "  距离: " << floydDist << " km\n";
    cout << "  时间: " << floydTime << " μs\n\n";
    
    cout << "比较结果:\n";
    if (abs(dijkstraDist - floydDist) < 0.01) {
        cout << "✓ 两种算法距离一致\n";
    } else {
        cout << "✗ 两种算法距离不一致\n";
    }
    
    if (dijkstraTime < floydTime) {
        cout << "✓ Dijkstra更快，适合单次查询\n";
    } else {
        cout << "✓ Floyd更快，适合多次查询\n";
    }
}

bool Graph::verifyWuhanCenter() {
    int wuhan = getCityIndex("武汉");
    if (wuhan == -1) {
        cout << "找不到武汉" << endl;
        return false;
    }
    
    vector<string> capitals = {
        "北京", "天津", "上海", "重庆", "哈尔滨", "长春", "沈阳",
        "呼和浩特", "石家庄", "太原", "西安", "济南", "郑州", "南京",
        "合肥", "杭州", "南昌", "福州", "长沙", "广州", "南宁",
        "成都", "贵阳", "昆明", "兰州", "西宁", "银川", "乌鲁木齐"
    };
    
    cout << "验证武汉中心位置假设...\n";
    cout << "================================\n";
    
    for (const auto& capital : capitals) {
        if (capital == "武汉") continue;
        
        int capitalIdx = getCityIndex(capital);
        if (capitalIdx == -1) continue;
        
        auto path = dijkstra(capitalIdx, wuhan);
        if (path.empty()) continue;
        
        int provinceCount = 0;
        for (int idx : path) {
            string name = getCityName(idx);
            if (find(capitals.begin(), capitals.end(), name) != capitals.end()) {
                provinceCount++;
            }
        }
        
        provinceCount--; // 减去终点武汉
        if (path[0] != wuhan) provinceCount--; // 减去起点
        
        cout << capital << " -> 武汉: " << path.size()-1 << "个城市, "
             << provinceCount << "个省会";
        
        if (provinceCount > 2) {
            cout << " ✗ 超出限制" << endl;
            return false;
        } else {
            cout << " ✓ 符合" << endl;
        }
    }
    
    cout << "================================\n";
    return true;
}

void Graph::dfsFindPaths(int current, int dest, int maxDepth,
                        vector<bool>& visited,
                        vector<int>& currentPath,
                        vector<Path>& allPaths,
                        double currentDist) {
    if (currentPath.size() > maxDepth) return;
    
    if (current == dest) {
        Path path;
        path.nodes = currentPath;
        path.totalDistance = currentDist;
        path.nodeCount = currentPath.size();
        allPaths.push_back(path);
        return;
    }
    
    for (const auto& [next, weight] : cities[current].neighbors) {
        if (!visited[next]) {
            visited[next] = true;
            currentPath.push_back(next);
            
            dfsFindPaths(next, dest, maxDepth, visited, 
                        currentPath, allPaths, currentDist + weight);
            
            currentPath.pop_back();
            visited[next] = false;
        }
    }
}

vector<Path> Graph::getAllPaths(int src, int dest, int maxNodes) {
    vector<Path> allPaths;
    if (src == dest || src < 0 || dest < 0) return allPaths;
    
    vector<bool> visited(cities.size(), false);
    vector<int> currentPath;
    currentPath.push_back(src);
    visited[src] = true;
    
    dfsFindPaths(src, dest, maxNodes, visited, currentPath, allPaths, 0);
    
    // 快速排序
    if (!allPaths.empty()) {
        quickSort(allPaths, 0, allPaths.size() - 1);
    }
    
    return allPaths;
}

void Graph::quickSort(vector<Path>& paths, int left, int right) {
    if (left >= right) return;
    
    int i = left, j = right;
    Path pivot = paths[(left + right) / 2];
    
    while (i <= j) {
        while (paths[i].totalDistance < pivot.totalDistance) i++;
        while (paths[j].totalDistance > pivot.totalDistance) j--;
        if (i <= j) {
            swap(paths[i], paths[j]);
            i++;
            j--;
        }
    }
    
    quickSort(paths, left, j);
    quickSort(paths, i, right);
}

vector<int> Graph::bypassCity(int src, int dest, const string& bypassCity) {
    int bypassIdx = getCityIndex(bypassCity);
    if (bypassIdx == -1 || bypassIdx == src || bypassIdx == dest) {
        return dijkstra(src, dest);
    }
    
    // 临时备份
    vector<vector<double>> backupMatrix = adjMatrix;
    
    // 移除绕过城市的连接
    for (int i = 0; i < cities.size(); i++) {
        adjMatrix[bypassIdx][i] = INF;
        adjMatrix[i][bypassIdx] = INF;
    }
    
    // 重新构建邻接表
    for (auto& city : cities) {
        city.neighbors.clear();
        int idx = getCityIndex(city.name);
        for (int i = 0; i < cities.size(); i++) {
            if (adjMatrix[idx][i] < INF && adjMatrix[idx][i] > 0) {
                city.neighbors.push_back({i, adjMatrix[idx][i]});
            }
        }
    }
    
    // 计算新路径
    auto path = dijkstra(src, dest);
    
    // 恢复数据
    adjMatrix = backupMatrix;
    
    // 重新构建邻接表
    for (auto& city : cities) {
        city.neighbors.clear();
        int idx = getCityIndex(city.name);
        for (int i = 0; i < cities.size(); i++) {
            if (adjMatrix[idx][i] < INF && adjMatrix[idx][i] > 0) {
                city.neighbors.push_back({i, adjMatrix[idx][i]});
            }
        }
    }
    
    return path;
}

vector<Path> Graph::getKShortestPaths(int src, int dest, int k) {
    vector<Path> result;
    if (src == dest || src < 0 || dest < 0 || k <= 0) return result;
    
    // 获取最短路径
    auto firstPath = dijkstra(src, dest);
    if (firstPath.empty()) return result;
    
    Path first;
    first.nodes = firstPath;
    first.totalDistance = calculateDistance(firstPath);
    first.nodeCount = firstPath.size();
    result.push_back(first);
    
    // 简化的K短路径：获取所有路径然后取前K条
    if (k > 1) {
        auto allPaths = getAllPaths(src, dest, 10);
        if (allPaths.size() > 1) {
            for (size_t i = 1; i < allPaths.size() && result.size() < k; i++) {
                result.push_back(allPaths[i]);
            }
        }
    }
    
    return result;
}

void Graph::saveToFile(const vector<Path>& paths, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "无法创建文件: " << filename << endl;
        return;
    }
    
    file << "序号,距离(km),城市数,路径\n";
    for (size_t i = 0; i < paths.size(); i++) {
        file << i+1 << "," << fixed << setprecision(2) << paths[i].totalDistance
             << "," << paths[i].nodes.size() << ",";
        
        for (size_t j = 0; j < paths[i].nodes.size(); j++) {
            file << getCityName(paths[i].nodes[j]);
            if (j < paths[i].nodes.size() - 1) file << "->";
        }
        file << "\n";
    }
    
    file.close();
    cout << "结果已保存到: " << filename << " (共" << paths.size() << "条路径)" << endl;
}
// 在Graph.cpp中添加这个方法作为备选
vector<Path> Graph::getAllPathsSimple(int src, int dest, int maxNodes) {
    vector<Path> allPaths;
    if (src == dest || src < 0 || dest < 0) return allPaths;
    
    // 先获取最短路径
    auto shortestPath = dijkstra(src, dest);
    if (!shortestPath.empty()) {
        Path path;
        path.nodes = shortestPath;
        path.totalDistance = calculateDistance(shortestPath);
        path.nodeCount = shortestPath.size();
        allPaths.push_back(path);
    }
    
    // 使用带限制的BFS查找更多路径
    struct BFSNode {
        int node;
        vector<int> path;
        double distance;
        int depth;
    };
    
    queue<BFSNode> q;
    BFSNode start;
    start.node = src;
    start.path = {src};
    start.distance = 0;
    start.depth = 1;
    q.push(start);
    
    int pathCount = 0;
    const int MAX_PATHS = 500; // 最多找500条路径
    
    while (!q.empty() && pathCount < MAX_PATHS) {
        BFSNode current = q.front();
        q.pop();
        
        if (current.node == dest && current.path.size() > 1) {
            // 检查是否重复路径
            bool duplicate = false;
            for (const auto& existing : allPaths) {
                if (existing.nodes == current.path) {
                    duplicate = true;
                    break;
                }
            }
            
            if (!duplicate) {
                Path path;
                path.nodes = current.path;
                path.totalDistance = current.distance;
                path.nodeCount = current.path.size();
                allPaths.push_back(path);
                pathCount++;
            }
            continue;
        }
        
        if (current.depth >= maxNodes) continue;
        
        // 探索邻居，但限制数量
        int neighborCount = 0;
        for (const auto& [next, weight] : cities[current.node].neighbors) {
            if (neighborCount >= 4) break; // 每个节点最多探索4个邻居
            
            // 检查环路（简单检查，不包含则添加）
            bool inPath = false;
            for (int node : current.path) {
                if (node == next) {
                    inPath = true;
                    break;
                }
            }
            
            if (!inPath) {
                BFSNode nextNode;
                nextNode.node = next;
                nextNode.path = current.path;
                nextNode.path.push_back(next);
                nextNode.distance = current.distance + weight;
                nextNode.depth = current.depth + 1;
                q.push(nextNode);
                neighborCount++;
            }
        }
    }
    
    // 排序
    if (allPaths.size() > 1) {
        quickSort(allPaths, 0, allPaths.size() - 1);
    }
    
    return allPaths;
}