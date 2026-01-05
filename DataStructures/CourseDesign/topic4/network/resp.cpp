#include "resp.h"
#include <sstream>
#include <iostream>

std::vector<std::string> RESP::parse(const std::string& input) {
    std::vector<std::string> result;
    std::istringstream stream(input);
    
    if (input.empty()) return result;
    
    char type;
    stream >> type;
    
    // 如果是 RESP 数组格式：*3\r\n$3\r\nSET\r\n$1\r\na\r\n$3\r\n123\r\n
    if (type == '*') {
        int array_len;
        stream >> array_len;
        
        // 跳过 \r\n
        stream.ignore(2);
        
        for (int i = 0; i < array_len; i++) {
            char bulk_type;
            stream >> bulk_type;  // 应该是 '$'
            
            int str_len;
            stream >> str_len;
            
            // 跳过 \r\n
            stream.ignore(2);
            
            // 读取字符串
            std::string str;
            for (int j = 0; j < str_len; j++) {
                char c;
                stream.get(c);
                str.push_back(c);
            }
            
            result.push_back(str);
            
            // 跳过末尾的 \r\n
            stream.ignore(2);
        }
    }
    // 如果不是 RESP 格式，按空格分割（兼容测试）
    else {
        // 把第一个字符放回去
        stream.unget();
        std::string all;
        std::getline(stream, all);
        
        std::istringstream token_stream(all);
        std::string token;
        while (token_stream >> token) {
            result.push_back(token);
        }
    }
    
    return result;
}