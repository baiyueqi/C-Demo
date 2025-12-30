#ifndef GRAPH_H
#define GRAPH_H

#include <vector>
#include <string>
#include <map>
#include <queue>
#include <limits>
#include "City.h"

class Graph {
private:
    std::vector<City> cities;               // 城市列表
    std::vector<std::vector<double>> adjMatrix;  // 邻接矩阵
    std::map<std::string, int> cityIndexMap; // 城市名->索引映射
    const double INF = std::numeric_limits<double>::max(); // 定义INF常量
    
    // 内部辅助函数
    double calculatePathDistance(const std::vector<int>& path);
    std::vector<int> reconstructPath(const std::vector<int>& prev, int dest);
    
public:
    Graph();
    ~Graph();
    
    // 数据加载
    bool loadFromCSV(const std::string& distanceFile, const std::string& adjacencyFile = "");
    
    // 基本操作
    int getCityIndex(const std::string& cityName);
    City getCity(int index);
    int getCityCount() const { return cities.size(); }
    std::vector<std::string> getAllCityNames();
    
    // 算法实现
    std::vector<int> dijkstra(int src, int dest);
    std::vector<std::vector<double>> floyd();
    std::vector<Path> findAllPaths(int src, int dest, int maxNodes = 10);
    std::vector<Path> findKShortestPaths(int src, int dest, int k);
    std::vector<int> findShortestBypass(int src, int dest, const std::string& bypassCity);
    void quickSortPaths(std::vector<Path>& paths, int left, int right);
    
    // 遍历算法
    std::vector<int> bfs(int src, int dest);
    
    // 验证功能
    bool verifyWuhanCenter(int maxProvinces = 2);
    
    // 输出功能
    void printPath(const std::vector<int>& path);
    void printPathDetails(const std::vector<int>& path);
    void savePathsToFile(const std::vector<Path>& paths, const std::string& filename);
    
    // 性能测试
    void compareAlgorithms();
    
private:
    // 深度优先搜索辅助函数
    void dfsFindAllPaths(int src, int dest, int maxNodes,
                         std::vector<bool>& visited,
                         std::vector<int>& currentPath,
                         double currentDistance,
                         std::vector<Path>& allPaths);
};

#endif // GRAPH_H