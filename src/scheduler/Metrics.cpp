#include "Metrics.h"
#include <fstream>
#include <iomanip>
#include <iostream>

MetricsSummary MetricsEngine::calculate(const std::vector<PCB> &processes, int totalTime) {
    MetricsSummary s;
    double totalWT = 0.0;
    double totalTAT = 0.0;
    for (const auto &p : processes) {
        totalWT += p.waitTime;
        totalTAT += p.turnaroundTime;
    }

    const int n = static_cast<int>(processes.size());
    if (n > 0) {
        s.avgWaitTime = totalWT / n;
        s.avgTurnaroundTime = totalTAT / n;
    }
    s.cpuUtilization = 100.0;
    s.throughput = totalTime > 0 ? static_cast<double>(n) / totalTime : 0.0;
    return s;
}

void MetricsEngine::printTable(const std::vector<PCB> &processes, const MetricsSummary &summary) {
    std::cout << "\n--- Scheduling Metrics ---\n";
    std::cout << std::left << std::setw(6) << "PID" << std::setw(12) << "Name" << std::setw(10) << "BurstTime" << std::setw(10) << "WaitTime"
              << std::setw(16) << "TurnaroundTime" << "\n";
    std::cout << std::string(54, '-') << "\n";

    for (const auto &p : processes) {
        std::cout << std::setw(6) << p.pid << std::setw(12) << p.name << std::setw(10) << p.burst_time << std::setw(10) << p.waitTime
                  << std::setw(16) << p.turnaroundTime << "\n";
    }

    std::cout << std::string(54, '-') << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "Avg Wait Time      : " << summary.avgWaitTime << "\n";
    std::cout << "Avg Turnaround Time: " << summary.avgTurnaroundTime << "\n";
    std::cout << "Throughput         : " << summary.throughput << " proc/unit\n";
}

void MetricsEngine::exportCSV(const std::vector<PCB> &processes, const MetricsSummary &summary, const std::string &schedulerName) {
    std::ofstream file("output/metrics_" + schedulerName + ".csv");
    file << "PID,Name,BurstTime,ArrivalTime,WaitTime,TurnaroundTime\n";
    for (const auto &p : processes) {
        file << p.pid << "," << p.name << "," << p.burst_time << "," << p.arrivalTime << "," << p.waitTime << "," << p.turnaroundTime << "\n";
    }

    file << "\nAvgWaitTime,AvgTurnaroundTime,Throughput\n";
    file << summary.avgWaitTime << "," << summary.avgTurnaroundTime << "," << summary.throughput << "\n";
    std::cout << "Metrics saved to output/metrics_" << schedulerName << ".csv\n";
}
