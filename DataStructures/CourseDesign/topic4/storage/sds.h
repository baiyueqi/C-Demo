#pragma once
#include <string>

class SDS {
public:
    SDS();
    explicit SDS(const std::string& str);

    size_t length() const;
    const char* c_str() const;
    std::string str() const;

    void set(const std::string& s);

private:
    std::string buffer;  // 简化实现，底层使用 std::string
};
