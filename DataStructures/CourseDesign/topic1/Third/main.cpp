#include "FlightManager.h"
#include <iostream>

using namespace std;

int main() {
    FlightManager manager;
    manager.loadFlights("flights.txt");

    while (true) {
        cout << "\n====== 飞机票管理系统 ======\n";
        cout << "1. 显示所有航班\n";
        cout << "2. 查询航班（按城市）\n";
        cout << "3. 购票\n";
        cout << "4. 退票\n";
        cout << "5. 推荐中转航班\n";
        cout << "0. 退出系统\n";
        cout << "请选择：";

        int choice;
        cin >> choice;

        switch (choice) {
        case 1: manager.showAllFlights(); break;
        case 2: manager.searchByCity(); break;
        case 3: manager.buyTicket(); break;
        case 4: manager.refundTicket(); break;
        case 5: manager.recommendTransfer(); break;
        case 0: return 0;
        default: cout << "无效选择\n";
        }
    }
}
