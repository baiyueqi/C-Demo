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
#include <unordered_set>

using namespace std;

/**
 * @brief 构造函数
 */
Graph::Graph() : floydComputed(false) {}

/**
 * @brief 析构函数
 */
Graph::~Graph() {}

/**
 * @brief 从CSV文件加载数据
 * @param distanceFile 距离文件路径
 * @param adjacencyFile 邻接文件路径（可选）
 * @return 成功返回true，失败返回false
 */
bool Graph::loadFromCSV(const string& distanceFile, const string& adjacencyFile) {
    ifstream distFile(distanceFile);
    if (!distFile.is_open()) {
        cerr << "无法打开距离文件: " << distanceFile << endl;
        return false;
    }
    
    string line;
    // 读取表头（第一行）
    if (getline(distFile, line)) {
        stringstream ss(line);
        string cityName;
        getline(ss, cityName, ','); // 跳过第一个空单元格
        
        // 读取所有城市名称
        while (getline(ss, cityName, ',')) {
            City city;
            city.name = Utils::trim(cityName);  // 去除空白字符
            cities.push_back(city);
            cityIndexMap[city.name] = cities.size() - 1;  // 建立映射
        }
    }
    
    // 初始化邻接矩阵（全部设为无穷大）
    int n = cities.size();
    adjMatrix.resize(n, vector<double>(n, INF));
    for (int i = 0; i < n; i++) {
        adjMatrix[i][i] = 0;  // 对角线为0
    }
    
    // 初始化Floyd路径重建矩阵
    nextMatrix.resize(n, vector<int>(n, -1));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            if (i != j && adjMatrix[i][j] != INF) {
                nextMatrix[i][j] = j;
            }
        }
    }
    
    // 读取距离数据
    int row = 0;
    while (getline(distFile, line) && row < n) {
        stringstream ss(line);
        string cell;
        
        // 读取行首城市名
        getline(ss, cell, ',');
        string currentCity = Utils::trim(cell);
        
        // 读取该城市到其他城市的距离
        int col = 0;
        while (getline(ss, cell, ',') && col < n) {
            if (!cell.empty() && cell != " ") {
                try {
                    double distance = stod(cell);
                    if (distance > 0) {
                        adjMatrix[row][col] = distance;
                        // 添加到邻接表
                        cities[row].neighbors.push_back({col, distance});
                        // 初始化Floyd路径重建矩阵
                        if (row != col) {
                            nextMatrix[row][col] = col;
                        }
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
    
    floydComputed = false;  // 重置Floyd计算标志
    
    cout << "成功加载 " << cities.size() << " 个城市的数据" << endl;
    return true;
}

/**
 * @brief 计算Floyd算法（全源最短路径）
 * @note 时间复杂度：O(V^3)，空间复杂度：O(V^2)
 */
void Graph::computeFloyd() {
    if (floydComputed) return;  // 如果已经计算过，直接返回
    
    int n = cities.size();
    vector<vector<double>> dist = adjMatrix;  // 复制邻接矩阵
    
    // Floyd算法核心：动态规划
    for (int k = 0; k < n; k++) {
        for (int i = 0; i < n; i++) {
            if (dist[i][k] == INF) continue;  // 跳过不可达情况
            for (int j = 0; j < n; j++) {
                if (dist[k][j] == INF) continue;
                // 如果通过k中转更短，则更新距离和路径
                if (dist[i][k] + dist[k][j] < dist[i][j]) {
                    dist[i][j] = dist[i][k] + dist[k][j];
                    nextMatrix[i][j] = nextMatrix[i][k];
                }
            }
        }
    }
    
    floydComputed = true;  // 标记为已计算
}

/**
 * @brief Dijkstra算法（单源最短路径）
 * @param src 起点索引
 * @param dest 终点索引
 * @return 最短路径的城市索引序列
 * @note 时间复杂度：O(E log V)，适合稀疏图
 */
vector<int> Graph::dijkstra(int src, int dest) {
    int n = cities.size();
    if (src < 0 || src >= n || dest < 0 || dest >= n) {
        return {};
    }
    
    vector<double> dist(n, INF);   // 距离数组
    vector<int> prev(n, -1);       // 前驱数组
    vector<bool> visited(n, false); // 访问标记
    
    // 使用最小堆（优先队列）存储<距离, 节点>
    priority_queue<pair<double, int>, vector<pair<double, int>>, 
                   greater<pair<double, int>>> pq;
    
    dist[src] = 0;
    pq.push({0, src});
    
    while (!pq.empty()) {
        double currentDist = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        
        if (visited[u]) continue;  // 已访问过，跳过
        visited[u] = true;
        
        // 如果找到目标，提前结束
        if (u == dest) break;
        
        // 遍历所有邻居（使用邻接表提高效率）
        for (const auto& neighbor : cities[u].neighbors) {
            int v = neighbor.first;
            double weight = neighbor.second;
            
            // 松弛操作
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
    
    return {};  // 没有路径
}

/**
 * @brief Floyd算法查询最短路径
 * @param src 起点索引
 * @param dest 终点索引
 * @return 最短路径的城市索引序列
 * @note 需要先调用computeFloyd()进行预处理
 */
vector<int> Graph::floyd(int src, int dest) {
    int n = cities.size();
    if (src < 0 || src >= n || dest < 0 || dest >= n) {
        return {};
    }
    
    // 如果Floyd算法尚未计算，先计算
    if (!floydComputed) {
        computeFloyd();
    }
    
    // 如果没有路径
    if (nextMatrix[src][dest] == -1) {
        return {};
    }
    
    // 重建路径
    return reconstructFloydPath(src, dest);
}

/**
 * @brief 从Dijkstra的前驱数组重建路径
 * @param prev 前驱数组
 * @param dest 终点索引
 * @return 从起点到终点的路径
 */
vector<int> Graph::reconstructPath(const vector<int>& prev, int dest) {
    vector<int> path;
    if (prev[dest] == -1 && dest != -1) {
        // 只有起点的情况
        path.push_back(dest);
        return path;
    }
    
    // 从终点回溯到起点，使用栈逆序
    stack<int> s;
    int current = dest;
    while (current != -1) {
        s.push(current);
        current = prev[current];
    }
    
    // 将栈中的路径转换为向量
    while (!s.empty()) {
        path.push_back(s.top());
        s.pop();
    }
    
    return path;
}

/**
 * @brief 从Floyd的nextMatrix重建路径
 * @param src 起点索引
 * @param dest 终点索引
 * @return 从起点到终点的路径
 */
vector<int> Graph::reconstructFloydPath(int src, int dest) {
    if (nextMatrix[src][dest] == -1) {
        return {};
    }
    
    vector<int> path;
    path.push_back(src);
    
    // 沿着nextMatrix指针重建路径
    while (src != dest) {
        src = nextMatrix[src][dest];
        path.push_back(src);
    }
    
    return path;
}

/**
 * @brief 获取最短路径（封装Dijkstra和Floyd）
 * @param src 起点索引
 * @param dest 终点索引
 * @param algorithm 算法选择（0-Dijkstra, 1-Floyd）
 * @return <路径, 距离>
 */
pair<vector<int>, double> Graph::getShortestPath(int src, int dest, int algorithm) {
    vector<int> path;
    double distance = 0.0;
    
    chrono::high_resolution_clock::time_point start, end;
    
    if (algorithm == 0) {  // Dijkstra算法
        start = chrono::high_resolution_clock::now();
        path = dijkstra(src, dest);
        end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        distance = calculatePathDistance(path);
        cout << "Dijkstra算法计算时间: " << duration << " μs" << endl;
    } else {  // Floyd算法
        start = chrono::high_resolution_clock::now();
        path = floyd(src, dest);
        end = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
        distance = calculatePathDistance(path);
        cout << "Floyd算法计算时间: " << duration << " μs" << endl;
    }
    
    return make_pair(path, distance);
}

/**
 * @brief 计算路径的总距离
 * @param path 路径的城市索引序列
 * @return 总距离
 */
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

/**
 * @brief 查找所有路径（功能2）
 * @param src 起点索引
 * @param dest 终点索引
 * @param maxNodes 最多经过的节点数
 * @return 所有路径的列表
 * @note 使用深度优先搜索，限制深度避免组合爆炸
 */
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

/**
 * @brief 深度优先搜索所有路径（递归）
 * @param src 起点索引（递归参数，实际是当前节点）
 * @param dest 终点索引
 * @param maxNodes 最大节点数
 * @param visited 访问标记数组
 * @param currentPath 当前路径
 * @param currentDistance 当前距离
 * @param allPaths 所有路径的集合（引用）
 */
void Graph::dfsFindAllPaths(int src, int dest, int maxNodes,
                           vector<bool>& visited,
                           vector<int>& currentPath,
                           double currentDistance,
                           vector<Path>& allPaths) {
    // 如果路径过长，剪枝
    if (currentPath.size() > maxNodes) {
        return;
    }
    
    int current = currentPath.back();
    // 如果到达终点，记录路径
    if (current == dest) {
        Path path;
        path.nodes = currentPath;
        path.totalDistance = currentDistance;
        path.nodeCount = currentPath.size();
        allPaths.push_back(path);
        return;
    }
    
    // 遍历所有邻居
    for (const auto& neighbor : cities[current].neighbors) {
        int next = neighbor.first;
        double weight = neighbor.second;
        
        if (!visited[next]) {
            visited[next] = true;
            currentPath.push_back(next);
            
            dfsFindAllPaths(src, dest, maxNodes, visited, currentPath, 
                           currentDistance + weight, allPaths);
            
            // 回溯
            currentPath.pop_back();
            visited[next] = false;
        }
    }
}

/**
 * @brief 基于DFS的K短路径（功能2的简单实现）
 * @param src 起点索引
 * @param dest 终点索引
 * @param k K值
 * @return 前K短路径
 * @note 先查找所有路径，然后排序取前K条（效率较低）
 */
vector<Path> Graph::findKShortestPaths(int src, int dest, int k) {
    // 先查找所有路径（最多10个节点）
    auto allPaths = findAllPaths(src, dest, 10);
    if (allPaths.empty()) return {};
    
    // 使用快速排序
    quickSortPaths(allPaths, 0, allPaths.size() - 1);
    
    // 返回前k条路径
    if (k > static_cast<int>(allPaths.size())) {
        k = allPaths.size();
    }
    
    return vector<Path>(allPaths.begin(), allPaths.begin() + k);
}

/**
 * @brief Yen's算法寻找K短路径（功能4）
 * @param src 起点索引
 * @param dest 终点索引
 * @param k K值
 * @return 前K短路径
 * @note 时间复杂度：O(kn(m + n log n))
 */
vector<Path> Graph::findKShortestPathsYen(int src, int dest, int k) {
    vector<Path> A;  // 存储找到的K最短路径
    priority_queue<CandidatePath, vector<CandidatePath>, 
                   greater<CandidatePath>> B;  // 候选路径（最小堆）
    
    if (src < 0 || dest < 0 || src >= static_cast<int>(cities.size()) || 
        dest >= static_cast<int>(cities.size()) || src == dest || k <= 0) {
        return A;
    }
    
    // 1. 使用Dijkstra计算第一条最短路径
    vector<int> firstPath = dijkstra(src, dest);
    if (firstPath.empty()) {
        return A;
    }
    
    // 转换为Path结构
    Path firstPathObj;
    firstPathObj.nodes = firstPath;
    firstPathObj.totalDistance = calculatePathDistance(firstPath);
    firstPathObj.nodeCount = firstPath.size();
    A.push_back(firstPathObj);
    
    // 2. 寻找后续的K-1条路径
    for (int ki = 1; ki < k; ki++) {
        if (ki - 1 >= static_cast<int>(A.size())) {
            break;  // 没有更多路径了
        }
        
        // 获取上一条路径作为基础
        vector<int> prevPath = A[ki-1].nodes;
        
        // 遍历路径上的每个节点作为spur node（除了终点）
        for (size_t i = 0; i < prevPath.size() - 1; i++) {
            int spurNode = prevPath[i];
            
            // 根路径：从起点到spurNode的部分
            vector<int> rootPath(prevPath.begin(), prevPath.begin() + i + 1);
            
            // 3. 临时移除不能使用的边
            vector<pair<int, int>> forbiddenEdges;
            
            // 移除那些与已有路径共享相同根路径的边
            for (const Path& p : A) {
                if (p.nodes.size() > i + 1) {
                    bool sameRoot = true;
                    for (size_t j = 0; j <= i; j++) {
                        if (j >= p.nodes.size() || prevPath[j] != p.nodes[j]) {
                            sameRoot = false;
                            break;
                        }
                    }
                    if (sameRoot) {
                        // 移除spurNode到下一个节点的边
                        int nextNode = p.nodes[i+1];
                        forbiddenEdges.push_back({spurNode, nextNode});
                    }
                }
            }
            
            // 4. 临时移除根路径中的节点（除了spurNode）
            vector<int> forbiddenNodes;
            for (size_t j = 0; j < rootPath.size() - 1; j++) {
                forbiddenNodes.push_back(rootPath[j]);
            }
            
            // 5. 计算从spurNode到终点的最短路径（带约束）
            vector<int> spurPath = dijkstraWithConstraints(spurNode, dest, 
                                                          forbiddenEdges, forbiddenNodes);
            
            // 6. 如果找到路径，构建完整路径
            if (!spurPath.empty()) {
                vector<int> totalPath = rootPath;
                totalPath.insert(totalPath.end(), spurPath.begin() + 1, spurPath.end());
                
                // 检查路径是否重复
                bool duplicate = false;
                for (const Path& existing : A) {
                    if (existing.nodes == totalPath) {
                        duplicate = true;
                        break;
                    }
                }
                
                if (!duplicate) {
                    double totalDist = calculatePathDistance(totalPath);
                    CandidatePath candidate;
                    candidate.nodes = totalPath;
                    candidate.distance = totalDist;
                    candidate.spurIndex = i;
                    
                    B.push(candidate);
                }
            }
        }
        
        // 7. 从候选路径中选择最短的加入A
        if (B.empty()) {
            break;  // 没有更多候选路径
        }
        
        CandidatePath shortest = B.top();
        B.pop();
        
        Path newPath;
        newPath.nodes = shortest.nodes;
        newPath.totalDistance = shortest.distance;
        newPath.nodeCount = shortest.nodes.size();
        A.push_back(newPath);
    }
    
    // 只返回前k条
    if (static_cast<int>(A.size()) > k) {
        A.resize(k);
    }
    
    return A;
}

/**
 * @brief 带约束的Dijkstra算法（用于Yen's算法）
 * @param src 起点索引
 * @param dest 终点索引
 * @param forbiddenEdges 禁止的边列表
 * @param forbiddenNodes 禁止的节点列表
 * @return 最短路径
 */
vector<int> Graph::dijkstraWithConstraints(int src, int dest,
                                          const vector<pair<int, int>>& forbiddenEdges,
                                          const vector<int>& forbiddenNodes) {
    int n = cities.size();
    if (src < 0 || src >= n || dest < 0 || dest >= n) {
        return {};
    }
    
    // 创建临时邻接矩阵副本
    vector<vector<double>> tempMatrix = adjMatrix;
    
    // 应用约束：禁用某些边
    for (const auto& edge : forbiddenEdges) {
        int u = edge.first;
        int v = edge.second;
        if (u >= 0 && u < n && v >= 0 && v < n) {
            tempMatrix[u][v] = INF;
        }
    }
    
    // 应用约束：禁用某些节点（通过将其所有边设为无穷大）
    for (int node : forbiddenNodes) {
        if (node >= 0 && node < n) {
            for (int i = 0; i < n; i++) {
                tempMatrix[node][i] = INF;
                tempMatrix[i][node] = INF;
            }
        }
    }
    
    // 使用修改后的矩阵运行Dijkstra
    vector<double> dist(n, INF);
    vector<int> prev(n, -1);
    vector<bool> visited(n, false);
    
    priority_queue<pair<double, int>, vector<pair<double, int>>, 
                   greater<pair<double, int>>> pq;
    
    dist[src] = 0;
    pq.push({0, src});
    
    while (!pq.empty()) {
        double currentDist = pq.top().first;
        int u = pq.top().second;
        pq.pop();
        
        if (visited[u]) continue;
        visited[u] = true;
        
        if (u == dest) break;
        
        for (int v = 0; v < n; v++) {
            if (!visited[v] && tempMatrix[u][v] != INF) {
                double newDist = dist[u] + tempMatrix[u][v];
                if (newDist < dist[v]) {
                    dist[v] = newDist;
                    prev[v] = u;
                    pq.push({dist[v], v});
                }
            }
        }
    }
    
    if (dist[dest] == INF) {
        return {};
    }
    
    // 重建路径
    vector<int> path;
    int current = dest;
    while (current != -1) {
        path.push_back(current);
        current = prev[current];
    }
    reverse(path.begin(), path.end());
    
    return path;
}

/**
 * @brief 快速排序算法（用于路径排序）
 * @param paths 路径数组
 * @param left 左边界
 * @param right 右边界
 * @note 时间复杂度：O(n log n) 平均情况
 */
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
    
    // 递归排序左右两部分
    if (left < j) quickSortPaths(paths, left, j);
    if (i < right) quickSortPaths(paths, i, right);
}

/**
 * @brief 查找绕过特定城市的最短路径（功能3）
 * @param src 起点索引
 * @param dest 终点索引
 * @param bypassCity 要绕过的城市名
 * @return 绕过后的最短路径
 */
vector<int> Graph::findShortestBypass(int src, int dest, const string& bypassCity) {
    int bypassIndex = getCityIndex(bypassCity);
    
    // 如果要绕过的城市不存在，或者就是起点/终点，使用普通Dijkstra
    if (bypassIndex == -1 || bypassIndex == src || bypassIndex == dest) {
        return dijkstra(src, dest);
    }
    
    // 临时保存原始距离
    vector<double> tempDist;
    for (const auto& dist : adjMatrix[bypassIndex]) {
        tempDist.push_back(dist);
    }
    
    // 临时移除绕过的城市：将其所有连接设为无穷大
    for (int i = 0; i < static_cast<int>(cities.size()); i++) {
        adjMatrix[bypassIndex][i] = INF;
        adjMatrix[i][bypassIndex] = INF;
    }
    
    // 重新计算邻接表
    cities[bypassIndex].neighbors.clear();
    
    // 使用Dijkstra计算绕过后的最短路径
    vector<int> path = dijkstra(src, dest);
    
    // 恢复原始数据
    for (int i = 0; i < static_cast<int>(cities.size()); i++) {
        adjMatrix[bypassIndex][i] = tempDist[i];
        adjMatrix[i][bypassIndex] = tempDist[i];
    }
    
    // 恢复邻接表
    cities[bypassIndex].neighbors.clear();
    for (int i = 0; i < static_cast<int>(cities.size()); i++) {
        if (adjMatrix[bypassIndex][i] != INF && adjMatrix[bypassIndex][i] > 0) {
            cities[bypassIndex].neighbors.push_back({i, adjMatrix[bypassIndex][i]});
        }
    }
    
    return path;
}

/**
 * @brief 广度优先搜索
 * @param src 起点索引
 * @param dest 终点索引
 * @return 路径（节点数最少，不考虑距离）
 */
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

/**
 * @brief 验证武汉中心位置假设（功能1）
 * @param maxProvinces 最大经过的省会数
 * @return 假设是否成立
 */
bool Graph::verifyWuhanCenter(int maxProvinces) {
    int wuhanIndex = getCityIndex("武汉");
    if (wuhanIndex == -1) {
        cout << "找不到武汉" << endl;
        return false;
    }
    
    cout << "验证武汉是否处于中心位置..." << endl;
    cout << "假设：从其他省会到武汉最多经过 " << maxProvinces << " 个省会城市" << endl;
    cout << "========================================" << endl;
    
    // 省会城市列表（不包括港澳台和海口）
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
        
        // 计算最短路径
        auto path = dijkstra(capitalIndex, wuhanIndex);
        
        if (path.empty()) {
            cout << capital << " -> 武汉: 无路径" << endl;
            allValid = false;
            continue;
        }
        
        // 统计路径中的省会城市数量
        int provinceCount = 0;
        for (int cityIdx : path) {
            string cityName = cities[cityIdx].name;
            if (find(provincialCapitals.begin(), provincialCapitals.end(), cityName) != 
                provincialCapitals.end()) {
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

/**
 * @brief 打印路径（简单格式）
 * @param path 路径的城市索引序列
 */
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

/**
 * @brief 打印路径详情（详细格式）
 * @param path 路径的城市索引序列
 * @param distance 路径距离（如果为-1则重新计算）
 */
void Graph::printPathDetails(const vector<int>& path, double distance) {
    if (path.empty()) {
        cout << "无路径" << endl;
        return;
    }
    
    cout << "路径详情:" << endl;
    cout << "----------------------------------------" << endl;
    
    double totalDistance = (distance >= 0) ? distance : calculatePathDistance(path);
    
    // 打印每一段路径
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

/**
 * @brief 保存路径到文件
 * @param paths 路径列表
 * @param filename 文件名
 */
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

/**
 * @brief 比较Dijkstra和Floyd算法性能
 */
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
    
    cout << "城市对\t\tDijkstra时间(μs)\tFloyd时间(μs)\t距离(km)" << endl;
    cout << "----------------------------------------------------------------" << endl;
    
    for (const auto& [srcCity, destCity] : testPairs) {
        int src = getCityIndex(srcCity);
        int dest = getCityIndex(destCity);
        
        if (src == -1 || dest == -1) {
            cout << "未找到城市: " << srcCity << " 或 " << destCity << endl;
            continue;
        }
        
        // 测试Dijkstra算法
        auto start = chrono::high_resolution_clock::now();
        auto dijkstraPath = dijkstra(src, dest);
        auto end = chrono::high_resolution_clock::now();
        auto dijkstraTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
        double dijkstraDistance = calculatePathDistance(dijkstraPath);
        
        // 测试Floyd算法
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

/**
 * @brief 根据城市名获取索引
 * @param cityName 城市名
 * @return 城市索引，不存在返回-1
 */
int Graph::getCityIndex(const string& cityName) {
    auto it = cityIndexMap.find(cityName);
    return (it != cityIndexMap.end()) ? it->second : -1;
}

/**
 * @brief 根据索引获取城市信息
 * @param index 城市索引
 * @return 城市结构体
 */
City Graph::getCity(int index) {
    if (index >= 0 && index < static_cast<int>(cities.size())) {
        return cities[index];
    }
    return City();
}

/**
 * @brief 获取所有城市名
 * @return 城市名列表
 */
vector<string> Graph::getAllCityNames() {
    vector<string> names;
    for (const auto& city : cities) {
        names.push_back(city.name);
    }
    return names;
}