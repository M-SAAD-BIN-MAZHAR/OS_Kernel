#include "Priority.h"
#include <algorithm>

std::vector<ScheduleResult> Priority::schedule(std::vector<PCB>& processes) {
    for (auto& p : processes)
        p.remainingTime = p.burst_time;

    std::vector<ScheduleResult> results;
    int currentTime = 0;
    int completed   = 0;
    int n           = processes.size();

    while (completed < n) {
        // Find highest priority process that has arrived
        // Lower priority number = higher priority
        int idx = -1;
        for (int i = 0; i < n; i++) {
            if (processes[i].arrivalTime <= currentTime &&
                processes[i].remainingTime > 0) {
                if (idx == -1 || processes[i].priority < processes[idx].priority)
                    idx = i;
            }
        }

        if (idx == -1) {
            // CPU idle — jump to next arrival
            currentTime++;
            continue;
        }

        PCB& p = processes[idx];

        // Run for 1 tick (preemptive — check every tick)
        ScheduleResult r;
        r.pid       = p.pid;
        r.name      = p.name;
        r.startTime = currentTime;
        r.endTime   = currentTime + 1;
        results.push_back(r);

        p.remainingTime--;
        currentTime++;

        if (p.remainingTime == 0) {
            p.waitTime       = currentTime - p.arrivalTime - p.burst_time;
            p.turnaroundTime = currentTime - p.arrivalTime;
            completed++;
        }
    }

    // Merge consecutive same-process results for cleaner Gantt
    std::vector<ScheduleResult> merged;
    for (auto& r : results) {
        if (!merged.empty() && merged.back().pid == r.pid)
            merged.back().endTime = r.endTime;
        else
            merged.push_back(r);
    }

    return merged;
}