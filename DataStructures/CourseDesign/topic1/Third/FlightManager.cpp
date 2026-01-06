#include "FlightManager.h"
#include <iostream>
#include <fstream>
#include <iomanip>

using namespace std;


// 加载航班数据
void FlightManager::loadFlights(const std::string& filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        std::cout << "无法打开航班文件\n";
        return;
    }

    flights.clear();

    Flight f;
    std::string statusStr;

    while (fin >> f.flightId
               >> f.from
               >> f.to
               >> f.departTime
               >> f.arriveTime
               >> f.seats
               >> f.price
               >> statusStr) {

        // 解析航班状态
        if (statusStr == "正常") {
            f.status = FlightStatus::NORMAL;
            f.delayMinutes = 0;
        } else if (statusStr == "延误") {
            f.status = FlightStatus::DELAYED;
            f.delayMinutes = 0; // 初始为 0，管理员再改
        } else if (statusStr == "取消") {
            f.status = FlightStatus::CANCELLED;
            f.delayMinutes = 0;
        } else {
            // 非法状态，跳过该航班
            continue;
        }

        flights.push_back(f);
    }

    fin.close();
    cout << "已加载航班数量：" << flights.size() << endl;
      
    // 构建航班图
    buildFlightGraph();
}

// 显示所有航班，包含时间信息
void FlightManager::showAllFlights() {
    if (flights.empty()) {
        cout << "没有航班数据\n";
        return;
    }

    cout << "\n";
    cout << "+----------+---------------+----------+----------+--------+--------+-------+\n";
    cout << "|✈️  航班号 | 出发->到达 | 起飞时间 | 到达时间 |  票价  |  座位  |   状态   |\n";
    cout << "+----------+---------------+----------+----------+--------+--------+-------+\n";
    
    for (auto& f : flights) {
        string route = f.from + "->" + f.to;
        string status;
        
        if (f.status == FlightStatus::NORMAL)
            status = "正常";
        else if (f.status == FlightStatus::DELAYED)
            status = "延误" + to_string(f.delayMinutes) + "分";
        else
            status = "取消";
        
        // 使用 printf 保持对齐
        printf("| %-8s | %-13s |  %-7s |  %-7s | %-6d | %-6d | %-10s |\n",
               f.flightId.c_str(),
               route.c_str(),
               f.departTime.c_str(),
               f.arriveTime.c_str(),
               f.price,
               f.seats,
               status.c_str());
    }
    
    cout <<"+----------+---------------+----------+----------+--------+--------+-------+\n";
}

// 按城市查询
void FlightManager::searchByCity() {
    string from, to;
    cout << "请输入出发城市：";
    cin >> from;
    cout << "请输入到达城市：";
    cin >> to;

    if (from.empty() || to.empty()) {
        cout << "城市名称不能为空！\n";
        return;
    }
    
    if (from == to) {
        cout << "出发和到达城市不能相同！\n";
        return;
    }

    vector<Flight*> results;// 存储查询结果指针
    // 查找符合条件的航班
    for (auto& f : flights) {
        if (f.from == from && f.to == to &&
            f.status != FlightStatus::CANCELLED) {
            results.push_back(&f);
        }
    }

    if (results.empty()) {
        cout << "\n┌─────────────────────────────────────────────┐\n";
        cout << "│            ⚠️ 查询结果通知                   │\n";
        cout << "├─────────────────────────────────────────────┤\n";
        cout << "│ 未找到从 " << setw(4) << left << from << " 到 " << setw(4) << left << to << " 的直飞航班            │\n";
        cout << "└─────────────────────────────────────────────┘\n";
        
        cout << "是否推荐中转方案？(y/n): ";
        char choice;
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            recommendTransfer();
        }
        return;
    }

    // 精美表格输出
    cout << "\n";
    cout << "┌─────────────────────────────────────────────────────────────────────────────────┐\n";
    cout << "│                          ✈️  航班查询结果： " << setw(4) << left << from 
         << " → " << setw(4) << left << to << "                          │\n";
    cout << "├──────────┬────────────┬────────────┬────────────┬────────────┬──────────────────┤\n";
    cout << "│  航班号  │  起飞时间  │  到达时间  │    票价    │    座位    │      状态        │\n";
    cout << "├──────────┼────────────┼────────────┼────────────┼────────────┼──────────────────┤\n";
    
    for (auto f : results) {
        string status;
        
        if (f->status == FlightStatus::NORMAL)
            status = "✅ 正常";
        else if (f->status == FlightStatus::DELAYED)
            status = "⚠️ 延误" + to_string(f->delayMinutes) + "分";
        
        printf("│ %-8s │   %-8s │   %-8s │  ¥%-8d │  %-8d  │  %-12s       │\n",
               f->flightId.c_str(),
               f->departTime.c_str(),
               f->arriveTime.c_str(),
               f->price,
               f->seats,
               status.c_str());
    }
    
    cout << "├──────────┴────────────┴────────────┴────────────┴────────────┴──────────────────┤\n";
    printf("│                             共找到 %-3d 个航班                                   │\n", results.size());
    cout << "└─────────────────────────────────────────────────────────────────────────────────┘\n";
}

// 实时购票
void FlightManager::buyTicket() {
    string id, user;
    cout << "请输入航班号：";
    cin >> id;
    
    Flight* f = findFlightById(id);
    if (!f) {
        cout << "航班不存在\n";
        return;
    }

    // 显示航班信息确认
    cout << "\n============== 确认购票 ==============\n";
    cout << "航班号: " << f->flightId << endl;
    cout << "航线: " << f->from << " -> " << f->to << endl;
    cout << "起飞时间: " << f->departTime << endl;
    cout << "到达时间: " << f->arriveTime << endl;
    cout << "票价: " << f->price << " 元" << endl;
    cout << "状态: ";
    
    if (f->status == FlightStatus::NORMAL) {
        cout << "正常";
    } else if (f->status == FlightStatus::DELAYED) {
        cout << "延误 (" << f->delayMinutes << "分钟)";
        cout << "\n提示：该航班已延误，确定继续购票吗？(y/n): ";
        char confirm;
        cin >> confirm;
        if (confirm != 'y' && confirm != 'Y') {
            cout << "购票取消\n";
            return;
        }
    } else {
        cout << "取消\n";
        cout << "航班已取消，无法购票\n";
        return;
    }
    cout << endl;

    cout << "请输入用户名：";
    cin >> user;

    if (f->seats > 0) {
        f->seats--;
        cout << "购票成功！" << endl;
        cout << "航班 " << f->flightId << " 的起飞时间是 " 
             << f->departTime << "，请提前到达机场\n";
    } else {
        cout << "该航班已售罄\n";
        cout << "是否加入预约队列？(y/n): ";
        char choice;
        cin >> choice;
        if (choice == 'y' || choice == 'Y') {
            addReservation(*f, user);
            cout << "已加入预约队列，当前排队号: " 
                 << f->waitQueue.size() << endl;
        } else {
            cout << "已取消预约\n";
        }
    }
}

// 添加预约(Flight.h)
void FlightManager::addReservation(Flight& flight, const string& user) {
    static int orderCounter = 1;//静态局部变量
    flight.waitQueue.push(
        Reservation(user, orderCounter++)// 使用静态变量确保唯一订单号
    );
}

// 退票
void FlightManager::refundTicket() {
    string id;
    cout << "请输入航班号：";
    cin >> id;

    Flight* f = findFlightById(id);
    if (!f) return;

    f->seats++;// 增加座位
    cout << "退票成功\n";

    // 处理预约队列
    if (!f->waitQueue.empty()) {
        Reservation r = f->waitQueue.top();
        f->waitQueue.pop();
        f->seats--;
        cout << "预约用户 "
             << r.userName
             << " 抢票成功\n";
    }
}

// 管理员修改航班状态
void FlightManager::updateFlightStatus() {
    string id;
    cout << "请输入航班号：";
    cin >> id;

    Flight* f = findFlightById(id);
    if (!f) return;

    cout << "1. 正常 2. 延误 3. 取消：";
    int c; cin >> c;

    if (c == 1) {
        f->status = FlightStatus::NORMAL;
        f->delayMinutes = 0;
    } else if (c == 2) {
        f->status = FlightStatus::DELAYED;
        cout << "输入延误分钟：";
        cin >> f->delayMinutes;
    } else if (c == 3) {
        f->status = FlightStatus::CANCELLED;
        recommendTransfer();
    }
}

// 推荐中转航班
void FlightManager::recommendTransfer() {
    string from, to;
    cout << "出发城市：";
    cin >> from;
    cout << "到达城市：";
    cin >> to;

    auto path = graph.recommendBestPath(from, to);
    if (path.empty()) {
        cout << "无可行中转\n";
        return;
    }

    cout << "推荐路径：";
    for (size_t i = 0; i < path.size(); ++i) {
        cout << path[i];
        if (i + 1 < path.size()) cout << " -> ";
    }
    cout << endl;
}

// 查找航班
Flight* FlightManager::findFlightById(const string& id) {
    for (auto& f : flights)
        if (f.flightId == id)
            return &f;
    return nullptr;
}

// 构建航班图,排除取消航班
void FlightManager::buildFlightGraph() {
    graph = FlightGraph(); // 清空现有图
    
    for (const auto& f : flights) {
        if (f.status != FlightStatus::CANCELLED) {
            graph.addEdge(f.from, f.to, f.price, 
                         f.flightId, f.departTime, f.arriveTime);
        }
    }
    cout << "航班图构建完成\n";
}

// 保存航班数据
void FlightManager::saveFlights(const std::string& filename) {
    std::ofstream fout(filename);
    if (!fout.is_open()) {
        std::cout << "无法保存到文件\n";
        return;
    }

    for (const auto& f : flights) {
        fout << f.flightId << " "
             << f.from << " "
             << f.to << " "
             << f.departTime << " "
             << f.arriveTime << " "
             << f.seats << " "
             << f.price << " ";

        switch (f.status) {
            case FlightStatus::NORMAL:
                fout << "正常";
                break;
            case FlightStatus::DELAYED:
                fout << "延误";
                break;
            case FlightStatus::CANCELLED:
                fout << "取消";
                break;
        }
        fout << "\n";
    }

    fout.close();
    cout << "航班数据已保存\n";
}

// 显示航班详细信息
void FlightManager::showFlightDetails(const string& flightId) {
    Flight* f = findFlightById(flightId);
    if (!f) {
        cout << "航班不存在\n";
        return;
    }

    cout << "\n============== 航班详细信息 ==============\n";
    cout << "航班号: " << f->flightId << endl;
    cout << "航线: " << f->from << " -> " << f->to << endl;
    cout << "起飞时间: " << f->departTime << endl;
    cout << "到达时间: " << f->arriveTime << endl;
    cout << "票价: " << f->price << " 元" << endl;
    cout << "剩余座位: " << f->seats << " 个" << endl;
    cout << "状态: ";
    
    switch (f->status) {
        case FlightStatus::NORMAL:
            cout << "正常";
            break;
        case FlightStatus::DELAYED:
            cout << "延误 (" << f->delayMinutes << "分钟)";
            break;
        case FlightStatus::CANCELLED:
            cout << "取消";
            break;
    }
    cout << endl;
    
    cout << "预约队列人数: " << f->waitQueue.size() << " 人" << endl;
    cout << "=========================================\n";
}