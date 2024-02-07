#include "Socket.h"
#include "Log.h"

Socket::Socket() : fd_(-1) {
}

Socket::~Socket() {
    if (fd_ != -1) {
        close(fd_);
        fd_ = -1;
    }
}
bool Socket::Creat() {
    fd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (fd_ == -1) {
        LOG_ERROR("Socket create error");
        return false;
    }
    return true;
}
bool Socket::Bind(sockaddr_in _addr) {
    socklen_t addr_len = sizeof(_addr);
    if (::bind(fd_, (sockaddr*)&_addr, addr_len) == -1) {
        LOG_ERROR("Socket bind error ");
        return false;
    }
    return true;
}
bool Socket::Connect(sockaddr_in _addr) {
    socklen_t addr_len = sizeof(_addr);
    if (::connect(fd_, (sockaddr*)&_addr, addr_len) == -1) {
        LOG_ERROR("Socket connect error");
        return false;
    }
    return true;
}
bool Socket::Listen() {
    if (::listen(fd_, SOMAXCONN) == -1) {
        LOG_ERROR("Socket listen error");
        return false;
    }
    return true;
}
bool Socket::Listen(int sockcnt) {
    if (::listen(fd_, sockcnt) == -1) {
        LOG_ERROR("Socket listen error");
        return false;
    }
    return true;
}
bool Socket::Accept(int& clnt_fd, sockaddr_in& _addr) const {
    socklen_t addr_len = sizeof(_addr);
    clnt_fd = ::accept(fd_, (sockaddr*)&_addr, &addr_len);
    if (clnt_fd <= 0) {
        LOG_ERROR("Failed to accept Socket");
        return false;
    }
    return true;
}

bool Socket::setnonblocking() {
    if (fcntl(fd_, F_SETFL, fcntl(fd_, F_GETFL) | O_NONBLOCK) == -1) {
        LOG_ERROR("Socket set No Blocking Error");
        return false;
    }
    return true;
}

bool Socket::Setsockopt() {
    int optval = 1;
    /* 端口复用 */
    /* 只有最后一个套接字会正常接收数据。 */
    if (setsockopt(fd_, SOL_SOCKET, SO_REUSEADDR, (const void*)&optval, sizeof(int)) == -1) {
        LOG_ERROR("set socket setsockopt error !");
        return false;
    }
    return true;
}
int Socket::get_fd() {
    return fd_;
}
void Socket::set_fd(int _fd) {
    fd_ = _fd;
}