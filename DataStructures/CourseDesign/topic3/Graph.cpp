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

Graph::Graph() : floydComputed(false) {}

Graph::~Graph() {}

bool Graph::loadFromCSV(const string& distanceFile, const string& adjacencyFile) {
    ifstream distFile(distanceFile);
    if (!distFile.is_open()) {
        cerr << "无法打开距离文件: " << distanceFile << endl;
        return false;
    }
    
    string line;
    if (getline(distFile, line)) {
        stringstream ss(line);
        string cityName;
        getline(ss, cityName, ',');
        
        while (getline(ss, cityName, ',')) {
            City city;
            city.name = Utils::trim(cityName);
            cities.push_back(city);
            cityIndexMap[city.name] = cities.size() - 1;
        }
    }
    
    int n = cities.size();
    adjMatrix.resize(n, vector<double>(n, INF));
    for (int i = 0; i < n; i++) {
        adjMatrix[i][i] = 0;
    }
    
    nextMatrix.resize(n, vector<int>(n, -1));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j && adjMatrix[i][j] != INF) {
                nextMatrix[i][j] = j;
            }
        }
    }
    
    int row = 0;
    while (getline(distFile, line) && row < n) {
        stringstream ss(line);
        string cell;
        
        getline(ss, cell, ',');
        string currentCity = Utils::trim(cell);
        
        int col = 0;
        while (getline(ss, cell, ',') && col < n) {
            if (!cell.empty() && cell != " ") {
                try {
                    double distance = stod(cell);
                    if (distance > 0) {
                        adjMatrix[row][col] = distance;
                        cities[row].neighbors.push_back({col, distance});
                        if (row != col) {
                            nextMatrix[row][col] = col;
                        }
                    }
                } catch (const invalid_argument& e) {
                }
            }
            col++;
        }
        row++;
    }
    distFile.close();
    
    floydComputed = false;
    cout << "成功加载 " << cities.size() << " 个城市的数据" << endl;
    return true;
}

void Graph::computeFloyd() {
    if (floydComputed) return;
    
    int n = cities.size();
    vector<vector<double>> dist = adjMatrix;
    
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (dist[i][k] == INF) continue;
            for (int j = 0; j < n; j++) {
                if (dist[k][j] == INF) continue;
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    nextMatrix[i][j] = nextMatrix[i][k];
                }
            }
        }
    }
    
    floydComputed = true;
}

vector<int> Graph::dijkstra(int src, int dest) {
    int n = cities.size();
    if (src < 0 || src >= n || dest < 0 || dest >= n) {
        return {};
    }
    
    vector<double> dist(n, INF);
    vector<int> prev(n, -1);
    vector<bool> visited(n, false);
    
    priority_queue<pair<double, int>, vector<pair<double, int>>, greater<pair<double, int>>> pq;
    
    dist[src] = 0;
    pq.push({0, src});
    
    while (!pq.empty()) {
        double currentDist = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        
        if (visited[u]) continue;
        visited[u] = true;
        
        if (u == dest) break;
        
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
    
    if (dist[dest] != INF) {
        return reconstructPath(prev, dest);
    }
    
    return {};
}

vector<int> Graph::floyd(int src, int dest) {
    int n = cities.size();
    if (src < 0 || src >= n || dest < 0 || dest >= n) {
        return {};
    }
    
    if (!floydComputed) {
        computeFloyd();
    }
    
    if (nextMatrix[src][dest] == -1) {
        return {};
    }
    
    return reconstructFloydPath(src, dest);
}

vector<int> Graph::reconstructPath(const vector<int>& prev, int dest) {
    vector<int> path;
    if (prev[dest] == -1 && dest != -1) {
        path.push_back(dest);
        return path;
    }
    
    stack<int> s;
    int current = dest;
    while (current != -1) {
        s.push(current);
        current = prev[current];
    }
    
    while (!s.empty()) {
        path.push_back(s.top());
        s.pop();
    }
    
    return path;
}

vector<int> Graph::reconstructFloydPath(int src, int dest) {
    if (nextMatrix[src][dest] == -1) {
        return {};
    }
    
    vector<int> path;
    path.push_back(src);
    
    while (src != dest) {
        src = nextMatrix[src][dest];
        path.push_back(src);
    }
    
    return path;
}

pair<vector<int>, double> Graph::getShortestPath(int src, int dest, int algorithm) {
    vector<int> path;
    double distance = 0.0;
    
    if (algorithm == 0) {
        auto start = chrono::high_resolution_clock::now();
        path = dijkstra(src, dest);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        distance = calculatePathDistance(path);
        cout << "Dijkstra算法计算时间: " << duration << " μs" << endl;
    } else {
        auto start = chrono::high_resolution_clock::now();
        path = floyd(src, dest);
        auto end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        distance = calculatePathDistance(path);
        cout << "Floyd算法计算时间: " << duration << " μs" << endl;
    }
    
    return make_pair(path, distance);
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

vector<Path> Graph::findAllPaths(int src, int dest, int maxNodes) {
    vector<Path> allPaths;
    if (src == dest || src < 0 || dest < 0 || 
        src >= static_cast<int>(cities.size()) || 
        dest >= static_cast<int>(cities.size())) {
        return allPaths;
    }
    
    vector<bool> visited(cities.size(), false);
    vector<int> currentPath;
    currentPath.push_back(src);
    visited[src] = true;
    
    dfsFindAllPaths(src, dest, maxNodes, visited, currentPath, 0, allPaths);
    
    return allPaths;
}

void Graph::dfsFindAllPaths(int src, int dest, int maxNodes,
                           vector<bool>& visited,
                           vector<int>& currentPath,
                           double currentDistance,
                           vector<Path>& allPaths) {
    if (currentPath.size() > maxNodes) {
        return;
    }
    
    int current = currentPath.back();
    if (current == dest) {
        Path path;
        path.nodes = currentPath;
        path.totalDistance = currentDistance;
        path.nodeCount = currentPath.size();
        allPaths.push_back(path);
        return;
    }
    
    for (const auto& neighbor : cities[current].neighbors) {
        int next = neighbor.first;
        double weight = neighbor.second;
        
        if (!visited[next]) {
            visited[next] = true;
            currentPath.push_back(next);
            
            dfsFindAllPaths(src, dest, maxNodes, visited, currentPath, 
                           currentDistance + weight, allPaths);
            
            currentPath.pop_back();
            visited[next] = false;
        }
    }
}

vector<Path> Graph::findKShortestPaths(int src, int dest, int k) {
    auto allPaths = findAllPaths(src, dest, 10);
    if (allPaths.empty()) return {};
    
    quickSortPaths(allPaths, 0, allPaths.size() - 1);
    
    if (k > static_cast<int>(allPaths.size())) {
        k = allPaths.size();
    }
    
    return vector<Path>(allPaths.begin(), allPaths.begin() + k);
}

void Graph::quickSortPaths(vector<Path>& paths, int left, int right) {
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
    
    if (left < j) quickSortPaths(paths, left, j);
    if (i < right) quickSortPaths(paths, i, right);
}

vector<int> Graph::findShortestBypass(int src, int dest, const string& bypassCity) {
    int bypassIndex = getCityIndex(bypassCity);
    if (bypassIndex == -1 || bypassIndex == src || bypassIndex == dest) {
        return dijkstra(src, dest);
    }
    
    vector<double> tempDist;
    for (const auto& dist : adjMatrix[bypassIndex]) {
        tempDist.push_back(dist);
    }
    
    for (int i = 0; i < static_cast<int>(cities.size()); i++) {
        adjMatrix[bypassIndex][i] = INF;
        adjMatrix[i][bypassIndex] = INF;
    }
    
    cities[bypassIndex].neighbors.clear();
    
    vector<int> path = dijkstra(src, dest);
    
    for (int i = 0; i < static_cast<int>(cities.size()); i++) {
        adjMatrix[bypassIndex][i] = tempDist[i];
        adjMatrix[i][bypassIndex] = tempDist[i];
    }
    
    cities[bypassIndex].neighbors.clear();
    for (int i = 0; i < static_cast<int>(cities.size()); i++) {
        if (adjMatrix[bypassIndex][i] != INF && adjMatrix[bypassIndex][i] > 0) {
            cities[bypassIndex].neighbors.push_back({i, adjMatrix[bypassIndex][i]});
        }
    }
    
    return path;
}

vector<int> Graph::bfs(int src, int dest) {
    int n = cities.size();
    if (src < 0 || src >= n || dest < 0 || dest >= n) {
        return {};
    }
    
    vector<bool> visited(n, false);
    vector<int> parent(n, -1);
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
                parent[v] = u;
                q.push(v);
            }
        }
    }
    
    if (parent[dest] == -1 && dest != src) {
        return {};
    }
    
    return reconstructPath(parent, dest);
}

bool Graph::verifyWuhanCenter(int maxProvinces) {
    int wuhanIndex = getCityIndex("武汉");
    if (wuhanIndex == -1) {
        cout << "找不到武汉" << endl;
        return false;
    }
    
    cout << "验证武汉是否处于中心位置..." << endl;
    cout << "假设：从其他省会到武汉最多经过 " << maxProvinces << " 个省会城市" << endl;
    cout << "========================================" << endl;
    
    vector<string> provincialCapitals = {
        "北京", "天津", "上海", "重庆", "哈尔滨", "长春", "沈阳",
        "呼和浩特", "石家庄", "太原", "西安", "济南", "郑州", "南京",
        "合肥", "杭州", "南昌", "福州", "长沙", "广州", "南宁",
        "成都", "贵阳", "昆明", "兰州", "西宁", "银川", "乌鲁木齐"
    };
    
    bool allValid = true;
    for (const auto& capital : provincialCapitals) {
        if (capital == "武汉") continue;
        
        int capitalIndex = getCityIndex(capital);
        if (capitalIndex == -1) continue;
        
        auto path = dijkstra(capitalIndex, wuhanIndex);
        
        if (path.empty()) {
            cout << capital << " -> 武汉: 无路径" << endl;
            allValid = false;
            continue;
        }
        
        int provinceCount = 0;
        for (int cityIdx : path) {
            string cityName = cities[cityIdx].name;
            if (find(provincialCapitals.begin(), provincialCapitals.end(), cityName) != provincialCapitals.end()) {
                provinceCount++;
            }
        }
        
        provinceCount--; // 排除起点和终点
        
        cout << capital << " -> 武汉: " << path.size() - 1 << " 个城市, "
             << provinceCount << " 个省会";
        
        if (provinceCount > maxProvinces) {
            cout << " ✗ 超出假设" << endl;
            allValid = false;
        } else {
            cout << " ✓ 符合假设" << endl;
        }
    }
    
    cout << "========================================" << endl;
    if (allValid) {
        cout << "✓ 假设成立：所有省会到武汉最多经过 " << maxProvinces << " 个省会" << endl;
    } else {
        cout << "✗ 假设不成立：部分省会到武汉超过 " << maxProvinces << " 个省会" << endl;
    }
    
    return allValid;
}

void Graph::printPath(const vector<int>& path) {
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] >= 0 && path[i] < static_cast<int>(cities.size())) {
            cout << cities[path[i]].name;
            if (i < path.size() - 1) {
                cout << " -> ";
            }
        }
    }
    cout << endl;
}

void Graph::printPathDetails(const vector<int>& path, double distance) {
    if (path.empty()) {
        cout << "无路径" << endl;
        return;
    }
    
    cout << "路径详情:" << endl;
    cout << "----------------------------------------" << endl;
    
    double totalDistance = (distance >= 0) ? distance : calculatePathDistance(path);
    
    for (size_t i = 0; i < path.size(); i++) {
        if (path[i] >= 0 && path[i] < static_cast<int>(cities.size())) {
            cout << i + 1 << ". " << cities[path[i]].name;
            
            if (i < path.size() - 1) {
                int nextIndex = path[i + 1];
                if (nextIndex >= 0 && nextIndex < static_cast<int>(cities.size())) {
                    double segmentDistance = adjMatrix[path[i]][nextIndex];
                    if (segmentDistance != INF) {
                        cout << " -> " << cities[nextIndex].name 
                             << " (" << segmentDistance << " km)";
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
        file << i + 1 << "," << fixed << setprecision(2) << paths[i].totalDistance
             << "," << paths[i].nodes.size() << ",";
        
        for (size_t j = 0; j < paths[i].nodes.size(); j++) {
            file << cities[paths[i].nodes[j]].name;
            if (j < paths[i].nodes.size() - 1) {
                file << "->";
            }
        }
        file << endl;
    }
    
    file.close();
    cout << "路径已保存到文件: " << filename << endl;
}

void Graph::compareAlgorithms() {
    cout << "\n算法比较测试:" << endl;
    cout << "==========================================" << endl;
    
    vector<pair<string, string>> testPairs = {
        {"北京", "上海"},
        {"广州", "哈尔滨"},
        {"成都", "武汉"},
        {"乌鲁木齐", "海口"}
    };
    
    cout << "城市对\t\tDijkstra时间(μs)\tFloyd时间(μs)\t距离(km)" << endl;
    cout << "----------------------------------------------------------------" << endl;
    
    for (const auto& [srcCity, destCity] : testPairs) {
        int src = getCityIndex(srcCity);
        int dest = getCityIndex(destCity);
        
        if (src == -1 || dest == -1) {
            cout << "未找到城市: " << srcCity << " 或 " << destCity << endl;
            continue;
        }
        
        auto start = chrono::high_resolution_clock::now();
        auto dijkstraPath = dijkstra(src, dest);
        auto end = chrono::high_resolution_clock::now();
        auto dijkstraTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
        double dijkstraDistance = calculatePathDistance(dijkstraPath);
        
        start = chrono::high_resolution_clock::now();
        auto floydPath = floyd(src, dest);
        end = chrono::high_resolution_clock::now();
        auto floydTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
        double floydDistance = calculatePathDistance(floydPath);
        
        cout << srcCity << "->" << destCity << "\t\t"
             << dijkstraTime << "\t\t"
             << floydTime << "\t\t"
             << (dijkstraPath.empty() ? "N/A" : to_string((int)dijkstraDistance)) << endl;
    }
    
    cout << "\n说明：" << endl;
    cout << "1. Dijkstra算法：单源最短路径，适合单次查询" << endl;
    cout << "2. Floyd算法：全源最短路径，适合多次查询（首次计算较慢）" << endl;
    cout << "3. Floyd算法首次计算需要O(V³)时间预处理" << endl;
    cout << "4. Floyd算法后续查询只需要O(1)时间获取结果" << endl;
    cout << "==========================================" << endl;
}

int Graph::getCityIndex(const string& cityName) {
    auto it = cityIndexMap.find(cityName);
    return (it != cityIndexMap.end()) ? it->second : -1;
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