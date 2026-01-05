#include "dict.h"
#include "storage.h"

bool Dict::set(const std::string& key, std::shared_ptr<RedisObject> value) {
    table[key] = value;
    return true;
}

std::shared_ptr<RedisObject> Dict::get(const std::string& key) {
    auto it = table.find(key);
    if (it == table.end()) {
        return nullptr;
    }
    return it->second;
}

bool Dict::del(const std::string& key) {
    return table.erase(key) > 0;
}
