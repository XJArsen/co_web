#include "Epoller.h"

Epoller::Epoller(int maxEvent) : epollFd(epoll_create(512)), events(maxEvent) {
    // assert(epollFd >= 0 && events_.size() > 0);
}

Epoller::~Epoller() {
    close(epollFd);
}

bool Epoller::AddFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev);
}

bool Epoller::ModFd(int fd, uint32_t events) {
    if (fd < 0) return false;
    epoll_event ev = {0};
    ev.data.fd = fd;
    ev.events = events;
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_MOD, fd, &ev);
}

bool Epoller::DelFd(int fd) {
    if (fd < 0) return false;
    return 0 == epoll_ctl(epollFd, EPOLL_CTL_DEL, fd, 0);
}

// 返回事件数量
int Epoller::Wait(int timeoutMs) {
    return epoll_wait(epollFd, &events[0], static_cast<int>(events.size()), timeoutMs);
}

// 获取事件的fd
int Epoller::GetEventFd(size_t i) const {
    assert(i < events.size() && i >= 0);
    return events[i].data.fd;
}

// 获取事件属性
uint32_t Epoller::GetEvents(size_t i) const {
    // assert(i < events_.size() && i >= 0);
    return events[i].events;
}