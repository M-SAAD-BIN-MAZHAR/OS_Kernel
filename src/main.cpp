#include "memory/MemorySimulator.h"
#include "network/SchedulerStateJSON.h"
#include "network/TCPServer.h"
#include "process/ProcessManager.h"
#include "process/StateTransition.h"
#include "process/ThreadManager.h"
#include "scheduler/FCFS.h"
#include "scheduler/Metrics.h"
#include "scheduler/Priority.h"
#include "scheduler/RoundRobin.h"
#include "sync/SyncDemoRunner.h"
#include "deadlock/DeadlockSimulator.h"
#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/json.hpp"
#include <atomic>
#include <chrono>
#include <csignal>
#include <filesystem>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

using json = nlohmann::json;

std::mutex algoMutex;
std::string currentAlgorithm;
std::atomic<bool> simulationStarted = false;
std::atomic<bool> shouldExit = false;

void handleShutdownSignal(int) {
    shouldExit = true;
}

std::string resolveConfigPath() {
    const std::vector<std::string> candidates = {"config.json", "../config.json", "OS_Kernel/config.json"};
    for (const auto &path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }
    throw std::runtime_error("Could not find config file");
}

void printGantt(const std::vector<ScheduleResult> &results) {
    std::cout << "\nGantt Chart:\n|";
    for (const auto &r : results) {
        std::cout << " " << r.name << " |";
    }
    std::cout << "\n0";
    for (const auto &r : results) {
        std::cout << "    " << r.endTime;
    }
    std::cout << "\n";
}

void simulationThread(TCPServer &server, SyncDemoRunner &syncRunner, MemorySimulator &memorySimulator, DeadlockSimulator &deadlockSimulator) {
    const std::vector<PCB> testProcesses = {
        {1, "P1", 5, 3, ProcessState::NEW, 0},
        {2, "P2", 3, 1, ProcessState::NEW, 1},
        {3, "P3", 8, 2, ProcessState::NEW, 2},
        {4, "P4", 6, 2, ProcessState::NEW, 3},
        {5, "P5", 4, 1, ProcessState::NEW, 4},
    };

    while (!shouldExit) {
        std::string algo;
        {
            std::lock_guard<std::mutex> lock(algoMutex);
            algo = currentAlgorithm;
        }

        if (!algo.empty() && simulationStarted) {
            std::vector<PCB> processes = testProcesses;
            Scheduler *scheduler = nullptr;
            if (algo == "FCFS") {
                scheduler = new FCFS();
            } else if (algo == "RoundRobin") {
                scheduler = new RoundRobin(2);
            } else if (algo == "Priority") {
                scheduler = new Priority();
            }

            if (scheduler) {
                auto results = scheduler->schedule(processes);
                const int totalTime = results.empty() ? 0 : results.back().endTime;
                const MetricsSummary summary = MetricsEngine::calculate(processes, totalTime);
                const std::string jsonData = SchedulerStateJSON::toJSON(algo, results, processes, summary, 0);
                server.broadcastMessage(jsonData);
                delete scheduler;
            }
        }

        syncRunner.tick();
        server.broadcastMessage(syncRunner.buildJSONSnapshot());

        if (memorySimulator.isRunning()) {
            memorySimulator.tick();
        }
        if (memorySimulator.hasSnapshot()) {
            server.broadcastMessage(memorySimulator.buildJSONSnapshot());
        }

        if (deadlockSimulator.isRunning()) {
            deadlockSimulator.tick();
        }
        if (deadlockSimulator.hasSnapshot()) {
            server.broadcastMessage(deadlockSimulator.buildJSONSnapshot());
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::signal(SIGINT, handleShutdownSignal);
    std::signal(SIGTERM, handleShutdownSignal);

    ProcessManager pm;
    Config::load(resolveConfigPath());

    for (int i = 0; i < Config::process_count; ++i) {
        pm.createProcess("P" + std::to_string(i + 1), Config::default_burst, Config::default_priority);
    }

    auto &processes = pm.getProcesses();
    changeState(processes[0], ProcessState::READY);
    changeState(processes[0], ProcessState::RUNNING);
    changeState(processes[0], ProcessState::BLOCKED);
    changeState(processes[0], ProcessState::TERMINATED);
    pm.displayProcesses();

    std::cout << "\n===== PHASE 0 Part 2: THREAD MANAGEMENT (TCB) =====\n";
    ThreadManager tm;
    for (int pid = 1; pid <= Config::process_count; ++pid) {
        for (int i = 0; i < 2; ++i) {
            int stackAddr = 0x00001000 + (pid * 0x100) + (i * 0x10);
            tm.createThread(pid, stackAddr);
        }
    }
    tm.displayThreads();
    std::cout << "\n--- Testing Thread State Transitions ---\n";
    tm.changeThreadState(1, ThreadState::READY);
    tm.changeThreadState(1, ThreadState::RUNNING);
    tm.changeThreadState(1, ThreadState::BLOCKED);
    tm.changeThreadState(1, ThreadState::TERMINATED);

    std::cout << "\n\n===== PHASE 1: CPU SCHEDULING WITH TCP SERVER =====\n";
    std::cout << "===== PHASE 2: PROCESS SYNCHRONIZATION DEMOS =====\n";
    std::cout << "===== PHASE 3: DEADLOCK DETECTION & PREVENTION =====\n";
    std::cout << "===== PHASE 4: MEMORY MANAGEMENT WITH PAGING =====\n";

    SyncDemoRunner syncRunner;
    DeadlockSimulator deadlockSimulator;
    MemorySimulator memorySimulator;
    TCPServer server(9000);

    server.setMessageHandler([&](const std::string &message) {
        try {
            const json payload = json::parse(message);
            const std::string command = payload.value("command", "");

            if (command == "select_scheduler") {
                std::lock_guard<std::mutex> lock(algoMutex);
                currentAlgorithm = payload.value("algorithm", "FCFS");
                Logger::info("Selected scheduler: " + currentAlgorithm);
            } else if (command == "start_simulation") {
                simulationStarted = true;
                Logger::info("Simulation start command received");
            } else if (command == "start_sync_demo") {
                const std::string demo = payload.value("demo", "producer_consumer");
                syncRunner.startDemo(demo);
                Logger::info("Sync demo started: " + demo);
            } else if (command == "select_memory_policy") {
                const std::string policy = payload.value("policy", "FIFO");
                memorySimulator.setPolicy(policy);
                Logger::info("Memory policy selected: " + policy);
            } else if (command == "configure_memory") {
                const int pageSize = payload.value("pageSize", 4096);
                const int frameCount = payload.value("frameCount", 16);
                const int tlbSize = payload.value("tlbSize", 8);
                memorySimulator.configure(pageSize, frameCount, tlbSize);
                Logger::info("Memory configured");
            } else if (command == "start_memory_simulation") {
                memorySimulator.start();
                Logger::info("Memory simulation start command received");
            } else if (command == "start_deadlock_demo") {
                const std::string scenario = payload.value("scenario", "safe");
                deadlockSimulator.setScenario(scenario);
                deadlockSimulator.start();
                Logger::info("Deadlock demo started: " + scenario);
            } else if (command == "stop_deadlock_demo") {
                deadlockSimulator.stop();
                Logger::info("Deadlock demo stopped");
            }
        } catch (const std::exception &ex) {
            Logger::warn(std::string("Invalid command payload: ") + ex.what());
        }
    });

    server.start();

    std::thread simThread(simulationThread, std::ref(server), std::ref(syncRunner), std::ref(memorySimulator), std::ref(deadlockSimulator));

    {
        std::lock_guard<std::mutex> lock(algoMutex);
        currentAlgorithm = "FCFS";
    }
    simulationStarted = true;

    while (!shouldExit) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    std::cout << "\n\nShutting down server...\n";
    shouldExit = true;
    syncRunner.stop();
    deadlockSimulator.stop();
    memorySimulator.stop();
    server.stop();
    simThread.join();

    std::cout << "Simulation complete\n";
    return 0;
}
