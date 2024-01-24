// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-21

#ifndef INCLUDE_BUFFER_H_
#define INCLUDE_BUFFER_H_
#include <sys/uio.h>
#include <unistd.h>
#include <atomic>
#include <string>
#include <vector>
#include "util.h"
class Buffer {
  private:
    char* BeginPtr_();  // buffer开头
    const char* BeginPtr_() const;
    void MakeSpace_(size_t len);

    std::vector<char> buf;
    std::atomic<std::size_t> readPos;   // 读的下标
    std::atomic<std::size_t> writePos;  // 写的下标

  public:
    Buffer(int initBuffSize = 1024);
    ~Buffer() = default;

    size_t WritableBytes() const;
    size_t ReadableBytes() const;
    size_t PrependableBytes() const;

    const char* Peek() const;
    void EnsureWriteable(size_t len);
    void HasWritten(size_t len);

    void Retrieve(size_t len);
    void RetrieveUntil(const char* end);

    void RetrieveAll();
    std::string RetrieveAllToStr();

    const char* BeginWriteConst() const;
    char* BeginWrite();

    void Append(const std::string& str);
    void Append(const char* str, size_t len);
    void Append(const void* data, size_t len);
    void Append(const Buffer& buff);

    ssize_t ReadFd(int fd);
    ssize_t WriteFd(int fd);

    ssize_t size();
    const char* c_str();
    void clear();
    void getline();
    void setBuf(const std::string&);
};

#endif
