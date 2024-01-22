// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-21

#ifndef INCLUDE_THREADPOOL_H_
#define INCLUDE_THREADPOOL_H_
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>
class ThreadPool {
  private:
    std::vector<std::thread> threads;
    std::queue<std::function<void()>> tasks;
    std::mutex tasks_mtx;
    std::condition_variable cv;
    bool stop;

  public:
    ThreadPool(int size = 10);
    ~ThreadPool();
    void add(std::function<void()>);
};

#endif
