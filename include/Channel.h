// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-19

#ifndef INCLUDE_CHANNEL_H_
#define INCLUDE_CHANNEL_H_
#include <sys/epoll.h>

class Epoll;
class Channel {
   private:
    Epoll* ep;
    int fd;
    uint32_t events, revents;
    bool inEpoll;

   public:
    Channel();
    Channel(Epoll* _ev, int fd);
    ~Channel();

    void enableReading();

    void setInEpoll();
    void setEvents(uint32_t);
    void setRevents(uint32_t);

    int getFd();
    bool getInEpoll();
    uint32_t getEvents();
    uint32_t getRevents();
};
#endif
