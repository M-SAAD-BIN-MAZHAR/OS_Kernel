#include "DeadlockDetector.h"
#include <algorithm>
#include <numeric>
#include <functional>

DeadlockDetector::DeadlockDetector() {
    initializeMatrices();
    
    // Initialize default resources
    addResource(Resource(0, ResourceType::PRINTER, "Printer", 3));
    addResource(Resource(1, ResourceType::SCANNER, "Scanner", 2));
    addResource(Resource(2, ResourceType::MODEM, "Modem", 2));
    addResource(Resource(3, ResourceType::PLOTTER, "Plotter", 1));
    addResource(Resource(4, ResourceType::DISK, "Disk", 5));
    addResource(Resource(5, ResourceType::MEMORY_BLOCK, "Memory", 10));
    
    resourceCount = 6;
}

void DeadlockDetector::initializeMatrices() {
    allocationMatrix.allocation.assign(processCount, std::vector<int>(resourceCount, 0));
    allocationMatrix.request.assign(processCount, std::vector<int>(resourceCount, 0));
    allocationMatrix.available.assign(resourceCount, 0);
}

void DeadlockDetector::addResource(const Resource& resource) {
    resources.push_back(resource);
    if (resource.resourceId >= static_cast<int>(allocationMatrix.available.size())) {
        allocationMatrix.available.resize(resource.resourceId + 1, 0);
    }
    allocationMatrix.available[resource.resourceId] = resource.availableInstances;
}

void DeadlockDetector::requestResource(int processId, int resourceId, int count) {
    if (processId < 0 || processId >= processCount || resourceId < 0 || resourceId >= resourceCount) {
        return;
    }
    
    allocationMatrix.request[processId][resourceId] += count;
}

void DeadlockDetector::releaseResource(int processId, int resourceId, int count) {
    if (processId < 0 || processId >= processCount || resourceId < 0 || resourceId >= resourceCount) {
        return;
    }
    
    int released = std::min(count, allocationMatrix.allocation[processId][resourceId]);
    allocationMatrix.allocation[processId][resourceId] -= released;
    allocationMatrix.available[resourceId] += released;
}

bool DeadlockDetector::canAllocate(int processId, int resourceId, int count) const {
    if (resourceId >= static_cast<int>(allocationMatrix.available.size())) {
        return false;
    }
    return allocationMatrix.available[resourceId] >= count;
}

DeadlockInfo DeadlockDetector::detectDeadlock() const {
    DeadlockInfo info;
    info.detectionTime = tickCount++;
    
    // Use wait-for graph detection
    auto waitForGraph = buildWaitForGraph();
    
    if (detectCycleInGraph(waitForGraph)) {
        info.isDeadlocked = true;
        info.detectionMethod = "Wait-For Graph Cycle";
        info.deadlockedProcesses = findCycle();
        
        std::string processes = "";
        for (int pid : info.deadlockedProcesses) {
            processes += std::to_string(pid) + " ";
        }
        info.explanation = "Circular wait detected involving processes: " + processes;
    } else {
        info.isDeadlocked = false;
        info.detectionMethod = "No Cycle Found";
        info.explanation = "System is in a safe state";
    }
    
    detectionHistory.push_back(info);
    return info;
}

std::vector<std::vector<int>> DeadlockDetector::buildWaitForGraph() const {
    std::vector<std::vector<int>> graph(processCount);
    
    for (int i = 0; i < processCount; ++i) {
        for (int j = 0; j < processCount; ++j) {
            if (i == j) continue;
            
            // Check if process i waits for process j
            bool waits = false;
            for (int r = 0; r < resourceCount; ++r) {
                if (allocationMatrix.request[i][r] > 0 && allocationMatrix.allocation[j][r] > 0) {
                    waits = true;
                    break;
                }
            }
            
            if (waits) {
                graph[i].push_back(j);
            }
        }
    }
    
    return graph;
}

bool DeadlockDetector::hasCycle() {
    auto graph = buildWaitForGraph();
    return detectCycleInGraph(graph);
}

std::vector<int> DeadlockDetector::findCycle() const {
    auto graph = buildWaitForGraph();
    std::vector<int> cycle;
    std::vector<int> visited(processCount, 0);  // 0: unvisited, 1: visiting, 2: visited
    
    std::function<bool(int, std::vector<int>&)> dfs = [&](int node, std::vector<int>& path) {
        visited[node] = 1;
        path.push_back(node);
        
        for (int neighbor : graph[node]) {
            if (visited[neighbor] == 0) {
                if (dfs(neighbor, path)) {
                    return true;
                }
            } else if (visited[neighbor] == 1) {
                // Found a cycle
                auto it = std::find(path.begin(), path.end(), neighbor);
                if (it != path.end()) {
                    cycle.assign(it, path.end());
                    cycle.push_back(neighbor);
                    return true;
                }
            }
        }
        
        path.pop_back();
        visited[node] = 2;
        return false;
    };
    
    std::vector<int> path;
    for (int i = 0; i < processCount; ++i) {
        if (visited[i] == 0) {
            if (dfs(i, path)) {
                break;
            }
        }
    }
    
    return cycle;
}

bool DeadlockDetector::detectCycleInGraph(const std::vector<std::vector<int>>& graph) const {
    std::vector<int> visited(graph.size(), 0);  // 0: white, 1: gray, 2: black
    
    std::function<bool(int)> hasCycleDFS = [&](int node) {
        visited[node] = 1;
        
        for (int neighbor : graph[node]) {
            if (visited[neighbor] == 1) {
                return true;  // Back edge found
            }
            if (visited[neighbor] == 0) {
                if (hasCycleDFS(neighbor)) {
                    return true;
                }
            }
        }
        
        visited[node] = 2;
        return false;
    };
    
    for (int i = 0; i < static_cast<int>(graph.size()); ++i) {
        if (visited[i] == 0) {
            if (hasCycleDFS(i)) {
                return true;
            }
        }
    }
    
    return false;
}

SafeSequence DeadlockDetector::findSafeSequence() const {
    SafeSequence result;
    result.timestamp = tickCount;
    
    std::vector<int> sequence;
    if (isSafeSequenceExists(sequence)) {
        result.isSafe = true;
        result.sequence = sequence;
    }
    
    return result;
}

bool DeadlockDetector::isSafeSequenceExists(std::vector<int>& sequence) const {
    std::vector<int> work = allocationMatrix.available;
    std::vector<bool> finish(processCount, false);
    sequence.clear();
    
    for (int count = 0; count < processCount; ++count) {
        bool found = false;
        
        for (int p = 0; p < processCount; ++p) {
            if (finish[p]) continue;
            
            bool canFinish = true;
            for (int r = 0; r < resourceCount; ++r) {
                if (allocationMatrix.request[p][r] > work[r]) {
                    canFinish = false;
                    break;
                }
            }
            
            if (canFinish) {
                sequence.push_back(p);
                finish[p] = true;
                
                for (int r = 0; r < resourceCount; ++r) {
                    work[r] += allocationMatrix.allocation[p][r];
                }
                
                found = true;
                break;
            }
        }
        
        if (!found) {
            return false;  // No safe sequence
        }
    }
    
    return true;
}

bool DeadlockDetector::isSafeToAllocate(int processId, int resourceId, int count) {
    if (!canAllocate(processId, resourceId, count)) {
        return false;
    }
    
    // Temporarily allocate
    allocationMatrix.allocation[processId][resourceId] += count;
    allocationMatrix.available[resourceId] -= count;
    
    // Check if system is still safe
    std::vector<int> sequence;
    bool isSafe = isSafeSequenceExists(sequence);
    
    // Revert allocation
    allocationMatrix.allocation[processId][resourceId] -= count;
    allocationMatrix.available[resourceId] += count;
    
    return isSafe;
}

bool DeadlockDetector::isSafeState() {
    std::vector<int> sequence;
    return isSafeSequenceExists(sequence);
}
