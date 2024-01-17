#ifndef EPOLL_H
#define EPOLL_H
#include <vector>
class Epoll {
   private:
    int epfd;
    struct epoll_event *events;

   public:
    Epoll();
    ~Epoll();
    void addFd(int, uint32_t);
    std::vector<epoll_event> poll(int timeout = -1);
};

#endif