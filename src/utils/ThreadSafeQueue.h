#pragma once

#include <chrono>
#include <condition_variable>
#include <mutex>
#include <queue>

template <typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mutex;
    std::queue<T> dataQueue;
    std::condition_variable condVar;

public:
    ThreadSafeQueue() = default;
    ThreadSafeQueue(const ThreadSafeQueue &) = delete;
    ThreadSafeQueue &operator=(const ThreadSafeQueue &) = delete;

    void push(T newValue) {
        std::lock_guard<std::mutex> lk(mutex);
        dataQueue.push(std::move(newValue));
        condVar.notify_one();
    }

    bool tryPopWait(T &value, int timeoutMs = 100) {
        std::unique_lock<std::mutex> lk(mutex);
        if (!condVar.wait_for(lk, std::chrono::milliseconds(timeoutMs), [this] { return !dataQueue.empty(); })) {
            return false;
        }
        value = std::move(dataQueue.front());
        dataQueue.pop();
        return true;
    }

    bool tryPop(T &value) {
        std::lock_guard<std::mutex> lk(mutex);
        if (dataQueue.empty()) {
            return false;
        }
        value = std::move(dataQueue.front());
        dataQueue.pop();
        return true;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lk(mutex);
        return dataQueue.empty();
    }
};
