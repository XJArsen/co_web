#ifndef CHANNEL_H
#define CHANNEL_H
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