#pragma once

#include "Scheduler.h"

class FCFS : public Scheduler {
public:
    std::vector<ScheduleResult> schedule(std::vector<PCB> &processes) override;
    std::string name() const override { return "FCFS"; }
};
