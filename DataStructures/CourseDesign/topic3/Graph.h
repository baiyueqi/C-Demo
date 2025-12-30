#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <string>
#include <map>
#include <queue>
#include <limits>
#include "City.h"

using namespace std;

class Graph {
private:
    vector<City> cities;               // 城市列表
    vector<vector<double>> adjMatrix;  // 邻接矩阵
    vector<vector<int>> nextMatrix;    // Floyd路径重建矩阵
    map<string, int> cityIndexMap;     // 城市名到索引的映射
    const double INF = numeric_limits<double>::max(); // 无穷大值
    bool floydComputed;                // Floyd算法是否已计算
    
    // 私有辅助函数
    vector<int> reconstructPath(const vector<int>& prev, int dest);
    vector<int> reconstructFloydPath(int src, int dest);
    void computeFloyd();
    
    // DFS辅助函数，用于查找所有路径
    void dfsFindAllPaths(int src, int dest, int maxNodes,
                        vector<bool>& visited,
                        vector<int>& currentPath,
                        double currentDistance,
                        vector<Path>& allPaths);
    
    // K短路径算法相关
    struct CandidatePath {
        vector<int> nodes;      // 路径节点
        double distance;        // 路径距离
        int spurIndex;          // 分歧点索引
        
        bool operator>(const CandidatePath& other) const {
            return distance > other.distance;
        }
    };
    
    // 带约束的Dijkstra算法，用于Yen's算法
    vector<int> dijkstraWithConstraints(int src, int dest,
                                       const vector<pair<int, int>>& forbiddenEdges,
                                       const vector<int>& forbiddenNodes);
    
public:
    // 构造函数和析构函数
    Graph();
    ~Graph();
    
    // 数据加载
    bool loadFromCSV(const string& distanceFile, const string& adjacencyFile = "");
    
    // 基本操作
    int getCityIndex(const string& cityName);
    City getCity(int index);
    int getCityCount() const { return cities.size(); }
    vector<string> getAllCityNames();
    
    // 路径距离计算
    double calculatePathDistance(const vector<int>& path);
    
    // 最短路径算法
    vector<int> dijkstra(int src, int dest);
    vector<int> floyd(int src, int dest);
    pair<vector<int>, double> getShortestPath(int src, int dest, int algorithm = 0);
    
    // 路径查找算法
    vector<Path> findAllPaths(int src, int dest, int maxNodes = 10);     // 功能2：所有路径
    vector<Path> findKShortestPaths(int src, int dest, int k);           // 功能2的K短路径
    vector<Path> findKShortestPathsYen(int src, int dest, int k);        // 功能4：Yen's算法
    vector<int> findShortestBypass(int src, int dest, const string& bypassCity); // 功能3：绕过城市
    void quickSortPaths(vector<Path>& paths, int left, int right);      // 快速排序
    
    // 遍历算法
    vector<int> bfs(int src, int dest);
    
    // 验证功能
    bool verifyWuhanCenter(int maxProvinces = 2);  // 功能1：验证武汉中心位置
    
    // 输出功能
    void printPath(const vector<int>& path);
    void printPathDetails(const vector<int>& path, double distance = -1);
    void savePathsToFile(const vector<Path>& paths, const string& filename);
    
    // 性能测试
    void compareAlgorithms();  // 比较Dijkstra和Floyd算法
};

#endif