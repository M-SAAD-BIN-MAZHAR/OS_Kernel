# OS Simulator - Phase 0 & Phase 1 Complete

A comprehensive **CPU Scheduling Simulator** with real-time process management and visualization.

## Status: ✅ **PHASE 0 & PHASE 1 COMPLETE**

**Last Updated: March 19, 2026**

---

## Quick Start

### For the Engine (Linux/WSL):

```bash
cd OS_Kernel
mkdir -p build && cd build
cmake -G Ninja .. && ninja
./simulator
```

**Output**: TCP server listening on port 9000, broadcasting JSON scheduling data every 200ms.

### For the Qt UI (Windows/Linux/macOS with Qt6.7):

```bash
cd OS_Simulator_Qt
mkdir -p build && cd build
cmake -G Ninja .. && ninja
./OS_Simulator_Qt
```

**Result**: Connects to localhost:9000, displays real-time Gantt charts and metrics.

---

## What's Implemented

### Phase 0 ✅ (Environment + Foundation)
- [x] ProcessManager with PCB (Process Control Block)
- [x] ThreadManager with TCB (Thread Control Block)
- [x] 5-State Process State Machine (NEW→READY→RUNNING→BLOCKED→TERMINATED)
- [x] Logger with timestamps and log levels
- [x] Config reader (JSON-based configuration)
- [x] Thread-safe queue implementation

### Phase 1 ✅ (CPU Scheduling with TCP + UI)
- [x] **FCFS Scheduler** - First Come First Served
- [x] **Round Robin Scheduler** - Configurable quantum (default 2ms)
- [x] **Priority Scheduler** - Preemptive priority-based scheduling
- [x] **Metrics Engine** - Wait time, turnaround time, throughput calculations
- [x] **TCP Server** - Real-time JSON broadcasting (200ms intervals)
- [x] **Qt UI** - 5-tab interface with Gantt charts and metrics tables

---

## Architecture

### Engine (C++ Server)
```
OS_Kernel/
├── src/process/    # PCB, TCB, ProcessManager, ThreadManager
├── src/scheduler/  # FCFS, RoundRobin, Priority, Metrics
├── src/network/    # TCPServer, SchedulerStateJSON
└── src/utils/      # Logger, Config, ThreadSafeQueue
```

### Client (Qt6 UI)
```
OS_Simulator_Qt/
├── src/main_window.h/.cpp    # 5-tab main interface
├── src/simulator_client.h/.cpp # TCP socket client
├── src/gantt_widget.h/.cpp    # Chart visualization
└── CMakeLists.txt
```

---

## Detailed Runtime Guide

### Step 1: Start the Engine
```bash
cd /home/msaad/OS_Simulator1/OS_Kernel
./build/simulator
```

You'll see:
```
[INFO] TCP Server started on port 9000
[INFO] Server listening on port 9000
Running FCFS scheduler...
Sent scheduling data (989 bytes)
...
```

**Keep this running!** (default 30 seconds, Ctrl+C to stop)

### Step 2: Start the Qt Client
In a **different terminal**:
```bash
cd /path/to/OS_Simulator_Qt/build
./OS_Simulator_Qt
```

### Step 3: Interact with the UI
1. **Status Bar** shows "Connected to server"
2. **Scheduler Tab**:
   - Select algorithm (FCFS, RoundRobin, Priority)
   - Click "Start Simulation"
   - Watch real-time Gantt chart update
3. **Metrics Tab**: View process metrics
4. **Logs Tab**: Real-time event log

---

## JSON Communication Format

**Engine ↔ Client**: JSON over TCP every 200ms

Example message:
```json
{
  "algorithm": "FCFS",
  "timestamp": 0,
  "totalTime": 26,
  "ganttChart": [
    {"processName":"P1","startTime":0,"endTime":5},
    {"processName":"P2","startTime":5,"endTime":8}
  ],
  "processes": [
    {
      "pid": 1,
      "name": "P1",
      "burstTime": 5,
      "arrivalTime": 0,
      "waitTime": 0,
      "turnaroundTime": 5,
      "priority": 3
    }
  ],
  "metrics": {
    "avgWaitTime": 3.33,
    "avgTurnaroundTime": 8.67,
    "cpuUtilization": 100.0,
    "throughput": 0.1875
  }
}
```

---

## Scheduling Algorithms

### FCFS (First Come First Served)
- Processes execute in arrival order
- No preemption
- Fair but can cause long wait times

Example:
```
| P1 | P2 | P3 |
0    5    8    16
Avg Wait: 3.33ms
```

### Round Robin (Quantum = 2ms)
- Each process gets fixed time slice
- Preempts at quantum expiry
- Better responsiveness

Example:
```
| P1 | P2 | P3 | P1 | P2 | P3 | P1 | P3 | P3 |
0    2    4    6    8    9    11   12   14   16
Avg Wait: 6.00ms
```

### Priority (Preemptive)
- Higher priority (lower number) executes first
- Preempts if higher priority arrives
- Can cause starvation

Example:
```
| P1 | P2 | P3 | P1 |
0    1    4    12   16
Avg Wait: 4.33ms
```

---

## Project Files Structure

### Engine Source Files
- **src/main.cpp** - Server loop, threading, simulation
- **src/process/PCB.h** - Process Control Block definition
- **src/process/TCB.h** - Thread Control Block definition (NEW)
- **src/process/ProcessManager.h/.cpp** - Process lifecycle management
- **src/process/ThreadManager.h/.cpp** - Thread lifecycle management (NEW)
- **src/process/StateTransition.h/.cpp** - State transition logic
- **src/scheduler/Scheduler.h** - Abstract scheduler interface
- **src/scheduler/FCFS.h/.cpp** - FCFS implementation
- **src/scheduler/RoundRobin.h/.cpp** - RR implementation with quantum
- **src/scheduler/Priority.h/.cpp** - Priority-based scheduler
- **src/scheduler/Metrics.h/.cpp** - Metrics calculation and export
- **src/network/TCPServer.h/.cpp** - TCP server (NEW)
- **src/network/SchedulerStateJSON.h/.cpp** - JSON serialization (NEW)
- **src/utils/Logger.h/.cpp** - Logging system
- **src/utils/Config.h/.cpp** - JSON configuration loader
- **src/utils/ThreadSafeQueue.h** - Thread-safe queue (NEW)
- **config.json** - Configuration file

### Client Source Files
- **src/main.cpp** - Qt application entry point
- **src/main_window.h/.cpp** - Main window with 5 tabs (UPDATED)
- **src/simulator_client.h/.cpp** - TCP socket client (UPDATED)
- **src/gantt_widget.h/.cpp** - Gantt chart visualization (UPDATED)
- **CMakeLists.txt** - Qt build configuration

---

## Build Details

### Engine Build
```bash
cmake -G Ninja ..
ninja
# Output: ./build/simulator (executable)
```

**Compiler**: g++ C++17
**Dependencies**: pthread (for threading)

### Client Build
```bash
cmake -G Ninja ..
ninja
# Output: ./build/OS_Simulator_Qt (executable)
```

**Compiler**: MSVC (Windows) or gcc/clang (Linux/macOS)
**Dependencies**: Qt6::Core, Qt6::Gui, Qt6::Widgets, Qt6::Network, Qt6::Charts

---

## Verification

### Engine Tests
Run and verify:
```bash
cd OS_Kernel
./build/simulator
```

Should show:
- ✅ Process creation and state transitions
- ✅ Thread creation with stack pointers
- ✅ TCP server starts on port 9000
- ✅ FCFS/RoundRobin/Priority scheduling runs
- ✅ JSON sent every 200ms

### Client Tests
Launch and verify:
```bash
./OS_Simulator_Qt/build/OS_Simulator_Qt
```

Should show:
- ✅ 5 tabs visible with content
- ✅ Status bar shows "Connected to server"
- ✅ Gantt chart displays process bars
- ✅ Metrics table shows data
- ✅ Logs tab shows connection events

---

## Troubleshooting

| Issue | Solution |
|-------|----------|
| "Connection Refused" | Ensure engine is running on port 9000 |
| Gantt chart empty | Check engine is sending JSON correctly |
| Qt won't compile | Install Qt6.7 and set CMAKE_PREFIX_PATH |
| Engine crashes | Check /tmp for crash dumps, verify CMake build |
| No metrics data | Verify JSON format from engine |

---

## Configuration

Edit `OS_Kernel/config.json`:
```json
{
  "process_count": 3,      // Number of test processes
  "default_burst": 5,      // Default CPU burst time
  "default_priority": 1    // Default priority (1=highest)
}
```

---

## Next Steps (Future Phases)

Possible enhancements:
- Dynamic process arrival simulation
- Real-time context switch visualization
- Memory management (paging/segmentation)
- Cache simulation with hit/miss rates
- Deadlock detection and recovery
- I/O scheduling (disk/network)
- Multi-processor scheduling
- Load balancing algorithms

---

## Files Changed This Session

**New Files**:
- src/process/TCB.h
- src/process/ThreadManager.h/.cpp
- src/network/TCPServer.h/.cpp
- src/network/SchedulerStateJSON.h/.cpp
- src/utils/ThreadSafeQueue.h
- OS_Simulator_Qt/* (complete Qt project)
- PHASE_0_COMPLETE.md
- PHASE_1_RUN_GUIDE.md

**Modified Files**:
- CMakeLists.txt (added network and utils)
- src/main.cpp (TCP server integration)
- OS_Simulator_Qt/src/*.cpp (full implementation)

---

## Summary

**Phase 0** established the process and thread management foundation with proper state machines, logging, and configuration management.

**Phase 1** added real-time scheduling simulation with three different algorithms, a TCP server for communication, and a comprehensive Qt UI for visualization and interaction.

The system is **production-ready** for demonstration and testing of CPU scheduling algorithms in a realistic simulation environment.

---

**Status: 100% Complete ✅**

For detailed run instructions, see [PHASE_1_RUN_GUIDE.md](../PHASE_1_RUN_GUIDE.md)

