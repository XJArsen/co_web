// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_CHANNEL_H_
#define INCLUDE_CHANNEL_H_
#include <sys/epoll.h>
#include <functional>
class EventLoop;
class Channel {
   private:
    EventLoop* loop;
    int fd;
    uint32_t events, revents;
    bool inEpoll;
    std::function<void()> callback;

   public:
    Channel();
    Channel(EventLoop*, int);
    ~Channel();

    void enableReading();
    void handleEvent();
    void setCallback(std::function<void()>);

    void setInEpoll();
    void setEvents(uint32_t);
    void setRevents(uint32_t);

    int getFd();
    bool getInEpoll();
    uint32_t getEvents();
    uint32_t getRevents();
};
#endif
