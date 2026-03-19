#pragma once
#include <string>
#include "PCB.h"

enum class ThreadState {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

struct TCB {
    // --- Thread Control Block (TCB) ---
    int tid;              // Thread ID
    int parentPID;        // Parent Process ID
    ThreadState state;    // Thread state (same as ProcessState enum)
    int stackPointer;     // Simulated stack pointer (memory address)
    
    // Constructor
    TCB(int tid, int parentPID, int stackPointer = 0x00001000)
        : tid(tid), parentPID(parentPID), state(ThreadState::NEW), stackPointer(stackPointer) {}
    
    // Default constructor
    TCB() : tid(0), parentPID(0), state(ThreadState::NEW), stackPointer(0x00001000) {}
};
