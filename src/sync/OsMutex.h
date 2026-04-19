#pragma once

#include "SyncTypes.h"
#include <functional>
#include <mutex>
#include <string>
#include <thread>

class OsMutex {
public:
    using EventSink = std::function<void(const SyncEvent &)>;

    explicit OsMutex(std::string name = "mutex") : name(std::move(name)) {}

    void setEventSink(EventSink sink) { eventSink = std::move(sink); }

    void lock(const std::string &demo, const std::string &actor, int tick) {
        emit(demo, actor, "acquire", "attempt", tick, 0);
        mutex.lock();
        owner = std::this_thread::get_id();
        emit(demo, actor, "acquire", "ok", tick, 0);
    }

    bool tryLock(const std::string &demo, const std::string &actor, int tick) {
        if (mutex.try_lock()) {
            owner = std::this_thread::get_id();
            emit(demo, actor, "acquire", "ok", tick, 0);
            return true;
        }
        emit(demo, actor, "acquire", "blocked", tick, 0);
        return false;
    }

    void unlock(const std::string &demo, const std::string &actor, int tick) {
        if (owner != std::this_thread::get_id()) {
            emit(demo, actor, "release", "invalid-owner", tick, 0);
            return;
        }
        owner = std::thread::id();
        mutex.unlock();
        emit(demo, actor, "release", "ok", tick, 0);
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

    std::string name;
    std::mutex mutex;
    std::thread::id owner;
    EventSink eventSink;
};
