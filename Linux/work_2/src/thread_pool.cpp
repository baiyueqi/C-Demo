// 实现固定数量线程池，用于并行执行目录扫描任务。
#include "../include/thread_pool.h"

ThreadPool::ThreadPool(size_t thread_count) : stop_(false), pending_(0) {
    // 创建指定数量的工作线程。
    for (size_t i = 0; i < thread_count; ++i) {
        workers_.emplace_back([this] { worker_loop(); });
    }
}

ThreadPool::~ThreadPool() {
    // 通知所有工作线程退出，并回收线程。
    {
        std::lock_guard<std::mutex> lock(mutex_);
        stop_ = true;
    }
    cv_.notify_all();
    for (std::thread& worker : workers_) {
        if (worker.joinable()) {
            worker.join();
        }
    }
}

void ThreadPool::submit(std::function<void()> task) {
    // 将任务加入队列并增加 pending_，唤醒一个空闲工作线程。
    {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
        ++pending_;
    }
    cv_.notify_one();
}

void ThreadPool::wait() {
    // pending_ 为 0 表示没有排队中或执行中的任务。
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [this] { return pending_ == 0; });
}

void ThreadPool::worker_loop() {
    // 工作线程循环：等待任务、执行任务，并在任务结束后递减 pending_。
    while (true) {
        std::function<void()> task;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return stop_ || !tasks_.empty(); });
            if (stop_ && tasks_.empty()) {
                return;
            }
            task = std::move(tasks_.front());
            tasks_.pop();
        }

        task();

        {
            std::lock_guard<std::mutex> lock(mutex_);
            --pending_;
        }
        cv_.notify_all();
    }
}
