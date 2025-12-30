#include "TrafficConsultSystem.h"
#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <limits>
#include <cstdlib>
#include <ctime>

using namespace std;

/**
 * @brief 构造函数
 */
TrafficConsultSystem::TrafficConsultSystem() : dataLoaded(false) {}

/**
 * @brief 初始化系统
 */
void TrafficConsultSystem::initialize() {
    printWelcome();
    
    cout << "正在加载城市数据..." << endl;
    if (graph.loadFromCSV(DISTANCE_CSV, ADJACENCY_CSV)) {
        dataLoaded = true;
        cout << "数据加载成功！" << endl;
    } else {
        cout << "数据加载失败，请检查文件是否存在。" << endl;
        cout << "需要的文件: " << DISTANCE_CSV << " 和 " << ADJACENCY_CSV << endl;
    }
}

/**
 * @brief 运行主循环
 */
void TrafficConsultSystem::run() {
    if (!dataLoaded) {
        cout << "系统初始化失败，无法运行。" << endl;
        return;
    }
    
    int choice;
    do {
        showMenu();
        choice = getIntInput("请选择操作", 0, 8);
        
        switch (choice) {
            case OPTION_EXIT:
                cout << "感谢使用，再见！" << endl;
                break;
            case OPTION_VERIFY_CENTER:
                handleVerifyCenter();
                break;
            case OPTION_SHORTEST_PATH:
                handleShortestPath();
                break;
            case OPTION_ALL_PATHS:
                handleAllPaths();
                break;
            case OPTION_BYPASS_PATH:
                handleKBypassPath();
                break;
            case OPTION_K_SHORTEST:
                handleKShortestPaths();
                break;
            case OPTION_COMPARE_ALGORITHMS:
                compareAlgorithms();
                break;
            case OPTION_TEST_PERFORMANCE:
                testPerformance();
                break;
            case OPTION_HELP:
                printHelp();
                break;
            default:
                cout << "无效选择，请重新输入。" << endl;
        }
        
        if (choice != OPTION_EXIT) {
            cout << "\n按回车键继续...";
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cin.get();
        }
    } while (choice != OPTION_EXIT);
}

/**
 * @brief 显示主菜单
 */
void TrafficConsultSystem::showMenu() {
    cout << "\n==========================================" << endl;
    cout << "       交通咨询系统 v1.0" << endl;
    cout << "==========================================" << endl;
    cout << "1. 验证武汉中心位置假设" << endl;
    cout << "2. 查询最短路径（Dijkstra/Floyd算法）" << endl;
    cout << "3. 查询所有可行路径（DFS+快速排序）" << endl;
    cout << "4. 查询绕过特定城市的最短路径" << endl;
    cout << "5. 查询第K短路径（Yen's算法，不依赖遍历）" << endl;
    cout << "6. 算法比较测试（Dijkstra vs Floyd）" << endl;
    cout << "7. 性能测试" << endl;
    cout << "8. 帮助" << endl;
    cout << "0. 退出" << endl;
    cout << "==========================================" << endl;
}

/**
 * @brief 打印欢迎信息
 */
void TrafficConsultSystem::printWelcome() {
    cout << "==========================================" << endl;
    cout << "    欢迎使用交通咨询系统" << endl;
    cout << "==========================================" << endl;
    cout << "系统功能：" << endl;
    cout << "1. 验证武汉中心位置假设" << endl;
    cout << "2. 最短路径查询（支持Dijkstra和Floyd算法）" << endl;
    cout << "3. 所有可行路径查找与快速排序" << endl;
    cout << "4. 绕过特定城市的最短路径" << endl;
    cout << "5. 第K短路径查询（Yen's算法）" << endl;
    cout << "6. 算法性能比较与分析" << endl;
    cout << "==========================================" << endl;
}

/**
 * @brief 打印帮助信息
 */
void TrafficConsultSystem::printHelp() {
    cout << "\n=== 系统帮助 ===" << endl;
    cout << "1. 确保以下文件在当前目录：" << endl;
    cout << "   - city_distances.csv：城市距离数据" << endl;
    cout << "   - city_adjacency.csv：城市邻接关系（可选）" << endl;
    cout << endl;
    cout << "2. 系统功能说明：" << endl;
    cout << "   [功能1] 验证武汉中心位置假设：" << endl;
    cout << "       验证全国其他省会城市（不包括港澳台和海口）到武汉" << endl;
    cout << "       中间不超过2个省会城市是否成立" << endl;
    cout << endl;
    cout << "   [功能2] 最短路径查询：" << endl;
    cout << "       提供Dijkstra和Floyd两种算法，可比较性能" << endl;
    cout << "       Dijkstra：适合单次查询，O(E log V)时间" << endl;
    cout << "       Floyd：适合多次查询，O(V³)预处理，O(1)查询" << endl;
    cout << endl;
    cout << "   [功能3] 所有可行路径查询：" << endl;
    cout << "       使用DFS查找所有路径（最多10个节点）" << endl;
    cout << "       使用快速排序对路径按距离排序" << endl;
    cout << "       结果保存到paths_output.txt文件" << endl;
    cout << endl;
    cout << "   [功能4] 绕过城市的最短路径：" << endl;
    cout << "       计算避开特定城市的最短路径" << endl;
    cout << "       实现方式：临时移除该城市，重新计算最短路径" << endl;
    cout << endl;
    cout << "   [功能5] 第K短路径查询：" << endl;
    cout << "       使用Yen's算法，不依赖遍历所有路径" << endl;
    cout << "       算法特点：O(kn(m + n log n))时间" << endl;
    cout << "       避免DFS遍历可能的内存爆炸问题" << endl;
    cout << endl;
    cout << "3. 城市名称请使用标准名称，如'北京'、'上海'等" << endl;
    cout << "4. 所有路径查询结果会自动保存到paths_output.txt文件" << endl;
}

/**
 * @brief 获取城市输入
 * @param prompt 提示信息
 * @return 城市名称
 */
string TrafficConsultSystem::getCityInput(const string& prompt) {
    string city;
    while (true) {
        cout << prompt << ": ";
        getline(cin, city);
        city = Utils::trim(city);
        
        if (city.empty()) {
            cout << "城市名不能为空，请重新输入。" << endl;
            continue;
        }
        
        if (graph.getCityIndex(city) != -1) {
            return city;
        } else {
            cout << "城市 '" << city << "' 不存在。" << endl;
            cout << "可用的城市有: ";
            auto cities = graph.getAllCityNames();
            for (size_t i = 0; i < min(cities.size(), size_t(5)); i++) {
                cout << cities[i];
                if (i < min(cities.size(), size_t(5)) - 1) cout << ", ";
            }
            if (cities.size() > 5) cout << " 等";
            cout << endl;
        }
    }
}

/**
 * @brief 获取整数输入
 * @param prompt 提示信息
 * @param min 最小值
 * @param max 最大值
 * @return 输入的整数
 */
int TrafficConsultSystem::getIntInput(const string& prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt << " [" << min << "-" << max << "]: ";
        if (!(cin >> value)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "请输入有效的数字。" << endl;
        } else if (value < min || value > max) {
            cout << "输入超出范围，请重新输入。" << endl;
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

/**
 * @brief 获取算法选择
 * @return 算法选择（0-Dijkstra, 1-Floyd）
 */
int TrafficConsultSystem::getAlgorithmChoice() {
    cout << "\n请选择最短路径算法：" << endl;
    cout << "0. Dijkstra算法（适合单次查询）" << endl;
    cout << "1. Floyd算法（适合多次查询）" << endl;
    return getIntInput("请选择算法", 0, 1);
}

/**
 * @brief 功能1：验证武汉中心位置假设
 */
void TrafficConsultSystem::handleVerifyCenter() {
    cout << "\n=== 验证武汉中心位置假设 ===" << endl;
    cout << "假设：其他省会城市到武汉中间不超过2个省会城市" << endl;
    cout << "验证对象：全国其他省会城市（不包括港澳台和海口）" << endl;
    cout << "========================================" << endl;
    
    graph.verifyWuhanCenter(2);
}

/**
 * @brief 功能2：最短路径查询
 */
void TrafficConsultSystem::handleShortestPath() {
    cout << "\n=== 最短路径查询 ===" << endl;
    cout << "支持Dijkstra和Floyd两种算法" << endl;
    
    // 获取城市输入
    string srcCity = getCityInput("请输入起点城市");
    string destCity = getCityInput("请输入终点城市");
    
    int srcIndex = graph.getCityIndex(srcCity);
    int destIndex = graph.getCityIndex(destCity);
    
    // 验证输入
    if (!validateCities(srcCity, destCity, srcIndex, destIndex)) {
        return;
    }
    
    // 获取算法选择
    int algorithm = getAlgorithmChoice();
    string algorithmName = (algorithm == ALGORITHM_DIJKSTRA) ? "Dijkstra" : "Floyd";
    
    cout << "\n正在使用" << algorithmName << "算法计算最短路径..." << endl;
    cout << "从 " << srcCity << " 到 " << destCity << endl;
    cout << "========================================" << endl;
    
    // 计算并显示最短路径
    calculateAndShowShortestPath(srcIndex, destIndex, algorithm);
}

/**
 * @brief 验证城市输入是否有效
 * @param srcCity 起点城市名
 * @param destCity 终点城市名
 * @param srcIndex 起点索引
 * @param destIndex 终点索引
 * @return 有效返回true，否则false
 */
bool TrafficConsultSystem::validateCities(const string& srcCity, const string& destCity, 
                                        int srcIndex, int destIndex) {
    if (srcIndex == -1 || destIndex == -1) {
        cout << "城市不存在，请检查输入。" << endl;
        return false;
    }
    
    if (srcCity == destCity) {
        cout << "起点和终点不能是同一个城市！" << endl;
        return false;
    }
    
    return true;
}

/**
 * @brief 计算并显示最短路径
 * @param srcIndex 起点索引
 * @param destIndex 终点索引
 * @param algorithm 算法选择
 */
void TrafficConsultSystem::calculateAndShowShortestPath(int srcIndex, int destIndex, 
                                                       int algorithm) {
    // 计算最短路径
    auto result = graph.getShortestPath(srcIndex, destIndex, algorithm);
    vector<int> path = result.first;
    double distance = result.second;
    
    if (path.empty()) {
        cout << "无法找到从起点到终点的路径！" << endl;
        return;
    }
    
    // 显示路径详情
    cout << "\n最短路径计算结果：" << endl;
    graph.printPathDetails(path, distance);
    
    // 显示算法信息
    showAlgorithmInfo(algorithm);
    
    // 询问是否比较两种算法
    askForComparison(srcIndex, destIndex);
}

/**
 * @brief 显示算法信息
 * @param algorithm 算法选择
 */
void TrafficConsultSystem::showAlgorithmInfo(int algorithm) {
    cout << "\n算法特点：" << endl;
    if (algorithm == ALGORITHM_DIJKSTRA) {
        cout << "✓ Dijkstra算法：单源最短路径算法" << endl;
        cout << "✓ 时间复杂度：O(E log V)，适合稀疏图" << endl;
        cout << "✓ 空间复杂度：O(V)" << endl;
        cout << "✓ 适合单次查询" << endl;
    } else {
        cout << "✓ Floyd算法：全源最短路径算法" << endl;
        cout << "✓ 时间复杂度：O(V³)预处理，O(1)查询" << endl;
        cout << "✓ 空间复杂度：O(V²)" << endl;
        cout << "✓ 适合多次查询" << endl;
    }
}

/**
 * @brief 询问是否比较算法性能
 * @param srcIndex 起点索引
 * @param destIndex 终点索引
 */
void TrafficConsultSystem::askForComparison(int srcIndex, int destIndex) {
    cout << "\n是否比较两种算法的性能？(y/n): ";
    char compareChoice;
    cin >> compareChoice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (compareChoice == 'y' || compareChoice == 'Y') {
        compareTwoAlgorithms(srcIndex, destIndex);
    }
}

/**
 * @brief 比较两种算法性能
 * @param srcIndex 起点索引
 * @param destIndex 终点索引
 */
void TrafficConsultSystem::compareTwoAlgorithms(int srcIndex, int destIndex) {
    cout << "\n性能比较：" << endl;
    cout << "----------------------------------------" << endl;
    
    // 测试Dijkstra算法
    auto start = chrono::high_resolution_clock::now();
    auto dijkstraPath = graph.dijkstra(srcIndex, destIndex);
    auto end = chrono::high_resolution_clock::now();
    auto dijkstraTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
    double dijkstraDistance = graph.calculatePathDistance(dijkstraPath);
    
    // 测试Floyd算法
    start = chrono::high_resolution_clock::now();
    auto floydPath = graph.floyd(srcIndex, destIndex);
    end = chrono::high_resolution_clock::now();
    auto floydTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
    double floydDistance = graph.calculatePathDistance(floydPath);
    
    cout << "Dijkstra算法：" << endl;
    cout << "  计算时间：" << dijkstraTime << " μs" << endl;
    cout << "  路径长度：" << dijkstraDistance << " km" << endl;
    
    cout << "\nFloyd算法：" << endl;
    cout << "  计算时间：" << floydTime << " μs" << endl;
    cout << "  路径长度：" << floydDistance << " km" << endl;
    
    // 比较结果
    cout << "\n比较结果：" << endl;
    if (abs(dijkstraDistance - floydDistance) < 0.01) {
        cout << "✓ 两种算法找到的路径长度一致" << endl;
    } else {
        cout << "✗ 两种算法找到的路径长度不一致" << endl;
    }
    
    if (dijkstraTime < floydTime) {
        cout << "✓ Dijkstra算法更快，适合单次查询" << endl;
    } else {
        cout << "✓ Floyd算法更快，适合多次查询" << endl;
        cout << "  （注意：Floyd算法首次查询包含预处理时间）" << endl;
    }
}

/**
 * @brief 功能2扩展：所有可行路径查询
 */
void TrafficConsultSystem::handleAllPaths() {
    cout << "\n=== 所有可行路径查询 ===" << endl;
    cout << "使用DFS遍历所有路径，最多10个节点" << endl;
    cout << "使用快速排序对路径按距离排序" << endl;
    
    // 获取城市输入
    string srcCity = getCityInput("请输入起点城市");
    string destCity = getCityInput("请输入终点城市");
    
    int srcIndex = graph.getCityIndex(srcCity);
    int destIndex = graph.getCityIndex(destCity);
    
    // 验证输入
    if (srcIndex == -1 || destIndex == -1) {
        cout << "城市不存在，请检查输入。" << endl;
        return;
    }
    
    if (srcCity == destCity) {
        cout << "起点和终点不能是同一个城市！" << endl;
        return;
    }
    
    // 获取最大节点数
    int maxNodes = getIntInput("请输入最多经过的城市数（1-10）", 1, 10);
    
    cout << "\n正在查找所有路径..." << endl;
    cout << "从 " << srcCity << " 到 " << destCity << endl;
    cout << "最多经过 " << maxNodes << " 个城市" << endl;
    cout << "========================================" << endl;
    
    // 查找所有路径
    auto start = chrono::high_resolution_clock::now();
    auto allPaths = graph.findAllPaths(srcIndex, destIndex, maxNodes);
    auto end = chrono::high_resolution_clock::now();
    auto findDuration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    if (allPaths.empty()) {
        cout << "未找到可行路径" << endl;
        return;
    }
    
    cout << "找到 " << allPaths.size() << " 条路径" << endl;
    
    // 使用快速排序
    start = chrono::high_resolution_clock::now();
    if (!allPaths.empty()) {
        graph.quickSortPaths(allPaths, 0, allPaths.size() - 1);
    }
    end = chrono::high_resolution_clock::now();
    auto sortDuration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    // 显示前5条最短路径
    cout << "\n前5条最短路径：" << endl;
    int showCount = min(5, static_cast<int>(allPaths.size()));
    for (int i = 0; i < showCount; i++) {
        cout << "\n第" << i + 1 << "条: " << allPaths[i].totalDistance << " km" << endl;
        cout << "路径: ";
        graph.printPath(allPaths[i].nodes);
    }
    
    // 保存到文件
    graph.savePathsToFile(allPaths, OUTPUT_FILE);
    
    // 性能统计
    cout << "\n性能统计：" << endl;
    cout << "查找时间（DFS遍历）: " << findDuration << " ms" << endl;
    cout << "排序时间（快速排序）: " << sortDuration << " μs" << endl;
    
    if (!allPaths.empty()) {
        double totalDistance = 0;
        for (const auto& path : allPaths) {
            totalDistance += path.totalDistance;
        }
        cout << "平均路径长度: " << totalDistance / allPaths.size() << " km" << endl;
        cout << "最短路径: " << allPaths[0].totalDistance << " km" << endl;
        cout << "最长路径: " << allPaths.back().totalDistance << " km" << endl;
    }
    
    // 与Dijkstra结果比较
    cout << "\n与Dijkstra算法比较：" << endl;
    auto dijkstraPath = graph.dijkstra(srcIndex, destIndex);
    double dijkstraDist = graph.calculatePathDistance(dijkstraPath);
    
    if (!allPaths.empty()) {
        if (abs(allPaths[0].totalDistance - dijkstraDist) < 0.01) {
            cout << "✓ DFS找到的最短路径与Dijkstra结果一致" << endl;
        } else {
            cout << "✗ DFS找到的最短路径与Dijkstra结果不一致" << endl;
            cout << "  DFS最短: " << allPaths[0].totalDistance << " km" << endl;
            cout << "  Dijkstra: " << dijkstraDist << " km" << endl;
        }
    }
}

/**
 * @brief 功能3：绕过特定城市的最短路径
 */
void TrafficConsultSystem::handleKBypassPath() {
    cout << "\n=== 绕过特定城市的最短路径 ===" << endl;
    cout << "计算避开特定城市的最短路径" << endl;
    
    // 获取城市输入
    string srcCity = getCityInput("请输入起点城市");
    string destCity = getCityInput("请输入终点城市");
    string bypassCity = getCityInput("请输入要绕过的城市");
    
    int srcIndex = graph.getCityIndex(srcCity);
    int destIndex = graph.getCityIndex(destCity);
    int bypassIndex = graph.getCityIndex(bypassCity);
    
    // 验证输入
    if (srcIndex == -1 || destIndex == -1 || bypassIndex == -1) {
        cout << "城市不存在，请检查输入。" << endl;
        return;
    }
    
    if (srcCity == destCity) {
        cout << "起点和终点不能是同一个城市！" << endl;
        return;
    }
    
    cout << "\n正在计算绕过 " << bypassCity << " 的最短路径..." << endl;
    cout << "从 " << srcCity << " 到 " << destCity << endl;
    cout << "========================================" << endl;
    
    // 计算绕过路径
    auto start = chrono::high_resolution_clock::now();
    auto path = graph.findShortestBypass(srcIndex, destIndex, bypassCity);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    double distance = graph.calculatePathDistance(path);
    
    if (path.empty()) {
        cout << "无法找到绕过 " << bypassCity << " 的路径！" << endl;
    } else {
        // 显示绕过后的路径
        graph.printPathDetails(path, distance);
        cout << "\n计算时间: " << duration << " μs" << endl;
        
        // 显示原始最短路径作为比较
        cout << "\n原始最短路径（不绕过）:" << endl;
        auto originalPath = graph.dijkstra(srcIndex, destIndex);
        double originalDist = graph.calculatePathDistance(originalPath);
        cout << "距离: " << originalDist << " km" << endl;
        
        cout << "\n绕行增加的距离: " << (distance - originalDist) << " km" << endl;
    }
}

/**
 * @brief 功能4：第K短路径查询（使用Yen's算法）
 */
void TrafficConsultSystem::handleKShortestPaths() {
    cout << "\n=== 第K短路径查询 ===" << endl;
    cout << "使用Yen's算法，不依赖遍历所有路径" << endl;
    cout << "算法特点：高效查找前K条最短路径" << endl;
    
    // 获取城市输入
    string srcCity = getCityInput("请输入起点城市");
    string destCity = getCityInput("请输入终点城市");
    
    int srcIndex = graph.getCityIndex(srcCity);
    int destIndex = graph.getCityIndex(destCity);
    
    // 验证输入
    if (srcIndex == -1 || destIndex == -1) {
        cout << "城市不存在，请检查输入。" << endl;
        return;
    }
    
    if (srcCity == destCity) {
        cout << "起点和终点不能是同一个城市！" << endl;
        return;
    }
    
    // 获取K值
    int k = getIntInput("请输入K值（1-10）", 1, MAX_K_PATHS);
    
    cout << "\n正在使用Yen's算法查找第" << k << "短路径..." << endl;
    cout << "从 " << srcCity << " 到 " << destCity << endl;
    cout << "========================================" << endl;
    
    // 查找第K短路径
    auto start = chrono::high_resolution_clock::now();
    auto kPaths = graph.findKShortestPathsYen(srcIndex, destIndex, k);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    if (kPaths.empty()) {
        cout << "未找到路径" << endl;
        return;
    }
    
    // 显示找到的路径数量
    if (static_cast<int>(kPaths.size()) < k) {
        cout << "只找到 " << kPaths.size() << " 条路径（不足" << k << "条）" << endl;
    } else {
        cout << "成功找到 " << kPaths.size() << " 条路径" << endl;
    }
    
    // 显示所有找到的路径
    cout << "\n找到的路径（按距离排序）：" << endl;
    for (size_t i = 0; i < kPaths.size(); i++) {
        cout << "\n第" << i + 1 << "短路径: " << kPaths[i].totalDistance << " km" << endl;
        cout << "路径: ";
        graph.printPath(kPaths[i].nodes);
    }
    
    // 与Dijkstra结果比较
    cout << "\n与Dijkstra算法比较：" << endl;
    auto dijkstraPath = graph.dijkstra(srcIndex, destIndex);
    double dijkstraDist = graph.calculatePathDistance(dijkstraPath);
    cout << "Dijkstra最短路径: " << dijkstraDist << " km" << endl;
    
    if (!kPaths.empty()) {
        cout << "Yen's第1短路径: " << kPaths[0].totalDistance << " km" << endl;
        if (abs(kPaths[0].totalDistance - dijkstraDist) < 0.01) {
            cout << "✓ Yen's算法第1短路径与Dijkstra结果一致" << endl;
        } else {
            cout << "✗ Yen's算法第1短路径与Dijkstra结果不一致" << endl;
        }
    }
    
    // 算法信息
    cout << "\n算法信息：" << endl;
    cout << "计算时间: " << duration << " ms" << endl;
    cout << "使用算法: Yen's K最短路径算法" << endl;
    cout << "时间复杂度: O(kn(m + n log n))" << endl;
    cout << "优点: 不依赖遍历所有路径，适合大型图" << endl;
    
    // 与DFS方法比较（可选）
    cout << "\n与DFS遍历方法比较：" << endl;
    if (k <= 5) {  // 只在小K值时比较
        start = chrono::high_resolution_clock::now();
        auto dfsPaths = graph.findKShortestPaths(srcIndex, destIndex, k);
        end = chrono::high_resolution_clock::now();
        auto dfsDuration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
        
        cout << "DFS遍历方法: " << dfsDuration << " ms" << endl;
        cout << "Yen's算法: " << duration << " ms" << endl;
        
        if (duration < dfsDuration) {
            cout << "✓ Yen's算法更快" << endl;
        } else {
            cout << "✗ DFS遍历更快（但可能内存占用更大）" << endl;
        }
    }
}

/**
 * @brief 算法比较测试
 */
void TrafficConsultSystem::compareAlgorithms() {
    cout << "\n=== 算法比较测试 ===" << endl;
    graph.compareAlgorithms();
}

/**
 * @brief 性能测试
 */
void TrafficConsultSystem::testPerformance() {
    cout << "\n=== 性能测试 ===" << endl;
    cout << "测试不同城市对的最短路径计算性能" << endl;
    cout << "========================================" << endl;
    
    // 测试用例
    vector<pair<string, string>> testCases = {
        {"北京", "上海"},     // 短距离，连接良好
        {"广州", "哈尔滨"},   // 长距离，南北跨度
        {"成都", "武汉"},     // 中等距离，中部连接
        {"乌鲁木齐", "海口"}, // 最长距离，对角线
        {"昆明", "沈阳"}      // 中等距离，南北连接
    };
    
    cout << "测试用例\t\tDijkstra(μs)\tFloyd(μs)\t距离(km)" << endl;
    cout << "----------------------------------------------------------------" << endl;
    
    for (const auto& [src, dest] : testCases) {
        int srcIndex = graph.getCityIndex(src);
        int destIndex = graph.getCityIndex(dest);
        
        if (srcIndex == -1 || destIndex == -1) continue;
        
        // 测试Dijkstra算法
        auto start = chrono::high_resolution_clock::now();
        auto dijkstraPath = graph.dijkstra(srcIndex, destIndex);
        auto end = chrono::high_resolution_clock::now();
        auto dijkstraTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
        double dijkstraDist = graph.calculatePathDistance(dijkstraPath);
        
        // 测试Floyd算法
        start = chrono::high_resolution_clock::now();
        auto floydPath = graph.floyd(srcIndex, destIndex);
        end = chrono::high_resolution_clock::now();
        auto floydTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
        double floydDist = graph.calculatePathDistance(floydPath);
        
        // 输出结果
        cout << src << "->" << dest << "\t\t"
             << dijkstraTime << "\t\t"
             << floydTime << "\t\t"
             << (int)dijkstraDist << endl;
    }
    
    cout << "========================================" << endl;
    cout << "\n性能分析：" << endl;
    cout << "1. Dijkstra算法：适合单次查询，时间稳定" << endl;
    cout << "2. Floyd算法：首次查询包含预处理时间，后续查询极快" << endl;
    cout << "3. 距离越远，计算时间可能增加（Dijkstra）" << endl;
    cout << "4. Floyd算法适合需要多次查询的场景" << endl;
}