// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_SERVER_H_
#define INCLUDE_SERVER_H_
class Socket;
class EventLoop;
class Acceptor;
class Server {
   private:
    EventLoop *loop;
    Acceptor *acceptor;

   public:
    Server(EventLoop *);
    ~Server();
    void handleReadEvent(int);
    void newConnection(Socket *);
};

#endif
