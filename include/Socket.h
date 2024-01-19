// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_SOCKET_H_
#define INCLUDE_SOCKET_H_
class InetAddress;
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
    int accept(InetAddress*);
    int getFd();
    void setnonblocking();
};
#endif
