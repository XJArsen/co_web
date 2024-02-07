// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-29

#ifndef INCLUDE_HTTP_HTTPCONN_H_
#define INCLUDE_HTTP_HTTPCONN_H_

#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <cstdlib>
#include "Buffer.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

class HttpConn {
  public:
    HttpConn();
    ~HttpConn();

    void init(int, const sockaddr_in&);
    ssize_t read(int*);
    ssize_t write(int*);
    void Close();
    int GetFd() const;
    int GetPort() const;
    const char* GetIP() const;
    sockaddr_in GetAddr() const;
    bool process();

    // 写的总长度
    int ToWriteBytes();

    bool IsKeepAlive() const;

    static bool isET;
    static const char* srcDir_;
    static std::atomic<int> userCount_;  // 原子，支持锁

  private:
    int fd;
    struct sockaddr_in addr_;

    bool isClose_;

    int iovCnt_;
    struct iovec iov[2];

    Buffer readBuff_;   // 读缓冲区
    Buffer writeBuff_;  // 写缓冲区

    HttpRequest request_;
    HttpResponse response_;
};

#endif
