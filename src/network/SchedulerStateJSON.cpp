#include "SchedulerStateJSON.h"
#include "../utils/json.hpp"
#include <sstream>

using json = nlohmann::json;

std::string SchedulerStateJSON::toJSON(
    const std::string &algorithmName,
    const std::vector<ScheduleResult> &scheduleResults,
    const std::vector<PCB> &processes,
    const MetricsSummary &summary,
    int currentTime
) {
    json root;
    
    // Algorithm info
    root["algorithm"] = algorithmName;
    root["timestamp"] = currentTime;
    root["totalTime"] = scheduleResults.empty() ? 0 : scheduleResults.back().endTime;
    
    // Gantt chart data
    json ganttArray = json::array();
    for (const auto &result : scheduleResults) {
        json ganttEntry;
        ganttEntry["processName"] = result.name;
        ganttEntry["processId"] = result.pid;
        ganttEntry["startTime"] = result.startTime;
        ganttEntry["endTime"] = result.endTime;
        ganttArray.push_back(ganttEntry);
    }
    root["ganttChart"] = ganttArray;
    
    // Process metrics
    json processArray = json::array();
    for (const auto &process : processes) {
        json procEntry;
        procEntry["pid"] = process.pid;
        procEntry["name"] = process.name;
        procEntry["burstTime"] = process.burst_time;
        procEntry["arrivalTime"] = process.arrivalTime;
        procEntry["waitTime"] = process.waitTime;
        procEntry["turnaroundTime"] = process.turnaroundTime;
        procEntry["priority"] = process.priority;
        processArray.push_back(procEntry);
    }
    root["processes"] = processArray;
    
    // Summary metrics
    json metricsObj;
    metricsObj["avgWaitTime"] = summary.avgWaitTime;
    metricsObj["avgTurnaroundTime"] = summary.avgTurnaroundTime;
    metricsObj["cpuUtilization"] = summary.cpuUtilization;
    metricsObj["throughput"] = summary.throughput;
    root["metrics"] = metricsObj;
    
    return root.dump();
}
