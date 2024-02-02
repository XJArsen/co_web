// // Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// // Author: XJArsen
// // Date: 2024-01-19

// #ifndef INCLUDE_EPOLL_H_
// #define INCLUDE_EPOLL_H_
// #include <sys/epoll.h>
// #include <vector>
// #include "Channel.h"
// class Epoll {
//   private:
//     int epfd;
//     struct epoll_event *events;

//   public:
//     Epoll();
//     ~Epoll();
//     void updateChannel(Channel *);
//     void deleteChannel(Channel *);
//     std::vector<Channel *> poll(int timeout = -1);
// };
// #endif
