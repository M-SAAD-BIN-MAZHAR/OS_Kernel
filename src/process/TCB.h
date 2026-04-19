#pragma once

#include "PCB.h"

enum class ThreadState {
    NEW,
    READY,
    RUNNING,
    BLOCKED,
    TERMINATED
};

struct TCB {
    int tid;
    int parentPID;
    ThreadState state;
    int stackPointer;

    TCB(int tid, int parentPID, int stackPointer = 0x00001000)
        : tid(tid), parentPID(parentPID), state(ThreadState::NEW), stackPointer(stackPointer) {}

    TCB() : tid(0), parentPID(0), state(ThreadState::NEW), stackPointer(0x00001000) {}
};
