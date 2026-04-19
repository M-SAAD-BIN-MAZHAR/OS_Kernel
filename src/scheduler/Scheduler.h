#pragma once

#include "../process/PCB.h"
#include <string>
#include <vector>

struct ScheduleResult {
    int pid = 0;
    std::string name;
    int startTime = 0;
    int endTime = 0;
};

class Scheduler {
public:
    virtual ~Scheduler() = default;
    virtual std::vector<ScheduleResult> schedule(std::vector<PCB> &processes) = 0;
    virtual std::string name() const = 0;
};
