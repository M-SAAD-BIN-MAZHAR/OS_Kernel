#include "../utils/Logger.h"
#include "ProcessManager.h"
#include <iostream>

using namespace std;

void ProcessManager::createProcess(string name, int burst, int priority) {
    PCB p;
    p.pid = nextPid++;
    p.name = name;
    p.burst_time = burst;
    p.priority = priority;
    p.state = ProcessState::NEW;

    processes.push_back(p);
 
    Logger::info("Process Created -> PID: " + to_string(p.pid) +
                 " Name: " + p.name);
}

void ProcessManager::displayProcesses() {
    cout << "\n=== Process Table ===\n";
    cout << "PID\tName\tBurst\tPriority\tState\n";

    for (auto &p : processes) {
        cout << p.pid << "\t"
             << p.name << "\t"
             << p.burst_time << "\t"
             << p.priority << "\t"
             << (int)p.state << "\n";
    }
}

vector<PCB>& ProcessManager::getProcesses() {
    return processes;
}