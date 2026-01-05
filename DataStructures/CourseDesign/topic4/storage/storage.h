// storage.h
#pragma once
#include <memory>
#include <string>
#include "dict.h"
#include "sds.h"

enum class ObjectType {
    STRING
};

class RedisObject {
public:
    explicit RedisObject(const std::string& value);

    ObjectType type() const;
    std::string value() const;

private:
    ObjectType objType;
    SDS data;
};

class StorageEngine {
public:
    std::string set(const std::string& key, const std::string& value);
    std::string get(const std::string& key);
    std::string del(const std::string& key);
    std::string incr(const std::string& key);  // 添加这行

private:
    Dict dict;
};