#include "EventLoop.h"
#include "Epoll.h"
#include "Channel.h"

EventLoop::EventLoop() {
    ep = new Epoll();
}
EventLoop::~EventLoop() {
    delete ep;
}

void EventLoop::loop() {
    while (!quit) {
        std::vector<Channel *> chs;
        chs = ep->poll();
        for (auto it : chs) {
            it->handleEvent();
        }
    }
}

void EventLoop::updateChannel(Channel *channel) {
    ep->updateChannel(channel);
}