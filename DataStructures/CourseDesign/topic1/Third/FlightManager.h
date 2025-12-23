#ifndef FLIGHT_MANAGER_H
#define FLIGHT_MANAGER_H

#include "Flight.h"
#include "FlightGraph.h"
#include <vector>
#include <string>

using std::vector;
using std::string;

class FlightManager {
public:
    // 加载航班数据
    void loadFlights(const string& file);

    // 显示航班详细信息
    void showFlightDetails(const string& flightId);

    // 显示所有航班
    void showAllFlights();

    // 按城市查询航班
    void searchByCity();

    // 实时购票 / 预约抢票
    void buyTicket();

    // 退票并自动分配
    void refundTicket();

    // 管理员手动修改航班状态
    void updateFlightStatus();

    // 推荐中转航班
    void recommendTransfer();
        
    // 保存航班数据
    void saveFlights(const string& file);

private:
    vector<Flight> flights; // 航班线性表
    FlightGraph graph;      // 航班图
    // 构建航班图
    void buildFlightGraph();

    // 根据航班号查找航班
    Flight* findFlightById(const string& id);

    // 添加预约
    void addReservation(Flight& flight, const string& user);
};

#endif
