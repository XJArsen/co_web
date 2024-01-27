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
        connnet_que.emplace(connect);
    }
    MAX_CONN = connSize;
    sem_init(&semId, 0, MAX_CONN);
}
MYSQL* SqlPool::GetConn() {
    MYSQL* connect = nullptr;
    if (connnet_que.empty()) {
        LOG_WARN("SqlConnPool busy!");
        return nullptr;
    }
    sem_wait(&semId);
    lock_guard<mutex> locker(mtx);
    connect = connnet_que.front();
    connnet_que.pop();
    return connect;
}
void SqlPool::FreeConn(MYSQL* connect) {
    lock_guard<mutex> locker(mtx);
    connnet_que.push(connect);
    sem_post(&semId);
}
int SqlPool::GetFreeConnCount() {
    lock_guard<mutex> locker(mtx);
    return connnet_que.size();
}

void SqlPool::ClosePool() {
    lock_guard<mutex> locker(mtx);
    while (!connnet_que.empty()) {
        MYSQL* connect = connnet_que.front();
        connnet_que.pop();
        mysql_close(connect);
    }
    mysql_library_end();
}
