#pragma once
#include <string>
#include <vector>
#include "../process/PCB.h"
#include "../scheduler/Scheduler.h"
#include "../scheduler/Metrics.h"

class SchedulerStateJSON {
public:
    // Convert scheduler results and metrics to JSON
    static std::string toJSON(
        const std::string &algorithmName,
        const std::vector<ScheduleResult> &scheduleResults,
        const std::vector<PCB> &processes,
        const MetricsSummary &summary,
        int currentTime
    );
};
