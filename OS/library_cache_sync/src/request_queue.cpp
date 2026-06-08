//实现队列的 push 和 pop
#include "request_queue.h"

LoadRequest LoadRequest::normal(int reader_id, const std::string& book_name, int request_id)
{
    // 普通请求由读者在缓存未命中后产生
    LoadRequest req;
    req.type = Type::Normal;
    req.reader_id = reader_id;
    req.book_name = book_name;
    req.request_id = request_id;
    return req;
}

//Shutdown 是给缓存管理器线程用的结束信号,让它跳出循环并正常退出，避免后台线程一直等待导致程序无法结束s
LoadRequest LoadRequest::shutdown()
{
    // 关闭请求由主线程在读者和写者全部结束后产生
    LoadRequest req;
    req.type = Type::Shutdown;
    return req;
}

void RequestQueue::push(const LoadRequest& req)
{
    queue_.push(req);
}

bool RequestQueue::pop(LoadRequest& req)
{
    if (queue_.empty()) {
        return false;
    }
    req = queue_.front();
    queue_.pop();
    return true;
}
