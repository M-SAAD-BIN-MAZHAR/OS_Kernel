#pragma once

#include <string>

struct SyncEvent {
    long long sequence = 0;
    int tick = 0;
    std::string demo;
    std::string actor;
    std::string action;
    std::string resource;
    std::string status;
    int value = 0;
};

struct TimelineSpan {
    std::string actor;
    int startTick = 0;
    int endTick = 0;
    std::string state;
};
