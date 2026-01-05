#ifndef GRAPH_H
#define GRAPH_H

#include "City.h"
#include <vector>
#include <string>
#include <map>
#include <queue>
using namespace std;

class Graph {
private:
    vector<City> cities;
    vector<vector<double>> adjMatrix;
    vector<vector<int>> next;
    map<string, int> cityIndexMap;
    const double INF = 1e9;
    bool floydComputed;
    
    // 私有算法
    vector<int> dijkstra(int src, int dest);
    vector<int> floydWarshall(int src, int dest);
    void computeFloyd();
    void dfsFindPaths(int current, int dest, int maxDepth,
                     vector<bool>& visited,
                     vector<int>& currentPath,
                     vector<Path>& allPaths,
                     double currentDist);
    void quickSort(vector<Path>& paths, int left, int right);
    
public:
    Graph();
    bool loadFromCSV(const string& filename);
    
    // 核心功能
    bool verifyWuhanCenter();
    pair<vector<int>, double> getShortestPath(int src, int dest, int algorithm);
    vector<Path> getAllPaths(int src, int dest, int maxNodes);
    vector<int> bypassCity(int src, int dest, const string& bypassCity);
    vector<Path> getKShortestPaths(int src, int dest, int k);
    vector<Path> getAllPathsSimple(int src, int dest, int maxNodes);
    
    // 辅助函数
    int getCityIndex(const string& name);
    double calculateDistance(const vector<int>& path);
    void printPath(const vector<int>& path);
    void saveToFile(const vector<Path>& paths, const string& filename);
    void compareAlgorithms(int src, int dest);
    
    // 获取城市信息
    string getCityName(int index) const;
    int getCityCount() const { return cities.size(); }
};

#endif