#include "utils/Config.h"
#include "process/ProcessManager.h"
#include "process/StateTransition.h"
 
int main() {
    ProcessManager pm;

  Config::load("config.json");

for (int i = 0; i < Config::process_count; i++) {
    pm.createProcess(
        "P" + std::to_string(i+1),
        Config::default_burst,
        Config::default_priority
    );
}
    // Get processes
    auto &processes = pm.getProcesses();

    // Simulate lifecycle for first process
    changeState(processes[0], ProcessState::READY);
    changeState(processes[0], ProcessState::RUNNING);
    changeState(processes[0], ProcessState::BLOCKED);
    changeState(processes[0], ProcessState::TERMINATED);

    // Display all
    pm.displayProcesses();

    return 0;
}