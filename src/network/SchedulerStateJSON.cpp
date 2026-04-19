#include "SchedulerStateJSON.h"
#include "../utils/json.hpp"

using json = nlohmann::json;

std::string SchedulerStateJSON::toJSON(const std::string &algorithmName,
                                       const std::vector<ScheduleResult> &scheduleResults,
                                       const std::vector<PCB> &processes,
                                       const MetricsSummary &summary,
                                       int currentTime) {
    json root;
    root["algorithm"] = algorithmName;
    root["timestamp"] = currentTime;
    root["totalTime"] = scheduleResults.empty() ? 0 : scheduleResults.back().endTime;

    json ganttArray = json::array();
    for (const auto &result : scheduleResults) {
        ganttArray.push_back({
            {"processName", result.name},
            {"processId", result.pid},
            {"startTime", result.startTime},
            {"endTime", result.endTime},
        });
    }
    root["ganttChart"] = ganttArray;

    json processArray = json::array();
    for (const auto &process : processes) {
        processArray.push_back({
            {"pid", process.pid},
            {"name", process.name},
            {"burstTime", process.burst_time},
            {"arrivalTime", process.arrivalTime},
            {"waitTime", process.waitTime},
            {"turnaroundTime", process.turnaroundTime},
            {"priority", process.priority},
        });
    }
    root["processes"] = processArray;
    root["metrics"] = {
        {"avgWaitTime", summary.avgWaitTime},
        {"avgTurnaroundTime", summary.avgTurnaroundTime},
        {"cpuUtilization", summary.cpuUtilization},
        {"throughput", summary.throughput},
    };

    return root.dump();
}
