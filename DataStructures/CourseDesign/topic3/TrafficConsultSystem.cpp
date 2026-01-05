#include "TrafficConsultSystem.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <limits>

using namespace std;

TrafficConsultSystem::TrafficConsultSystem() : dataLoaded(false) {}

void TrafficConsultSystem::initialize() {
    cout << "\n================================\n";
    cout << "   交通咨询系统 v1.0\n";
    cout << "================================\n\n";
    
    cout << "正在加载数据..." << endl;
    if (graph.loadFromCSV("city_distances.csv")) {
        dataLoaded = true;
        cout << "数据加载成功！" << endl;
    } else {
        cout << "数据加载失败，请确保 city_distances.csv 文件存在" << endl;
    }
}

void TrafficConsultSystem::showMenu() {
    cout << "\n=== 主菜单 ===\n";
    cout << "1. 验证武汉中心位置\n";
    cout << "2. 查询最短路径（两种算法）\n";
    cout << "3. 绕过城市的最短路径\n";
    cout << "4. 第K短路径\n";
    cout << "0. 退出\n";
    cout << "===============\n";
}

int TrafficConsultSystem::getAlgorithmChoice() {
    cout << "\n请选择算法：\n";
    cout << "0. Dijkstra算法（适合单次查询）\n";
    cout << "1. Floyd算法（适合多次查询）\n";
    cout << "2. 两种算法都使用并比较\n";
    
    int choice;
    while (true) {
        cout << "选择 [0-2]: ";
        cin >> choice;
        if (cin.fail() || choice < 0 || choice > 2) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "输入无效，请重试\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return choice;
        }
    }
}

void TrafficConsultSystem::run() {
    if (!dataLoaded) {
        cout << "系统初始化失败，无法运行" << endl;
        return;
    }
    
    int choice;
    do {
        showMenu();
        cout << "请选择: ";
        cin >> choice;
        cin.ignore();
        
        switch (choice) {
            case 1: handleFunction1(); break;
            case 2: handleFunction2(); break;
            case 3: handleFunction3(); break;
            case 4: handleFunction4(); break;
            case 0: cout << "再见！\n"; break;
            default: cout << "无效选择，请重试\n";
        }
        
        if (choice != 0) {
            cout << "\n按回车继续...";
            cin.get();
        }
    } while (choice != 0);
}

string TrafficConsultSystem::getCityInput(const string& prompt) {
    string city;
    while (true) {
        cout << prompt << ": ";
        getline(cin, city);
        
        if (graph.getCityIndex(city) != -1) {
            return city;
        } else {
            cout << "城市不存在，请重新输入\n";
        }
    }
}

int TrafficConsultSystem::getIntInput(const string& prompt, int min, int max) {
    int value;
    while (true) {
        cout << prompt << " [" << min << "-" << max << "]: ";
        cin >> value;
        if (cin.fail() || value < min || value > max) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "输入无效，请重试\n";
        } else {
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            return value;
        }
    }
}

void TrafficConsultSystem::handleFunction1() {
    cout << "\n=== 功能1：验证武汉中心位置 ===\n";
    cout << "验证其他省会到武汉不超过2个省会...\n";
    
    if (graph.verifyWuhanCenter()) {
        cout << "\n✓ 假设成立：武汉确实处于中心位置\n";
        cout << "（这也解释了为何疫情传播如此广泛）\n";
    } else {
        cout << "\n✗ 假设不成立\n";
    }
}

void TrafficConsultSystem::handleFunction2() {
    cout << "\n=== 功能2：最短路径查询 ===\n";
    cout << "支持Dijkstra和Floyd两种算法\n";
    
    string src = getCityInput("起点城市");
    string dest = getCityInput("终点城市");
    
    int srcIdx = graph.getCityIndex(src);
    int destIdx = graph.getCityIndex(dest);
    
    if (srcIdx == -1 || destIdx == -1) {
        cout << "城市不存在\n";
        return;
    }
    
    if (src == dest) {
        cout << "起点和终点不能相同\n";
        return;
    }
    
    int algorithmChoice = getAlgorithmChoice();
    
    cout << "\n从 " << src << " 到 " << dest << endl;
    cout << "================================\n";
    
    if (algorithmChoice == 0 || algorithmChoice == 1) {
        auto [path, dist] = graph.getShortestPath(srcIdx, destIdx, algorithmChoice);
        
        if (path.empty()) {
            cout << "无法找到路径\n";
            return;
        }
        
        cout << "\n最短路径结果：\n";
        cout << "路径: ";
        graph.printPath(path);
        cout << "总距离: " << fixed << setprecision(2) << dist << " km\n";
        
    } else if (algorithmChoice == 2) {
        graph.compareAlgorithms(srcIdx, destIdx);
    }
    
    // 查询可行路径（最多10个节点）
    cout << "\n================================\n";
    cout << "查找可行路径（最多10个节点）...\n";
    
    auto start = chrono::high_resolution_clock::now();
    auto allPaths = graph.getAllPaths(srcIdx, destIdx, 10);
    auto end = chrono::high_resolution_clock::now();
    auto duration = chrono::duration_cast<chrono::milliseconds>(end - start);
    
    if (allPaths.empty()) {
        cout << "未找到可行路径\n";
        return;
    }
    
    cout << "找到 " << allPaths.size() << " 条路径\n";
    cout << "查找时间: " << duration.count() << " ms\n";
    
    // 保存到文件
    graph.saveToFile(allPaths, "paths_output.txt");
    
    // 显示所有找到的路径
    cout << "\n找到的路径（按距离排序）：\n";
    for (size_t i = 0; i < allPaths.size(); i++) {
        cout << i+1 << ". " << allPaths[i].totalDistance << " km (" 
             << allPaths[i].nodes.size() << "个城市): ";
        graph.printPath(allPaths[i].nodes);
    }
    
    // 与算法结果比较
    cout << "\n与Dijkstra算法结果比较：\n";
    auto [dijkstraPath, dijkstraDist] = graph.getShortestPath(srcIdx, destIdx, 0);
    
    if (!allPaths.empty() && abs(allPaths[0].totalDistance - dijkstraDist) < 0.01) {
        cout << "✓ 找到的最短路径与Dijkstra结果一致\n";
    } else {
        cout << "⚠ 注意：找到的最短路径与Dijkstra结果不同\n";
        if (!allPaths.empty()) {
            cout << "  当前最短: " << allPaths[0].totalDistance << " km\n";
        }
        cout << "  Dijkstra: " << dijkstraDist << " km\n";
        
        // 将Dijkstra路径添加到结果中
        if (!dijkstraPath.empty()) {
            bool alreadyExists = false;
            for (const auto& path : allPaths) {
                if (path.nodes == dijkstraPath) {
                    alreadyExists = true;
                    break;
                }
            }
            
            if (!alreadyExists) {
                Path dijkstraPathObj;
                dijkstraPathObj.nodes = dijkstraPath;
                dijkstraPathObj.totalDistance = dijkstraDist;
                dijkstraPathObj.nodeCount = dijkstraPath.size();
                allPaths.insert(allPaths.begin(), dijkstraPathObj);
                
                // 重新排序
                graph.saveToFile(allPaths, "paths_output.txt");
                cout << "  ✓ 已将Dijkstra路径添加到结果中\n";
            }
        }
    }
}

void TrafficConsultSystem::handleFunction3() {
    cout << "\n=== 功能3：绕过城市最短路径 ===\n";
    
    string src = getCityInput("起点城市");
    string dest = getCityInput("终点城市");
    string bypass = getCityInput("要绕过的城市");
    
    int srcIdx = graph.getCityIndex(src);
    int destIdx = graph.getCityIndex(dest);
    
    if (srcIdx == -1 || destIdx == -1) {
        cout << "城市不存在\n";
        return;
    }
    
    cout << "\n计算绕过 " << bypass << " 的最短路径...\n";
    auto path = graph.bypassCity(srcIdx, destIdx, bypass);
    
    if (path.empty()) {
        cout << "无可行路径\n";
    } else {
        cout << "路径: ";
        graph.printPath(path);
        double dist = graph.calculateDistance(path);
        cout << "距离: " << dist << " km\n";
    }
}

void TrafficConsultSystem::handleFunction4() {
    cout << "\n=== 功能4：第K短路径 ===\n";
    
    string src = getCityInput("起点城市");
    string dest = getCityInput("终点城市");
    int k = getIntInput("K值", 1, 10);
    
    int srcIdx = graph.getCityIndex(src);
    int destIdx = graph.getCityIndex(dest);
    
    if (srcIdx == -1 || destIdx == -1) {
        cout << "城市不存在\n";
        return;
    }
    
    cout << "\n查找前" << k << "短路径...\n";
    auto paths = graph.getKShortestPaths(srcIdx, destIdx, k);
    
    if (paths.empty()) {
        cout << "未找到路径\n";
    } else {
        cout << "找到 " << paths.size() << " 条路径:\n";
        for (size_t i = 0; i < paths.size(); i++) {
            cout << i+1 << ". " << paths[i].totalDistance << " km: ";
            graph.printPath(paths[i].nodes);
        }
    }
}