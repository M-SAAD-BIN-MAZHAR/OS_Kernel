#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include "Resource.h"

struct AllocationMatrix {
    std::vector<std::vector<int>> allocation;  // allocation[process][resource]
    std::vector<std::vector<int>> request;     // request[process][resource]
    std::vector<int> available;                // available[resource]
};

struct DeadlockInfo {
    bool isDeadlocked = false;
    std::vector<int> deadlockedProcesses;
    std::string detectionMethod;
    int detectionTime = 0;
    std::string explanation;
};

struct SafeSequence {
    bool isSafe = false;
    std::vector<int> sequence;
    int timestamp = 0;
};

class DeadlockDetector {
public:
    DeadlockDetector();
    
    // Resource management
    void addResource(const Resource& resource);
    void requestResource(int processId, int resourceId, int count);
    void releaseResource(int processId, int resourceId, int count);
    
    // Deadlock detection
    DeadlockInfo detectDeadlock() const;
    SafeSequence findSafeSequence() const;
    
    // Deadlock prevention
    bool isSafeToAllocate(int processId, int resourceId, int count);
    
    // State queries
    const AllocationMatrix& getAllocationMatrix() const { return allocationMatrix; }
    const std::vector<Resource>& getResources() const { return resources; }
    const std::vector<DeadlockInfo>& getDetectionHistory() const { return detectionHistory; }
    
    // Graph-based detection
    bool hasCycle();
    std::vector<int> findCycle() const;
    
    // Banker's algorithm
    bool isSafeState();
    
private:
    std::vector<Resource> resources;
    AllocationMatrix allocationMatrix;
    mutable std::vector<DeadlockInfo> detectionHistory;
    int processCount = 10;
    int resourceCount = 6;
    mutable int tickCount = 0;
    
    // Helper functions
    bool canAllocate(int processId, int resourceId, int count) const;
    std::vector<std::vector<int>> buildWaitForGraph() const;
    bool detectCycleInGraph(const std::vector<std::vector<int>>& graph) const;
    bool isSafeSequenceExists(std::vector<int>& sequence) const;
    void initializeMatrices();
};
