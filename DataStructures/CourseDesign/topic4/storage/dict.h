#pragma once
#include <vector>
#include "sds.h"

struct DictEntry {
    SDS* key;
    void* value;
    DictEntry* next;
};

class Dict {
public:
    Dict(size_t size = 1024);
    ~Dict();

    void set(SDS* key, void* value);
    void* get(const char* key);
    bool del(const char* key);

private:
    size_t hash(const char* key);
    std::vector<DictEntry*> table;
};
