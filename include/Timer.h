// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-31

#ifndef INCLUDE_TIMER_H_
#define INCLUDE_TIMER_H_
#include <arpa/inet.h>
#include <algorithm>
#include <chrono>
#include <ctime>
#include <functional>
#include <queue>
#include <unordered_map>

typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds MS;
typedef Clock::time_point TimeStamp;
struct TimerNode {
    int id_;
    TimeStamp expires_;                   // 超时时间点
    std::function<void()> cb_;            // 回调function<void()>
    bool operator<(const TimerNode& t) {  // 重载比较运算符
        return expires_ < t.expires_;
    }
    bool operator>(const TimerNode& t) {  // 重载比较运算符
        return expires_ > t.expires_;
    }
};
class Timer {
  public:
    Timer(/* args */);
    ~Timer();
    void adjust(int id, int newExpires);
    void add(int id, int timeOut, const std::function<void()>& cb);
    void doWork(int id);
    void clear();
    void tick();
    void pop();
    int GetNextTick();

  private:
    void del(size_t i);
    void shiftup(size_t i);
    bool shiftdown(size_t i, size_t n);
    void SwapNode(size_t i, size_t j);
    std::vector<TimerNode> heap;
    std::unordered_map<int, size_t> ref;  // id对应的在heap_中的下标，方便用heap_的时候查找 public:
};

#endif
