#pragma once
#include <cstddef>

struct SDS {
    size_t len;
    size_t cap;
    char* buf;
};

SDS* sdsCreate(const char* init);
void sdsFree(SDS* s);
