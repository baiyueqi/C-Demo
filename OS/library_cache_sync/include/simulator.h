#ifndef LIBRARY_CACHE_SYNC_SIMULATOR_H
#define LIBRARY_CACHE_SYNC_SIMULATOR_H

//声明整个并发模拟系统
#include "cache.h"
#include "common.h"
#include "database.h"
#include "request_queue.h"

#include <atomic>
#include <chrono>
#include <pthread.h>
#include <random>
#include <semaphore.h>
#include <string>
#include <vector>

class Simulator {
public:
    explicit Simulator(const Config& config);
    ~Simulator();

    Simulator(const Simulator&) = delete;
    Simulator& operator=(const Simulator&) = delete;

    int run();

private:
    struct ThreadArg {
        Simulator* simulator;
        int id;
    };

    static void* readerEntry(void* arg);
    static void* writerEntry(void* arg);
    static void* cacheManagerEntry(void* arg);

    void readerThread(int reader_id);
    void writerThread(int writer_id);
    void cacheManagerThread();

    void enterReadSection(int reader_id);
    void leaveReadSection(int reader_id);
    void sendShutdown();

    std::string chooseBook(std::mt19937& gen) const;
    void logLine(const std::string& actor, const std::string& message);
    long elapsedMs() const;

    //核心成员
    Config config_;
    Database database_;//主数据库
    Cache cache_;//缓存区
    RequestQueue request_queue_;//加载请求队列
    std::vector<std::string> book_names_;

    //核心信号量
    // 读者-写者同步：第一个读者锁住 db_sem_，最后一个读者释放它
    sem_t db_sem_;
    sem_t read_mutex_;//保护 read_count_
    // 缓存区和生产者-消费者同步
    sem_t cache_mutex_;//保护缓存区
    sem_t empty_slots_;//有界容量
    sem_t full_slots_;
    sem_t cache_unpinned_;// 在缓存满且全部被保护时用于等待
    // 请求队列同步，以及按读者编号单独唤醒
    sem_t request_mutex_;//保护请求队列
    sem_t request_count_;//表示待处理请求数量
    sem_t log_mutex_;//保证日志不交叉
    std::vector<sem_t> reader_ready_;//用于缓存管理器唤醒指定读者

    int read_count_ = 0;
    std::atomic<int> next_request_id_{1};
    std::chrono::steady_clock::time_point started_at_;
};

#endif
