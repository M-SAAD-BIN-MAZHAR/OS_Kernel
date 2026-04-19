#include "FCFS.h"
#include <algorithm>

std::vector<ScheduleResult> FCFS::schedule(std::vector<PCB> &processes) {
    std::sort(processes.begin(), processes.end(), [](const PCB &a, const PCB &b) { return a.arrivalTime < b.arrivalTime; });

    std::vector<ScheduleResult> results;
    int currentTime = 0;
    for (auto &p : processes) {
        if (currentTime < p.arrivalTime) {
            currentTime = p.arrivalTime;
        }

        ScheduleResult r;
        r.pid = p.pid;
        r.name = p.name;
        r.startTime = currentTime;
        r.endTime = currentTime + p.burst_time;
        p.waitTime = r.startTime - p.arrivalTime;
        p.turnaroundTime = r.endTime - p.arrivalTime;
        currentTime = r.endTime;
        results.push_back(r);
    }
    return results;
}
