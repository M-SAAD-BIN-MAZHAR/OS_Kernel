#pragma once

#include "../process/PCB.h"
#include "../scheduler/Metrics.h"
#include "../scheduler/Scheduler.h"
#include <string>
#include <vector>

class SchedulerStateJSON {
public:
    static std::string toJSON(const std::string &algorithmName,
                              const std::vector<ScheduleResult> &scheduleResults,
                              const std::vector<PCB> &processes,
                              const MetricsSummary &summary,
                              int currentTime);
};
