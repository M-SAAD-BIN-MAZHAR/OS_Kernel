#pragma once

#include "DeadlockDetector.h"
#include "../utils/json.hpp"
#include <vector>
#include <string>
#include <deque>

using json = nlohmann::json;

struct DeadlockEvent {
    int tick = 0;
    int processId = -1;
    std::string action;      // "request", "allocate", "release", "detect", "deadlock", "prevention"
    int resourceId = -1;
    std::string resourceName;
    int count = 0;
    bool success = true;
    std::string detail;
};

class DeadlockSimulator {
public:
    DeadlockSimulator();
    
    void start();
    void stop();
    void tick();
    
    bool isRunning() const { return running; }
    bool hasSnapshot() const { return hasStarted; }
    
    void setScenario(const std::string& scenario);
    std::string buildJSONSnapshot() const;
    
private:
    DeadlockDetector detector;
    std::vector<DeadlockEvent> recentEvents;
    bool running = false;
    bool hasStarted = false;
    int tickCount = 0;
    std::string currentScenario = "safe";
    int detectionInterval = 5;
    int nextDetectionTick = 5;
    
    void pushEvent(const DeadlockEvent& event);
    void runSafeScenario();
    void runDeadlockScenario();
    void runBankersAlgorithmScenario();
    void runPreventionScenario();
};
