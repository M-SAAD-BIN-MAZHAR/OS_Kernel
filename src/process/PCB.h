#pragma once
#include <string>

enum class ProcessState {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

struct PCB {
    // --- Your existing fields (UNCHANGED) ---
    int pid;
    std::string name;
    int burst_time;
    int priority;
    ProcessState state;

    int arrivalTime    = 0;
    int waitTime       = 0;
    int turnaroundTime = 0;
    int remainingTime  = 0;  // for Round Robin preemption
};