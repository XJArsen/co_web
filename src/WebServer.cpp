#include "WebServer.h"
#include <iostream>
#include "Epoll.h"
#include "Log.h"
#include "SqlPool.h"
WebServer::WebServer(int _port, int _trigMode, int _timeoutMS, bool _OptLinger, int sqlPort,
                     const char* sqlUser, const char* sqlPwd, const char* dbName, int connPoolNum,
                     int threadNum, bool openLog, int logLevel, int logQueSize)
    : port(_port),
      openLinger(_OptLinger),
      timeoutMS(_timeoutMS),
      isClose(false),
      timer(new Timer()),
      threadpool(new ThreadPool(threadNum)),
      epoll(new Epoller()) {
    // 是否打开日志标志
    if (openLog) {
        Log::Instance()->init(logLevel, "./log", ".log", logQueSize);

        if (isClose) {
            LOG_ERROR("========== Server init error!==========");
        } else {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s", (listenEvent & EPOLLET ? "ET" : "LT"),
                     (connEvent & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", logLevel);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }

    srcDir = getcwd(nullptr, 256);
    assert(srcDir);
    strcat(srcDir, "/resources/");
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir;
    LOG_INFO("srcDir: %s", HttpConn::srcDir);

    // 初始化操作
    SqlPool::Instance()->Init("localhost", sqlPort, sqlUser, sqlPwd, dbName,
                              connPoolNum);  // 连接池单例的初始化
    // 初始化事件和初始化socket(监听)
    InitEventMode(_trigMode);
    if (!InitSocket()) {
        isClose = true;
    }
}

WebServer::~WebServer() {
    close(sock->get_fd());
    isClose = true;
    free(srcDir);
    SqlPool::Instance()->ClosePool();
}

void WebServer::InitEventMode(int trigMode) {
    listenEvent = EPOLLRDHUP;               // 检测socket关闭
    connEvent = EPOLLONESHOT | EPOLLRDHUP;  // EPOLLONESHOT由一个线程处理
    switch (trigMode) {
        case 0:
            break;
        case 1:
            connEvent |= EPOLLET;
            break;
        case 2:
            listenEvent |= EPOLLET;
            break;
        case 3:
            listenEvent |= EPOLLET;
            connEvent |= EPOLLET;
            break;
        default:
            listenEvent |= EPOLLET;
            connEvent |= EPOLLET;
            break;
    }
    HttpConn::isET = (connEvent & EPOLLET);
}

void WebServer::Start() {
    int timeMS = -1; /* epoll wait timeout == -1 无事件将阻塞 */
    if (!isClose) {
        LOG_INFO("========== Server start ==========");
    }
    while (!isClose) {
        if (timeoutMS > 0) {
            timeMS = timer->GetNextTick();
        }
        int eventCnt = epoll->Wait(timeMS);
        for (int i = 0; i < eventCnt; i++) {
            /* 处理事件 */
            int fd = epoll->GetEventFd(i);
            uint32_t events = epoll->GetEvents(i);
            if (fd == sock->get_fd()) {
                DealListen();
            } else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                assert(users.count(fd) > 0);
                CloseConn(&users[fd]);
            } else if (events & EPOLLIN) {
                assert(users.count(fd) > 0);
                DealRead(&users[fd]);
            } else if (events & EPOLLOUT) {
                assert(users.count(fd) > 0);
                DealWrite(&users[fd]);
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
    epoll->DelFd(client->GetFd());
    client->Close();
}

void WebServer::AddClient(int fd, sockaddr_in addr) {
    assert(fd > 0);
    users[fd].init(fd, addr);
    if (timeoutMS > 0) {
        timer->add(fd, timeoutMS, std::bind(&WebServer::CloseConn, this, &users[fd]));
    }
    epoll->AddFd(fd, EPOLLIN | connEvent);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users[fd].GetFd());
}

// 处理监听套接字，主要逻辑是accept新的套接字，并加入timer和epoller中
void WebServer::DealListen() {
    struct sockaddr_in addr;
    do {
        int fd = -1;
        if (sock->Accept(fd, addr)) {
            if (HttpConn::userCount >= MAX_FD) {
                SendError(fd, "Server busy!");
                LOG_WARN("Clients is full!");
                return;
            }
        } else {
            return;
        }
        AddClient(fd, addr);
    } while (listenEvent & EPOLLET);
}

// 处理读事件，主要逻辑是将OnRead加入线程池的任务队列中
void WebServer::DealRead(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    threadpool->add(
        std::bind(&WebServer::OnRead, this, client));  // 这是一个右值，bind将参数和函数绑定
}

// 处理写事件，主要逻辑是将OnWrite加入线程池的任务队列中
void WebServer::DealWrite(HttpConn* client) {
    assert(client);
    ExtentTime(client);
    threadpool->add(std::bind(&WebServer::OnWrite, this, client));
}

void WebServer::ExtentTime(HttpConn* client) {
    assert(client);
    if (timeoutMS > 0) {
        timer->adjust(client->GetFd(), timeoutMS);
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
        epoll->ModFd(client->GetFd(),
                     connEvent | EPOLLOUT);  // 响应成功，修改监听事件为写,等待OnWrite_()发送
    } else {
        //写完事件就跟内核说可以读了
        epoll->ModFd(client->GetFd(), connEvent | EPOLLIN);
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
            epoll->ModFd(client->GetFd(), connEvent | EPOLLIN);  // 回归换成监测读事件
            return;
        }
    } else if (ret < 0) {
        if (writeErrno == EAGAIN) {  // 缓冲区满了
            /* 继续传输 */
            epoll->ModFd(client->GetFd(), connEvent | EPOLLOUT);
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
    addr.sin_port = htons(port);

    sock = new Socket();
    if (sock->Creat() && sock->Setsockopt() && sock->Bind(addr) && sock->Listen(6) &&
        sock->setnonblocking()) {
        int ret = epoll->AddFd(sock->get_fd(), listenEvent | EPOLLIN);  // 将监听套接字加入epoller
        if (ret == 0) {
            LOG_ERROR("Add listen error!");
            return false;
        }
        LOG_INFO("Server port:%d", port);
    }
    LOG_INFO("Server port:%d", port);
    return true;
}

// 设置非阻塞
int WebServer::SetFdNonblock(int fd) {
    assert(fd > 0);
    return fcntl(fd, F_SETFL, fcntl(fd, F_GETFD, 0) | O_NONBLOCK);
}