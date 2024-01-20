#include "Connection.h"
#include <cstring>
#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
const short READ_BUFFER = 1024;
Connection::Connection(EventLoop *_loop, Socket *_sock)
    : loop(_loop), sock(_sock), connectionChannel(nullptr) {
    connectionChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb =
        std::bind(&Connection::echo, this, sock->getFd());
    sock->setnonblocking();
    connectionChannel->setCallback(cb);
    connectionChannel->enableReading();
}

Connection::~Connection() {
    delete connectionChannel;
    delete sock;
}
void Connection::echo(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {
        bzero(&buf, sizeof buf);
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        } else if (read_bytes == 0) {
            printf("EOF, client fd %d disconnected", sockfd);
            deleteConnectionCallback(sock);
            break;
        } else if (read_bytes == -1) {
            if (errno == EINTR) {
                printf("continue reading");
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("finish reading once, errno: %d\n", errno);
                break;
            }
        }
    }
}
void Connection::setDeleteConnectionCallback(
    std::function<void(Socket *)> _cb) {
    deleteConnectionCallback = _cb;
}