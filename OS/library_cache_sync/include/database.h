#ifndef LIBRARY_CACHE_SYNC_DATABASE_H
#define LIBRARY_CACHE_SYNC_DATABASE_H

#include "common.h"

#include <string>
#include <unordered_map>
#include <vector>

class Database {
public:
    Database();

    // 返回一本书的副本；外部同步由 Simulator 统一负责
    Book readBook(const std::string& name) const;
    // 更新一本书并提升版本号，然后返回更新后的副本
    Book writeBook(const std::string& name, int writer_id);
    // 返回稳定的书名列表，供读者和写者随机选择
    std::vector<std::string> allBookNames() const;

private:
    std::vector<std::string> book_order_;
    std::unordered_map<std::string, Book> books_;
};

#endif
