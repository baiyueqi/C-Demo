#ifndef TRAFFICCONSULTSYSTEM_H
#define TRAFFICCONSULTSYSTEM_H

#include "Graph.h"
#include <string>

enum MenuOption {
    OPTION_EXIT = 0,
    OPTION_SHORTEST_PATH = 1,
    OPTION_ALL_PATHS = 2,
    OPTION_BYPASS_PATH = 3,
    OPTION_K_SHORTEST = 4,
    OPTION_VERIFY_CENTER = 5,
    OPTION_COMPARE_ALGORITHMS = 6,
    OPTION_TEST_PERFORMANCE = 7,
    OPTION_HELP = 8
};

enum AlgorithmOption {
    ALGORITHM_DIJKSTRA = 0,
    ALGORITHM_FLOYD = 1
};

class TrafficConsultSystem {
private:
    Graph graph;
    bool dataLoaded;
    
    const std::string DISTANCE_CSV = "city_distances.csv";
    const std::string ADJACENCY_CSV = "city_adjacency.csv";
    const std::string OUTPUT_FILE = "paths_output.txt";
    const int MAX_K_PATHS = 20;
    
public:
    TrafficConsultSystem();
    
    void initialize();
    void run();
    void showMenu();
    
    void handleShortestPath();
    void handleAllPaths();
    void handleKBypassPath();
    void handleKShortestPaths();
    void verifyCenterCity();
    void testPerformance();
    void testAlgorithms();
    
private:
    void printWelcome();
    void printHelp();
    std::string getCityInput(const std::string& prompt);
    int getIntInput(const std::string& prompt, int min, int max);
    int getAlgorithmChoice();
    
    // 新增的函数
    bool validateCities(const std::string& srcCity, const std::string& destCity, 
                       int srcIndex, int destIndex);
    void calculateAndShowShortestPath(int srcIndex, int destIndex, int algorithm);
    void showAlgorithmInfo(int algorithm);
    void askForComparison(int srcIndex, int destIndex);
    void compareAlgorithms(int srcIndex, int destIndex);
};

#endif