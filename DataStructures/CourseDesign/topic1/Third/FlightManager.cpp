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

// 显示所有航班,包含时间信息
void FlightManager::showAllFlights() {
    if (flights.empty()) {
        cout << "没有航班数据\n";
        return;
    }
    
    cout << "\n================================ 所有航班信息 ================================\n";
    cout << left 
         << setw(10) << "航班号"
         << setw(15) << "出发->到达"
         << setw(10) << "起飞时间"
         << setw(10) << "到达时间"
         << setw(10) << "票价"
         << setw(10) << "座位"
         << "状态" << endl;
    
    cout << string(75, '-') << endl;
    
    for (auto& f : flights) {
        string route = f.from + "->" + f.to;
        string status;
        
        if (f.status == FlightStatus::NORMAL)
            status = "正常";
        else if (f.status == FlightStatus::DELAYED)
            status = "延误(" + to_string(f.delayMinutes) + "分钟)";
        else
            status = "取消";
        
        cout << left
             << setw(10) << f.flightId
             << setw(15) << route
             << setw(10) << f.departTime
             << setw(10) << f.arriveTime
             << setw(10) << f.price
             << setw(10) << f.seats
             << status << endl;
    }
    cout << string(75, '=') << endl;
}

// 按城市查询
void FlightManager::searchByCity() {
    string from, to;
    cout << "请输入出发城市：";
    cin >> from;
    cout << "请输入到达城市：";
    cin >> to;

    bool found = false;
    
    cout << "\n============== 查询结果 ==============\n";
    cout << left 
         << setw(10) << "航班号"
         << setw(12) << "起飞时间"
         << setw(12) << "到达时间"
         << setw(10) << "票价"
         << setw(10) << "座位"
         << "状态" << endl;
    
    cout << "--------------------------------------------------------\n";
    
    for (auto& f : flights) {
        if (f.from == from && f.to == to &&
            f.status != FlightStatus::CANCELLED) {
            
            cout << left
                 << setw(10) << f.flightId
                 << setw(12) << f.departTime
                 << setw(12) << f.arriveTime
                 << setw(10) << (to_string(f.price) + "元")
                 << setw(10) << (to_string(f.seats) + "个");
                 
            if (f.status == FlightStatus::NORMAL)
                cout << "正常";
            else if (f.status == FlightStatus::DELAYED)
                cout << "延误(" << f.delayMinutes << "分钟)";
            
            cout << endl;
            found = true;
        }
    }

    if (!found) {
        cout << "无直飞航班\n";
        recommendTransfer();
    }
    cout << "===================================\n";
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

// 添加预约
void FlightManager::addReservation(Flight& flight, const string& user) {
    static int orderCounter = 1;
    flight.waitQueue.push(
        Reservation(user, orderCounter++)
    );
}

// 退票
void FlightManager::refundTicket() {
    string id;
    cout << "请输入航班号：";
    cin >> id;

    Flight* f = findFlightById(id);
    if (!f) return;

    f->seats++;
    cout << "退票成功\n";

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