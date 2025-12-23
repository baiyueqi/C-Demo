//所有功能实现
#include "FlightManager.h"
#include <iostream>     // cout cin
#include <fstream>      // ifstream
#include <string>       // string


using namespace std;

// 加载航班数据
void FlightManager::loadFlights(const string& file) {
    ifstream fin(file);
    Flight f;
    while (fin >> f.flightId >> f.company >> f.from >> f.to
               >> f.departTime >> f.arriveTime >> f.price >> f.seats) {
        f.status = FlightStatus::NORMAL;
        flights.push_back(f);
        graph.addEdge(f.from, f.to);
    }
    fin.close();
}

// 显示所有航班
void FlightManager::showAllFlights() {
    for (auto& f : flights) {
        cout << f.flightId << " " << f.from << "->" << f.to
             << " 票价:" << f.price << " 座位:" << f.seats << endl;
    }
}

// 按城市查询
void FlightManager::searchByCity() {
    string from, to;
    cout << "请输入出发城市：";
    cin >> from;
    cout << "请输入到达城市：";
    cin >> to;

    bool found = false;
    for (auto& f : flights) {
        if (f.from == from && f.to == to && f.status == FlightStatus::NORMAL) {
            cout << f.flightId << " 票价:" << f.price << " 剩余:" << f.seats << endl;
            found = true;
        }
    }

    if (!found) {
        cout << "无直飞航班，是否推荐中转？(y/n): ";
        char c; cin >> c;
        if (c == 'y') recommendTransfer();
    }
}

// 购票功能
void FlightManager::buyTicket() {
    string id, name;
    cout << "请输入航班号：";
    cin >> id;
    cout << "请输入姓名：";
    cin >> name;

    Flight* f = findFlightById(id);
    if (!f) {
        cout << "航班不存在\n";
        return;
    }

    if (f->seats > 0) {
        f->seats--;
        cout << "购票成功！\n";
    } else {
        cout << "无票，已加入预约队列\n";
        reservations.push({name, id, (int)reservations.size()});
    }
}

// 退票功能
void FlightManager::refundTicket() {
    string id;
    cout << "请输入退票航班号：";
    cin >> id;

    Flight* f = findFlightById(id);
    if (!f) return;

    f->seats++;
    cout << "退票成功\n";

    // 有预约则分配
    if (!reservations.empty()) {
        auto r = reservations.top();
        reservations.pop();
        f->seats--;
        cout << "已为预约用户 " << r.userName << " 分配座位\n";
    }
}

// 推荐中转
void FlightManager::recommendTransfer() {
    string from, to;
    cout << "请输入出发城市：";
    cin >> from;
    cout << "请输入到达城市：";
    cin >> to;

    auto path = graph.findTransferPath(from, to);
    if (path.empty()) {
        cout << "无可行中转方案\n";
        return;
    }

    cout << "推荐中转路径：";
    for (auto& city : path)
        cout << city << " ";
    cout << endl;
}

// 查找航班
Flight* FlightManager::findFlightById(const string& id) {
    for (auto& f : flights)
        if (f.flightId == id) return &f;
    return nullptr;
}
