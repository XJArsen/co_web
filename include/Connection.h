// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-20

#ifndef INCLUDE_CONNECTION_H_
#define INCLUDE_CONNECTION_H_
#include <functional>
#include "EventLoop.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Buffer.h"
class EventLoop;
class Socket;
class Channel;
class InetAddress;
class Buffer;
class Connection {
   private:
    EventLoop *loop;
    Socket *sock;
    Channel *connectionChannel;
    Buffer *send_buffer{nullptr};
    Buffer *read_buffer{nullptr};
    std::function<void(Socket *)> deleteConnectionCallback;

   public:
    Connection(EventLoop *, Socket *);
    ~Connection();
    void echo(int);
    void setDeleteConnectionCallback(std::function<void(Socket *)>);
};

#endif
