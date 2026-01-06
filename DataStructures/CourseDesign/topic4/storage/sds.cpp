#include "sds.h"
#include <cstring>
#include <cstdlib>

SDS* sdsCreate(const char* init) {
    size_t len = strlen(init);
    SDS* s = new SDS;
    s->len = len;
    s->cap = len + 1;
    s->buf = new char[s->cap];
    memcpy(s->buf, init, len);
    s->buf[len] = '\0';
    return s;
}

void sdsFree(SDS* s) {
    if (!s) return;
    delete[] s->buf;
    delete s;
}
