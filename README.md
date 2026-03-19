# 🖥️ OS Process Scheduler Simulator

A comprehensive CPU scheduling simulator built in C++ with Qt6 real-time visualization. Features three classic scheduling algorithms (FCFS, Round Robin, Priority) with detailed performance metrics and interactive Gantt charts.

---

## 📋 Table of Contents

- [Overview](#-overview)
- [Architecture](#-architecture)
- [Phase 0: Foundation](#-phase-0--process-management--threading)
- [Phase 1: Scheduling Engine](#-phase-1--cpu-scheduling-with-networking)
- [Project Structure](#-project-structure)
- [System Requirements](#-system-requirements)
- [Building & Running](#-building--running)
- [Usage Guide](#-usage-guide)
- [Features](#-features)
- [Configuration](#-configuration)
- [Troubleshooting](#-troubleshooting)

---

## 🎯 Overview

This project demonstrates a complete **OS Process Scheduler Simulator** that implements fundamental operating systems concepts. It consists of two main components:

1. **Engine** (C++ backend) - Runs on WSL/Linux with TCP server
2. **Client** (Qt6 UI) - Runs on Windows with real-time visualization

The simulator supports multiple scheduling algorithms and provides detailed metrics analysis and visual Gantt chart representation.

---

## 🏗️ Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                                                             │
│  ┌─────────────────────┐         ┌──────────────────────┐  │
│  │   WSL Linux Engine  │         │   Windows Qt6 Client │  │
│  │                     │         │                      │  │
│  │  • ProcessManager   │◄────TCP───►  • MainWindow      │  │
│  │  • Schedulers (3x)  │  9000   │  • GanttWidget       │  │
│  │  • Metrics Engine   │         │  • MetricsTable      │  │
│  │  • TCP Server       │         │  • SettingsTab       │  │
│  │  • ThreadManager    │         │  • LogsViewer        │  │
│  │                     │         │                      │  │
│  └─────────────────────┘         └──────────────────────┘  │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

**Communication Protocol:** JSON over TCP (port 9000)

---

## 🔧 Phase 0 — Process Management & Threading

### Objectives Completed
✅ Initialize process management system  
✅ Implement thread control blocks (TCB)  
✅ Create state machine for process lifecycle  
✅ Set up logging infrastructure  
✅ Configure application settings  

### Key Components

#### 1. **ProcessManager** (`src/process/ProcessManager.h/cpp`)
- Manages process creation and lifecycle
- Automatic PID generation
- Process state tracking (NEW → READY → RUNNING → BLOCKED → TERMINATED)
- Process display with formatted tables

```cpp
ProcessManager pm;
pm.createProcess("P1", burst_time, priority);
pm.displayProcesses();
```

#### 2. **Process Control Block (PCB)** (`src/process/PCB.h`)
Stores per-process information:
- **PID**: Process identifier (auto-incremented)
- **Name**: Process name (e.g., "P1", "P2")
- **State**: Current process state (enum-based)
- **Burst Time**: CPU time needed
- **Priority**: Scheduling priority level
- **Arrival Time**: When process enters system
- **Wait Time**: Total time waiting for CPU
- **Turnaround Time**: Total time from arrival to completion

#### 3. **Thread Control Block (TCB)** (`src/process/PCB.h`)
Manages thread-level information:
- **TID**: Thread identifier (unique within process)
- **PID**: Parent process ID
- **State**: Thread state (READY, RUNNING, BLOCKED, TERMINATED)
- **Stack Address**: Virtual memory location
- **Creation Time**: When thread was created

#### 4. **ThreadManager** (`src/process/ProcessManager.h`)
Thread lifecycle management:
- Creates 2 threads per process
- Manages thread state transitions
- Tracks thread stack allocations
- Displays thread table with detailed info

#### 5. **StateTransition** (`src/process/StateTransition.h/cpp`)
Implements the 5-state process model:
```
NEW → READY ↔ RUNNING ↔ BLOCKED → TERMINATED
```
- Validates state transitions
- Logs state changes with timestamps
- Ensures process flow compliance

#### 6. **Logger** (`src/utils/Logger.h/cpp`)
Centralized logging system:
- **INFO**: General information
- **WARNING**: Non-critical issues
- **ERROR**: Critical failures
- Timestamps on all messages
- Color-coded console output

#### 7. **Config System** (`src/utils/Config.h/cpp`)
JSON-based configuration:
```json
{
  "process_count": 5,
  "default_burst_time": 5,
  "default_priority": 2,
  "time_quantum": 2
}
```

### Phase 0 Runtime Test Output
```
[INFO] Process Created -> PID: 1 Name: P1
[INFO] Process Created -> PID: 2 Name: P2
[INFO] Process Created -> PID: 3 Name: P3
[INFO] Process Created -> PID: 4 Name: P4
[INFO] Process Created -> PID: 5 Name: P5

===== Phase 0 Test: State Transitions =====
[INFO] P1: NEW → READY
[INFO] P1: READY → RUNNING
[INFO] P1: RUNNING → BLOCKED
[INFO] P1: BLOCKED → TERMINATED

===== PHASE 0 Part 2: THREAD MANAGEMENT (TCB) =====
[INFO] Thread Created -> TID: 1 PID: 1
[INFO] Thread Created -> TID: 2 PID: 1
...
[INFO] TID 1: READY → RUNNING
[INFO] TID 1: RUNNING → BLOCKED
[INFO] TID 1: BLOCKED → TERMINATED
```

---

## ⚙️ Phase 1 — CPU Scheduling with Networking

### Objectives Completed
✅ Implement three CPU scheduling algorithms  
✅ Calculate detailed performance metrics  
✅ Build TCP server for real-time communication  
✅ Serialize scheduling data to JSON  
✅ Create Qt6 interactive dashboard  
✅ Display Gantt charts and metrics tables  

### Scheduling Algorithms

#### 1. **FCFS (First-Come-First-Served)**
- **Type**: Non-preemptive
- **Behavior**: Processes execute in arrival order, complete execution without interruption
- **Advantages**: Simple, no context switching overhead
- **Disadvantages**: Long wait times for subsequent processes (convoy effect)

Example Schedule:
```
P1  P2  P3  P4  P5
|---|---|--------|--|
0   5   8   16  22 26
```

#### 2. **Round Robin**
- **Type**: Preemptive
- **Time Quantum**: 2ms per process
- **Behavior**: Each process gets fixed time slice; if not complete, goes to end of queue
- **Advantages**: Fair CPU distribution, good response time
- **Disadvantages**: Higher context switching overhead

Example Schedule:
```
P1  P2  P3  P1  P4  P2  P3  ...
|-|-|--|-|-|--|-|-...
0 2 4 6 8 10 12 14
```

#### 3. **Priority-Based Scheduling**
- **Type**: Preemptive
- **Behavior**: Higher priority (lower number) processes run first
- **Aging**: Prevents starvation (priority decreases with wait time)
- **Advantages**: Supports different process importance levels
- **Disadvantages**: Lower priority processes may starve

Example Schedule (based on priority):
```
P2(1) P5(1) P3(2) P4(2) P1(3)
|-----|--------|--|---------|
0     5       14  19       26
```

### Key Engine Components

#### **Scheduler Base Class** (`src/scheduler/Scheduler.h`)
Abstract interface for all schedulers:
```cpp
virtual std::vector<ScheduleResult> schedule(
    const std::vector<PCB>& processes
) = 0;
```

#### **Metrics Engine** (`src/scheduler/Metrics.h/cpp`)
Calculates performance metrics for each algorithm:

| Metric | Formula | Meaning |
|--------|---------|---------|
| **Wait Time** | Completion - Arrival - Burst | Time spent waiting for CPU |
| **Turnaround Time** | Completion - Arrival | Total time in system |
| **CPU Utilization** | (Busy Time / Total Time) × 100 | CPU usage percentage |
| **Throughput** | Processes / Total Time | Processes completed per unit time |

#### **ScheduleResult** (`src/scheduler/Scheduler.h`)
Stores individual scheduling decision:
```cpp
struct ScheduleResult {
    std::string processName;
    int processId;
    int startTime;
    int endTime;
    ProcessState state;  // RUNNING, BLOCKED, etc.
};
```

#### **TCP Server** (`src/network/TCPServer.h/cpp`)
- Listens on port 9000
- Accepts multiple client connections
- Broadcasts scheduling updates every 200ms
- Thread-safe socket management
- Graceful connection handling

#### **JSON Serialization** (`src/network/SchedulerStateJSON.h/cpp`)
Converts scheduling state to JSON:
```json
{
  "algorithm": "FCFS",
  "timestamp": 0,
  "totalTime": 26,
  "ganttChart": [
    {
      "processName": "P1",
      "processId": 1,
      "startTime": 0,
      "endTime": 5
    },
    ...
  ],
  "metrics": {
    "avgWaitTime": 8.2,
    "avgTurnaroundTime": 13.4,
    "cpuUtilization": 100.0,
    "throughput": 0.192
  },
  "processes": [
    {
      "name": "P1",
      "pid": 1,
      "arrivalTime": 0,
      "burstTime": 5,
      "priority": 3,
      "waitTime": 0,
      "turnaroundTime": 5
    },
    ...
  ]
}
```

### Phase 1 Runtime Output
```
===== PHASE 1: CPU SCHEDULING WITH TCP SERVER =====
[INFO] TCP Server started on port 9000
[INFO] Server listening on port 9000
Sent scheduling data to FCFS (989 bytes)
Sent scheduling data to RoundRobin (1205 bytes)
Sent scheduling data to Priority (1150 bytes)
```

---

## 📁 Project Structure

```
OS_Simulator1/
├── README.md                          # Project documentation
├── config.json                        # Simulation configuration
│
├── OS_Kernel/                         # Engine (WSL/Linux)
│   ├── CMakeLists.txt
│   ├── config.json
│   ├── README.md
│   ├── build/                         # Build directory
│   │   ├── CMakeCache.txt
│   │   ├── simulator                  # Compiled executable
│   │   └── compile_commands.json
│   │
│   └── src/
│       ├── main.cpp                   # Entry point
│       │
│       ├── process/
│       │   ├── PCB.h
│       │   ├── ProcessManager.h
│       │   ├── ProcessManager.cpp
│       │   ├── StateTransition.h
│       │   └── StateTransition.cpp
│       │
│       ├── scheduler/
│       │   ├── Scheduler.h            # Base class
│       │   ├── FCFS.h/cpp
│       │   ├── RoundRobin.h/cpp
│       │   ├── Priority.h/cpp
│       │   ├── Metrics.h/cpp
│       │   └── Metrics.csv export
│       │
│       ├── network/
│       │   ├── TCPServer.h/cpp        # Socket server
│       │   ├── ThreadSafeQueue.h      # Concurrent queue template
│       │   └── SchedulerStateJSON.h/cpp  # JSON serializer
│       │
│       └── utils/
│           ├── Config.h/cpp
│           ├── Logger.h/cpp
│           └── json.hpp               # nlohmann/json
│
├── OS_Simulator_Qt/                   # Client UI (Windows)
│   ├── CMakeLists.txt
│   ├── README.md
│   ├── build/                         # Build directory (Qt)
│   │   └── OS_Simulator_Qt.exe        # Compiled executable
│   │
│   └── src/
│       ├── main.cpp
│       ├── main_window.h/cpp          # Main UI window (5 tabs)
│       ├── simulator_client.h/cpp     # TCP client
│       │
│       └── widgets/
│           ├── gantt_widget.h/cpp     # Gantt chart visualization
│           └── metrics_table.h/cpp    # Metrics display table
│
└── output/
    ├── metrics_FCFS.csv               # Metrics data export
    ├── metrics_RoundRobin.csv
    └── metrics_Priority.csv
```

---

## 💻 System Requirements

### For Engine (WSL/Linux)
- **OS**: Ubuntu 20.04+ or WSL2
- **Compiler**: GCC 9+ with C++17 support
- **Build Tool**: CMake 3.16+, Ninja
- **Libraries**: pthread (standard on Linux)
- **Dependencies**:
  - CMake 3.16 or later
  - Ninja (recommended)
  - nlohmann/json (included)

### For Client (Windows)
- **OS**: Windows 10/11
- **IDE**: Qt Creator 11+ or Visual Studio 2019+
- **Qt Version**: Qt 6.7 or later
- **Compiler**: MinGW 11.2+ or MSVC 2019+
- **Dependencies**:
  - Qt6 Core, Gui, Widgets, Network, Charts
  - CMake 3.16+

### Network
- **WSL Network Access**: Must have WSL IP address (e.g., 172.21.89.117)
- **Port 9000**: Must be available and not in use
- **Firewall**: WSL↔Windows communication allowed

---

## 🔨 Building & Running

### Step 1: Build the Engine (WSL/Linux)

```bash
# Navigate to engine directory
cd OS_Simulator1/OS_Kernel

# Create build directory (if not exists)
mkdir -p build
cd build

# Configure with CMake
cmake -G Ninja ..

# Build with Ninja
ninja

# Verify build succeeded
ls -la simulator
```

**Expected Output:**
```
[INFO] TCP Server started on port 9000
[INFO] Server listening on port 9000
Sent scheduling data to FCFS (989 bytes)
...
```

### Step 2: Get WSL IP Address

```bash
# In WSL terminal
hostname -I

# Look for output like: 172.21.89.117 192.168.1.100
# Use the first IP (172.x.x.x format)
```

### Step 3: Configure Qt Client

In Windows, edit `OS_Simulator_Qt/src/main_window.cpp` and update the IP:

```cpp
// Line 20
: QMainWindow(parent), client(std::make_unique<SimulatorClient>("172.21.89.117", 9000))
```

Replace `172.21.89.117` with your actual WSL IP from Step 2.

### Step 4: Build Qt Client (Windows)

```bash
# In Qt Creator on Windows:
1. File → Open File or Project
2. Select OS_Simulator_Qt/CMakeLists.txt
3. Select Kit: Desktop Qt 6.x MinGW 64-bit (or MSVC)
4. Click Configure Project
5. Build → Build Project "OS_Simulator_Qt"

# Or via command line:
cd OS_Simulator_Qt/build
cmake -G "MinGW Makefiles" ..
cmake --build .
```

### Step 5: Run Both Applications

**Terminal 1 (WSL):** Start the engine server
```bash
cd OS_Simulator1/OS_Kernel/build
./simulator
# Keep running for 5 minutes
```

**Terminal 2 (Windows):** Start the Qt client
```bash
# From Qt Creator: Click "Run" button
# Or from command line:
.\build\OS_Simulator_Qt.exe
```

**Expected Results:**
1. Engine terminal shows:
   ```
   [INFO] TCP Server started on port 9000
   [INFO] Server listening on port 9000
   Sent scheduling data to FCFS (989 bytes)
   ```

2. Qt window appears with:
   - Connection status: "Connected to server" ✅
   - Gantt chart showing FCFS schedule
   - Metrics table populated
   - All 5 tabs functional

---

## 📖 Usage Guide

### Main Window Layout

The Qt application provides an intuitive 5-tab interface:

#### **Tab 1: Scheduler** 
- **Algorithm Selector**: Dropdown to switch between FCFS, Round Robin, Priority
- **Gantt Chart**: Visual representation of process scheduling over time
- **Color Coding**:
  - 🟢 Green: Process RUNNING
  - 🔵 Blue: Process READY
  - 🟡 Yellow: Process BLOCKED
- **Time Scale**: X-axis shows CPU time in milliseconds
- **Process Timeline**: Each process shown as a horizontal bar

#### **Tab 2: Metrics**
- **Performance Table** with columns:
  - Process Name (P1, P2, P3, ...)
  - Arrival Time
  - Burst Time
  - Wait Time
  - Turnaround Time
  - Priority (if applicable)
- **Summary Statistics**:
  - Average Wait Time
  - Average Turnaround Time
  - CPU Utilization (%)
  - Throughput (processes/time unit)

#### **Tab 3: Settings**
- **Server Configuration**:
  - Host: IP address (configure before running)
  - Port: 9000 (default)
  - Connection Status: Connected/Disconnected
- **Simulation Settings**:
  - Time Quantum (Round Robin): 2ms
  - Process Count: Configurable
  - Default Burst Time: Configurable

#### **Tab 4: Logs**
- **Real-time Log Viewer** showing:
  - Connection status changes
  - JSON reception events
  - Parsing status
  - Error messages
  - Scheduling updates

#### **Tab 5: About**
- **Project Information**:
  - Title: OS Process Scheduler Simulator
  - Version: 1.0
  - Authors: Development Team
  - Features overview

### Switching Between Algorithms

1. Open the Qt application
2. Go to **Scheduler** tab
3. Click algorithm dropdown (top-left)
4. Select: **FCFS** | **Round Robin** | **Priority**
5. Gantt chart automatically updates with new schedule
6. Metrics table refreshes with new statistics

### Analyzing Metrics

Compare performance across algorithms:

| Algorithm | Avg Wait | Avg Turnaround | CPU Util. | Best For |
|-----------|----------|----------------|-----------|----------|
| FCFS | Higher | Higher | 100% | Simple, predictable |
| Round Robin | Medium | Medium | 100% | Fair, general purpose |
| Priority | Lower | Lower | 100% | Important tasks first |

---

## ⚙️ Configuration

### config.json (Engine)

Located at `OS_Kernel/config.json`:

```json
{
  "process_count": 5,
  "default_burst_time": 5,
  "default_priority": 2,
  "time_quantum": 2
}
```

**Parameters:**
- `process_count`: Number of test processes (2-10 recommended)
- `default_burst_time`: CPU time each process needs (milliseconds)
- `default_priority`: Default priority level (1-10, lower = higher priority)
- `time_quantum`: Time slice for Round Robin (milliseconds)

### CMakeLists.txt Customization

**Engine** `OS_Kernel/CMakeLists.txt`:
- C++ standard: C++17
- Optimization: -O3 (Release)
- Target executable: `simulator`

**Client** `OS_Simulator_Qt/CMakeLists.txt`:
- Qt Version: Qt 6.x
- Modules: Core, Gui, Widgets, Network, Charts
- Target executable: `OS_Simulator_Qt`

---

## 🐛 Troubleshooting

### Engine Issues

#### "Could not open config file!"
**Cause**: Engine can't find `config.json`  
**Solution**:
```bash
cd OS_Kernel/build
# Ensure config.json exists at parent level
ls ../config.json
# Or copy it
cp ../config.json .
./simulator
```

#### Port 9000 already in use
**Cause**: Another process using port 9000  
**Solution**:
```bash
# Find process using port
lsof -i :9000

# Kill the process
kill -9 <PID>

# Or change port in TCPServer and client code
```

#### "Linking CXX executable simulator" fails
**Cause**: Missing pthread library  
**Solution**:
```bash
# Ensure pthread is linked in CMakeLists.txt
target_link_libraries(simulator pthread)

# Rebuild
cd build && ninja
```

### Qt Client Issues

#### "Connection refused" error
**Cause**: Engine not running or IP address incorrect  
**Solution**:
```bash
# 1. Verify engine is running
cd OS_Kernel/build && ./simulator

# 2. Check WSL IP
hostname -I

# 3. Update main_window.cpp with correct IP
# 4. Rebuild Qt application
```

#### "Cannot connect to server" (blank Gantt chart)
**Cause**: Firewall blocking WSL↔Windows communication  
**Solution**:
```bash
# Windows: Allow TCP port 9000 in Windows Defender Firewall
# Or test connectivity:
# From Windows CMD:
telnet 172.21.89.117 9000

# Should show connection attempt
```

#### Qt build fails with "Qt6 not found"
**Cause**: Qt Creator not configured properly  
**Solution**:
1. In Qt Creator: Tools → Options → Kits
2. Add new desktop kit with Qt 6.x
3. Set compiler to MinGW 11.2+ or MSVC
4. Reconfigure project

#### "Socket error: Remote host closed connection"
**Cause**: Engine shutdown after 5 minutes or unhandled exception  
**Solution**:
```bash
# Check engine terminal for error messages
# Verify engine is still running
ps aux | grep simulator

# Restart engine
cd OS_Kernel/build && ./simulator
```

### Network Issues

#### WSL IP Changes After Reboot
**Cause**: WSL IP is dynamic  
**Solution**:
```bash
# WSL2 uses dynamic IP; check before each run
hostname -I

# Update Qt client's IP address in main_window.cpp if changed
# Rebuild Qt application
```

#### "Connection timeout"
**Cause**: Network connectivity issue  
**Solution**:
```bash
# Test WSL ↔ Windows connectivity
# From WSL:
curl http://localhost:9000

# From Windows:
# Use PowerShell:
Test-NetConnection -ComputerName 172.21.89.117 -Port 9000
```

---

## 🎓 Learning Outcomes

After completing this project, you will understand:

### Operating Systems Concepts
✅ **Process Management**: PCB, PID, process states, lifecycle  
✅ **Thread Management**: TCB, thread states, multi-threading  
✅ **CPU Scheduling**: Algorithms, preemption, context switching  
✅ **Metrics Analysis**: Wait time, turnaround, utilization  
✅ **State Machines**: Process state transitions, validation  

### Software Engineering
✅ **Architecture Design**: Engine-client separation, TCP/JSON protocol  
✅ **Real-time Systems**: Concurrent programming, thread safety  
✅ **Qt Framework**: GUI development, signals/slots, threading  
✅ **C++ Modern Features**: Smart pointers, templates, lambda functions  
✅ **Build Systems**: CMake, Ninja, cross-platform compilation  

### Tools & Technologies
✅ **Linux Development**: WSL, GCC, Ninja, command-line debugging  
✅ **Windows Development**: Qt Creator, Visual Studio, MinGW toolchain  
✅ **Version Control**: Git, commit history, branching  
✅ **Networking**: TCP sockets, POSIX networking, JSON protocol  

---

## 📚 References

- **Operating Systems Concepts**: PCB, TCB, Scheduling algorithms
- **C++ Standards**: C++17 features, smart pointers, templates
- **Qt Documentation**: https://doc.qt.io/
- **JSON Format**: https://www.json.org/
- **CMake Guide**: https://cmake.org/cmake/help/latest/

---

## 📝 License

This project is created for educational purposes as part of an Operating Systems course.

---

## 📧 Support

For questions or issues:
1. Check the **Troubleshooting** section above
2. Review engine/client terminal output for error messages
3. Verify all configuration steps completed correctly
4. Ensure network connectivity between WSL and Windows

---

**Happy Scheduling! 🚀**

Last Updated: March 2026  
Version: 1.0 (Phase 0 & 1 Complete)
