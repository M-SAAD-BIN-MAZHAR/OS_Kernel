#include "ProcessManager.h"
#include "../utils/Logger.h"
#include <iostream>

void ProcessManager::createProcess(std::string name, int burst, int priority) {
    PCB p;
    p.pid = nextPid++;
    p.name = std::move(name);
    p.burst_time = burst;
    p.priority = priority;
    p.state = ProcessState::NEW;
    processes.push_back(p);

    Logger::info("Process Created -> PID: " + std::to_string(p.pid) + " Name: " + p.name);
}

void ProcessManager::displayProcesses() {
    std::cout << "\n=== Process Table ===\n";
    std::cout << "PID\tName\tBurst\tPriority\tState\n";
    for (auto &p : processes) {
        std::cout << p.pid << "\t" << p.name << "\t" << p.burst_time << "\t" << p.priority << "\t" << static_cast<int>(p.state) << "\n";
    }
}

std::vector<PCB> &ProcessManager::getProcesses() {
    return processes;
}
