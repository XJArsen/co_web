// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-21

#ifndef INCLUDE_THREADPOOL_H_
#define INCLUDE_THREADPOOL_H_
#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <type_traits>
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
    // void add(std::function<void()>);
    template <class F, class... Args>
    auto add(F&& f, Args&&... args) -> std::future<typename std::invoke_result_t<F, Args...>>;
};
template <class F, class... Args>
auto ThreadPool::add(F&& f, Args&&... args)
    -> std::future<typename std::invoke_result_t<F, Args...>> {
    using result_type = std::invoke_result_t<F, Args...>;

    auto task = std::make_shared<std::packaged_task<result_type()>>(
        std::bind(std::forward<F>(f), std::forward<Args>(args)...));

    std::future<result_type> res = task->get_future();
    {
        std::unique_lock<std::mutex> lock(tasks_mtx);
        if (stop) throw std::runtime_error("enqueue on stopped ThreadPool");

        tasks.emplace([task]() { (*task)(); });
    }
    cv.notify_one();
    return res;
}
#endif
