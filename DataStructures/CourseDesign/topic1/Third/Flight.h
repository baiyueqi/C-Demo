//航班与枚举定义（线性表元素）
#ifndef FLIGHT_H
#define FLIGHT_H

#include <string>

using std::string;

// 航班状态枚举
enum class FlightStatus {
    NORMAL,
    DELAYED,
    CANCELLED
};

// 航班结构体
struct Flight {
    string flightId;        // 航班号
    string company;         // 航空公司
    string from;            // 起飞城市
    string to;              // 到达城市
    string departTime;      // 起飞时间
    string arriveTime;      // 到达时间
    int price;              // 价格
    int seats;              // 剩余座位
    FlightStatus status;    // 航班状态
};

#endif
