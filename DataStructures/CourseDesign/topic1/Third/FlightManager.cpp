#include "FlightManager.h"
#include <iostream>
#include <fstream>

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
}

// 显示所有航班
void FlightManager::showAllFlights() {
    for (auto& f : flights) {
        cout << f.flightId << " "
             << f.from << "->" << f.to
             << " 票价:" << f.price
             << " 座位:" << f.seats << " ";

        if (f.status == FlightStatus::NORMAL)
            cout << "正常";
        else if (f.status == FlightStatus::DELAYED)
            cout << "延误(" << f.delayMinutes << "分钟)";
        else
            cout << "取消";

        cout << endl;
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
        if (f.from == from && f.to == to &&
            f.status != FlightStatus::CANCELLED) {
            cout << f.flightId << " 票价:" << f.price
                 << " 剩余:" << f.seats << endl;
            found = true;
        }
    }

    if (!found) {
        cout << "无直飞航班，推荐中转方案\n";
        recommendTransfer();
    }
}

// 实时购票
void FlightManager::buyTicket() {
    string id, user;
    cout << "请输入航班号：";
    cin >> id;
    cout << "请输入用户名：";
    cin >> user;

    Flight* f = findFlightById(id);
    if (!f) {
        cout << "航班不存在\n";
        return;
    }

    if (f->status == FlightStatus::CANCELLED) {
        cout << "航班已取消\n";
        return;
    }

    if (f->status == FlightStatus::DELAYED) {
    cout << "提示：该航班已延误 "
         << f->delayMinutes
         << " 分钟\n";
    }

    if (f->seats > 0) {
        f->seats--;
        cout << "购票成功\n";
    } else {
        cout << "无票，已进入预约队列\n";
        addReservation(*f, user);
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
