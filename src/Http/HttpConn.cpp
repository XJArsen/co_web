#include "HttpConn.h"
#include "Log.h"
HttpConn::HttpConn() : fd(-1), isClose(false) {
    bzero(&addr, sizeof addr);
}
HttpConn::~HttpConn() {
    Close();
}

void HttpConn::init(int _fd, const sockaddr_in& _addr) {
    userCount++;
    addr = _addr;
    fd = _fd;
    writeBuff.RetrieveAll();
    readBuff.RetrieveAll();
    isClose = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd, GetIP(), GetPort(), (int)userCount);
}
ssize_t HttpConn::read(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = readBuff.ReadFd(fd);
        if (len <= 0) {
            break;
        }
    } while (isET);
    return len;
}
ssize_t HttpConn::write(int* saveErrno) {
    ssize_t len = -1;
    do {
        len = writev(fd, iov, iovCnt);
        if (len <= 0) {
            break;
        }
        if (iov[0].iov_len + iov[1].iov_len == 0) {
            break;
        } else if (static_cast<size_t>(len) > iov[0].iov_len) {
            iov[1].iov_base = (uint8_t*)iov[1].iov_base + (len - iov[0].iov_len);
            iov[1].iov_len -= (len - iov[0].iov_len);
            if (iov[0].iov_len) {
                writeBuff.RetrieveAll();
                iov[0].iov_len = 0;
            }
        } else {
            iov[0].iov_base = (uint8_t*)iov[0].iov_base + len;
            iov[0].iov_len -= len;
            writeBuff.Retrieve(len);
        }

    } while (isET || ToWriteBytes() > 10240);
    return len;
}
void HttpConn::Close() {
    response.UnmapFile();
    if (isClose == false) {
        isClose = true;
        userCount--;
        close(fd);
        LOG_INFO("Client[%d](%s:%d) quit, userCount:%d", fd, GetIP(), GetPort(), (int)userCount);
    }
}
int HttpConn::GetFd() const {
    return fd;
}
int HttpConn::GetPort() const {
    return addr.sin_port;
}
const char* HttpConn::GetIP() const {
    return inet_ntoa(addr.sin_addr);
}
sockaddr_in HttpConn::GetAddr() const {
    return addr;
}
bool HttpConn::process() {
    request.Init();
    if (readBuff.ReadableBytes() <= 0) {
        return false;
    } else if (request.parse(readBuff)) {
        LOG_DEBUG("%s", request.Path().c_str());
        response.Init(srcDir, request.Path(), request.IsKeepAlive(), 200);
    } else {
        response.Init(srcDir, request.Path(), false, 400);
    }
    response.MakeResponse(writeBuff);
    iov[0].iov_base = const_cast<char*>(writeBuff.Peek());
    iov[0].iov_len = writeBuff.ReadableBytes();
    iovCnt = 1;

    // 文件
    if (response.FileLen() > 0 && response.File()) {
        iov[1].iov_base = response.File();
        iov[1].iov_len = response.FileLen();
        iovCnt = 2;
    }
    LOG_DEBUG("filesize:%d, %d  to %d", response.FileLen(), iovCnt, ToWriteBytes());
    return true;
}

// 写的总长度
int HttpConn::ToWriteBytes() {
    return iov[0].iov_len + iov[1].iov_len;
}

bool HttpConn::IsKeepAlive() const {
    return request.IsKeepAlive();
}