#ifndef LIBRARY_CACHE_SYNC_REQUEST_QUEUE_H
#define LIBRARY_CACHE_SYNC_REQUEST_QUEUE_H
//声明读者缓存 miss 后提交给缓存管理器的请求
#include <queue>
#include <string>

struct LoadRequest {
    //区分普通请求和 shutdown 请求
    enum class Type {
        Normal,
        Shutdown
    };

    Type type = Type::Normal;
    int reader_id = -1;
    std::string book_name;//要加载哪本书
    // 请求唯一编号，用于把队列请求和缓存保护标记对应起来
    int request_id = -1;

    static LoadRequest normal(int reader_id, const std::string& book_name, int request_id);
    static LoadRequest shutdown();
};

class RequestQueue {
public:
    // 队列内部不加锁，统一由 Simulator 的 request_mutex 保护
    void push(const LoadRequest& req);
    bool pop(LoadRequest& req);

private:
    std::queue<LoadRequest> queue_;
};

#endif
