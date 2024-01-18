#ifndef EPOLL_H
#define EPOLL_H
#include <vector>
class Channel;
class Epoll {
   private:
    int epfd;
    struct epoll_event *events;

   public:
    Epoll();
    ~Epoll();
    void updateChannel(Channel *);
    std::vector<Channel *> poll(int timeout = -1);
};

#endif