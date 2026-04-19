#include "ThreadManager.h"
#include "../utils/Logger.h"
#include <cstdio>
#include <iomanip>
#include <iostream>

void ThreadManager::createThread(int parentPID, int stackPointer) {
    TCB thread(nextTID++, parentPID, stackPointer);
    threads.push_back(thread);

    char hexBuffer[16];
    std::snprintf(hexBuffer, sizeof(hexBuffer), "0x%08X", stackPointer);
    Logger::info("Thread Created -> TID: " + std::to_string(thread.tid) +
                 " Parent PID: " + std::to_string(parentPID) +
                 " Stack Pointer: " + hexBuffer);
}

TCB *ThreadManager::getThreadByTID(int tid) {
    for (auto &t : threads) {
        if (t.tid == tid) {
            return &t;
        }
    }
    return nullptr;
}

std::vector<TCB *> ThreadManager::getThreadsByPID(int parentPID) {
    std::vector<TCB *> result;
    for (auto &t : threads) {
        if (t.parentPID == parentPID) {
            result.push_back(&t);
        }
    }
    return result;
}

void ThreadManager::changeThreadState(int tid, ThreadState newState) {
    if (TCB *thread = getThreadByTID(tid)) {
        std::cout << "Thread " << tid << " State: " << static_cast<int>(thread->state) << " -> " << static_cast<int>(newState) << std::endl;
        thread->state = newState;
    }
}

void ThreadManager::displayThreads() {
    std::cout << "\n=== Thread Table ===\n";
    std::cout << std::left << std::setw(6) << "TID" << std::setw(12) << "Parent PID" << std::setw(16) << "Stack Pointer" << std::setw(10) << "State\n";
    std::cout << std::string(44, '-') << "\n";

    for (auto &t : threads) {
        char hexBuffer[16];
        std::snprintf(hexBuffer, sizeof(hexBuffer), "0x%08X", t.stackPointer);
        std::cout << std::setw(6) << t.tid << std::setw(12) << t.parentPID << std::setw(16) << hexBuffer << std::setw(10)
                  << static_cast<int>(t.state) << "\n";
    }
}

std::vector<TCB> &ThreadManager::getAllThreads() {
    return threads;
}

void ThreadManager::terminateThread(int tid) {
    if (TCB *thread = getThreadByTID(tid)) {
        thread->state = ThreadState::TERMINATED;
        Logger::info("Thread Terminated -> TID: " + std::to_string(tid));
    }
}
