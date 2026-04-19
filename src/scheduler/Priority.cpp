#include "Priority.h"

std::vector<ScheduleResult> Priority::schedule(std::vector<PCB> &processes) {
    for (auto &p : processes) {
        p.remainingTime = p.burst_time;
    }

    std::vector<ScheduleResult> results;
    int currentTime = 0;
    int completed = 0;
    const int n = static_cast<int>(processes.size());

    while (completed < n) {
        int idx = -1;
        for (int i = 0; i < n; ++i) {
            if (processes[i].arrivalTime <= currentTime && processes[i].remainingTime > 0) {
                if (idx == -1 || processes[i].priority < processes[idx].priority) {
                    idx = i;
                }
            }
        }

        if (idx == -1) {
            ++currentTime;
            continue;
        }

        PCB &p = processes[idx];
        results.push_back({p.pid, p.name, currentTime, currentTime + 1});
        --p.remainingTime;
        ++currentTime;

        if (p.remainingTime == 0) {
            p.waitTime = currentTime - p.arrivalTime - p.burst_time;
            p.turnaroundTime = currentTime - p.arrivalTime;
            completed++;
        }
    }

    std::vector<ScheduleResult> merged;
    for (const auto &r : results) {
        if (!merged.empty() && merged.back().pid == r.pid) {
            merged.back().endTime = r.endTime;
        } else {
            merged.push_back(r);
        }
    }
    return merged;
}
