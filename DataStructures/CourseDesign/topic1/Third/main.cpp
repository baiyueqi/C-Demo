#include "FlightManager.h"
#include <iostream>

using namespace std;

int main() {
    FlightManager manager;
    manager.loadFlights("flights.txt");

    while (true) {
        cout << "\n====== 飞机票管理系统 ======\n";
        cout << "1. 显示所有航班\n";
        cout << "2. 查询航班\n";
        cout << "3. 购票\n";
        cout << "4. 退票\n";
        cout << "5. 航班状态管理（管理员）\n";
        cout << "6. 推荐中转航班\n";
        cout << "7. 保存数据\n";  // 新增
        cout << "0. 退出\n";
        cout << "请选择：";

        int c;
        cin >> c;

        switch (c) {
        case 1: manager.showAllFlights(); break;
        case 2: manager.searchByCity(); break;
        case 3: manager.buyTicket(); break;
        case 4: manager.refundTicket(); break;
        case 5: manager.updateFlightStatus(); break;
        case 6: manager.recommendTransfer(); break;
        case 7: manager.saveFlights("flights.txt"); break;  // 新增
        case 0: 
            manager.saveFlights("flights.txt");  // 退出前自动保存
            return 0;
        default: cout << "无效选择\n";
        }
    }
}