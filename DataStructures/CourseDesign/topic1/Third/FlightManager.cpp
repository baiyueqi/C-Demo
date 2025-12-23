//所有功能实现
#include "FlightManager.h"
#include <iostream>     // cout cin
#include <fstream>      // ifstream
#include <string>       // string


using namespace std;

// 加载航班数据
void FlightManager::loadFlights(const string& file) {
    ifstream fin(file);
    if (!fin.is_open()) {
        cout << "航班数据文件打开失败\n";
        return;
    }

    Flight f;
    while (fin >> f.flightId
               >> f.from
               >> f.to
               >> f.departTime
               >> f.arriveTime
               >> f.seats
               >> f.price
               >> f.company) {

        f.status = FlightStatus::NORMAL;
        flights.push_back(f);          // 加入线性表
        graph.addEdge(f.from, f.to);   // 建图（中转推荐）
    }
    cout << "已加载航班数量: " << flights.size() << endl;

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

void FlightManager::updateFlightStatus() {
    string id;
    cout << "请输入要修改状态的航班号：";
    cin >> id;

    Flight* f = findFlightById(id);
    if (!f) {
        cout << "航班不存在\n";
        return;
    }

    cout << "\n当前航班信息：\n";
    cout << "航班号：" << f->flightId << endl;
    cout << "航线：" << f->from << " -> " << f->to << endl;
    cout << "当前状态：";

    if (f->status == FlightStatus::NORMAL) cout << "正常\n";
    else if (f->status == FlightStatus::DELAYED) cout << "延误\n";
    else cout << "取消\n";

    cout << "\n请选择新状态：\n";
    cout << "1. 正常\n";
    cout << "2. 延误\n";
    cout << "3. 取消\n";
    cout << "请选择：";

    int choice;
    cin >> choice;

    switch (choice) {
    case 1:
        f->status = FlightStatus::NORMAL;
        f->delayMinutes = 0;
        cout << "航班状态已修改为【正常】\n";
        break;

    case 2:
        f->status = FlightStatus::DELAYED;
        cout << "请输入延误分钟数：";
        cin >> f->delayMinutes;
        cout << "航班状态已修改为【延误】\n";
        cout << "延误时间：" << f->delayMinutes << " 分钟\n";
        cout << "【系统提示】请及时通知已购票乘客\n";
        break;

    case 3:
        f->status = FlightStatus::CANCELLED;
        cout << "航班状态已修改为【取消】\n";
        cout << "【系统提示】建议为乘客提供替代航班方案\n";
        cout << "是否立即推荐中转航班？(y/n)：";

        char ch;
        cin >> ch;
        if (ch == 'y' || ch == 'Y') {
            recommendTransfer();
        }
        break;

    default:
        cout << "无效选择，状态未修改\n";
    }
}
