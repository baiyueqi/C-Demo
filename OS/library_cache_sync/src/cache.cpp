#include "cache.h"

#include <sstream>
#include <stdexcept>

//作用：创建固定容量的缓存区
Cache::Cache(int capacity)
{
    if (capacity <= 0) {
        throw std::invalid_argument("缓存容量必须为正数");
    }
    slots_.resize(static_cast<std::size_t>(capacity));//缓存槽（池）
}

//缓存命中,读者正常查缓存
bool Cache::findAndCopy(const std::string& name, Book& out)
{
    for (CacheEntry& entry : slots_) {
        if (entry.valid && entry.book.name == name) {
            /*每次成功读取都刷新 LRU 访问状态。每次访问成功，全局时间 tick_ 加 1，并记录到该缓存项。
            *以后淘汰时，谁的 last_access_tick 最小，谁就是最久没被访问的*/
            entry.last_access_tick = ++tick_;
            out = entry.book;
            return true;
        }
    }
    return false;
}

//带 pin 的查询,作用：读者 miss 后，被缓存管理器唤醒，再回来找“专门为自己加载的缓存项”
bool Cache::findPinnedAndCopy(const std::string& name, int request_id, Book& out)
{
    for (CacheEntry& entry : slots_) {
        if (entry.valid && entry.book.name == name &&
            entry.pinned_request_ids.count(request_id) != 0) {//不只要书名相同，还要这个缓存项上有当前请求的 request_id 保护标记
            entry.last_access_tick = ++tick_;
            out = entry.book;
            return true;
        }
    }
    return false;
}

//判断缓存是否存在某书
bool Cache::contains(const std::string& name) const
{
    for (const CacheEntry& entry : slots_) {
        if (entry.valid && entry.book.name == name) {
            return true;
        }
    }
    return false;
}

//判断是否有空槽
bool Cache::hasFreeSlot() const
{
    for (const CacheEntry& entry : slots_) {
        if (!entry.valid) {
            return true;
        }
    }
    return false;
}

//缓存管理器加载一本书后，如果缓存里已经有同名书，就直接更新这个缓存项，并加 pin
bool Cache::updateExistingAndPin(const Book& book, int request_id)
{
    for (CacheEntry& entry : slots_) {
        if (entry.valid && entry.book.name == book.name) {
            // 多个读者可能请求同一本书；每个请求都保留自己的保护标记
            entry.book = book;
            entry.last_access_tick = ++tick_;
            entry.pinned_request_ids.insert(request_id);
            return true;
        }
    }
    return false;
}

//缓存还没满时，把新书放到第一个空槽里,并加pin
void Cache::insertIntoFreeSlotAndPin(const Book& book, int request_id)
{
    for (CacheEntry& entry : slots_) {
        if (!entry.valid) {
            entry.valid = true;
            entry.book = book;
            entry.last_access_tick = ++tick_;//更新时间戳
            entry.pinned_request_ids.clear();
            entry.pinned_request_ids.insert(request_id);
            return;
        }
    }
    throw std::runtime_error("没有空闲缓存槽");
}

//LRU 替换并 pin,缓存满了以后，选择一个“最近最少使用且没有被 pin 的缓存项”替换掉
bool Cache::replaceLruAndPin(const Book& book, int request_id)
{
    CacheEntry* candidate = nullptr;
    for (CacheEntry& entry : slots_) {
        // 带保护标记的缓存项会一直受保护，直到等待它的读者复制完成
        if (!entry.valid || !entry.pinned_request_ids.empty()) {
            continue;
        }
        if (candidate == nullptr || entry.last_access_tick < candidate->last_access_tick) {
            candidate = &entry;
        }
    }

    //所有槽都被 pin
    if (candidate == nullptr) {
        return false;//让缓存管理器去等待 cache_unpinned
    }

    candidate->valid = true;
    candidate->book = book;
    candidate->last_access_tick = ++tick_;
    candidate->pinned_request_ids.clear();
    candidate->pinned_request_ids.insert(request_id);
    return true;
}

//读者已经复制完缓存副本后，释放自己的保护标记
void Cache::releasePin(const std::string& name, int request_id)
{
    for (CacheEntry& entry : slots_) {
        if (entry.valid && entry.book.name == name) {
            entry.pinned_request_ids.erase(request_id);
            return;
        }
    }
}

//写者刷新缓存,写者更新主数据库后，如果缓存里有这本书，就同步刷新缓存副本
bool Cache::updateIfPresent(const Book& book)
{
    for (CacheEntry& entry : slots_) {
        if (entry.valid && entry.book.name == book.name) {
            entry.book = book;
            entry.last_access_tick = ++tick_;
            return true;
        }
    }
    return false;
}

//缓存快照,生成日志用的缓存状态字符串
std::string Cache::snapshot() const
{
    std::ostringstream out;
    out << "[";
    bool first = true;
    for (const CacheEntry& entry : slots_) {
        if (!entry.valid) {
            continue;
        }
        if (!first) {
            out << ", ";
        }
        first = false;
        out << entry.book.name << ":版本" << entry.book.version;
        if (!entry.pinned_request_ids.empty()) {
            out << ":保护" << entry.pinned_request_ids.size();
        }
    }
    out << "]";
    return out.str();
}
