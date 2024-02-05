#include "Socket.h"
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>
#include "InetAddress.h"
#include "util.h"

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
void Socket::bind(InetAddress* _addr) {
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    errif(::bind(fd, (sockaddr*)&addr, addr_len) == -1, "socket bind error");
}
void Socket::connect(InetAddress* _addr) {
    struct sockaddr_in addr = _addr->getAddr();
    socklen_t addr_len = _addr->getAddr_len();
    errif(::connect(fd, (sockaddr*)&addr, addr_len) == -1, "socket connect error");
}
void Socket::listen() {
    errif(::listen(fd, SOMAXCONN) == -1, "socket listen error");
}
void Socket::listen(int sockcnt) {
    errif(::listen(fd, sockcnt) == -1, "socket listen error");
}
void Socket::accept(int& clnt_fd, sockaddr_in& _addr) const {
    socklen_t addr_len = sizeof(_addr);
    clnt_fd = ::accept(fd, (sockaddr*)&_addr, &addr_len);
    if (clnt_fd == -1) {
        LOG_ERROR("Failed to accept socket");
    }
}
int Socket::getFd() {
    return fd;
}
void Socket::setnonblocking() {
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}