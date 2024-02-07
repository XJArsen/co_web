#include "HttpConn.h"
#include "Log.h"
const char* HttpConn::srcDir_;
std::atomic<int> HttpConn::userCount_;
bool HttpConn::isET;
HttpConn::HttpConn() : fd(-1), isClose_(false) {
    bzero(&addr_, sizeof addr_);
}
HttpConn::~HttpConn() {
    Close();
}

void HttpConn::init(int _fd, const sockaddr_in& _addr) {
    userCount_++;
    addr_ = _addr;
    fd = _fd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd, GetIP(), GetPort(), (int)userCount_);
}
ssize_t HttpConn::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff_.ReadFd(fd, saveErrno);
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}
ssize_t HttpConn::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd, iov, iovCnt_);
        if (len <= 0) {
            *saveErrno = errno;
            break;
        }
        if (iov[0].iov_len + iov[1].iov_len == 0) {
            break;
        } else if (static_cast<size_t>(len) > iov[0].iov_len) {
            iov[1].iov_base = (uint8_t*)iov[1].iov_base + (len - iov[0].iov_len);
            iov[1].iov_len -= (len - iov[0].iov_len);
            if (iov[0].iov_len) {
                writeBuff_.RetrieveAll();
                iov[0].iov_len = 0;
            }
        } else {
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len;
            iov[0].iov_len -= len;
            writeBuff_.Retrieve(len);
        }

    } while (isET || ToWriteBytes() > 10240);
    return len;
}
void HttpConn::Close() {
    response_.UnmapFile();
    if (isClose_ == false) {
        isClose_ = true;
        userCount_--;
        close(fd);
        LOG_INFO("Client[%d](%s:%d) quit, userCount:%d", fd, GetIP(), GetPort(), (int)userCount_);
    }
}
int HttpConn::GetFd() const {
    return fd;
}
int HttpConn::GetPort() const {
    return addr_.sin_port;
}
const char* HttpConn::GetIP() const {
    return inet_ntoa(addr_.sin_addr);
}
sockaddr_in HttpConn::GetAddr() const {
    return addr_;
}
bool HttpConn::process() {
    request_.Init();
    if (readBuff_.ReadableBytes() <= 0) {
        return false;
    } else if (request_.parse(readBuff_)) {
        LOG_DEBUG("%s", request_.Path().c_str());
        response_.Init(srcDir_, request_.Path(), request_.IsKeepAlive(), 200);
    } else {
        response_.Init(srcDir_, request_.Path(), false, 400);
    }
    response_.MakeResponse(writeBuff_);
    iov[0].iov_base = const_cast<char*>(writeBuff_.Peek());
    iov[0].iov_len = writeBuff_.ReadableBytes();
    iovCnt_ = 1;

    // 文件
    if (response_.FileLen() > 0 && response_.File()) {
        iov[1].iov_base = response_.File();
        iov[1].iov_len = response_.FileLen();
        iovCnt_ = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response_.FileLen(), iovCnt_, ToWriteBytes());
    return true;
}

// 写的总长度
int HttpConn::ToWriteBytes() {
    return iov[0].iov_len + iov[1].iov_len;
}

bool HttpConn::IsKeepAlive() const {
    return request_.IsKeepAlive();
}