#include "Timer.h"
#include "Log.h"
Timer::Timer(/* args */) {
    heap.reserve(64);
}
Timer::~Timer() {
    clear();
}
void Timer::del(size_t i) {
}
void Timer::shiftup(size_t i) {
    size_t p = (i - 1) / 2;
    while (p >= 0) {
        if (heap[p] > heap[i]) {
            SwapNode(i, p);
            i = p;
            p = (i - 1) / 2;
        } else {
            break;
        }
    }
}
bool Timer::shiftdown(size_t i, size_t n) {
    auto idx = i;
    auto child = idx * 2 + 1;
    while (child < n) {
        if (child + 1 < n && heap[child + 1] < heap[child]) {
            child++;
        }
        if (heap[child] < heap[idx]) {
            SwapNode(idx, child);
            idx = child;
            child = 2 * child + 1;
        }
        break;  // 需要跳出循环
    }
    return idx > i;
}
void Timer::SwapNode(size_t i, size_t j) {
    swap(heap[i], heap[j]);
    ref[heap[i].id] = i;
    ref[heap[j].id] = j;
}

void Timer::adjust(int id, int newExpires) {
    heap[ref[id]].expires = Clock::now() + MS(newExpires);
    shiftdown(ref[id], heap.size());
}
void Timer::add(int id, int timeOut, const std::function<void()>& cb) {
    if (ref.count(id)) {
        int tmp = ref[id];
        heap[tmp].expires = Clock::now() + MS(timeOut);
        heap[tmp].cb = cb;
        if (!shiftdown(tmp, heap.size())) {
            shiftup(tmp);
        }
    } else {
        size_t n = heap.size();
        ref[id] = n;
        heap.push_back({id, Clock::now() + MS(timeOut)});
    }
}
void Timer::doWork(int id) {
    if (heap.empty() || (!ref.count(id))) {
        return;
    }
    size_t i = ref[id];
    auto node = heap[i];
    node.cb();
    del(i);
}
void Timer::clear() {
    ref.clear();
    heap.clear();
}

void Timer::pop() {
    del(0);
}
void Timer::tick() {
    while (!heap.empty()) {
        TimerNode node = heap.front();
        if (std::chrono::duration_cast<MS>(node.expires - Clock::now()).count() > 0) {
            break;
        }

        node.cb();
        pop();
    }
}
int Timer::GetNextTick() {
    tick();
    size_t res = -1;
    if (!heap.empty()) {
        res = std::chrono::duration_cast<MS>(heap.front().expires - Clock::now()).count();
    }
    return res;
}
