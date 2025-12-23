//购票 / 预约信息（队列 & 优先队列）
#ifndef TICKET_H
#define TICKET_H

#include <string>

using std::string;

// 预约抢票结构
struct Reservation {
    string userName;   // 用户名
    string flightId;   // 航班号
    int priority;      // 优先级（越小越早）
};

#endif
