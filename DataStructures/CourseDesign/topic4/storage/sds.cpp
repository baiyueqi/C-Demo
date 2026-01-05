#include "sds.h"

SDS::SDS() = default;

SDS::SDS(const std::string& str) : buffer(str) {}

size_t SDS::length() const {
    return buffer.size();
}

const char* SDS::c_str() const {
    return buffer.c_str();
}

std::string SDS::str() const {
    return buffer;
}

void SDS::set(const std::string& s) {
    buffer = s;
}
