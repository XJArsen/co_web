#include "Buffer.h"
#include <iostream>

void Buffer::append(const char* _str, int _size) {
    for (int idx = 0; idx < _size; idx++) {
        if (_str[idx] == '\0') break;
        buf.push_back(_str[idx]);
    }
}
ssize_t Buffer::size() {
    return buf.size();
}
const char* Buffer::c_str() {
    return buf.c_str();
}
void Buffer::clear() {
    buf.clear();
}
void Buffer::getline() {
    buf.clear();
    std::getline(std::cin, buf);
}
void Buffer::setBuf(const char* _buf) {
    buf.clear();
    buf.append(_buf);
}