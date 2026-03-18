#pragma once
#include <vector>
#include <string>
#include "../process/PCB.h"

struct ScheduleResult {
    int pid;
    std::string name;
    int startTime;
    int endTime;
};

struct Metrics {
    int pid;
    std::string name;
    double waitTime;
    double turnaroundTime;
    double responseTime;
};

class Scheduler {
public:
    virtual ~Scheduler() = default;
    virtual std::vector<ScheduleResult> schedule(std::vector<PCB>& processes) = 0;
    virtual std::string name() const = 0;
};