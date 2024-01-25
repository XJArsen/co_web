// Copyright (c) RealCoolEngineer. 2024. All rights reserved.
// Author: XJArsen
// Date: 2024-01-24

#ifndef INCLUDE_BLOCKQUEUE_H_
#define INCLUDE_BLOCKQUEUE_H_

#include <sys/time.h>
#include <condition_variable>
#include <deque>
#include <mutex>
using namespace std;

template <typename T>
class BlockQueue {
  public:
    explicit BlockQueue(size_t maxsize = 1000);
    ~BlockQueue();
    bool empty();
    bool full();
    void push_back(const T& item);
    void push_front(const T& item);
    bool pop(T& item);               // 弹出的任务放入item
    bool pop(T& item, int timeout);  // 等待时间
    void clear();
    T front();
    T back();
    size_t capacity();
    size_t size();

    void flush();
    void Close();

  private:
    deque<T> deq;
    mutex mtx;
    bool isClose;                     // 关闭标志
    size_t capacities;                // 容量
    condition_variable condConsumer;  // 消费者条件变量
    condition_variable condProducer;  // 生产者条件变量
};

template <typename T>
BlockQueue<T>::BlockQueue(size_t maxsize) : capacities(maxsize), isClose(false) {
}

template <typename T>
BlockQueue<T>::~BlockQueue() {
    Close();
}

template <typename T>
bool BlockQueue<T>::empty() {
    lock_guard<mutex> locker(mtx);
    deq.empty();
}

template <typename T>
bool BlockQueue<T>::full() {
    lock_guard<mutex> locker(mtx);
    return deq.size() >= capacities;
}

template <typename T>
void BlockQueue<T>::push_back(const T& item) {
    lock_guard<mutex> locker(mtx);
    return deq.push_back(item);
}

template <typename T>
void BlockQueue<T>::push_front(const T& item) {
    lock_guard<mutex> locker(mtx);
    deq.push_front(item);
}

template <typename T>
bool BlockQueue<T>::pop(T& item) {
    unique_lock<mutex> locker(mtx);
    while (deq.empty()) {
        condConsumer.wait(locker);
    }
    item = deq.front();
    deq.pop_front();
    condProducer.notify_one();  // 唤醒生产者
    return true;
}

template <typename T>
bool BlockQueue<T>::pop(T& item, int timeout) {
    unique_lock<std::mutex> locker(mtx);
    while (deq_.empty()) {
        if (condConsumer.wait_for(locker, std::chrono::seconds(timeout)) ==
            std::cv_status::timeout) {
            return false;
        }
        if (isClose) {
            return false;
        }
    }
    item = deq.front();
    deq.pop_front();
    condProducer.notify_one();
    return true;
}

template <typename T>
void BlockQueue<T>::clear() {
    lock_guard<mutex> locker(mtx);
    deq.clear();
}

template <typename T>
T BlockQueue<T>::front() {
    lock_guard<mutex> locker(mtx);
    return deq.front();
}

template <typename T>
T BlockQueue<T>::back() {
    lock_guard<mutex> locker(mtx);
    return deq.back();
}

template <typename T>
size_t BlockQueue<T>::capacity() {
    lock_guard<mutex> locker(mtx);
    return capacities;
}

template <typename T>
size_t BlockQueue<T>::size() {
    lock_guard<mutex> locker(mtx);
    return deq.size();
}

template <typename T>
void BlockQueue<T>::flush() {
    condConsumer.notify_one();
}

template <typename T>
void BlockQueue<T>::Close() {
    clear();
    isClose = true;
    condConsumer.notify_all();
    condProducer.notify_all();
}

#endif
