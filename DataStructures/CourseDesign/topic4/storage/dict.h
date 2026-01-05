#pragma once
#include <unordered_map>
#include <string>
#include <memory>

class RedisObject;

class Dict {
public:
    bool set(const std::string& key, std::shared_ptr<RedisObject> value);
    std::shared_ptr<RedisObject> get(const std::string& key);
    bool del(const std::string& key);

private:
    std::unordered_map<std::string, std::shared_ptr<RedisObject>> table;
};
