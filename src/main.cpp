#include <iostream>
#include <vector>
#include "utils/Config.h"
#include "process/ProcessManager.h"
#include "process/StateTransition.h"
#include "scheduler/FCFS.h"
#include "scheduler/RoundRobin.h"
#include "scheduler/Priority.h"
#include "scheduler/Metrics.h"

void printGantt(const std::vector<ScheduleResult>& results) {
    std::cout << "\nGantt Chart:\n|";
    for (const auto& r : results)
        std::cout << " " << r.name << " |";
    std::cout << "\n0";
    for (const auto& r : results)
        std::cout << "    " << r.endTime;
    std::cout << "\n";
}

void runScheduler(Scheduler& scheduler, std::vector<PCB> processes) {
    std::cout << "\n========== " << scheduler.name() << " ==========\n";
    auto results = scheduler.schedule(processes);
    printGantt(results);

    int totalTime = results.back().endTime;
    MetricsSummary summary = MetricsEngine::calculate(processes, totalTime);
    MetricsEngine::printTable(processes, summary);
    MetricsEngine::exportCSV(processes, summary, scheduler.name());
}

int main() {
    // --- Phase 0: ProcessManager (your existing code) ---
    ProcessManager pm;
    Config::load("config.json");

    for (int i = 0; i < Config::process_count; i++) {
        pm.createProcess(
            "P" + std::to_string(i+1),
            Config::default_burst,
            Config::default_priority
        );
    }

    auto& processes = pm.getProcesses();
    changeState(processes[0], ProcessState::READY);
    changeState(processes[0], ProcessState::RUNNING);
    changeState(processes[0], ProcessState::BLOCKED);
    changeState(processes[0], ProcessState::TERMINATED);
    pm.displayProcesses();

    // --- Phase 1: Scheduler Test ---
    std::cout << "\n\n===== PHASE 1: CPU SCHEDULING =====\n";

    std::vector<PCB> testProcesses = {
        {1, "P1", 5, 3, ProcessState::NEW, 0},
        {2, "P2", 3, 1, ProcessState::NEW, 1},
        {3, "P3", 8, 2, ProcessState::NEW, 2}
    };

    FCFS fcfs;
    RoundRobin rr(2);
    Priority priority;

    runScheduler(fcfs,     testProcesses);
    runScheduler(rr,       testProcesses);
    runScheduler(priority, testProcesses);

    return 0;
}