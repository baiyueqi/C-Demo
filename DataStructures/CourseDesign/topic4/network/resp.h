// resp.h
#pragma once
#include <vector>
#include <string>
#include <variant>

class RESP {
public:
    // 解析 RESP 协议数据
    static std::vector<std::string> parse(const std::string& input);
    
    // 编码响应
    static std::string encodeSimpleString(const std::string& str);
    static std::string encodeError(const std::string& err);
    static std::string encodeBulkString(const std::string& str);
    static std::string encodeNull();
    static std::string encodeInteger(int value);
    
    // 工具函数
    static std::string ok();
    static std::string pong();
};