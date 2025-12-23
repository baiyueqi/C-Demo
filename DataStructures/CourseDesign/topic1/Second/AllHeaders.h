#ifndef ALL_HEADERS_H
#define ALL_HEADERS_H

#include <string>
#include <vector>
#include <unordered_map>
#include <map>
#include <queue>
#include <mutex>

//航班结构体（线性表）

struct Flight {
    std::string flightNumber;   // 航班号
    std::string airline;        // 航空公司
    std::string fromCity;       // 出发城市
    std::string toCity;         // 到达城市
    std::string departTime;     // 起飞时间
    std::string arriveTime;     // 到达时间
    int price;                  // 票价
    int availableSeats;         // 剩余座位
    std::string status;         // 正常 / 延误 / 取消
};


//航班信息管理类：增删改查

class FlightManager {
public:
    void addFlight(const Flight& flight);

    Flight* findFlight(const std::string& flightNumber);// 根据航班号查找航班

    void updateFlightStatus(const std::string& flightNumber,
                            const std::string& newStatus);// 更新航班状态

    const std::vector<Flight>& getAllFlights() const;

private:
    std::vector<Flight> flights;                    // 航班线性表
    std::unordered_map<std::string, int> indexMap;  // 航班号索引
};


//城市航线图管理，用于转机推荐

class GraphManager {
public:
    void buildGraph(const std::vector<Flight>& flights);

    void recommendTransfer(const std::string& from,
                            const std::string& to,
                            const std::vector<Flight>& flights);

private:
    std::map<std::string, std::vector<int>> adjacencyList;
};

// 票务管理：购票 / 退票 / 预约抢票

class TicketManager {
public:
    void buyTicket(const std::string& flightNumber,
                   const std::string& userName,
                   FlightManager& flightManager);

    void refundTicket(const std::string& flightNumber,
                      FlightManager& flightManager);

private:
    struct Reservation {
        std::string userName;
        int priority;  // 时间戳，越小优先级越高

        bool operator<(const Reservation& other) const {
            return priority > other.priority;
        }
    };

    std::unordered_map<std::string,
        std::priority_queue<Reservation>> waitingList;

    std::mutex ticketMutex;
};

//从文件加载航班数据

void loadFlightData(const std::string& filename,
                    FlightManager& manager);

#endif // ALL_HEADERS_H
