#include "DeadlockSimulator.h"
#include "../utils/Logger.h"
#include <algorithm>
#include <random>

namespace {
constexpr int kMaxDeadlockEvents = 100;
}

DeadlockSimulator::DeadlockSimulator() {
    detector = DeadlockDetector();
}

void DeadlockSimulator::start() {
    running = true;
    hasStarted = true;
    tickCount = 0;
    nextDetectionTick = detectionInterval;
    recentEvents.clear();
    
    DeadlockEvent startEvent;
    startEvent.tick = tickCount;
    startEvent.action = "simulation_start";
    startEvent.detail = "Deadlock simulation started with scenario: " + currentScenario;
    pushEvent(startEvent);
    
    Logger::info("Deadlock simulation started: " + currentScenario);
}

void DeadlockSimulator::stop() {
    running = false;
    Logger::info("Deadlock simulation stopped");
}

void DeadlockSimulator::tick() {
    if (!running) {
        return;
    }
    
    ++tickCount;
    
    // Run scenario-specific logic
    if (currentScenario == "safe") {
        runSafeScenario();
    } else if (currentScenario == "deadlock") {
        runDeadlockScenario();
    } else if (currentScenario == "bankers") {
        runBankersAlgorithmScenario();
    } else if (currentScenario == "prevention") {
        runPreventionScenario();
    }
    
    // Periodic deadlock detection
    if (tickCount >= nextDetectionTick) {
        auto deadlockInfo = detector.detectDeadlock();
        
        DeadlockEvent detectEvent;
        detectEvent.tick = tickCount;
        detectEvent.action = "detect";
        detectEvent.detail = deadlockInfo.explanation;
        
        if (deadlockInfo.isDeadlocked) {
            detectEvent.action = "deadlock_detected";
            detectEvent.success = false;
            std::string processes = "";
            for (int pid : deadlockInfo.deadlockedProcesses) {
                processes += "P" + std::to_string(pid) + " ";
            }
            detectEvent.detail = "Circular wait: " + processes;
        } else {
            detectEvent.success = true;
            detectEvent.detail = "System is in safe state";
        }
        
        pushEvent(detectEvent);
        nextDetectionTick += detectionInterval;
    }
}

void DeadlockSimulator::setScenario(const std::string& scenario) {
    currentScenario = scenario;
    recentEvents.clear();
    detector = DeadlockDetector();
    tickCount = 0;
    nextDetectionTick = detectionInterval;
}

void DeadlockSimulator::runSafeScenario() {
    // Safe scenario: processes request and release resources without deadlock
    if (tickCount % 3 == 0) {
        int processId = (tickCount / 3) % 5;
        int resourceId = (tickCount / 2) % 4;
        
        if (detector.isSafeToAllocate(processId, resourceId, 1)) {
            detector.requestResource(processId, resourceId, 1);
            
            DeadlockEvent event;
            event.tick = tickCount;
            event.processId = processId;
            event.action = "allocate";
            event.resourceId = resourceId;
            event.count = 1;
            event.success = true;
            event.detail = "Safe allocation to P" + std::to_string(processId);
            pushEvent(event);
        }
    }
    
    if (tickCount % 5 == 2) {
        int processId = (tickCount / 5) % 5;
        int resourceId = (tickCount / 3) % 4;
        
        detector.releaseResource(processId, resourceId, 1);
        
        DeadlockEvent event;
        event.tick = tickCount;
        event.processId = processId;
        event.action = "release";
        event.resourceId = resourceId;
        event.count = 1;
        event.success = true;
        event.detail = "Released by P" + std::to_string(processId);
        pushEvent(event);
    }
}

void DeadlockSimulator::runDeadlockScenario() {
    // Create circular wait scenario
    if (tickCount < 15) {
        if (tickCount == 1) {
            detector.requestResource(0, 0, 1);  // P0 requests R0
            DeadlockEvent event;
            event.tick = tickCount;
            event.processId = 0;
            event.action = "request";
            event.resourceId = 0;
            event.count = 1;
            event.detail = "P0 requests R0";
            pushEvent(event);
        }
        
        if (tickCount == 2) {
            detector.requestResource(1, 1, 1);  // P1 requests R1
            DeadlockEvent event;
            event.tick = tickCount;
            event.processId = 1;
            event.action = "request";
            event.resourceId = 1;
            event.count = 1;
            event.detail = "P1 requests R1";
            pushEvent(event);
        }
        
        if (tickCount == 5) {
            detector.requestResource(0, 1, 1);  // P0 requests R1 (already held by P1)
            DeadlockEvent event;
            event.tick = tickCount;
            event.processId = 0;
            event.action = "request";
            event.resourceId = 1;
            event.count = 1;
            event.detail = "P0 requests R1 (held by P1) - WAIT";
            pushEvent(event);
        }
        
        if (tickCount == 6) {
            detector.requestResource(1, 0, 1);  // P1 requests R0 (held by P0)
            DeadlockEvent event;
            event.tick = tickCount;
            event.processId = 1;
            event.action = "request";
            event.resourceId = 0;
            event.count = 1;
            event.detail = "P1 requests R0 (held by P0) - CIRCULAR WAIT";
            pushEvent(event);
        }
    }
}

void DeadlockSimulator::runBankersAlgorithmScenario() {
    // Use Banker's algorithm to prevent deadlock
    if (tickCount % 4 == 0 && tickCount < 30) {
        int processId = (tickCount / 4) % 4;
        int resourceId = (tickCount / 2) % 3;
        
        if (detector.isSafeToAllocate(processId, resourceId, 1)) {
            detector.requestResource(processId, resourceId, 1);
            
            DeadlockEvent event;
            event.tick = tickCount;
            event.processId = processId;
            event.action = "allocate";
            event.resourceId = resourceId;
            event.count = 1;
            event.success = true;
            event.detail = "Banker's algorithm: Safe allocation";
            pushEvent(event);
        } else {
            DeadlockEvent event;
            event.tick = tickCount;
            event.processId = processId;
            event.action = "request_denied";
            event.resourceId = resourceId;
            event.count = 1;
            event.success = false;
            event.detail = "Banker's algorithm: Request denied (would be unsafe)";
            pushEvent(event);
        }
    }
}

void DeadlockSimulator::runPreventionScenario() {
    // Deadlock prevention using ordered resource allocation
    static int allocationOrder = 0;
    
    if (tickCount % 3 == 0 && tickCount < 25) {
        int processId = (tickCount / 3) % 3;
        // Always request in order: R0, then R1, then R2
        int resourceId = allocationOrder % 3;
        
        if (detector.isSafeToAllocate(processId, resourceId, 1)) {
            detector.requestResource(processId, resourceId, 1);
            
            DeadlockEvent event;
            event.tick = tickCount;
            event.processId = processId;
            event.action = "allocate";
            event.resourceId = resourceId;
            event.count = 1;
            event.success = true;
            event.detail = "Prevention: Ordered allocation (R0 → R1 → R2)";
            pushEvent(event);
            
            allocationOrder++;
        }
    }
    
    if (tickCount % 5 == 3 && tickCount < 25) {
        int processId = (tickCount / 5) % 3;
        int resourceId = (tickCount / 3) % 3;
        
        detector.releaseResource(processId, resourceId, 1);
        
        DeadlockEvent event;
        event.tick = tickCount;
        event.processId = processId;
        event.action = "release";
        event.resourceId = resourceId;
        event.count = 1;
        event.success = true;
        event.detail = "Prevention: Resource released in reverse order";
        pushEvent(event);
    }
}

void DeadlockSimulator::pushEvent(const DeadlockEvent& event) {
    recentEvents.push_back(event);
    while (static_cast<int>(recentEvents.size()) > kMaxDeadlockEvents) {
        recentEvents.erase(recentEvents.begin());
    }
}

std::string DeadlockSimulator::buildJSONSnapshot() const {
    json root;
    root["type"] = "deadlock";
    root["tick"] = tickCount;
    root["running"] = running;
    root["scenario"] = currentScenario;
    
    json eventsJson = json::array();
    for (const auto& event : recentEvents) {
        eventsJson.push_back({
            {"tick", event.tick},
            {"processId", event.processId},
            {"action", event.action},
            {"resourceId", event.resourceId},
            {"count", event.count},
            {"success", event.success},
            {"detail", event.detail}
        });
    }
    root["events"] = eventsJson;
    
    auto deadlockInfo = detector.detectDeadlock();
    root["deadlock"] = {
        {"detected", deadlockInfo.isDeadlocked},
        {"method", deadlockInfo.detectionMethod},
        {"processes", json::array()},
        {"explanation", deadlockInfo.explanation}
    };
    
    for (int pid : deadlockInfo.deadlockedProcesses) {
        root["deadlock"]["processes"].push_back(pid);
    }
    
    auto safeSeq = detector.findSafeSequence();
    root["safeSequence"] = {
        {"exists", safeSeq.isSafe},
        {"sequence", json::array()}
    };
    
    for (int pid : safeSeq.sequence) {
        root["safeSequence"]["sequence"].push_back(pid);
    }
    
    const auto& allocationMatrix = detector.getAllocationMatrix();
    json allocJson = json::array();
    json requestJson = json::array();
    
    for (int i = 0; i < 5; ++i) {
        json alloc = json::array();
        json req = json::array();
        for (int j = 0; j < 6; ++j) {
            alloc.push_back(allocationMatrix.allocation[i][j]);
            req.push_back(allocationMatrix.request[i][j]);
        }
        allocJson.push_back(alloc);
        requestJson.push_back(req);
    }
    
    root["allocation"] = allocJson;
    root["request"] = requestJson;
    root["available"] = allocationMatrix.available;
    
    return root.dump();
}
