// 声明固定数量线程池类接口。
#ifndef FS_AUDITOR_THREAD_POOL_H
#define FS_AUDITOR_THREAD_POOL_H

#include <condition_variable>
#include <cstddef>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

// 固定数量线程池，用于并行扫描子目录。
class ThreadPool {
public:
    // 创建 thread_count 个工作线程。
    explicit ThreadPool(size_t thread_count);

    // 停止所有工作线程并等待它们退出。
    ~ThreadPool();

    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // 提交一个待执行任务。
    void submit(std::function<void()> task);

    // 等待所有已提交任务执行完成。
    void wait();

private:
    void worker_loop();

    std::vector<std::thread> workers_;              // 工作线程集合
    std::queue<std::function<void()>> tasks_;       // 待执行任务队列
    std::mutex mutex_;                              // 保护任务队列和计数
    std::condition_variable cv_;                    // 用于等待任务或停止信号
    bool stop_;                                     // 是否要求线程池停止
    size_t pending_;                                // 排队中和正在执行的任务总数
};

#endif
