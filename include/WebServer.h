// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-31

#ifndef INCLUDE_WEBSERVER_H_
#define INCLUDE_WEBSERVER_H_

#include <arpa/inet.h>
#include <assert.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <memory>
#include <unordered_map>
#include "Epoller.h"
#include "HttpConn.h"
#include "Socket.h"
#include "ThreadPool.h"
#include "Timer.h"
class WebServer {
  public:
    WebServer(int port, int trigMode, int timeoutMS_, bool OptLinger, int sqlPort,
              const char* sqlUser, const char* sqlPwd, const char* dbName, int connPoolNum,
              int threadNum, bool openLog, int logLevel, int logQueSize);

    ~WebServer();
    void Start();

  private:
    bool InitSocket();
    void InitEventMode(int trigMode);
    void AddClient(int fd, sockaddr_in addr);

    void DealListen();
    void DealWrite(HttpConn* client);
    void DealRead(HttpConn* client);

    void SendError(int fd, const char* info);
    void ExtentTime(HttpConn* client);
    void CloseConn(HttpConn* client);

    void OnRead(HttpConn* client);
    void OnWrite(HttpConn* client);
    void OnProcess(HttpConn* client);

    static const int MAX_FD = 65536;

    static int SetFdNonblock(int fd);

    int port_;
    bool openLinger_;
    int timeoutMS_; /* 毫秒MS */
    bool isClose_;
    Socket* socket_;
    char* srcDir_;

    uint32_t listenEvent_;  // 监听事件
    uint32_t connEvent_;    // 连接事件

    std::unique_ptr<Timer> timer_;
    std::unique_ptr<ThreadPool> threadpool_;
    std::unique_ptr<Epoller> epoll_;
    std::unordered_map<int, HttpConn> users_;
};

#endif
