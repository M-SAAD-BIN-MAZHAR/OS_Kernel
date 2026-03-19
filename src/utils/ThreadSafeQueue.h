#pragma once
#include <queue>
#include <mutex>
#include <memory>
#include <condition_variable>

template <typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mutex;
    std::queue<T> dataQueue;
    std::condition_variable condVar;

public:
    ThreadSafeQueue() = default;
    
    // Delete copy operations
    ThreadSafeQueue(const ThreadSafeQueue& other) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue& other) = delete;
    
    // Push element to queue
    void push(T newValue) {
        std::lock_guard<std::mutex> lk(mutex);
        dataQueue.push(std::move(newValue));
        condVar.notify_one();
    }
    
    // Try pop with waiting
    bool tryPop(T& value, int timeoutMs = 100) {
        std::unique_lock<std::mutex> lk(mutex);
        if (!condVar.wait_for(lk, std::chrono::milliseconds(timeoutMs),
                              [this] { return !dataQueue.empty(); })) {
            return false;
        }
        value = std::move(dataQueue.front());
        dataQueue.pop();
        return true;
    }
    
    // Non-blocking pop
    bool tryPop(T& value) const {
        std::lock_guard<std::mutex> lk(mutex);
        if (dataQueue.empty()) {
            return false;
        }
        value = std::move(dataQueue.front());
        dataQueue.pop();
        return true;
    }
    
    // Check if queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lk(mutex);
        return dataQueue.empty();
    }
    
    // Get size
    size_t size() const {
        std::lock_guard<std::mutex> lk(mutex);
        return dataQueue.size();
    }
    
    // Clear queue
    void clear() {
        std::lock_guard<std::mutex> lk(mutex);
        std::queue<T> empty;
        std::swap(dataQueue, empty);
    }
};
