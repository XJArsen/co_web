#include "Connection.h"
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include "Buffer.h"
#include "util.h"
const short READ_BUFFER = 1024;
Connection::Connection(EventLoop *_loop, Socket *_sock) : loop(_loop), sock(_sock) {
    connectionChannel = new Channel(loop, sock->getFd());
    std::function<void()> cb = std::bind(&Connection::echo, this, sock->getFd());
    sock->setnonblocking();
    connectionChannel->enableReading();
    connectionChannel->useET();
    connectionChannel->setReadCallback(cb);
    connectionChannel->setUseThreadPool(true);
    read_buffer = new Buffer();
}

Connection::~Connection() {
    delete connectionChannel;
    delete read_buffer;
    delete sock;
}
void Connection::setDeleteConnectionCallback(std::function<void(int)> _cb) {
    deleteConnectionCallback = _cb;
}

void Connection::echo(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {
        bzero(&buf, sizeof buf);
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            read_buffer->append(buf, read_bytes);
        } else if (read_bytes == 0) {
            printf("EOF, client fd %d disconnected\n", sockfd);
            deleteConnectionCallback(sock->getFd());
            break;
        } else if (read_bytes == -1) {
            if (errno == EINTR) {
                printf("continue reading");
                continue;
            } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
                printf("message from client fd %d: %s\n", sockfd, read_buffer->c_str());
                send(sockfd);
                read_buffer->clear();
                break;
            } else {
                printf("Connection reset by peer\n");
                deleteConnectionCallback(sockfd);
                break;
            }
        }
    }
}
void Connection::send(int sockfd) {
    char buf[read_buffer->size()];
    strcpy(buf, read_buffer->c_str());
    int data_size = read_buffer->size();
    int data_left = data_size;
    while (data_left > 0) {
        ssize_t bytes_write = write(sockfd, buf + data_size - data_left, data_left);
        if (bytes_write == -1 && errno == EAGAIN) {
            break;
        }
        data_left -= bytes_write;
    }
}
