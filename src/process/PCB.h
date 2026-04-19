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
    int pid = 0;
    std::string name;
    int burst_time = 0;
    int priority = 0;
    ProcessState state = ProcessState::NEW;

    int arrivalTime = 0;
    int waitTime = 0;
    int turnaroundTime = 0;
    int remainingTime = 0;
};
