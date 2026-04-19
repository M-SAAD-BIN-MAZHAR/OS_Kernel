#include "StateTransition.h"
#include <iostream>

void changeState(PCB &p, ProcessState newState) {
    std::cout << "Process " << p.pid << " State: " << static_cast<int>(p.state) << " -> " << static_cast<int>(newState) << std::endl;
    p.state = newState;
}
