#include "Channel.h"
#include "Epoll.h"
#include "EventLoop.h"
Channel::Channel() {
}
Channel::~Channel() {
}
Channel::Channel(EventLoop* _loop, int _fd)
    : loop(_loop), fd(_fd), events(0), revents(0), inEpoll(false) {
}

int Channel::getFd() {
    return fd;
}
void Channel::enableReading() {
    events = EPOLLIN | EPOLLET;
    loop->updateChannel(this);
}
void Channel::setInEpoll() {
    inEpoll = true;
}
bool Channel::getInEpoll() {
    return inEpoll;
}

void Channel::setEvents(uint32_t _ev) {
    events = _ev;
}
void Channel::setRevents(uint32_t _ev) {
    revents = _ev;
}
uint32_t Channel::getEvents() {
    return events;
}
uint32_t Channel::getRevents() {
    return revents;
}
void Channel::handleEvent() {
    callback();
}
void Channel::setCallback(std::function<void()> _cb) {
    callback = _cb;
}