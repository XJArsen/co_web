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
    uint32_t events, ready;
    bool inEpoll, useThreadPool;
    std::function<void()> readCallback, writedCallback;

  public:
    Channel();
    Channel(EventLoop*, int);
    ~Channel();

    void enableReading();
    void handleEvent();
    void useET();

    void setInEpoll(bool);
    void setUseThreadPool(bool);
    void setEvents(uint32_t);
    void setReady(uint32_t);
    void setReadCallback(std::function<void()>);
    void setWritedCallback(std::function<void()>);

    int getFd();
    bool getInEpoll();
    uint32_t getEvents();
    uint32_t getReady();
};
#endif
