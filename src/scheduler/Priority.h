#pragma once
#include "Scheduler.h"

class Priority : public Scheduler {
public:
    std::vector<ScheduleResult> schedule(std::vector<PCB>& processes) override;
    std::string name() const override { return "Priority"; }
};