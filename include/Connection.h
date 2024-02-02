// // Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// // Author: XJArsen
// // Date: 2024-01-20

// #ifndef INCLUDE_CONNECTION_H_
// #define INCLUDE_CONNECTION_H_
// #include <functional>
// #include "Buffer.h"
// #include "Channel.h"
// #include "EventLoop.h"
// #include "InetAddress.h"
// #include "Socket.h"
// class EventLoop;
// class Socket;
// class Channel;
// class InetAddress;
// class Buffer;
// class Connection {
//   private:
//     EventLoop *loop;
//     Socket *sock;
//     Channel *connectionChannel;
//     Buffer *send_buffer{nullptr};
//     Buffer *read_buffer{nullptr};
//     std::function<void(int)> deleteConnectionCallback;

//   public:
//     Connection(EventLoop *, Socket *);
//     ~Connection();
//     void echo(int);
//     void send(int);
//     void setDeleteConnectionCallback(std::function<void(int)>);
// };

// #endif
