#include <cstring>
#include <cstdio>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <errno.h>
#include "util.h"
#include "Socket.h"
#include "InetAddress.h"
#include "Channel.h"
#include "Server.h"
#include "EventLoop.h"
const short MAX_EVENTS = 1024;
const short READ_BUFFER = 1024;

Server::Server(EventLoop *_loop) : loop(_loop) {
    Socket *serv_socket = new Socket();
    InetAddress *serv_addr = new InetAddress("127.0.0.1", 8888);
    serv_socket->bind(serv_addr);
    serv_socket->listen();
    Channel *servChannel = new Channel(loop, serv_socket->getFd());
    std::function<void()> cb =
        std::bind(&Server::newConnection, this, serv_socket);

    serv_socket->setnonblocking();
    servChannel->setCallback(cb);
    servChannel->enableReading();
}

Server::~Server() {
}

void Server::handleReadEvent(int sockfd) {
    char buf[READ_BUFFER];
    while (true) {
        bzero(&buf, sizeof buf);
        ssize_t read_bytes = read(sockfd, buf, sizeof(buf));
        if (read_bytes > 0) {
            printf("message from client fd %d: %s\n", sockfd, buf);
            write(sockfd, buf, sizeof(buf));
        } else if (read_bytes == 0) {
            printf("EOF, client fd %d disconnected", sockfd);
            close(sockfd);
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

void Server::newConnection(Socket *serv_socket) {
    InetAddress *client_addr = new InetAddress();
    Socket *client_sock = new Socket(serv_socket->accept(client_addr));

    printf("new client fd %d! IP : %s Port: %d\n", client_sock->getFd(),
           inet_ntoa(client_addr->addr.sin_addr),
           ntohs(client_addr->addr.sin_port));

    Channel *clientChannel = new Channel(loop, client_sock->getFd());
    std::function<void()> cb =
        std::bind(&Server::handleReadEvent, this, client_sock->getFd());

    clientChannel->setCallback(cb);
    clientChannel->enableReading();
    client_sock->setnonblocking();
}
