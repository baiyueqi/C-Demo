#include "database.h"

#include <stdexcept>

Database::Database()
{
    // 固定数据集让随机选择在演示和报告中可复现
    book_order_ = {
        "操作系统",
        "Linux基础",
        "数据库系统",
        "编译原理",
        "计算机网络",
        "信息安全",
        "算法设计",
    };

    for (const std::string& name : book_order_) {
        books_[name] = Book{name, name + "的初始内容", 1};
    }
}

//读操作
Book Database::readBook(const std::string& name) const
{
    const auto it = books_.find(name);
    if (it == books_.end()) {
        throw std::runtime_error("未知书籍：" + name);
    }
    return it->second;
}

//写操作
Book Database::writeBook(const std::string& name, int writer_id)
{
    auto it = books_.find(name);
    if (it == books_.end()) {
        throw std::runtime_error("未知书籍：" + name);
    }

    Book& book = it->second;
    // 版本号递增可以让写操作效果在日志和缓存快照中可见
    ++book.version;
    book.content = "由写者" + std::to_string(writer_id) +
                   "更新，版本" + std::to_string(book.version);
    return book;
}

std::vector<std::string> Database::allBookNames() const
{
    return book_order_;
}
