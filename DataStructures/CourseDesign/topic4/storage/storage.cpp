#include "storage.h"
#include "resp.h"  // 添加这行

RedisObject::RedisObject(const std::string& value)
    : objType(ObjectType::STRING), data(value) {}

ObjectType RedisObject::type() const {
    return objType;
}

std::string RedisObject::value() const {
    return data.str();
}

std::string StorageEngine::set(const std::string& key, const std::string& value) {
    auto obj = std::make_shared<RedisObject>(value);
    dict.set(key, obj);
    return "+OK\r\n";
}

std::string StorageEngine::get(const std::string& key) {
    auto obj = dict.get(key);
    if (!obj) {
        return "$-1\r\n";
    }
    std::string val = obj->value();
    return "$" + std::to_string(val.size()) + "\r\n" + val + "\r\n";
}

std::string StorageEngine::del(const std::string& key) {
    bool removed = dict.del(key);
    return ":" + std::to_string(removed ? 1 : 0) + "\r\n";
}

std::string StorageEngine::incr(const std::string& key) {
    auto obj = dict.get(key);
    if (!obj) {
        // 键不存在，创建并设置为 1
        auto new_obj = std::make_shared<RedisObject>("1");
        dict.set(key, new_obj);
        return ":1\r\n";
    }
    
    try {
        std::string current = obj->value();
        int value = std::stoi(current);
        value++;
        
        auto new_obj = std::make_shared<RedisObject>(std::to_string(value));
        dict.set(key, new_obj);
        return ":" + std::to_string(value) + "\r\n";
    } catch (...) {
        // 这里不能直接调用 RESP::encodeError，因为 storage.cpp 不知道 RESP 类
        // 改为返回一个错误响应
        return "-ERR value is not an integer\r\n";
    }
}