#pragma once
#include <string>
#include <optional>
#include "dict.h"

class StorageEngine {
public:
    bool set(const std::string& key, const std::string& value);
    std::optional<std::string> get(const std::string& key);
    bool del(const std::string& key);

private:
    Dict dict;
};
