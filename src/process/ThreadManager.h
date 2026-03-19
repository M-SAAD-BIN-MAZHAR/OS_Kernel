#pragma once
#include "TCB.h"
#include <vector>
#include <algorithm>

class ThreadManager {
private:
    std::vector<TCB> threads;
    int nextTID = 1;

public:
    // Create a new thread within a process
    void createThread(int parentPID, int stackPointer = 0x00001000);
    
    // Get thread by TID
    TCB* getThreadByTID(int tid);
    
    // Get all threads of a process
    std::vector<TCB*> getThreadsByPID(int parentPID);
    
    // Change thread state
    void changeThreadState(int tid, ThreadState newState);
    
    // Display all threads
    void displayThreads();
    
    // Get all threads
    std::vector<TCB>& getAllThreads();
    
    // Terminate thread
    void terminateThread(int tid);
};
