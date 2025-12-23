#include <iostream>
#include <vector>
#include <string>
#include <unordered_map>
#include <queue>
#include <map>
#include <fstream>
#include <algorithm>
#include <mutex>
#include <ctime>

using namespace std;

//航班结构体（线性表）

struct Flight {
    string flightNo;      // 航班号
    string airline;       // 航空公司
    string fromCity;      // 出发城市
    string toCity;        // 到达城市
    string departTime;    // 起飞时间
    string arriveTime;    // 到达时间
    int price;            // 票价
    int seats;            // 剩余座位
    string status;        // 正常 / 延误 / 取消
};

//抢票预约结构（优先队列）

struct Reservation {
    string userName;
    int priority; // 时间戳，越小优先级越高

    bool operator<(const Reservation& other) const {
        return priority > other.priority;
    }
};

//全局数据结构

vector<Flight> flights;                                      // 航班线性表
unordered_map<string, int> flightIndex;                      // 航班号索引
map<string, vector<int>> graph;                              // 城市图（邻接表）
unordered_map<string, priority_queue<Reservation>> waitList; // 预约抢票
mutex ticketMutex;


//航班信息管理

void addFlight(const Flight& f) {
    flights.push_back(f);
    int idx = flights.size() - 1;
    flightIndex[f.flightNo] = idx;
    graph[f.fromCity].push_back(idx);
}

void updateFlightStatus(const string& flightNo, const string& status) {
    if (!flightIndex.count(flightNo)) {
        cout << "航班不存在\n";
        return;
    }
    flights[flightIndex[flightNo]].status = status;
    cout << "航班状态已更新\n";
}


//航班动态管理（转机推荐）

void recommendTransfer(const string& from, const string& to) {
    cout << "无直飞航班，推荐转机方案：\n";
    for (int first : graph[from]) {
        if (flights[first].status != "正常") continue;
        string midCity = flights[first].toCity;
        for (int second : graph[midCity]) {
            if (flights[second].toCity == to &&
                flights[second].status == "正常") {
                cout << flights[first].flightNo << " -> "
                     << flights[second].flightNo << endl;
                return;
            }
        }
    }
    cout << "暂无合适转机方案\n";
}

//购票

void buyTicket(const string& flightNo, const string& userName) {
    lock_guard<mutex> lock(ticketMutex);

    if (!flightIndex.count(flightNo)) {
        cout << "航班不存在\n";
        return;
    }

    Flight& f = flights[flightIndex[flightNo]];

    if (f.status != "正常") {
        cout << "航班不可售\n";
        return;
    }

    if (f.seats > 0) {
        f.seats--;
        cout << "购票成功，剩余座位：" << f.seats << endl;
    } else {
        cout << "无座位，已加入预约抢票队列\n";
        waitList[flightNo].push({userName, (int)time(nullptr)});
    }
}


//退票

void refundTicket(const string& flightNo) {
    lock_guard<mutex> lock(ticketMutex);

    if (!flightIndex.count(flightNo)) {
        cout << "航班不存在\n";
        return;
    }

    Flight& f = flights[flightIndex[flightNo]];
    f.seats++;

    if (!waitList[flightNo].empty()) {
        Reservation r = waitList[flightNo].top();
        waitList[flightNo].pop();
        f.seats--;
        cout << "座位自动分配给预约用户：" << r.userName << endl;
    } else {
        cout << "退票成功\n";
    }
}


//航班查询

void queryByFlightNo(const string& flightNo) {
    if (!flightIndex.count(flightNo)) {
        cout << "航班不存在\n";
        return;
    }

    const Flight& f = flights[flightIndex[flightNo]];
    cout << "航班号：" << f.flightNo
         << " " << f.fromCity << "->" << f.toCity
         << " 起飞：" << f.departTime
         << " 到达：" << f.arriveTime
         << " 价格：" << f.price
         << " 座位：" << f.seats
         << " 状态：" << f.status << endl;
}

void queryByCity(const string& from, const string& to) {
    vector<Flight> result;

    for (const auto& f : flights) {
        if (f.fromCity == from && f.toCity == to && f.status == "正常") {
            result.push_back(f);
        }
    }

    sort(result.begin(), result.end(),
         [](const Flight& a, const Flight& b) {
             return a.price < b.price;
         });

    if (result.empty()) {
        recommendTransfer(from, to);
        return;
    }

    for (const auto& f : result) {
        cout << f.flightNo << " 价格：" << f.price
             << " 剩余座位：" << f.seats << endl;
    }
}


//文件读入

void loadFlights(const string& filename) {
    ifstream fin(filename);
    if (!fin) {
        cout << "航班数据文件打开失败\n";
        return;
    }

    Flight f;
    while (fin >> f.flightNo >> f.airline >> f.fromCity
               >> f.toCity >> f.departTime >> f.arriveTime
               >> f.price >> f.seats >> f.status) {
        addFlight(f);
    }
    fin.close();
}


//主函数

int main() {
    loadFlights("flights.txt");

    int choice;
    while (true) {
        cout << "\n====== 飞机票管理系统 ======\n";
        cout << "1. 按航班号查询\n";
        cout << "2. 按城市查询\n";
        cout << "3. 购票\n";
        cout << "4. 退票\n";
        cout << "5. 修改航班状态\n";
        cout << "0. 退出\n";
        cout << "请选择：";

        cin >> choice;
        if (choice == 0) break;

        string a, b;
        switch (choice) {
        case 1:
            cin >> a;
            queryByFlightNo(a);
            break;
        case 2:
            cin >> a >> b;
            queryByCity(a, b);
            break;
        case 3:
            cin >> a >> b;
            buyTicket(a, b);
            break;
        case 4:
            cin >> a;
            refundTicket(a);
            break;
        case 5:
            cin >> a >> b;
            updateFlightStatus(a, b);
            break;
        default:
            cout << "无效选择\n";
        }
    }
    return 0;
}
