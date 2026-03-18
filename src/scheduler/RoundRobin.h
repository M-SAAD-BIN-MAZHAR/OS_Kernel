#pragma once
#include "Scheduler.h"

class RoundRobin : public Scheduler {
private:
    int quantum;
public:
    RoundRobin(int quantum = 2) : quantum(quantum) {}
    std::vector<ScheduleResult> schedule(std::vector<PCB>& processes) override;
    std::string name() const override { return "RoundRobin"; }
};