#include "Graph.h"
#include "Utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <queue>
#include <stack>
#include <functional>
#include <cmath>
#include <iomanip>
#include <chrono>

using namespace std;

Graph::Graph() {
    // 构造函数
}

Graph::~Graph() {
    // 析构函数
}

bool Graph::loadFromCSV(const string& distanceFile, const string& adjacencyFile) {
    // 读取距离CSV文件
    ifstream distFile(distanceFile);
    if (!distFile.is_open()) {
        cerr << "无法打开距离文件: " << distanceFile << endl;
        return false;
    }
    
    string line;
    // 读取表头
    if (getline(distFile, line)) {
        stringstream ss(line);
        string cityName;
        getline(ss, cityName, ','); // 跳过第一个空单元格
        
        // 读取所有城市名称
        while (getline(ss, cityName, ',')) {
            City city;
            city.name = Utils::trim(cityName);
            cities.push_back(city);
            cityIndexMap[city.name] = cities.size() - 1;
        }
    }
    
    // 初始化邻接矩阵
    int n = cities.size();
    adjMatrix.resize(n, vector<double>(n, INF));
    for (int i = 0; i < n; i++) {
        adjMatrix[i][i] = 0;
    }
    
    // 读取距离数据
    int row = 0;
    while (getline(distFile, line) && row < n) {
        stringstream ss(line);
        string cell;
        
        // 读取行首城市名
        getline(ss, cell, ',');
        string currentCity = Utils::trim(cell);
        
        // 读取距离数据
        int col = 0;
        while (getline(ss, cell, ',') && col < n) {
            if (!cell.empty() && cell != " ") {
                try {
                    double distance = stod(cell);
                    if (distance > 0) {
                        adjMatrix[row][col] = distance;
                        // 添加到邻接表
                        cities[row].neighbors.push_back({col, distance});
                    }
                } catch (const invalid_argument& e) {
                    // 忽略转换错误
                }
            }
            col++;
        }
        row++;
    }
    distFile.close();
    
    cout << "成功加载 " << cities.size() << " 个城市的数据" << endl;
    return true;
}

int Graph::getCityIndex(const string& cityName) {
    auto it = cityIndexMap.find(cityName);
    if (it != cityIndexMap.end()) {
        return it->second;
    }
    return -1;
}

City Graph::getCity(int index) {
    if (index >= 0 && index < static_cast<int>(cities.size())) {
        return cities[index];
    }
    return City();
}

vector<string> Graph::getAllCityNames() {
    vector<string> names;
    for (const auto& city : cities) {
        names.push_back(city.name);
    }
    return names;
}

vector<int> Graph::dijkstra(int src, int dest) {
    int n = cities.size();
    if (src < 0 || src >= n || dest < 0 || dest >= n) {
        return {};
    }
    
    vector<double> dist(n, INF);
    vector<int> prev(n, -1);
    vector<bool> visited(n, false);
    
    // 使用优先队列（最小堆）
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
    
    dist[src] = 0;
    pq.push({0, src});
    
    while (!pq.empty()) {
        double currentDist = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        
        if (visited[u]) continue;
        visited[u] = true;
        
        // 如果找到目标，提前结束
        if (u == dest) break;
        
        // 遍历所有邻居（使用邻接表）
        for (const auto& neighbor : cities[u].neighbors) {
            int v = neighbor.first;
            double weight = neighbor.second;
            
            if (!visited[v] && dist[u] + weight < dist[v]) {
                dist[v] = dist[u] + weight;
                prev[v] = u;
                pq.push({dist[v], v});
            }
        }
    }
    
    // 如果找到路径，重建路径
    if (dist[dest] != INF) {
        return reconstructPath(prev, dest);
    }
    
    return {};
}

vector<vector<double>> Graph::floyd() {
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
                }
            }
        }
    }
    
    return dist;
}

vector<Path> Graph::findAllPaths(int src, int dest, int maxNodes) {
    vector<Path> allPaths;
    if (src < 0 || src >= static_cast<int>(cities.size()) || 
        dest < 0 || dest >= static_cast<int>(cities.size())) {
        return allPaths;
    }
    
    vector<bool> visited(cities.size(), false);
    vector<int> currentPath;
    
    dfsFindAllPaths(src, dest, maxNodes, visited, currentPath, 0.0, allPaths);
    return allPaths;
}

void Graph::dfsFindAllPaths(int src, int dest, int maxNodes,
                           vector<bool>& visited,
                           vector<int>& currentPath,
                           double currentDistance,
                           vector<Path>& allPaths) {
    if (currentPath.size() > maxNodes) return;
    
    currentPath.push_back(src);
    visited[src] = true;
    
    if (src == dest) {
        Path path;
        path.nodes = currentPath;
        path.totalDistance = currentDistance;
        path.nodeCount = currentPath.size();
        allPaths.push_back(path);
    } else {
        for (const auto& neighbor : cities[src].neighbors) {
            int next = neighbor.first;
            double weight = neighbor.second;
            
            if (!visited[next]) {
                dfsFindAllPaths(next, dest, maxNodes, visited, currentPath, 
                              currentDistance + weight, allPaths);
            }
        }
    }
    
    currentPath.pop_back();
    visited[src] = false;
}

vector<Path> Graph::findKShortestPaths(int src, int dest, int k) {
    vector<Path> kPaths;
    
    // 先获取一条最短路径
    auto shortestPath = dijkstra(src, dest);
    if (shortestPath.empty()) return kPaths;
    
    Path firstPath;
    firstPath.nodes = shortestPath;
    firstPath.totalDistance = calculatePathDistance(shortestPath);
    firstPath.nodeCount = shortestPath.size();
    kPaths.push_back(firstPath);
    
    // 简化版本：使用所有路径排序
    if (k > 1) {
        auto allPaths = findAllPaths(src, dest, 15); // 放宽限制
        if (allPaths.size() > 1) {
            // 移除第一条路径（已包含）
            if (!allPaths.empty() && allPaths[0].nodes == shortestPath) {
                allPaths.erase(allPaths.begin());
            }
            
            // 按距离排序
            sort(allPaths.begin(), allPaths.end(),
                [](const Path& a, const Path& b) {
                    return a.totalDistance < b.totalDistance;
                });
            
            // 取前k-1条
            for (int i = 0; i < min(k - 1, static_cast<int>(allPaths.size())); i++) {
                kPaths.push_back(allPaths[i]);
            }
        }
    }
    
    return kPaths;
}

vector<int> Graph::findShortestBypass(int src, int dest, const string& bypassCity) {
    int bypassIndex = getCityIndex(bypassCity);
    if (bypassIndex == -1) {
        return dijkstra(src, dest);
    }
    
    // 临时保存与绕过城市相关的边
    vector<double> savedEdges;
    for (int i = 0; i < static_cast<int>(cities.size()); i++) {
        savedEdges.push_back(adjMatrix[bypassIndex][i]);
        if (adjMatrix[bypassIndex][i] != INF) {
            adjMatrix[bypassIndex][i] = INF;
            adjMatrix[i][bypassIndex] = INF;
            // 也从邻接表中移除
            auto& neighbors = cities[bypassIndex].neighbors;
            neighbors.erase(remove_if(neighbors.begin(), neighbors.end(),
                [i](const pair<int, double>& p) { return p.first == i; }),
                neighbors.end());
        }
    }
    
    // 计算绕过城市的最短路径
    auto path = dijkstra(src, dest);
    
    // 恢复边
    for (int i = 0; i < static_cast<int>(cities.size()); i++) {
        adjMatrix[bypassIndex][i] = savedEdges[i];
        adjMatrix[i][bypassIndex] = savedEdges[i];
        if (savedEdges[i] != INF && savedEdges[i] > 0) {
            // 恢复邻接表
            cities[bypassIndex].neighbors.push_back({i, savedEdges[i]});
        }
    }
    
    return path;
}

void Graph::quickSortPaths(vector<Path>& paths, int left, int right) {
    if (left >= right) return;
    
    double pivot = paths[left + (right - left) / 2].totalDistance;
    int i = left, j = right;
    
    while (i <= j) {
        while (paths[i].totalDistance < pivot) i++;
        while (paths[j].totalDistance > pivot) j--;
        
        if (i <= j) {
            swap(paths[i], paths[j]);
            i++;
            j--;
        }
    }
    
    quickSortPaths(paths, left, j);
    quickSortPaths(paths, i, right);
}

vector<int> Graph::bfs(int src, int dest) {
    int n = cities.size();
    if (src < 0 || src >= n || dest < 0 || dest >= n) {
        return {};
    }
    
    vector<bool> visited(n, false);
    vector<int> prev(n, -1);
    queue<int> q;
    
    visited[src] = true;
    q.push(src);
    
    while (!q.empty()) {
        int u = q.front();
        q.pop();
        
        if (u == dest) break;
        
        for (const auto& neighbor : cities[u].neighbors) {
            int v = neighbor.first;
            if (!visited[v]) {
                visited[v] = true;
                prev[v] = u;
                q.push(v);
            }
        }
    }
    
    return reconstructPath(prev, dest);
}

bool Graph::verifyWuhanCenter(int maxProvinces) {
    int wuhanIndex = getCityIndex("武汉");
    if (wuhanIndex == -1) {
        cout << "未找到武汉数据" << endl;
        return false;
    }
    
    // 定义省会城市列表（排除港澳台海口）
    vector<string> provincialCapitals = {
        "北京", "天津", "上海", "重庆", "哈尔滨", "长春", "沈阳",
        "呼和浩特", "石家庄", "太原", "西安", "济南", "郑州", "南京", 
        "合肥", "杭州", "南昌", "福州", "长沙", "武汉", "广州", "南宁",
        "成都", "贵阳", "昆明", "兰州", "西宁", "银川", "乌鲁木齐"
    };
    
    bool allPassed = true;
    cout << "\n验证武汉中心位置假设..." << endl;
    cout << "条件：其他省会城市到武汉中间不超过" << maxProvinces << "个省会城市" << endl;
    cout << "==========================================" << endl;
    
    for (const auto& capital : provincialCapitals) {
        if (capital == "武汉") continue;
        
        int capitalIndex = getCityIndex(capital);
        if (capitalIndex == -1) {
            cout << "未找到城市: " << capital << endl;
            continue;
        }
        
        // 使用BFS查找路径
        auto path = bfs(wuhanIndex, capitalIndex);
        
        if (path.empty()) {
            cout << capital << " -> 武汉: 无法到达" << endl;
            allPassed = false;
        } else {
            // 中间节点数 = 总节点数 - 2（去掉首尾）
            int intermediateNodes = static_cast<int>(path.size()) - 2;
            string status = (intermediateNodes <= maxProvinces) ? "✓ 通过" : "✗ 失败";
            
            cout << capital << " -> 武汉: " << intermediateNodes 
                 << " 个中间城市 " << status << endl;
            
            if (intermediateNodes > maxProvinces) {
                allPassed = false;
                cout << "  路径: ";
                printPath(path);
            }
        }
    }
    
    cout << "==========================================" << endl;
    if (allPassed) {
        cout << "✓ 所有省会城市到武汉中间不超过" << maxProvinces << "个省会城市" << endl;
    } else {
        cout << "✗ 存在省会城市到武汉中间超过" << maxProvinces << "个省会城市" << endl;
    }
    
    return allPassed;
}

double Graph::calculatePathDistance(const vector<int>& path) {
    if (path.size() < 2) return 0.0;
    
    double total = 0.0;
    for (size_t i = 0; i < path.size() - 1; i++) {
        int u = path[i];
        int v = path[i + 1];
        if (u >= 0 && u < static_cast<int>(cities.size()) && 
            v >= 0 && v < static_cast<int>(cities.size())) {
            total += adjMatrix[u][v];
        }
    }
    return total;
}

vector<int> Graph::reconstructPath(const vector<int>& prev, int dest) {
    vector<int> path;
    if (prev[dest] == -1) return path;
    
    // 反向重建路径
    for (int at = dest; at != -1; at = prev[at]) {
        path.push_back(at);
    }
    
    // 反转路径
    reverse(path.begin(), path.end());
    return path;
}

void Graph::printPath(const vector<int>& path) {
    if (path.empty()) {
        cout << "无路径" << endl;
        return;
    }
    
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] >= 0 && path[i] < static_cast<int>(cities.size())) {
            cout << cities[path[i]].name;
        } else {
            cout << "未知城市";
        }
        if (i < path.size() - 1) {
            cout << " -> ";
        }
    }
    cout << endl;
}

void Graph::printPathDetails(const vector<int>& path) {
    if (path.empty()) {
        cout << "无路径" << endl;
        return;
    }
    
    cout << "路径详情:" << endl;
    cout << "----------------------------------------" << endl;
    
    double totalDistance = 0.0;
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] >= 0 && path[i] < static_cast<int>(cities.size())) {
            cout << i + 1 << ". " << cities[path[i]].name;
            
            if (i < path.size() - 1) {
                int nextIndex = path[i + 1];
                if (nextIndex >= 0 && nextIndex < static_cast<int>(cities.size())) {
                    double distance = adjMatrix[path[i]][nextIndex];
                    if (distance != INF) {
                        totalDistance += distance;
                        cout << " -> " << cities[nextIndex].name 
                             << " (" << distance << " km)";
                    } else {
                        cout << " -> " << cities[nextIndex].name << " (不可达)";
                    }
                }
            }
            cout << endl;
        }
    }
    
    cout << "----------------------------------------" << endl;
    cout << "总距离: " << totalDistance << " km" << endl;
    cout << "经过城市数: " << path.size() << endl;
}

void Graph::savePathsToFile(const vector<Path>& paths, const string& filename) {
    ofstream file(filename);
    if (!file.is_open()) {
        cerr << "无法打开文件: " << filename << endl;
        return;
    }
    
    file << "序号,总距离(km),城市数,路径" << endl;
    for (size_t i = 0; i < paths.size(); i++) {
        const auto& path = paths[i];
        file << i + 1 << "," 
             << fixed << setprecision(2) << path.totalDistance << ","
             << path.nodeCount << ",";
        
        for (size_t j = 0; j < path.nodes.size(); j++) {
            if (path.nodes[j] >= 0 && path.nodes[j] < static_cast<int>(cities.size())) {
                file << cities[path.nodes[j]].name;
            } else {
                file << "未知城市";
            }
            if (j < path.nodes.size() - 1) {
                file << "->";
            }
        }
        file << endl;
    }
    
    file.close();
    cout << "结果已保存到: " << filename << endl;
}

void Graph::compareAlgorithms() {
    cout << "\n算法比较测试:" << endl;
    cout << "==========================================" << endl;
    
    // 测试几个典型城市对
    vector<pair<string, string>> testPairs = {
        {"北京", "上海"},
        {"广州", "哈尔滨"},
        {"成都", "武汉"},
        {"乌鲁木齐", "海口"}
    };
    
    for (const auto& [srcCity, destCity] : testPairs) {
        int src = getCityIndex(srcCity);
        int dest = getCityIndex(destCity);
        
        if (src == -1 || dest == -1) {
            cout << "未找到城市: " << srcCity << " 或 " << destCity << endl;
            continue;
        }
        
        cout << "\n测试: " << srcCity << " -> " << destCity << endl;
        
        // Dijkstra算法
        auto start = chrono::high_resolution_clock::now();
        auto dijkstraPath = dijkstra(src, dest);
        auto end = chrono::high_resolution_clock::now();
        auto dijkstraTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
        
        double dijkstraDistance = calculatePathDistance(dijkstraPath);
        
        cout << "Dijkstra: " << (dijkstraPath.empty() ? "无路径" : to_string(dijkstraDistance) + " km")
             << ", 时间: " << dijkstraTime << " μs" << endl;
    }
    cout << "==========================================" << endl;
}