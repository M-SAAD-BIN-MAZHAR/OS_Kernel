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
    int pid;
    std::string name;
    int burst_time;
    int priority;
    ProcessState state;
};