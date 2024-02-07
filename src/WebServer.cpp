#include "WebServer.h"
#include <iostream>
#include "Log.h"
#include "SqlPool.h"
WebServer::WebServer(int _port, int _trigMode, int _timeoutMS, bool _OptLinger, int sqlPort,
                     const char* sqlUser, const char* sqlPwd, const char* dbName, int connPoolNum,
                     int threadNum, bool openLog, int logLevel, int logQueSize)
    : port_(_port),
      openLinger_(_OptLinger),
      timeoutMS_(_timeoutMS),
      isClose_(false),
      timer_(new Timer()),
      threadpool_(new ThreadPool(threadNum)),
      epoll_(new Epoller()) {
    // 是否打开日志标志
    if (openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);

        if (isClose_) {
            LOG_ERROR("========== Server init error!==========");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s", (listenEvent_ & EPOLLET ? "ET" : "LT"),
                     (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }

    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strcat(srcDir_, "/resources/");
    HttpConn::userCount_ = 0;
    HttpConn::srcDir_ = srcDir_;
    LOG_INFO("srcDir: %s", HttpConn::srcDir_);

    // 初始化操作
    SqlPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName,
                              connPoolNum);  // 连接池单例的初始化
    // 初始化事件和初始化socket(监听)
    InitEventMode(_trigMode);
    if (!InitSocket()) {
        isClose_ = true;
    }
}

WebServer::~WebServer() {
    close(socket_->get_fd());
    isClose_ = true;
    free(srcDir_);
    SqlPool::Instance()->ClosePool();
}

void WebServer::InitEventMode(int trigMode) {
    listenEvent_ = EPOLLRDHUP;               // 检测socket关闭
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;  // EPOLLONESHOT由一个线程处理
    switch (trigMode) {
        case 0:
            break;
        case 1:
            connEvent_ |= EPOLLET;
            break;
        case 2:
            listenEvent_ |= EPOLLET;
            break;
        case 3:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
        default:
            listenEvent_ |= EPOLLET;
            connEvent_ |= EPOLLET;
            break;
    }
    HttpConn::isET = (connEvent_ & EPOLLET);
}

void WebServer::Start() {
    int timeMS = -1; /* epoll_ wait timeout == -1 无事件将阻塞 */
    if (!isClose_) {
        LOG_INFO("========== Server start ==========");
    }
    while (!isClose_) {
        if (timeoutMS_ > 0) {
            timeMS = timer_->GetNextTick();
        }
        int eventCnt = epoll_->Wait(timeMS);
        for (int i = 0; i < eventCnt; i++) {
            /* 处理事件 */
            int fd = epoll_->GetEventFd(i);
            uint32_t events = epoll_->GetEvents(i);
            if (fd == socket_->get_fd()) {
                DealListen();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users_.count(fd) > 0);
                CloseConn(&users_[fd]);
            } else if (events & EPOLLIN) {
                assert(users_.count(fd) > 0);
                DealRead(&users_[fd]);
            } else if (events & EPOLLOUT) {
                assert(users_.count(fd) > 0);
                DealWrite(&users_[fd]);
            } else {
                LOG_ERROR("Unexpected event");
            }
        }
    }
}

void WebServer::SendError(int fd, const char* info) {
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0) {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::CloseConn(HttpConn* client) {
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoll_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if (timeoutMS_ > 0) {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn, this, &users_[fd]));
    }
    epoll_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

// 处理监听套接字，主要逻辑是accept新的套接字，并加入timer和epoller中
void WebServer::DealListen() {
    struct sockaddr_in addr;
    do {
        int fd = -1;
        if (socket_->Accept(fd, addr)) {
            if (HttpConn::userCount_ >= MAX_FD) {
                SendError(fd, "Server busy!");
                LOG_WARN("Clients is full!");
                return;
            }
        } else {
            return;
        }
        AddClient(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

// 处理读事件，主要逻辑是将OnRead加入线程池的任务队列中
void WebServer::DealRead(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    threadpool_->add(
        std::bind(&WebServer::OnRead, this, client));  // 这是一个右值，bind将参数和函数绑定
}

// 处理写事件，主要逻辑是将OnWrite加入线程池的任务队列中
void WebServer::DealWrite(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    threadpool_->add(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::ExtentTime(HttpConn* client) {
    assert(client);
    if (timeoutMS_ > 0) {
        timer_->adjust(client->GetFd(), timeoutMS_);
    }
}

void WebServer::OnRead(HttpConn* client) {
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);  // 读取客户端套接字的数据，读到httpconn的读缓存区
    if (ret <= 0 && readErrno != EAGAIN) {  // 读异常就关闭客户端
        CloseConn(client);
        return;
    }
    // 业务逻辑的处理（先读后处理）
    OnProcess(client);
}

/* 处理读（请求）数据的函数 */
void WebServer::OnProcess(HttpConn* client) {
    // 首先调用process()进行逻辑处理
    if (client->process()) {  // 根据返回的信息重新将fd置为EPOLLOUT（写）或EPOLLIN（读）
                              //读完事件就跟内核说可以写了
        epoll_->ModFd(client->GetFd(),
                      connEvent_ | EPOLLOUT);  // 响应成功，修改监听事件为写,等待OnWrite_()发送
    } else {
        //写完事件就跟内核说可以读了
        epoll_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

void WebServer::OnWrite(HttpConn* client) {
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if (client->ToWriteBytes() == 0) {
        /* 传输完成 */
        if (client->IsKeepAlive()) {
            // OnProcess(client);
            epoll_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);  // 回归换成监测读事件
            return;
        }
    } else if (ret < 0) {
        if (writeErrno == EAGAIN) {  // 缓冲区满了
            /* 继续传输 */
            epoll_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn(client);
}

/* Create listenFd */
bool WebServer::InitSocket() {
    int ret;
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(port_);

    socket_ = new Socket();
    if (socket_->Creat() && socket_->Setsockopt() && socket_->Bind(addr) && socket_->Listen(6) &&
        socket_->setnonblocking()) {
        int ret =
            epoll_->AddFd(socket_->get_fd(), listenEvent_ | EPOLLIN);  // 将监听套接字加入epoller
        if (ret == 0) {
            LOG_ERROR("Add listen error!");
            return false;
        }
        LOG_INFO("Server port:%d", port_);
    }
    LOG_INFO("Server port:%d", port_);
    return true;
}

// 设置非阻塞
int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}