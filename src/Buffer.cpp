#include "Buffer.h"
#include <iostream>

Buffer::Buffer(int initBuffSize) : buf(initBuffSize), readPos(0), writePos(0) {
}

size_t Buffer::WritableBytes() const {
    return buf.size() - writePos;
}
size_t Buffer::ReadableBytes() const {
    return writePos - readPos;
}
size_t Buffer::PrependableBytes() const {
    return readPos;
}

const char* Buffer::Peek() const {
    return &buf[readPos];
}
void Buffer::EnsureWriteable(size_t len) {
    if (len > WritableBytes()) {
        MakeSpace(len);
    }
}
void Buffer::HasWritten(size_t len) {
    writePos += len;
}

void Buffer::Retrieve(size_t len) {
    readPos += len;
}
void Buffer::RetrieveUntil(const char* end) {
    // assert(Peek() <= end);
    Retrieve(end - Peek());
}

void Buffer::RetrieveAll() {
    bzero(&buf[0], buf.size());
    readPos = writePos = 0;
}
std::string Buffer::RetrieveAllToStr() {
    std::string str(Peek(), ReadableBytes());
    RetrieveAll();
    return str;
}

const char* Buffer::BeginWriteConst() const {
    return &buf[writePos];
}
char* Buffer::BeginWrite() {
    return &buf[writePos];
}

void Buffer::Append(const char* str, size_t len) {
    EnsureWriteable(len);
    std::copy(str, str + len, BeginWrite());
    HasWritten(len);
}
void Buffer::Append(const std::string& str) {
    Append(str.c_str(), str.size());
}
void Buffer::Append(const void* data, size_t len) {
    Append(static_cast<const char*>(data), len);
}
void Buffer::Append(const Buffer& buff) {
    Append(buff.Peek(), buff.ReadableBytes());
}

ssize_t Buffer::ReadFd(int fd, int* Errno) {
    char buff[65535];  // 栈区
    struct iovec iov[2];
    size_t writeable = WritableBytes();
    iov[0].iov_base = BeginWrite();
    iov[0].iov_len = writeable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    ssize_t len = readv(fd, iov, 2);
    if (len < 0) {
        *Errno = errno;
    } else if (static_cast<size_t>(len) <= writeable) {
        writePos += len;
    } else {
        writePos = buf.size();
        Append(buff, static_cast<size_t>(len - writeable));
    }
    return len;
}
ssize_t Buffer::WriteFd(int fd, int* Errno) {
    ssize_t len = write(fd, Peek(), ReadableBytes());
    if (len < 0) {
        *Errno = errno;
    } else {
        Retrieve(len);
    }
    return len;
}
char* Buffer::BeginPtr() {
    return &buf[0];
}
const char* Buffer::BeginPtr() const {
    return &buf[0];
}
void Buffer::MakeSpace(size_t len) {
    if (WritableBytes() + PrependableBytes() < len) {
        buf.resize(writePos + len + 1);
    } else {
        size_t readable = ReadableBytes();
        std::copy(BeginPtr() + readPos, BeginPtr() + writePos, BeginPtr());
        readPos = 0;
        writePos = readable;
        errif(readable == ReadableBytes(), "MakeSpace error!\n");
    }
}
// ssize_t Buffer::size() {
//     return buf.size();
// }
// const char* Buffer::c_str() {
//     return &*buf.begin();
// }
// void Buffer::clear() {
//     buf.clear();
// }
// void Buffer::getline() {
//     buf.clear();
//     std::string temp;
//     std::getline(std::cin, temp);
//     buf = std::vector<char>(temp.begin(), temp.end());
// }
// void Buffer::setBuf(const std::string& str) {
//     buf.clear();
//     Append(str);
// }