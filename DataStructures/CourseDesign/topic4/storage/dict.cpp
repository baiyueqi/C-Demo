#include "dict.h"
#include <cstring>

Dict::Dict(size_t size) : table(size, nullptr) {}

Dict::~Dict() {
    for (auto& head : table) {
        while (head) {
            DictEntry* tmp = head;
            head = head->next;
            sdsFree(tmp->key);
            delete tmp;
        }
    }
}

size_t Dict::hash(const char* key) {
    size_t h = 5381;
    while (*key)
        h = ((h << 5) + h) + *key++;
    return h % table.size();
}

void Dict::set(SDS* key, void* value) {
    size_t idx = hash(key->buf);
    DictEntry* e = table[idx];
    while (e) {
        if (strcmp(e->key->buf, key->buf) == 0) {
            e->value = value;
            return;
        }
        e = e->next;
    }
    auto* ne = new DictEntry{key, value, table[idx]};
    table[idx] = ne;
}

void* Dict::get(const char* key) {
    size_t idx = hash(key);
    DictEntry* e = table[idx];
    while (e) {
        if (strcmp(e->key->buf, key) == 0)
            return e->value;
        e = e->next;
    }
    return nullptr;
}

bool Dict::del(const char* key) {
    size_t idx = hash(key);
    DictEntry* e = table[idx];
    DictEntry* prev = nullptr;
    while (e) {
        if (strcmp(e->key->buf, key) == 0) {
            if (prev) prev->next = e->next;
            else table[idx] = e->next;
            sdsFree(e->key);
            delete e;
            return true;
        }
        prev = e;
        e = e->next;
    }
    return false;
}
