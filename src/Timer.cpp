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
    ref[heap[i].id_] = i;
    ref[heap[j].id_] = j;
}

void Timer::adjust(int id_, int newExpires) {
    heap[ref[id_]].expires_ = Clock::now() + MS(newExpires);
    shiftdown(ref[id_], heap.size());
}
void Timer::add(int id_, int timeOut, const std::function<void()>& cb_) {
    if (ref.count(id_)) {
        int tmp = ref[id_];
        heap[tmp].expires_ = Clock::now() + MS(timeOut);
        heap[tmp].cb_ = cb_;
        if (!shiftdown(tmp, heap.size())) {
            shiftup(tmp);
        }
    } else {
        size_t n = heap.size();
        ref[id_] = n;
        heap.push_back({id_, Clock::now() + MS(timeOut)});
    }
}
void Timer::doWork(int id_) {
    if (heap.empty() || (!ref.count(id_))) {
        return;
    }
    size_t i = ref[id_];
    auto node = heap[i];
    node.cb_();
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
        if (std::chrono::duration_cast<MS>(node.expires_ - Clock::now()).count() > 0) {
            break;
        }

        node.cb_();
        pop();
    }
}
int Timer::GetNextTick() {
    tick();
    size_t res = -1;
    if (!heap.empty()) {
        res = std::chrono::duration_cast<MS>(heap.front().expires_ - Clock::now()).count();
    }
    return res;
}
