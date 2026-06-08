#ifndef LIBRARY_CACHE_SYNC_COMMON_H
#define LIBRARY_CACHE_SYNC_COMMON_H

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <semaphore.h>
#include <string>

// 数据库和缓存层共用的电子书记录
struct Book {
    std::string name;//书名
    std::string content;//内容
    int version;//版本号，用来观察写者更新后数据是否变化
};

// 模拟器运行参数；默认值对应课程演示规模
struct Config {
    int readers = 8;
    int writers = 3;
    int cache_size = 5;
    int rounds = 5;
    long long student_no = 20241001655LL;
    int delay_min_ms = 1000;
    int delay_max_ms = 5000;
    unsigned int random_seed = 0;
};

// P 操作：等待信号量；如果被信号中断则重试
inline void P(sem_t* sem)
{
    while (sem_wait(sem) == -1) {
        if (errno != EINTR) {
            std::perror("sem_wait 等待失败");
            std::abort();
        }
    }
}

// V 操作：释放一个信号量资源
inline void V(sem_t* sem)
{
    if (sem_post(sem) == -1) {
        std::perror("sem_post 释放失败");
        std::abort();
    }
}

// 四类延时接口题目一使用用户态睡眠；题目二可编译为自定义 Linux 系统调用
void configure_delays(long long student_no, int min_ms, int max_ms);
void delay_read(long long student_no);
void delay_load(long long student_no);
void delay_write(long long student_no);
void delay_refresh(long long student_no);

#endif
