# OS Kernel CPU Scheduling Simulator

A C++17 educational simulator that models core operating-system concepts: process lifecycle management, CPU scheduling, and performance-metric collection. The simulator runs three well-known scheduling algorithms side-by-side, renders Gantt charts on the console, and exports per-algorithm CSV reports.

---

## Table of Contents

1. [Features](#features)
2. [Project Structure](#project-structure)
3. [Architecture Overview](#architecture-overview)
4. [Process Lifecycle](#process-lifecycle)
5. [Scheduling Algorithms](#scheduling-algorithms)
6. [Performance Metrics](#performance-metrics)
7. [Prerequisites](#prerequisites)
8. [Build Instructions](#build-instructions)
9. [Configuration](#configuration)
10. [Running the Simulator](#running-the-simulator)
11. [Sample Output](#sample-output)
12. [Output Files](#output-files)

---

## Features

- **Process Control Block (PCB)** — full process descriptor with PID, name, burst time, priority, arrival time, wait time, turnaround time, and remaining time.
- **Process Manager** — creates and tracks processes; drives explicit state transitions with console logging.
- **Three CPU Scheduling Algorithms** — First Come First Served (FCFS), Round Robin (RR), and Priority Scheduling, all sharing a common abstract interface.
- **Gantt Chart** — text-based timeline printed to `stdout` after each scheduling run.
- **Metrics Engine** — computes average wait time, average turnaround time, and throughput; prints a formatted table and exports a CSV file.
- **JSON Configuration** — runtime parameters (process count, default burst, default priority) loaded from `config.json`.
- **Structured Logging** — timestamped INFO / WARN / ERROR log messages via a dedicated `Logger` utility.

---

## Project Structure

```
OS_Kernel/
├── CMakeLists.txt            # CMake build configuration (C++17)
├── config.json               # Runtime configuration (process_count, burst, priority)
├── README.md
├── src/
│   ├── main.cpp              # Entry point — Phase 0 (ProcessManager demo) + Phase 1 (scheduler tests)
│   ├── process/
│   │   ├── PCB.h             # Process Control Block struct + ProcessState enum
│   │   ├── ProcessManager.h / .cpp   # Creates and stores PCB objects
│   │   └── StateTransition.h / .cpp  # Applies and logs state changes
│   ├── scheduler/
│   │   ├── Scheduler.h       # Abstract base class (schedule() + name())
│   │   ├── FCFS.h / .cpp     # First Come First Served
│   │   ├── RoundRobin.h / .cpp  # Round Robin with configurable quantum
│   │   ├── Priority.h / .cpp    # Preemptive Priority Scheduling
│   │   └── Metrics.h / .cpp     # MetricsEngine: calculate, printTable, exportCSV
│   └── utils/
│       ├── Config.h / .cpp   # Loads config.json via nlohmann/json
│       ├── Logger.h / .cpp   # Timestamped logging utility
│       └── json.hpp          # nlohmann/json header-only library
├── build/                    # CMake-generated build artifacts (git-ignored)
└── output/                   # Generated CSV metric reports (git-ignored)
    ├── metrics_FCFS.csv
    ├── metrics_RoundRobin.csv
    └── metrics_Priority.csv
```

---

## Architecture Overview

```
┌─────────────────────────────────────────────────────┐
│                      main.cpp                       │
│  Phase 0: ProcessManager + StateTransition demo     │
│  Phase 1: run FCFS / RoundRobin / Priority          │
└──────────┬──────────────────────────────────────────┘
           │ uses
    ┌──────▼────────────────────────────────────────┐
    │            Scheduler (abstract)               │
    │  schedule(vector<PCB>&) → vector<ScheduleResult>│
    └──────┬───────────────┬───────────────┬────────┘
           │               │               │
       ┌───▼───┐     ┌─────▼──────┐  ┌────▼─────┐
       │ FCFS  │     │ RoundRobin │  │ Priority │
       └───┬───┘     └─────┬──────┘  └────┬─────┘
           └───────────────┴───────────────┘
                           │ results fed to
                    ┌──────▼────────┐
                    │ MetricsEngine │
                    │ calculate()   │
                    │ printTable()  │
                    │ exportCSV()   │
                    └───────────────┘

┌────────────────────┐     ┌──────────────────────┐
│  ProcessManager    │     │  Config / Logger      │
│  createProcess()   │     │  load("config.json")  │
│  displayProcesses()│     │  INFO / WARN / ERROR  │
└────────────────────┘     └──────────────────────┘
```

---

## Process Lifecycle

Each process moves through five states. `StateTransition` enforces and logs every transition.

```
  ┌─────┐
  │ NEW │
  └──┬──┘
     │ admit
  ┌──▼────┐      preempt / I/O done
  │ READY │◄─────────────────────────┐
  └──┬────┘                          │
     │ dispatch                      │
  ┌──▼──────┐   I/O wait        ┌────┴─────┐
  │ RUNNING ├──────────────────►│ BLOCKED  │
  └──┬──────┘                   └──────────┘
     │ exit
  ┌──▼──────────┐
  │ TERMINATED  │
  └─────────────┘
```

**ProcessState enum values:** `NEW`, `READY`, `RUNNING`, `BLOCKED`, `TERMINATED`

---

## Scheduling Algorithms

### First Come First Served (FCFS)

- **Type:** Non-preemptive
- **Policy:** Processes are executed in order of arrival time.
- **Implementation:** Sorts the process list by `arrivalTime`, then runs each process to completion before moving to the next.
- **Pros:** Simple, fair in arrival order, no starvation.
- **Cons:** Long processes block shorter ones (convoy effect).

### Round Robin (RR)

- **Type:** Preemptive
- **Policy:** Each process receives a fixed CPU *quantum*. If it does not finish within the quantum, it is returned to the back of the ready queue.
- **Default quantum:** 2 time units (configurable via `RoundRobin(int quantum)` constructor).
- **Pros:** Fair CPU sharing, good response time for interactive processes.
- **Cons:** High context-switch overhead if quantum is too small.

### Priority Scheduling

- **Type:** Preemptive
- **Policy:** At each time tick the ready process with the **lowest priority number** (highest urgency) runs for one tick. Newly arrived higher-priority processes can preempt the current one.
- **Pros:** Important processes receive preferential treatment.
- **Cons:** Low-priority processes may starve if high-priority work keeps arriving.

All three algorithms implement the same abstract interface:

```cpp
class Scheduler {
public:
    virtual std::vector<ScheduleResult> schedule(std::vector<PCB>& processes) = 0;
    virtual std::string name() const = 0;
};
```

---

## Performance Metrics

`MetricsEngine::calculate()` computes the following after each scheduling run:

| Metric | Formula |
|--------|---------|
| **Wait Time** (per process) | Turnaround Time − Burst Time |
| **Turnaround Time** (per process) | Completion Time − Arrival Time |
| **Average Wait Time** | Sum of all wait times ÷ number of processes |
| **Average Turnaround Time** | Sum of all turnaround times ÷ number of processes |
| **Throughput** | Number of processes ÷ total simulation time |

Results are printed as a table and saved to `output/metrics_<AlgorithmName>.csv`.

---

## Prerequisites

| Tool | Minimum Version |
|------|----------------|
| C++ compiler (GCC / Clang / MSVC) | C++17 support |
| CMake | 3.22 |
| Make (Linux/macOS) or Ninja / MSBuild (Windows) | any recent version |

---

## Build Instructions

```bash
# 1. Clone the repository (if you haven't already)
git clone https://github.com/M-SAAD-BIN-MAZHAR/OS_Kernel.git
cd OS_Kernel

# 2. Create and enter the build directory
mkdir -p build && cd build

# 3. Configure with CMake
cmake ..

# 4. Compile
make

# 5. The simulator binary is now at build/simulator
```

> **Windows (Visual Studio):** Replace step 3 with `cmake .. -G "Visual Studio 17 2022"` and open the generated solution, or use `cmake --build .` after configuration.

---

## Configuration

Edit `config.json` in the repository root to change Phase 0 (ProcessManager demo) parameters:

```json
{
  "process_count": 3,
  "default_burst": 5,
  "default_priority": 1
}
```

| Key | Type | Description |
|-----|------|-------------|
| `process_count` | integer | Number of processes created in Phase 0 |
| `default_burst` | integer | CPU burst time assigned to each Phase 0 process |
| `default_priority` | integer | Priority level assigned to each Phase 0 process |

> **Note:** The Phase 1 scheduler test processes (`P1`, `P2`, `P3`) are hardcoded in `main.cpp` and are not affected by `config.json`.

---

## Running the Simulator

```bash
# From the build directory
./simulator
```

The simulator runs two phases:

**Phase 0 — ProcessManager Demo**
Creates `process_count` processes using values from `config.json`, then cycles the first process through all states (NEW → READY → RUNNING → BLOCKED → TERMINATED) and prints the full process table.

**Phase 1 — Scheduler Comparison**
Runs three hardcoded test processes through all three scheduling algorithms:

| PID | Name | Burst Time | Priority | Arrival Time |
|-----|------|-----------|---------|-------------|
| 1   | P1   | 5         | 3       | 0           |
| 2   | P2   | 3         | 1       | 1           |
| 3   | P3   | 8         | 2       | 2           |

For each algorithm it prints a Gantt chart, a metrics table, and exports a CSV file.

---

## Sample Output

```
===== PHASE 1: CPU SCHEDULING =====

========== FCFS ==========

Gantt Chart:
| P1 | P2 | P3 |
0    5    8    16

PID  Name  BurstTime  ArrivalTime  WaitTime  TurnaroundTime
1    P1    5          0            0         5
2    P2    3          1            4         7
3    P3    8          2            6         14

AvgWaitTime: 3.33  AvgTurnaroundTime: 8.67  Throughput: 0.1875

========== RoundRobin (quantum=2) ==========

Gantt Chart:
| P1 | P2 | P3 | P1 | P2 | P3 | P1 | P3 | P3 |
0    2    4    6    8    9    11   12   14   16

...

========== Priority ==========

Gantt Chart:
| P2 | P2 | P2 | P3 | P1 | ... |
0    1    2    3    ...
```

---

## Output Files

After a successful run the `output/` directory contains one CSV file per algorithm:

```
output/
├── metrics_FCFS.csv
├── metrics_RoundRobin.csv
└── metrics_Priority.csv
```

Each CSV has the format:

```
PID,Name,BurstTime,ArrivalTime,WaitTime,TurnaroundTime
1,P1,5,0,0,5
2,P2,3,1,4,7
3,P3,8,2,6,14

AvgWaitTime,AvgTurnaroundTime,Throughput
3.33333,8.66667,0.1875
```
