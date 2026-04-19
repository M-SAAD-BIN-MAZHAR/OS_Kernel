#pragma once

#include "../process/PCB.h"
#include <string>
#include <vector>

struct MetricsSummary {
    double avgWaitTime = 0.0;
    double avgTurnaroundTime = 0.0;
    double cpuUtilization = 0.0;
    double throughput = 0.0;
};

class MetricsEngine {
public:
    static MetricsSummary calculate(const std::vector<PCB> &processes, int totalTime);
    static void printTable(const std::vector<PCB> &processes, const MetricsSummary &summary);
    static void exportCSV(const std::vector<PCB> &processes, const MetricsSummary &summary, const std::string &schedulerName);
};
