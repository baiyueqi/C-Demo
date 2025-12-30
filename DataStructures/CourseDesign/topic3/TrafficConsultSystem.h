#ifndef TRAFFICCONSULTSYSTEM_H
#define TRAFFICCONSULTSYSTEM_H

#include "Graph.h"
#include <string>

using namespace std;

// 菜单选项枚举
enum MenuOption {
    OPTION_EXIT = 0,               // 退出系统
    OPTION_VERIFY_CENTER = 1,      // 功能1：验证武汉中心位置
    OPTION_SHORTEST_PATH = 2,      // 功能2：最短路径查询
    OPTION_ALL_PATHS = 3,          // 功能2扩展：所有路径查询
    OPTION_BYPASS_PATH = 4,        // 功能3：绕过城市路径
    OPTION_K_SHORTEST = 5,         // 功能4：第K短路径（Yen's算法）
    OPTION_COMPARE_ALGORITHMS = 6, // 算法比较
    OPTION_TEST_PERFORMANCE = 7,   // 性能测试
    OPTION_HELP = 8                // 帮助信息
};

// 算法选择枚举
enum AlgorithmOption {
    ALGORITHM_DIJKSTRA = 0,        // Dijkstra算法
    ALGORITHM_FLOYD = 1            // Floyd算法
};

class TrafficConsultSystem {
private:
    Graph graph;                    // 图数据结构
    bool dataLoaded;                // 数据是否加载成功
    
    // 常量定义
    const string DISTANCE_CSV = "city_distances.csv";
    const string ADJACENCY_CSV = "city_adjacency.csv";
    const string OUTPUT_FILE = "paths_output.txt";
    const int MAX_K_PATHS = 10;     // 最大K值
    
public:
    TrafficConsultSystem();
    
    // 系统控制
    void initialize();              // 初始化系统
    void run();                     // 运行主循环
    void showMenu();                // 显示主菜单
    
    // 业务功能（对应题目要求）
    void handleVerifyCenter();      // 功能1：验证武汉中心位置
    void handleShortestPath();      // 功能2：最短路径查询
    void handleAllPaths();          // 功能2扩展：所有路径查询
    void handleKBypassPath();       // 功能3：绕过城市路径
    void handleKShortestPaths();    // 功能4：第K短路径
    
    // 测试功能
    void testPerformance();         // 性能测试
    void compareAlgorithms();       // 算法比较
    
private:
    // 辅助函数
    void printWelcome();            // 打印欢迎信息
    void printHelp();               // 打印帮助信息
    string getCityInput(const string& prompt);  // 获取城市输入
    int getIntInput(const string& prompt, int min, int max);  // 获取整数输入
    int getAlgorithmChoice();       // 获取算法选择
    
    // 功能2的辅助函数（拆解长函数）
    bool validateCities(const string& srcCity, const string& destCity, 
                       int srcIndex, int destIndex);  // 验证城市输入
    void calculateAndShowShortestPath(int srcIndex, int destIndex, 
                                     int algorithm);  // 计算并显示最短路径
    void showAlgorithmInfo(int algorithm);           // 显示算法信息
    void askForComparison(int srcIndex, int destIndex);  // 询问是否比较算法
    void compareTwoAlgorithms(int srcIndex, int destIndex);  // 比较两种算法
};

#endif