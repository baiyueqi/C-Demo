#include "AllHeaders.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <ctime>

// 添加航班信息
void FlightManager::addFlight(const Flight& flight) {// 添加航班信息
    flights.push_back(flight);// 将航班添加到航班列表
    // 更新航班索引映射
    indexMap[flight.flightNumber] = flights.size() - 1;
}

// 查找航班信息
Flight* FlightManager::findFlight(const std::string& flightNumber) {
    if (!indexMap.count(flightNumber)) {
        return nullptr;
    }
    return &flights[indexMap[flightNumber]];
}

// 更新航班状态
void FlightManager::updateFlightStatus(const std::string& flightNumber,
                                       const std::string& newStatus) {
    Flight* flight = findFlight(flightNumber);
    if (flight == nullptr) {
        std::cout << "航班不存在\n";
        return;
    }
    flight->status = newStatus;
}

// 获取所有航班信息
const std::vector<Flight>& FlightManager::getAllFlights() const {
    return flights;
}

// 构建航班图
void GraphManager::buildGraph(const std::vector<Flight>& flights) {// 构建航班图
    adjacencyList.clear();
    for (size_t i = 0; i < flights.size(); ++i) {
        adjacencyList[flights[i].fromCity].push_back(i);
    }
}

// 推荐转机方案
void GraphManager::recommendTransfer(const std::string& from,
                                     const std::string& to,
                                     const std::vector<Flight>& flights) {
    for (int first : adjacencyList[from]) {
        if (flights[first].status != "正常") continue;

        std::string midCity = flights[first].toCity;
        for (int second : adjacencyList[midCity]) {
            if (flights[second].toCity == to &&
                flights[second].status == "正常") {
                std::cout << "推荐转机方案："
                          << flights[first].flightNumber
                          << " -> "
                          << flights[second].flightNumber
                          << std::endl;
                return;
            }
        }
    }
    std::cout << "暂无合适转机方案\n";
}

//购票
void TicketManager::buyTicket(const std::string& flightNumber,
                              const std::string& userName,
                              FlightManager& flightManager) {
    std::lock_guard<std::mutex> lock(ticketMutex);

    Flight* flight = flightManager.findFlight(flightNumber);
    if (flight == nullptr || flight->status != "正常") {
        std::cout << "航班不可售\n";
        return;
    }

    if (flight->availableSeats > 0) {
        flight->availableSeats--;
        std::cout << "购票成功\n";
    } else {
        waitingList[flightNumber].push(
            {userName, static_cast<int>(time(nullptr))}
        );
        std::cout << "无座位，已加入预约队列\n";
    }
}

//退票
void TicketManager::refundTicket(const std::string& flightNumber,
                                 FlightManager& flightManager) {
    std::lock_guard<std::mutex> lock(ticketMutex);

    Flight* flight = flightManager.findFlight(flightNumber);
    if (flight == nullptr) return;

    flight->availableSeats++;

    if (!waitingList[flightNumber].empty()) {
        waitingList[flightNumber].pop();
        flight->availableSeats--;
        std::cout << "座位已分配给预约用户\n";
    } else {
        std::cout << "退票成功\n";
    }
}

// 加载航班数据
void loadFlightData(const std::string& filename,
                    FlightManager& manager) {
    std::ifstream fin(filename);
    if (!fin) {
        std::cout << "文件读取失败\n";
        return;
    }

    Flight flight;
    while (fin >> flight.flightNumber
               >> flight.airline
               >> flight.fromCity
               >> flight.toCity
               >> flight.departTime
               >> flight.arriveTime
               >> flight.price
               >> flight.availableSeats
               >> flight.status) {
        manager.addFlight(flight);
    }
}


int main() {
    FlightManager flightManager;
    TicketManager ticketManager;
    GraphManager graphManager;

    loadFlightData("flights.txt", flightManager);
    graphManager.buildGraph(flightManager.getAllFlights());

    int choice;
    while (true) {
        std::cout << "\n1 查询航班  2 购票  3 退票  4 修改状态  0 退出\n";
        std::cin >> choice;

        if (choice == 0) break;

        std::string a, b;
        switch (choice) {
        case 1:
            std::cin >> a;
            if (auto f = flightManager.findFlight(a)) {
                std::cout << f->flightNumber << " "
                          << f->fromCity << "->"
                          << f->toCity << " "
                          << f->price << "元\n";
            } else {
                std::cout << "航班不存在\n";
            }
            break;
        case 2:
            std::cin >> a >> b;
            ticketManager.buyTicket(a, b, flightManager);
            break;
        case 3:
            std::cin >> a;
            ticketManager.refundTicket(a, flightManager);
            break;
        case 4:
            std::cin >> a >> b;
            flightManager.updateFlightStatus(a, b);
            break;
        default:
            std::cout << "无效选择\n";
        }
    }
    return 0;
}
