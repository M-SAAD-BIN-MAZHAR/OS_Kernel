#pragma once

#include "OsMutex.h"
#include "OsSemaphore.h"
#include "SyncTypes.h"
#include <atomic>
#include <deque>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

class SyncDemoRunner {
public:
    SyncDemoRunner();
    ~SyncDemoRunner();

    void startDemo(const std::string &demoName);
    void stop();
    void tick();
    std::string buildJSONSnapshot() const;

private:
    void pushEvent(SyncEvent event);
    void updateTimelineLocked(const std::string &actor, const std::string &state);

    void runProducerConsumer();
    void runPhilosophers(bool safeMode);
    void runRaceCondition();

    void clearState();

    mutable std::mutex dataMutex;
    mutable std::atomic<long long> sequence;

    std::string activeDemo;
    int currentTick;
    std::atomic<bool> running;
    std::atomic<bool> stopFlag;

    std::deque<SyncEvent> recentEvents;
    std::vector<TimelineSpan> timeline;
    std::vector<std::string> philosopherStates;
    bool deadlockDetected;

    int raceWithoutMutex;
    int raceWithMutex;

    std::vector<std::thread> workers;
};
