#pragma once

#include "PCB.h"
#include <vector>

class ProcessManager {
private:
    std::vector<PCB> processes;
    int nextPid = 1;

public:
    void createProcess(std::string name, int burst, int priority);
    void displayProcesses();
    std::vector<PCB> &getProcesses();
};
