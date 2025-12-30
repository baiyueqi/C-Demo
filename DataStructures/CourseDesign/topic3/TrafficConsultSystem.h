//主系统类声明
#ifndef TRAFFICCONSULTSYSTEM_H
#define TRAFFICCONSULTSYSTEM_H

#include "Graph.h"
#include <string>

// 菜单选项枚举
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

class TrafficConsultSystem {
private:
    Graph graph;
    bool dataLoaded;
    
    // 常量定义
    const std::string DISTANCE_CSV = "city_distances.csv";
    const std::string ADJACENCY_CSV = "city_adjacency.csv";
    const std::string OUTPUT_FILE = "paths_output.txt";
    const int MAX_K_PATHS = 20;
    
public:
    TrafficConsultSystem();
    
    // 系统控制
    void initialize();
    void run();
    void showMenu();
    
    // 业务功能
    void handleUserQuery();
    void handleShortestPath();
    void handleAllPaths();
    void handleKBypassPath();
    void handleKShortestPaths();
    
    // 验证功能
    void verifyCenterCity();
    
    // 测试功能
    void testPerformance();
    void testAlgorithms();
    
private:
    // 辅助函数
    void printWelcome();
    void printHelp();
    std::string getCityInput(const std::string& prompt);
    int getIntInput(const std::string& prompt, int min, int max);
};

#endif // TRAFFICCONSULTSYSTEM_H