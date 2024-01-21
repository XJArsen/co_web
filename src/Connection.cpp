#include "Connection.h"
#include <cstring>
#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include "Buffer.h"
#include "util.h"
const short READ_BUFFER = 1024;
Connection::Connection(EventLoop *_loop, Socket *_sock)
    : loop(_loop), sock(_sock) {
    connectionChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb =
        std::bind(&Connection::echo, this, sock->getFd());
    sock->setnonblocking();
    connectionChannel->setCallback(cb);
    connectionChannel->enableReading();
    read_buffer = new Buffer();
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
            read_buffer->append(buf, read_bytes);
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
                printf("message from client fd %d: %s\n", sockfd,
                       read_buffer->c_str());
                errif(write(sockfd, read_buffer->c_str(),
                            read_buffer->size()) == -1,
                      "socket write error");

                read_buffer->clear();

                break;
            }
        }
    }
}
void Connection::setDeleteConnectionCallback(
    std::function<void(Socket *)> _cb) {
    deleteConnectionCallback = _cb;
}