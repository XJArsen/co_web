// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-02-01

#ifndef INCLUDE_EPOLLER_H_
#define INCLUDE_EPOLLER_H_

#include <assert.h>  // close()
#include <errno.h>
#include <sys/epoll.h>  //epoll_ctl()
#include <unistd.h>     // close()
#include <vector>

class Epoller {
  public:
    explicit Epoller(int maxEvent = 1024);
    ~Epoller();

    bool AddFd(int fd, uint32_t events);
    bool ModFd(int fd, uint32_t events);
    bool DelFd(int fd);
    int Wait(int timeoutMs = -1);
    int GetEventFd(size_t i) const;
    uint32_t GetEvents(size_t i) const;

  private:
    int epollFd;
    std::vector<struct epoll_event> events;
};
#endif
