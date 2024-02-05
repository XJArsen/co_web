// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_SOCKET_H_
#define INCLUDE_SOCKET_H_
#include "InetAddress.h"
class Socket {
  private:
    int fd;

  public:
    Socket();
    Socket(int);
    ~Socket();

    void bind(InetAddress*);
    void connect(InetAddress*);
    void listen();
    void listen(int);
    void accept(int&, sockaddr_in&) const;
    int getFd();
    void setnonblocking();
};
#endif
