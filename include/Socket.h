// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_SOCKET_H_
#define INCLUDE_SOCKET_H_
#include <arpa/inet.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
class Socket {
  private:
    int fd_;

  public:
    Socket();
    ~Socket();
    bool Creat();
    bool Bind(sockaddr_in);
    bool Connect(sockaddr_in);
    bool Listen();
    bool Listen(int);
    bool Accept(int&, sockaddr_in&) const;
    bool setnonblocking();
    bool Setsockopt();

    int get_fd();
    void set_fd(int);
};
#endif
