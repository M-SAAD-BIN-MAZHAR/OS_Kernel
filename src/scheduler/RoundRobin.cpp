#include "RoundRobin.h"
#include <algorithm>
#include <queue>

std::vector<ScheduleResult> RoundRobin::schedule(std::vector<PCB> &processes) {
    std::sort(processes.begin(), processes.end(), [](const PCB &a, const PCB &b) { return a.arrivalTime < b.arrivalTime; });

    for (auto &p : processes) {
        p.remainingTime = p.burst_time;
    }

    std::vector<ScheduleResult> results;
    std::queue<int> readyQueue;
    int currentTime = 0;
    int completed = 0;
    const int n = static_cast<int>(processes.size());
    int i = 0;

    if (n == 0) {
        return results;
    }

    readyQueue.push(0);
    i = 1;

    while (completed < n) {
        if (readyQueue.empty()) {
            currentTime = processes[i].arrivalTime;
            readyQueue.push(i++);
        }

        int idx = readyQueue.front();
        readyQueue.pop();
        PCB &p = processes[idx];
        const int runTime = std::min(quantum, p.remainingTime);

        results.push_back({p.pid, p.name, currentTime, currentTime + runTime});
        currentTime += runTime;
        p.remainingTime -= runTime;

        while (i < n && processes[i].arrivalTime <= currentTime) {
            readyQueue.push(i++);
        }

        if (p.remainingTime > 0) {
            readyQueue.push(idx);
        } else {
            p.waitTime = currentTime - p.arrivalTime - p.burst_time;
            p.turnaroundTime = currentTime - p.arrivalTime;
            completed++;
        }
    }

    return results;
}
