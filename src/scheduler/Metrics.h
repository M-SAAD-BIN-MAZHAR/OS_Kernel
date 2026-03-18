#pragma once
#include <vector>
#include <string>
#include "../process/PCB.h"
#include "Scheduler.h"

struct MetricsSummary {
    double avgWaitTime;
    double avgTurnaroundTime;
    double cpuUtilization;
    double throughput;
};

class MetricsEngine {
public:
    static MetricsSummary calculate(const std::vector<PCB>& processes, int totalTime);
    static void printTable(const std::vector<PCB>& processes, const MetricsSummary& summary);
    static void exportCSV(const std::vector<PCB>& processes, const MetricsSummary& summary, const std::string& schedulerName);
};