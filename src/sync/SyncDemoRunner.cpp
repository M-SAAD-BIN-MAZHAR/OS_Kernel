#include "SyncDemoRunner.h"
#include "../utils/json.hpp"
#include <algorithm>
#include <chrono>
#include <condition_variable>
#include <queue>
#include <sstream>

using json = nlohmann::json;

namespace {
constexpr int kMaxEvents = 120;
constexpr int kRaceTarget = 100000;
}

SyncDemoRunner::SyncDemoRunner()
    : sequence(1), activeDemo("idle"), currentTick(0), running(false), stopFlag(false), deadlockDetected(false),
      raceWithoutMutex(0), raceWithMutex(0) {
    philosopherStates = std::vector<std::string>(5, "THINKING");
}

SyncDemoRunner::~SyncDemoRunner() {
    stop();
}

void SyncDemoRunner::startDemo(const std::string &demoName) {
    stop();

    {
        std::lock_guard<std::mutex> lock(dataMutex);
        clearState();
        activeDemo = demoName;
    }

    stopFlag = false;
    running = true;

    if (demoName == "producer_consumer") {
        workers.emplace_back(&SyncDemoRunner::runProducerConsumer, this);
    } else if (demoName == "philosophers_deadlock") {
        workers.emplace_back(&SyncDemoRunner::runPhilosophers, this, false);
    } else if (demoName == "philosophers_safe") {
        workers.emplace_back(&SyncDemoRunner::runPhilosophers, this, true);
    } else if (demoName == "race") {
        workers.emplace_back(&SyncDemoRunner::runRaceCondition, this);
    } else {
        running = false;
    }
}

void SyncDemoRunner::stop() {
    stopFlag = true;
    running = false;

    for (std::thread &worker : workers) {
        if (worker.joinable()) {
            worker.join();
        }
    }
    workers.clear();
}

void SyncDemoRunner::tick() {
    std::lock_guard<std::mutex> lock(dataMutex);
    ++currentTick;
}

std::string SyncDemoRunner::buildJSONSnapshot() const {
    std::lock_guard<std::mutex> lock(dataMutex);

    json root;
    root["type"] = "sync";
    root["demo"] = activeDemo;
    root["tick"] = currentTick;
    root["running"] = running.load();
    root["deadlockDetected"] = deadlockDetected;

    json events = json::array();
    for (const SyncEvent &event : recentEvents) {
        json e;
        e["sequence"] = event.sequence;
        e["tick"] = event.tick;
        e["demo"] = event.demo;
        e["actor"] = event.actor;
        e["action"] = event.action;
        e["resource"] = event.resource;
        e["status"] = event.status;
        e["value"] = event.value;
        events.push_back(e);
    }
    root["events"] = events;

    json timelineData = json::array();
    for (const TimelineSpan &span : timeline) {
        json t;
        t["actor"] = span.actor;
        t["startTick"] = span.startTick;
        t["endTick"] = span.endTick;
        t["state"] = span.state;
        timelineData.push_back(t);
    }
    root["timeline"] = timelineData;

    json philosophers = json::array();
    for (size_t i = 0; i < philosopherStates.size(); ++i) {
        json p;
        p["id"] = static_cast<int>(i);
        p["state"] = philosopherStates[i];
        philosophers.push_back(p);
    }
    root["philosophers"] = philosophers;

    root["race"] = {
        {"withoutMutex", raceWithoutMutex},
        {"withMutex", raceWithMutex},
        {"target", kRaceTarget}
    };

    return root.dump();
}

void SyncDemoRunner::pushEvent(SyncEvent event) {
    std::lock_guard<std::mutex> lock(dataMutex);
    event.sequence = sequence.fetch_add(1);
    event.tick = currentTick;

    recentEvents.push_back(event);
    while (static_cast<int>(recentEvents.size()) > kMaxEvents) {
        recentEvents.pop_front();
    }
}

void SyncDemoRunner::updateTimelineLocked(const std::string &actor, const std::string &state) {
    if (!timeline.empty() && timeline.back().actor == actor && timeline.back().state == state) {
        timeline.back().endTick = currentTick;
        return;
    }

    TimelineSpan span;
    span.actor = actor;
    span.state = state;
    span.startTick = currentTick;
    span.endTick = currentTick + 1;
    timeline.push_back(span);

    if (timeline.size() > 80) {
        timeline.erase(timeline.begin());
    }
}

void SyncDemoRunner::runProducerConsumer() {
    std::queue<int> buffer;
    std::mutex plainBufferMutex;
    OsMutex bufferMutex("buffer_mutex");
    OsSemaphore emptySlots(5, "empty_slots");
    OsSemaphore fullSlots(0, "full_slots");

    auto sink = [this](const SyncEvent &event) { pushEvent(event); };
    bufferMutex.setEventSink(sink);
    emptySlots.setEventSink(sink);
    fullSlots.setEventSink(sink);

    std::atomic<int> itemId = 1;

    auto producerFn = [&](int id) {
        const std::string actor = "Producer-" + std::to_string(id);
        while (!stopFlag) {
            if (!emptySlots.waitFor("producer_consumer", actor, currentTick, 100)) {
                continue;
            }
            bufferMutex.lock("producer_consumer", actor, currentTick);

            int next = itemId.fetch_add(1);
            {
                std::lock_guard<std::mutex> g(plainBufferMutex);
                buffer.push(next);
            }

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                updateTimelineLocked(actor, "LOCK_HELD");
            }

            SyncEvent event;
            event.demo = "producer_consumer";
            event.actor = actor;
            event.action = "produce";
            event.resource = "buffer";
            event.status = "ok";
            event.value = next;
            pushEvent(event);

            bufferMutex.unlock("producer_consumer", actor, currentTick);
            fullSlots.signal("producer_consumer", actor, currentTick);
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
    };

    auto consumerFn = [&](int id) {
        const std::string actor = "Consumer-" + std::to_string(id);
        while (!stopFlag) {
            if (!fullSlots.waitFor("producer_consumer", actor, currentTick, 100)) {
                continue;
            }
            bufferMutex.lock("producer_consumer", actor, currentTick);

            int value = -1;
            {
                std::lock_guard<std::mutex> g(plainBufferMutex);
                if (!buffer.empty()) {
                    value = buffer.front();
                    buffer.pop();
                }
            }

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                updateTimelineLocked(actor, "LOCK_HELD");
            }

            SyncEvent event;
            event.demo = "producer_consumer";
            event.actor = actor;
            event.action = "consume";
            event.resource = "buffer";
            event.status = "ok";
            event.value = value;
            pushEvent(event);

            bufferMutex.unlock("producer_consumer", actor, currentTick);
            emptySlots.signal("producer_consumer", actor, currentTick);
            std::this_thread::sleep_for(std::chrono::milliseconds(95));
        }
    };

    std::vector<std::thread> localThreads;
    localThreads.emplace_back(producerFn, 1);
    localThreads.emplace_back(producerFn, 2);
    localThreads.emplace_back(consumerFn, 1);
    localThreads.emplace_back(consumerFn, 2);

    while (!stopFlag) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    for (std::thread &thread : localThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    running = false;
}

void SyncDemoRunner::runPhilosophers(bool safeMode) {
    std::vector<std::unique_ptr<OsMutex>> forks;
    forks.reserve(5);
    for (int i = 0; i < 5; ++i) {
        forks.push_back(std::make_unique<OsMutex>("fork_" + std::to_string(i)));
    }

    auto sink = [this](const SyncEvent &event) { pushEvent(event); };
    for (const auto &fork : forks) {
        fork->setEventSink(sink);
    }

    auto philosopherFn = [&](int id) {
        const int left = id;
        const int right = (id + 1) % 5;
        const std::string actor = "Philosopher-" + std::to_string(id);

        while (!stopFlag) {
            {
                std::lock_guard<std::mutex> lock(dataMutex);
                philosopherStates[id] = "HUNGRY";
                updateTimelineLocked(actor, "HUNGRY");
            }

            int first = left;
            int second = right;
            if (safeMode && second < first) {
                std::swap(first, second);
            }

            forks[first]->lock(safeMode ? "philosophers_safe" : "philosophers_deadlock", actor, currentTick);
            std::this_thread::sleep_for(std::chrono::milliseconds(40));

            if (!safeMode && !forks[second]->tryLock("philosophers_deadlock", actor, currentTick)) {
                {
                    std::lock_guard<std::mutex> lock(dataMutex);
                    philosopherStates[id] = "BLOCKED";
                    updateTimelineLocked(actor, "BLOCKED");
                    deadlockDetected = true;
                }

                SyncEvent blockedEvent;
                blockedEvent.demo = "philosophers_deadlock";
                blockedEvent.actor = actor;
                blockedEvent.action = "deadlock";
                blockedEvent.resource = "fork_" + std::to_string(second);
                blockedEvent.status = "stalled";
                pushEvent(blockedEvent);
                break;
            }

            if (safeMode) {
                forks[second]->lock("philosophers_safe", actor, currentTick);
            }

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                philosopherStates[id] = "EATING";
                updateTimelineLocked(actor, "EATING");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(70));

            forks[second]->unlock(safeMode ? "philosophers_safe" : "philosophers_deadlock", actor, currentTick);
            forks[first]->unlock(safeMode ? "philosophers_safe" : "philosophers_deadlock", actor, currentTick);

            {
                std::lock_guard<std::mutex> lock(dataMutex);
                philosopherStates[id] = "THINKING";
                updateTimelineLocked(actor, "THINKING");
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(80));
        }
    };

    std::vector<std::thread> localThreads;
    for (int i = 0; i < 5; ++i) {
        localThreads.emplace_back(philosopherFn, i);
    }

    if (safeMode) {
        const auto safeEnd = std::chrono::steady_clock::now() + std::chrono::seconds(30);
        while (!stopFlag && std::chrono::steady_clock::now() < safeEnd) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    } else {
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    stopFlag = true;

    for (std::thread &thread : localThreads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    running = false;
}

void SyncDemoRunner::runRaceCondition() {
    SyncEvent header;
    header.demo = "race";
    header.actor = "RaceDemo";
    header.action = "start";
    header.resource = "counter";
    header.status = "without-mutex";
    pushEvent(header);

    int counterNoMutex = 0;

    auto unsafeIncrement = [&counterNoMutex]() {
        for (int i = 0; i < kRaceTarget / 2; ++i) {
            int snapshot = counterNoMutex;
            if (i % 11 == 0) {
                std::this_thread::yield();
            }
            counterNoMutex = snapshot + 1;
        }
    };

    std::thread t1(unsafeIncrement);
    std::thread t2(unsafeIncrement);
    t1.join();
    t2.join();

    {
        std::lock_guard<std::mutex> lock(dataMutex);
        raceWithoutMutex = counterNoMutex;
        updateTimelineLocked("RaceDemo", "NO_MUTEX_DONE");
    }

    SyncEvent noMutexResult;
    noMutexResult.demo = "race";
    noMutexResult.actor = "RaceDemo";
    noMutexResult.action = "result";
    noMutexResult.resource = "counter_without_mutex";
    noMutexResult.status = (counterNoMutex == kRaceTarget) ? "unexpected" : "race-detected";
    noMutexResult.value = counterNoMutex;
    pushEvent(noMutexResult);

    std::mutex raceMutex;
    int counterWithMutex = 0;

    auto safeIncrement = [&]() {
        for (int i = 0; i < kRaceTarget / 2; ++i) {
            std::lock_guard<std::mutex> lock(raceMutex);
            ++counterWithMutex;
        }
    };

    std::thread s1(safeIncrement);
    std::thread s2(safeIncrement);
    s1.join();
    s2.join();

    {
        std::lock_guard<std::mutex> lock(dataMutex);
        raceWithMutex = counterWithMutex;
        updateTimelineLocked("RaceDemo", "WITH_MUTEX_DONE");
    }

    SyncEvent withMutexResult;
    withMutexResult.demo = "race";
    withMutexResult.actor = "RaceDemo";
    withMutexResult.action = "result";
    withMutexResult.resource = "counter_with_mutex";
    withMutexResult.status = (counterWithMutex == kRaceTarget) ? "ok" : "mismatch";
    withMutexResult.value = counterWithMutex;
    pushEvent(withMutexResult);

    running = false;
}

void SyncDemoRunner::clearState() {
    recentEvents.clear();
    timeline.clear();
    philosopherStates.assign(5, "THINKING");
    deadlockDetected = false;
    raceWithoutMutex = 0;
    raceWithMutex = 0;
    currentTick = 0;
}
