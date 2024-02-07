#include "SqlPool.h"
SqlPool::SqlPool() = default;
SqlPool::~SqlPool() {
    ClosePool();
}

SqlPool* SqlPool::Instance() {
    static SqlPool pool;
    return &pool;
}
void SqlPool::Init(const char* host, int port, const char* user, const char* pwd,
                   const char* dbName, int connSize) {
    for (int i = 0; i < connSize; i++) {
        MYSQL* connect = nullptr;
        connect = mysql_init(connect);
        if (!connect) {
            LOG_ERROR("MySql init error!");
        }
        connect = mysql_real_connect(connect, host, user, pwd, dbName, port, nullptr, 0);
        if (!connect) {
            LOG_ERROR("MySql Connect error!");
        }
        connnet_que_.emplace(connect);
    }
    MAX_CONN = connSize;
    sem_init(&semId_, 0, MAX_CONN);
}
MYSQL* SqlPool::GetConn() {
    MYSQL* connect = nullptr;
    if (connnet_que_.empty()) {
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&semId_);
    lock_guard<mutex> locker(mtx_);
    connect = connnet_que_.front();
    connnet_que_.pop();
    return connect;
}
void SqlPool::FreeConn(MYSQL* connect) {
    lock_guard<mutex> locker(mtx_);
    connnet_que_.push(connect);
    sem_post(&semId_);
}
int SqlPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mtx_);
    return connnet_que_.size();
}

void SqlPool::ClosePool() {
    lock_guard<mutex> locker(mtx_);
    while (!connnet_que_.empty()) {
        MYSQL* connect = connnet_que_.front();
        connnet_que_.pop();
        mysql_close(connect);
    }
    mysql_library_end();
}
