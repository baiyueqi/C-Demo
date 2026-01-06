#include "storage.h"

bool StorageEngine::set(const std::string& key, const std::string& value) {
    SDS* k = sdsCreate(key.c_str());
    SDS* v = sdsCreate(value.c_str());
    dict.set(k, v);
    return true;
}

std::optional<std::string> StorageEngine::get(const std::string& key) {
    auto* v = static_cast<SDS*>(dict.get(key.c_str()));
    if (!v) return std::nullopt;
    return std::string(v->buf);
}

bool StorageEngine::del(const std::string& key) {
    return dict.del(key.c_str());
}
