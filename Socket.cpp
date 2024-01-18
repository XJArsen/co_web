#include <sys/socket.h>
#include <fcntl.h>
#include <unistd.h>
#include "Socket.h"
#include "util.h"
#include "InetAddress.h"

Socket::Socket() : fd(-1) {
    fd = socket(AF_INET, SOCK_STREAM, 0);
    errif(fd == -1, "socket create error");
}
Socket::Socket(int _fd) : fd(_fd) {
    errif(fd == -1, "socket create error");
}
Socket::~Socket() {
    if (fd != -1) {
        close(fd);
        fd = -1;
    }
}
void Socket::bind(InetAddress* addr) {
    errif(::bind(fd, (sockaddr*)&addr->addr, addr->addr_len) == -1,
          "socket bind error");
}
void Socket::connect(InetAddress* addr) {
    errif(::connect(fd, (sockaddr*)&addr->addr, addr->addr_len) == -1,
          "socket connect error");
}
void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}
int Socket::accept(InetAddress* addr) {
    int client_sockfd = ::accept(fd, (sockaddr*)&addr->addr, &addr->addr_len);
    errif(client_sockfd == -1, "socket accept error");
    return client_sockfd;
}
int Socket::getFd() {
    return fd;
}
void Socket::setnonblocking() {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}