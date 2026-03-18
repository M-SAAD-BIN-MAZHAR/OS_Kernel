#include "RoundRobin.h"
#include <queue>
#include <algorithm>

std::vector<ScheduleResult> RoundRobin::schedule(std::vector<PCB>& processes) {
    // Sort by arrival time
    std::sort(processes.begin(), processes.end(), [](const PCB& a, const PCB& b) {
        return a.arrivalTime < b.arrivalTime;
    });

    // Initialize remaining time for each process
    for (auto& p : processes)
        p.remainingTime = p.burst_time;

    std::vector<ScheduleResult> results;
    std::queue<int> readyQueue;  // stores index into processes[]
    int currentTime = 0;
    int completed   = 0;
    int n           = processes.size();
    int i           = 0;  // next process to arrive

    // Push first arriving process
    readyQueue.push(0);
    i = 1;

    while (completed < n) {
        if (readyQueue.empty()) {
            // CPU idle — jump to next arrival
            currentTime = processes[i].arrivalTime;
            readyQueue.push(i++);
        }

        int idx = readyQueue.front();
        readyQueue.pop();
        PCB& p = processes[idx];

        int runTime = std::min(quantum, p.remainingTime);

        ScheduleResult r;
        r.pid       = p.pid;
        r.name      = p.name;
        r.startTime = currentTime;
        r.endTime   = currentTime + runTime;
        results.push_back(r);

        currentTime        += runTime;
        p.remainingTime    -= runTime;

        // Enqueue newly arrived processes during this slice
        while (i < n && processes[i].arrivalTime <= currentTime)
            readyQueue.push(i++);

        if (p.remainingTime > 0) {
            readyQueue.push(idx);  // re-queue unfinished process
        } else {
            p.waitTime       = currentTime - p.arrivalTime - p.burst_time;
            p.turnaroundTime = currentTime - p.arrivalTime;
            completed++;
        }
    }

    return results;
}