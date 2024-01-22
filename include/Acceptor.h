// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_ACCPETOR_H_
#define INCLUDE_ACCPETOR_H_
#include <functional>
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "Socket.h"
class EventLoop;
class Socket;
class Channel;
class InetAddress;
class Acceptor {
  private:
    EventLoop *loop;
    Socket *sock;
    Channel *acceptChannel;
    InetAddress *addr;
    std::function<void(Socket *)> newConnectionCallback;

  public:
    Acceptor(EventLoop *);
    ~Acceptor();
    void acceptConnection();
    void setNewConnectionCallback(std::function<void(Socket *)>);
};

#endif
