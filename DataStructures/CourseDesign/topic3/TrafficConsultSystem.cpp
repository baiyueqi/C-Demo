#include "TrafficConsultSystem.h"
#include "Utils.h"
#include <iostream>
#include <iomanip>
#include <chrono>
#include <limits>
#include <cstdlib>
#include <ctime>

using namespace std;

TrafficConsultSystem::TrafficConsultSystem() : dataLoaded(false) {}

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
            case OPTION_VERIFY_CENTER:
                verifyCenterCity();
                break;
            case OPTION_COMPARE_ALGORITHMS:
                graph.compareAlgorithms();
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

void TrafficConsultSystem::showMenu() {
    cout << "\n==========================================" << endl;
    cout << "       交通咨询系统 v1.0" << endl;
    cout << "==========================================" << endl;
    cout << "1. 查询最短路径（支持两种算法）" << endl;
    cout << "2. 查询所有可行路径（最多10个城市）" << endl;
    cout << "3. 查询绕过特定城市的最短路径" << endl;
    cout << "4. 查询第K短路径" << endl;
    cout << "5. 验证武汉中心位置假设" << endl;
    cout << "6. 算法比较测试" << endl;
    cout << "7. 性能测试" << endl;
    cout << "8. 帮助" << endl;
    cout << "0. 退出" << endl;
    cout << "==========================================" << endl;
}

void TrafficConsultSystem::printWelcome() {
    cout << "==========================================" << endl;
    cout << "    欢迎使用交通咨询系统" << endl;
    cout << "==========================================" << endl;
    cout << "系统功能：" << endl;
    cout << "1. 最短路径查询（支持Dijkstra和Floyd算法）" << endl;
    cout << "2. 所有可行路径查找与排序" << endl;
    cout << "3. 绕过特定城市的最短路径" << endl;
    cout << "4. 第K短路径查询" << endl;
    cout << "5. 武汉中心位置验证" << endl;
    cout << "6. 算法性能比较" << endl;
    cout << "==========================================" << endl;
}

void TrafficConsultSystem::printHelp() {
    cout << "\n=== 系统帮助 ===" << endl;
    cout << "1. 确保city_distances.csv文件在当前目录" << endl;
    cout << "2. 城市名称请使用标准名称，如'北京'、'上海'等" << endl;
    cout << "3. 所有路径结果会自动保存到paths_output.txt文件" << endl;
    cout << "4. 系统支持以下功能：" << endl;
    cout << "   - 最短路径：支持Dijkstra和Floyd两种算法" << endl;
    cout << "   - 所有路径：查找所有可行路径并快速排序" << endl;
    cout << "   - 绕过城市：计算避开特定城市的最短路径" << endl;
    cout << "   - 第K短路径：查找第K短路径" << endl;
    cout << "   - 武汉中心验证：验证其他省会到武汉的中间城市数" << endl;
    cout << "   - 算法比较：比较Dijkstra和Floyd算法性能" << endl;
    cout << "\n算法说明：" << endl;
    cout << "- Dijkstra算法：适合单次查询，计算速度快" << endl;
    cout << "- Floyd算法：首次计算较慢，但后续查询极快" << endl;
}

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

int TrafficConsultSystem::getAlgorithmChoice() {
    cout << "\n请选择最短路径算法：" << endl;
    cout << "0. Dijkstra算法（推荐单次查询）" << endl;
    cout << "1. Floyd算法（推荐多次查询）" << endl;
    return getIntInput("请选择算法", 0, 1);
}

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

void TrafficConsultSystem::handleShortestPath() {
    cout << "\n=== 最短路径查询 ===" << endl;
    
    string srcCity = getCityInput("请输入起点城市");
    string destCity = getCityInput("请输入终点城市");
    
    int srcIndex = graph.getCityIndex(srcCity);
    int destIndex = graph.getCityIndex(destCity);
    
    if (!validateCities(srcCity, destCity, srcIndex, destIndex)) {
        return;
    }
    
    int algorithm = getAlgorithmChoice();
    string algorithmName = (algorithm == ALGORITHM_DIJKSTRA) ? "Dijkstra" : "Floyd";
    
    cout << "\n正在使用" << algorithmName << "算法计算最短路径..." << endl;
    cout << "从 " << srcCity << " 到 " << destCity << endl;
    cout << "========================================" << endl;
    
    calculateAndShowShortestPath(srcIndex, destIndex, algorithm);
}

void TrafficConsultSystem::calculateAndShowShortestPath(int srcIndex, int destIndex, 
                                                       int algorithm) {
    auto result = graph.getShortestPath(srcIndex, destIndex, algorithm);
    vector<int> path = result.first;
    double distance = result.second;
    
    if (path.empty()) {
        cout << "无法找到路径！" << endl;
        return;
    }
    
    cout << "\n最短路径计算结果：" << endl;
    graph.printPathDetails(path, distance);
    
    showAlgorithmInfo(algorithm);
    askForComparison(srcIndex, destIndex);
}

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

void TrafficConsultSystem::askForComparison(int srcIndex, int destIndex) {
    cout << "\n是否比较两种算法的性能？(y/n): ";
    char compareChoice;
    cin >> compareChoice;
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    
    if (compareChoice == 'y' || compareChoice == 'Y') {
        compareAlgorithms(srcIndex, destIndex);
    }
}

void TrafficConsultSystem::compareAlgorithms(int srcIndex, int destIndex) {
    cout << "\n性能比较：" << endl;
    cout << "----------------------------------------" << endl;
    
    auto start = chrono::high_resolution_clock::now();
    auto dijkstraPath = graph.dijkstra(srcIndex, destIndex);
    auto end = chrono::high_resolution_clock::now();
    auto dijkstraTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    start = chrono::high_resolution_clock::now();
    auto floydPath = graph.floyd(srcIndex, destIndex);
    end = chrono::high_resolution_clock::now();
    auto floydTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    cout << "Dijkstra算法：" << endl;
    cout << "  计算时间：" << dijkstraTime << " μs" << endl;
    cout << "  路径长度：" << graph.calculatePathDistance(dijkstraPath) << " km" << endl;
    
    cout << "\nFloyd算法：" << endl;
    cout << "  计算时间：" << floydTime << " μs" << endl;
    cout << "  路径长度：" << graph.calculatePathDistance(floydPath) << " km" << endl;
    
    if (dijkstraTime < floydTime) {
        cout << "\n✓ Dijkstra算法更快，适合单次查询" << endl;
    } else {
        cout << "\n✓ Floyd算法更快，适合多次查询" << endl;
    }
}

void TrafficConsultSystem::handleAllPaths() {
    cout << "\n=== 所有可行路径查询 ===" << endl;
    
    string srcCity = getCityInput("请输入起点城市");
    string destCity = getCityInput("请输入终点城市");
    
    int srcIndex = graph.getCityIndex(srcCity);
    int destIndex = graph.getCityIndex(destCity);
    
    if (srcIndex == -1 || destIndex == -1) {
        cout << "城市不存在，请检查输入。" << endl;
        return;
    }
    
    if (srcCity == destCity) {
        cout << "起点和终点不能是同一个城市！" << endl;
        return;
    }
    
    int maxNodes = getIntInput("请输入最多经过的城市数（1-10）", 1, 10);
    
    cout << "\n正在查找所有路径..." << endl;
    cout << "从 " << srcCity << " 到 " << destCity << endl;
    cout << "最多经过 " << maxNodes << " 个城市" << endl;
    cout << "========================================" << endl;
    
    auto start = chrono::high_resolution_clock::now();
    auto allPaths = graph.findAllPaths(srcIndex, destIndex, maxNodes);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    if (allPaths.empty()) {
        cout << "未找到可行路径" << endl;
        return;
    }
    
    cout << "找到 " << allPaths.size() << " 条路径" << endl;
    
    start = chrono::high_resolution_clock::now();
    if (!allPaths.empty()) {
        graph.quickSortPaths(allPaths, 0, allPaths.size() - 1);
    }
    end = chrono::high_resolution_clock::now();
    auto sortDuration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    cout << "\n前5条最短路径：" << endl;
    int showCount = min(5, static_cast<int>(allPaths.size()));
    for (int i = 0; i < showCount; i++) {
        cout << "\n第" << i + 1 << "条: " << allPaths[i].totalDistance << " km" << endl;
        graph.printPath(allPaths[i].nodes);
    }
    
    graph.savePathsToFile(allPaths, OUTPUT_FILE);
    
    cout << "\n性能统计：" << endl;
    cout << "查找时间: " << duration << " ms" << endl;
    cout << "快速排序时间: " << sortDuration << " μs" << endl;
    if (!allPaths.empty()) {
        double totalDistance = 0;
        for (const auto& path : allPaths) {
            totalDistance += path.totalDistance;
        }
        cout << "平均每条路径长度: " << totalDistance / allPaths.size() << " km" << endl;
    }
}

void TrafficConsultSystem::handleKBypassPath() {
    cout << "\n=== 绕过特定城市的最短路径 ===" << endl;
    
    string srcCity = getCityInput("请输入起点城市");
    string destCity = getCityInput("请输入终点城市");
    string bypassCity = getCityInput("请输入要绕过的城市");
    
    int srcIndex = graph.getCityIndex(srcCity);
    int destIndex = graph.getCityIndex(destCity);
    int bypassIndex = graph.getCityIndex(bypassCity);
    
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
    
    auto start = chrono::high_resolution_clock::now();
    auto path = graph.findShortestBypass(srcIndex, destIndex, bypassCity);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::microseconds>(end - start).count();
    
    double distance = graph.calculatePathDistance(path);
    
    if (path.empty()) {
        cout << "无法找到绕过 " << bypassCity << " 的路径！" << endl;
    } else {
        graph.printPathDetails(path, distance);
        cout << "计算时间: " << duration << " μs" << endl;
    }
}

void TrafficConsultSystem::handleKShortestPaths() {
    cout << "\n=== 第K短路径查询 ===" << endl;
    
    string srcCity = getCityInput("请输入起点城市");
    string destCity = getCityInput("请输入终点城市");
    
    int srcIndex = graph.getCityIndex(srcCity);
    int destIndex = graph.getCityIndex(destCity);
    
    if (srcIndex == -1 || destIndex == -1) {
        cout << "城市不存在，请检查输入。" << endl;
        return;
    }
    
    if (srcCity == destCity) {
        cout << "起点和终点不能是同一个城市！" << endl;
        return;
    }
    
    int k = getIntInput("请输入K值（1-20）", 1, MAX_K_PATHS);
    
    cout << "\n正在查找第" << k << "短路径..." << endl;
    cout << "从 " << srcCity << " 到 " << destCity << endl;
    cout << "========================================" << endl;
    
    auto start = chrono::high_resolution_clock::now();
    auto kPaths = graph.findKShortestPaths(srcIndex, destIndex, k);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    if (kPaths.empty()) {
        cout << "未找到足够多的路径" << endl;
        return;
    }
    
    cout << "\n前" << kPaths.size() << "条最短路径：" << endl;
    for (size_t i = 0; i < kPaths.size(); i++) {
        cout << "\n第" << i + 1 << "条: " << kPaths[i].totalDistance << " km" << endl;
        graph.printPath(kPaths[i].nodes);
    }
    
    cout << "\n计算时间: " << duration << " ms" << endl;
}

void TrafficConsultSystem::verifyCenterCity() {
    cout << "\n=== 验证武汉中心位置假设 ===" << endl;
    cout << "假设：其他省会到武汉最多经过2个省会城市" << endl;
    cout << "========================================" << endl;
    
    graph.verifyWuhanCenter(2);
}

void TrafficConsultSystem::testPerformance() {
    cout << "\n=== 性能测试 ===" << endl;
    cout << "测试不同城市对的最短路径计算性能" << endl;
    cout << "========================================" << endl;
    
    vector<pair<string, string>> testCases = {
        {"北京", "上海"},
        {"广州", "哈尔滨"},
        {"成都", "武汉"},
        {"乌鲁木齐", "海口"},
        {"昆明", "沈阳"}
    };
    
    cout << "测试用例\tDijkstra(μs)\tFloyd(μs)" << endl;
    cout << "----------------------------------------" << endl;
    
    for (const auto& [src, dest] : testCases) {
        int srcIndex = graph.getCityIndex(src);
        int destIndex = graph.getCityIndex(dest);
        
        if (srcIndex == -1 || destIndex == -1) continue;
        
        auto start = chrono::high_resolution_clock::now();
        graph.dijkstra(srcIndex, destIndex);
        auto end = chrono::high_resolution_clock::now();
        auto dijkstraTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
        
        start = chrono::high_resolution_clock::now();
        graph.floyd(srcIndex, destIndex);
        end = chrono::high_resolution_clock::now();
        auto floydTime = chrono::duration_cast<chrono::microseconds>(end - start).count();
        
        cout << src << "->" << dest << "\t\t"
             << dijkstraTime << "\t\t"
             << floydTime << endl;
    }
    
    cout << "========================================" << endl;
}

void TrafficConsultSystem::testAlgorithms() {
    graph.compareAlgorithms();
}