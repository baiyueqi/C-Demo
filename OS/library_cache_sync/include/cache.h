#ifndef LIBRARY_CACHE_SYNC_CACHE_H
#define LIBRARY_CACHE_SYNC_CACHE_H

#include "common.h"

#include <set>
#include <string>
#include <vector>

struct CacheEntry {
    bool valid = false;//表示槽位是否已经存放书籍
    Book book;//缓存中的电子书副本
    unsigned long last_access_tick = 0;// 单调递增的访问计数，用于 LRU 替换
    std::set<int> pinned_request_ids;// 这些请求复制完成前，该缓存项不能被淘汰
};

class Cache {
public:
    explicit Cache(int capacity);

    // 读者缓存命中时使用的普通复制路径
    bool findAndCopy(const std::string& name, Book& out);
    // 加载完成后的复制路径：只有拥有 request_id 的读者能复制对应保护项
    bool findPinnedAndCopy(const std::string& name, int request_id, Book& out);
    bool contains(const std::string& name) const;
    bool hasFreeSlot() const;
    // 缓存管理器插入/更新时先加保护标记，直到读者复制完成
    bool updateExistingAndPin(const Book& book, int request_id);
    void insertIntoFreeSlotAndPin(const Book& book, int request_id);
    bool replaceLruAndPin(const Book& book, int request_id);
    void releasePin(const std::string& name, int request_id);
    // 写者刷新路径；没有读者在等待该刷新，因此不需要加保护标记
    bool updateIfPresent(const Book& book);
    std::string snapshot() const;

private:
    std::vector<CacheEntry> slots_;
    unsigned long tick_ = 0;
};

#endif
