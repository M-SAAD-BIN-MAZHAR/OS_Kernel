#include "StateTransition.h"
#include <iostream>

void changeState(PCB &p, ProcessState newState) {
    std::cout << "Process " << p.pid
              << " State: " << (int)p.state
              << " -> " << (int)newState << std::endl;

    p.state = newState;
}