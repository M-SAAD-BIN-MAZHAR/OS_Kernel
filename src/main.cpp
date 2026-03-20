#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>
#include <string>
#include <filesystem>
#include <vector>
#include <memory>
#include <csignal>
#include "utils/Config.h"
#include "utils/Logger.h"
#include "utils/json.hpp"
#include "process/ProcessManager.h"
#include "process/StateTransition.h"
#include "process/ThreadManager.h"
#include "scheduler/FCFS.h"
#include "scheduler/RoundRobin.h"
#include "scheduler/Priority.h"
#include "scheduler/Metrics.h"
#include "network/TCPServer.h"
#include "network/SchedulerStateJSON.h"
#include "sync/SyncDemoRunner.h"

using json = nlohmann::json;

std::mutex algoMutex;
std::string currentAlgorithm = "";
std::atomic<bool> simulationStarted = false;
std::atomic<bool> shouldExit = false;

void handleShutdownSignal(int) {
    shouldExit = true;
}

std::string resolveConfigPath() {
    const std::vector<std::string> candidates = {
        "config.json",
        "../config.json",
        "OS_Kernel/config.json"
    };

    for (const auto& path : candidates) {
        if (std::filesystem::exists(path)) {
            return path;
        }
    }

    throw std::runtime_error("Could not find config file (tried config.json, ../config.json, OS_Kernel/config.json)");
}

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

void simulationThread(TCPServer &server, SyncDemoRunner &syncRunner) {
    ProcessManager pm;

    // Create test processes with custom arrival times
    std::vector<PCB> testProcesses = {
        {1, "P1", 5, 3, ProcessState::NEW, 0},
        {2, "P2", 3, 1, ProcessState::NEW, 1},
        {3, "P3", 8, 2, ProcessState::NEW, 2},
        {4, "P4", 6, 2, ProcessState::NEW, 3},
        {5, "P5", 4, 1, ProcessState::NEW, 4}
    };

    while (!shouldExit) {
        std::string algo;
        {
            std::lock_guard<std::mutex> lock(algoMutex);
            algo = currentAlgorithm;
        }
        
        if (!algo.empty() && simulationStarted) {
            // Create fresh copies for each run
            std::vector<PCB> processes = testProcesses;
            
            std::cout << "\nRunning " << algo << " scheduler...\n";
            
            Scheduler* scheduler = nullptr;
            if (algo == "FCFS") {
                scheduler = new FCFS();
            } else if (algo == "RoundRobin") {
                scheduler = new RoundRobin(2);
            } else if (algo == "Priority") {
                scheduler = new Priority();
            }
            
            if (scheduler) {
                auto results = scheduler->schedule(processes);
                int totalTime = results.back().endTime;
                MetricsSummary summary = MetricsEngine::calculate(processes, totalTime);
                
                // Send JSON with scheduling state
                std::string jsonData = SchedulerStateJSON::toJSON(algo, results, processes, summary, 0);
                server.broadcastMessage(jsonData);
                
                std::cout << "Sent scheduling data to " << algo << " (" << jsonData.length() << " bytes)\n";
                
                delete scheduler;
            }
        }

        syncRunner.tick();
        const std::string syncJson = syncRunner.buildJSONSnapshot();
        server.broadcastMessage(syncJson);
        std::cout << "Sent sync data (" << syncJson.length() << " bytes)\n";

        // Wait 200ms before next broadcast
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
}

int main() {
    std::signal(SIGINT, handleShutdownSignal);
    std::signal(SIGTERM, handleShutdownSignal);

    // --- Phase 0: ProcessManager ---
    ProcessManager pm;
    Config::load(resolveConfigPath());

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

    // --- Phase 0 Part 2: ThreadManager (TCB/TID) ---
    std::cout << "\n===== PHASE 0 Part 2: THREAD MANAGEMENT (TCB) =====\n";
    ThreadManager tm;
    
    for (int pid = 1; pid <= Config::process_count; pid++) {
        for (int i = 0; i < 2; i++) {
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
    
    // --- Phase 1: TCP Server + Simulation ---
    std::cout << "\n\n===== PHASE 1: CPU SCHEDULING WITH TCP SERVER =====\n";
    std::cout << "===== PHASE 2: PROCESS SYNCHRONIZATION DEMOS =====\n";

    SyncDemoRunner syncRunner;
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
            }
        } catch (const std::exception &ex) {
            Logger::warn(std::string("Invalid command payload: ") + ex.what());
        }
    });

    server.start();

    // Start simulation thread
    std::thread simThread(simulationThread, std::ref(server), std::ref(syncRunner));

    // Set default scheduler
    {
        std::lock_guard<std::mutex> lock(algoMutex);
        currentAlgorithm = "FCFS";
    }
    simulationStarted = true;

    // Keep server alive until interrupted (Ctrl+C / termination signal)
    while (!shouldExit) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Shutdown
    std::cout << "\n\nShutting down server...\n";
    shouldExit = true;
    syncRunner.stop();
    server.stop();
    simThread.join();

    std::cout << "Simulation complete\n";
    return 0;
}