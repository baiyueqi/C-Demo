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
    std::vector<City> cities;
    std::vector<std::vector<double>> adjMatrix;
    std::vector<std::vector<int>> nextMatrix;
    std::map<std::string, int> cityIndexMap;
    const double INF = std::numeric_limits<double>::max();
    bool floydComputed;
    
    std::vector<int> reconstructPath(const std::vector<int>& prev, int dest);
    std::vector<int> reconstructFloydPath(int src, int dest);
    void computeFloyd();
    void dfsFindAllPaths(int src, int dest, int maxNodes,
                        std::vector<bool>& visited,
                        std::vector<int>& currentPath,
                        double currentDistance,
                        std::vector<Path>& allPaths);
    
public:
    Graph();
    ~Graph();
    
    bool loadFromCSV(const std::string& distanceFile, const std::string& adjacencyFile = "");
    int getCityIndex(const std::string& cityName);
    City getCity(int index);
    int getCityCount() const { return cities.size(); }
    std::vector<std::string> getAllCityNames();
    
    double calculatePathDistance(const std::vector<int>& path);
    
    std::vector<int> dijkstra(int src, int dest);
    std::vector<int> floyd(int src, int dest);
    std::pair<std::vector<int>, double> getShortestPath(int src, int dest, int algorithm = 0);
    
    std::vector<Path> findAllPaths(int src, int dest, int maxNodes = 10);
    std::vector<Path> findKShortestPaths(int src, int dest, int k);
    std::vector<int> findShortestBypass(int src, int dest, const std::string& bypassCity);
    void quickSortPaths(std::vector<Path>& paths, int left, int right);
    
    std::vector<int> bfs(int src, int dest);
    bool verifyWuhanCenter(int maxProvinces = 2);
    
    void printPath(const std::vector<int>& path);
    void printPathDetails(const std::vector<int>& path, double distance = -1);
    void savePathsToFile(const std::vector<Path>& paths, const std::string& filename);
    void compareAlgorithms();
};

#endif