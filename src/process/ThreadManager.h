#pragma once

#include "TCB.h"
#include <vector>

class ThreadManager {
private:
    std::vector<TCB> threads;
    int nextTID = 1;

public:
    void createThread(int parentPID, int stackPointer = 0x00001000);
    TCB *getThreadByTID(int tid);
    std::vector<TCB *> getThreadsByPID(int parentPID);
    void changeThreadState(int tid, ThreadState newState);
    void displayThreads();
    std::vector<TCB> &getAllThreads();
    void terminateThread(int tid);
};
