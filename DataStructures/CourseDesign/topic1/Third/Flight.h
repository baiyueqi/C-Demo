#ifndef FLIGHT_H
#define FLIGHT_H

#include <string>
#include <queue>
#include <vector>

using std::string;
using std::priority_queue;
using std::vector;

// 航班状态枚举
enum class FlightStatus {
    NORMAL,     // 正常
    DELAYED,    // 延误
    CANCELLED   // 取消
};

// 预约信息结构体
struct Reservation {
    string userName;   // 用户名
    int order;         // 预约顺序（越小优先级越高）

    Reservation(const string& name, int o)
        : userName(name), order(o) {}
};

// 预约优先级比较器（先预约先得票）
struct ReservationCmp {
    bool operator()(const Reservation& a,
                    const Reservation& b) const {
        return a.order > b.order;
    }
};

// 航班结构体
struct Flight {
    string flightId;        // 航班号
    string company;         // 航空公司
    string from;            // 出发城市
    string to;              // 到达城市
    string departTime;      // 起飞时间
    string arriveTime;      // 到达时间
    int seats;              // 剩余座位
    int price;              // 价格

    FlightStatus status;    // 航班状态
    int delayMinutes;       // 延误分钟数

    // 预约抢票队列
    priority_queue<
        Reservation,
        vector<Reservation>,
        ReservationCmp
    > waitQueue;

    Flight()
        : seats(0),
          price(0),
          status(FlightStatus::NORMAL),
          delayMinutes(0) {}
};

#endif
