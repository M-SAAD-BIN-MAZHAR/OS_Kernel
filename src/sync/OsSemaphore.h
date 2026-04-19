#pragma once

#include "SyncTypes.h"
#include <chrono>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <string>

class OsSemaphore {
public:
    using EventSink = std::function<void(const SyncEvent &)>;

    OsSemaphore(int initialCount, std::string name) : count(initialCount), name(std::move(name)) {}

    void setEventSink(EventSink sink) { eventSink = std::move(sink); }

    void wait(const std::string &demo, const std::string &actor, int tick) {
        std::unique_lock<std::mutex> lock(mutex);
        while (count == 0) {
            emit(demo, actor, "wait", "blocked", tick, count);
            cv.wait(lock);
        }
        --count;
        emit(demo, actor, "wait", "ok", tick, count);
    }

    bool waitFor(const std::string &demo, const std::string &actor, int tick, int timeoutMs) {
        std::unique_lock<std::mutex> lock(mutex);
        while (count == 0) {
            emit(demo, actor, "wait", "blocked", tick, count);
            if (cv.wait_for(lock, std::chrono::milliseconds(timeoutMs)) == std::cv_status::timeout) {
                emit(demo, actor, "wait", "timeout", tick, count);
                return false;
            }
        }
        --count;
        emit(demo, actor, "wait", "ok", tick, count);
        return true;
    }

    void signal(const std::string &demo, const std::string &actor, int tick) {
        {
            std::lock_guard<std::mutex> lock(mutex);
            ++count;
            emit(demo, actor, "signal", "ok", tick, count);
        }
        cv.notify_one();
    }

private:
    void emit(const std::string &demo, const std::string &actor, const std::string &action, const std::string &status, int tick, int value) {
        if (!eventSink) {
            return;
        }

        SyncEvent event;
        event.tick = tick;
        event.demo = demo;
        event.actor = actor;
        event.action = action;
        event.resource = name;
        event.status = status;
        event.value = value;
        eventSink(event);
    }

    int count;
    std::string name;
    std::mutex mutex;
    std::condition_variable cv;
    EventSink eventSink;
};
