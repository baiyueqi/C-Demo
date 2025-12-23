//系统核心管理类
#ifndef FLIGHT_MANAGER_H
#define FLIGHT_MANAGER_H

#include "Flight.h"
#include "Ticket.h"
#include "FlightGraph.h"
#include <vector>
#include <queue>

using namespace std;

// 比较预约优先级
struct ReservationCmp {
    bool operator()(const Reservation& a, const Reservation& b) {
        return a.priority > b.priority;
    }
};

class FlightManager {
public:
    void loadFlights(const string& file);
    void showAllFlights();
    void searchByCity();
    void buyTicket();
    void refundTicket();
    void recommendTransfer();
    // 航班状态管理（管理员手动修改）
    void updateFlightStatus();
    
private:
    vector<Flight> flights;   // 航班线性表
    priority_queue<Reservation, vector<Reservation>, ReservationCmp> reservations;
    FlightGraph graph;

    Flight* findFlightById(const string& id);
};

#endif
