#include "Channel.h"
#include <sys/epoll.h>
#include <unistd.h>
#include <iostream>
#include "Epoll.h"
#include "EventLoop.h"
Channel::Channel() = default;
Channel::Channel(EventLoop* _loop, int _fd)
    : loop(_loop), fd(_fd), events(0), ready(0), inEpoll(false), useThreadPool(false) {
}

Channel::~Channel() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}

void Channel::enableReading() {
    events = EPOLLIN | EPOLLPRI;
    loop->updateChannel(this);
}
void Channel::handleEvent() {
    if (read && (EPOLLIN | EPOLLPRI)) {
        if (useThreadPool) {
            loop->addThread(readCallback);
        } else {
            readCallback();
        }
    }

    if (ready & EPOLLOUT) {
        if (useThreadPool) {
            loop->addThread(writedCallback);
        } else {
            writedCallback();
        }
    }
}
void Channel::useET() {
    events |= EPOLLET;
    loop->updateChannel(this);
}
void Channel::setEvents(uint32_t _ev) {
    events = _ev;
}
void Channel::setReady(uint32_t _ev) {
    ready = _ev;
}

void Channel::setReadCallback(std::function<void()> _cb) {
    readCallback = _cb;
}
void Channel::setWritedCallback(std::function<void()> _cb) {
    writedCallback = _cb;
}
void Channel::setUseThreadPool(bool use) {
    useThreadPool = use;
}
void Channel::setInEpoll(bool in) {
    inEpoll = in;
}

int Channel::getFd() {
    return fd;
}
bool Channel::getInEpoll() {
    return inEpoll;
}
uint32_t Channel::getEvents() {
    return events;
}
uint32_t Channel::getReady() {
    return ready;
}