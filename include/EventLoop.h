// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_EVENTLOOP_H_
#define INCLUDE_EVENTLOOP_H_
class Channel;
class Epoll;
class EventLoop {
   private:
    Epoll *ep;
    bool quit;

   public:
    EventLoop();
    ~EventLoop();
    void loop();
    void updateChannel(Channel *);
};
#endif
