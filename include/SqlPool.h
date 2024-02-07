// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-26

#ifndef INCLUDE_SQLPOOL_H_
#define INCLUDE_SQLPOOL_H_
#include <mysql/mysql.h>
#include <semaphore.h>
#include <mutex>
#include <queue>
#include "Log.h"
class SqlPool {
  public:
    static SqlPool* Instance();

    MYSQL* GetConn();
    void FreeConn(MYSQL* conn);
    int GetFreeConnCount();

    void Init(const char* host, int port, const char* user, const char* pwd, const char* dbName,
              int connSize = 10);
    void ClosePool();

  private:
    SqlPool();
    ~SqlPool();

    int MAX_CONN;

    std::queue<MYSQL*> connnet_que_;
    std::mutex mtx_;
    sem_t semId_;
};

class SqlRAII {
  private:
    MYSQL* sql;
    SqlPool* pool;

  public:
    SqlRAII(MYSQL** _sql, SqlPool* _pool) : pool(_pool) {
        *_sql = pool->GetConn();
        sql = *_sql;
    }
    ~SqlRAII() {
        if (sql) {
            pool->FreeConn(sql);
        }
    }
};

#endif
