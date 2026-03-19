# Phase 1 Completion Guide - How to Run

**Status: ✅ COMPLETE**

**Date: March 19, 2026**

---

## Overview

Phase 1 (Week 2: CPU Scheduling) is now **fully implemented** with:
- ✅ **Engine (WSL/Linux)**: TCP server sending scheduling data every 200ms
- ✅ **Qt UI (Windows/Cross-platform)**: Complete UI with 5 tabs, Gantt charts, metrics tables
- ✅ **Real-time Synchronization**: JSON-based communication between engine and client
- ✅ **All 3 Schedulers**: FCFS, RoundRobin, Priority fully integrated

---

## Part 1: Running the CPU Scheduling Engine (WSL/Linux)

### Prerequisites
- WSL2 with Ubuntu 22.04 (or Linux directly)
- g++ compiler
- CMake 3.22+
- Ninja build system

### Build Instructions

```bash
# 1. Navigate to the OS_Kernel directory
cd /home/msaad/OS_Simulator1/OS_Kernel

# 2. Create and enter build directory
mkdir -p build
cd build

# 3. Configure with CMake
cmake -G Ninja ..

# 4. Build with Ninja
ninja
```

### Run the Engine

```bash
# From the OS_Kernel directory
./build/simulator
```

**Expected Output:**
```
[INFO] Thu Mar 19 08:27:53 2026 : Process Created -> PID: 1 Name: P1
[INFO] Thu Mar 19 08:27:53 2026 : Process Created -> PID: 2 Name: P2
[INFO] Thu Mar 19 08:27:53 2026 : Process Created -> PID: 3 Name: P3
...
===== PHASE 1: CPU SCHEDULING WITH TCP SERVER =====
[INFO] Thu Mar 19 08:27:53 2026 : TCP Server started on port 9000
[INFO] Thu Mar 19 08:27:53 2026 : Server listening on port 9000

Running FCFS scheduler...
Sent scheduling data to FCFS (989 bytes)

Running FCFS scheduler...
Sent scheduling data to FCFS (989 bytes)
...
```

**The server will:**
- Start listening on `localhost:9000`
- Send JSON scheduling data every 200ms
- Continue running until terminated (Ctrl+C or 30 seconds by default)

### Server Output Format

Each broadcast sends JSON like:
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
    {
      "processName": "P2",
      "processId": 2,
      "startTime": 5,
      "endTime": 8
    }
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
    "avgWaitTime": 3.33333,
    "avgTurnaroundTime": 8.66667,
    "cpuUtilization": 100.0,
    "throughput": 0.1875
  }
}
```

---

## Part 2: Building the Qt UI (Windows or Linux/macOS with Qt6)

### Prerequisites
- **Qt Creator** 2024+ with **Qt 6.7**
- CMake 3.22+
- A C++ 17 compiler (MSVC on Windows, gcc/clang on Linux/macOS)
- nlohmann/json library (included as header)

### Installation Instructions

#### On Windows:
1. **Download Qt Online Installer** from https://www.qt.io/download
2. **Install Qt 6.7** (full installation)
3. **Install Qt Creator** (comes with the installer)
4. Install a compiler:
   - **Option A**: MSVC (Visual Studio Community 2022 - free)
   - **Option B**: MinGW (included with some Qt distributions)

#### On Linux/macOS:
```bash
# Ubuntu/Debian
sudo apt-get install qt6-base-dev qt6-network-dev qt6-charts-dev cmake ninja-build

# macOS with Homebrew
brew install qt cmake ninja
```

### Build Instructions

#### Using Qt Creator (Recommended for Windows):

1. **Open Qt Creator**
2. **File → Open File or Project**
3. Navigate to `/path/to/OS_Simulator1/OS_Simulator_Qt/CMakeLists.txt`
4. Click **Open**
5. Configure the project:
   - Select **Qt 6.7** kit
   - Click **Configure Project**
6. Click **Build** (or Ctrl+B)

#### Using CMake (All Platforms):

```bash
# 1. Navigate to Qt project directory
cd /path/to/OS_Simulator1/OS_Simulator_Qt

# 2. Create and enter build directory
mkdir -p build
cd build

# 3. Configure with CMake (specify Qt path if needed)
# For Windows MSVC:
cmake .. -G "Visual Studio 17 2022" -DCMAKE_PREFIX_PATH="C:/Qt/6.7.0/msvc2019_64"

# For Linux/macOS with Ninja:
cmake .. -G Ninja

# 4. Build
cmake --build .
# OR with Ninja:
ninja
```

---

## Part 3: Running the Application

### Starting the Engine First

```bash
# Terminal 1: On WSL/Linux machine
cd /home/msaad/OS_Simulator1/OS_Kernel
./build/simulator
```

**Keep this running.** The engine will:
- Start the TCP server on port 9000
- Listen for client connections
- Broadcast scheduling data every 200ms

### Starting the Qt Client

```bash
# Terminal 2: On Windows or another machine

# If built with Qt Creator:
# Run directly from Qt Creator (green play button)

# If built with CMake:
cd /path/to/OS_Simulator1/OS_Simulator_Qt/build
./OS_Simulator_Qt  # or ./OS_Simulator_Qt.exe on Windows
```

**The Qt application will:**
- Launch with 5 tabs (Scheduler, Metrics, Settings, Logs, About)
- Automatically connect to `localhost:9000` (engine)
- Display "Connected to server" in status bar when connected
- Update Gantt chart and metrics table every 200ms

---

## Part 3.5: Remote Connection (If Running on Different Machines)

If the engine and client are on different machines:

### On WSL/Linux (Engine):
```bash
cd /home/msaad/OS_Simulator1/OS_Kernel
./build/simulator
# Note the IP address: use `hostname -I` to find it
```

### On Windows/Other Machine (Client):
Edit [OS_Simulator_Qt/src/simulator_client.h](OS_Simulator_Qt/src/simulator_client.h):

```cpp
// Line 28: Change "localhost" to the engine's IP address
explicit SimulatorClient(const QString &host = "192.168.1.100", quint16 port = 9000, QObject *parent = nullptr);
```

Then rebuild and run.

---

## Part 4: Using the Qt Application

### Main Window

The Qt application has 5 tabs:

#### **Tab 1: Scheduler**
- **Algorithm Selector**: Dropdown to choose FCFS, RoundRobin, or Priority
- **Start Simulation Button**: Starts the simulation
- **Gantt Chart**: Real-time visualization of process scheduling
  - Each colored bar represents a process execution
  - Green = Running, Blue = Ready, Yellow = Blocked

#### **Tab 2: Metrics**
- **Process Metrics Table**: Shows for each process:
  - Process name
  - Burst time
  - Arrival time
  - Wait time
  - Turnaround time
  - Priority

#### **Tab 3: Settings**
- Placeholder for future configuration options

#### **Tab 4: Logs**
- Real-time event log showing:
  - Server connection status
  - Algorithm changes
  - Simulation start/stop events
  - Any errors

#### **Tab 5: About**
- Application information and version

### Status Bar
- **Bottom of window**: Shows current connection status
  - "Connected to server" = Engine is running and client is receiving data
  - "Disconnected from server" = Engine not running or network issue

### Workflow Example

```bash
# Terminal 1: Start engine
cd /home/msaad/OS_Simulator1/OS_Kernel
./build/simulator

# Terminal 2: Start Qt client (or launch from Qt Creator)
cd /path/to/OS_Simulator_Qt/build
./OS_Simulator_Qt
```

In the Qt application:
1. Wait for "Connected to server" in status bar
2. Select "FCFS" from Algorithm Selector dropdown
3. Click "Start Simulation"
4. Watch the Gantt chart update in real-time
5. View metrics in the Metrics tab
6. Switch to other algorithms using the dropdown

---

## Troubleshooting

### "Connection Refused" or "Unable to Connect"
**Problem**: Qt client can't connect to engine
**Solutions**:
1. Ensure engine is running: `ps aux | grep simulator`
2. Check port is listening: `netstat -an | grep 9000` (Linux) or `netstat -ano | findstr 9000` (Windows)
3. Check firewall isn't blocking port 9000
4. Verify IP address if connecting remotely

### "Received JSON but chart doesn't update"
**Problem**: Data arrives but UI changes are missing
**Solutions**:
1. Check JSON format in engine output
2. Verify nlohmann/json library is found by CMake
3. Check Qt Charts library is properly linked

### Engine crashes after 30 seconds
**Expected behavior**: The simulator runs for 30 seconds by default
**To change**:
Edit [OS_Kernel/src/main.cpp](OS_Kernel/src/main.cpp) line ~120:
```cpp
// Change from:
std::this_thread::sleep_for(std::chrono::seconds(30));
// To (for 60 seconds):
std::this_thread::sleep_for(std::chrono::seconds(60));
```

### "Qt6 not found" when building
**Problem**: CMake can't locate Qt6
**Solutions**:
1. Ensure Qt6 is installed properly
2. Set `CMAKE_PREFIX_PATH`:
   ```bash
   cmake .. -DCMAKE_PREFIX_PATH=/path/to/Qt/6.7.0/gcc_64
   ```
3. Or specify full path in CMakeLists.txt

---

## Verification Checklist

✅ **Engine**
- [ ] Compiles without errors
- [ ] Starts TCP server on port 9000
- [ ] Sends JSON data every 200ms
- [ ] Phase 0 tests run (ProcessManager, ThreadManager, state transitions)

✅ **Qt Client**
- [ ] Compiles without errors
- [ ] Launches with 5 tabs visible
- [ ] Shows "Connected to server" when engine is running
- [ ] Gantt chart populates with process bars
- [ ] Metrics table shows process data
- [ ] Logs tab shows connection events

✅ **Integration**
- [ ] Engine and client communicate over TCP
- [ ] Switching algorithms updates the chart
- [ ] Data updates every 200ms (not blocking UI)
- [ ] Status bar shows real-time connection status

---

## Quick Start (TL;DR)

### Build
```bash
# Engine
cd /home/msaad/OS_Simulator1/OS_Kernel && mkdir -p build && cd build && cmake -G Ninja .. && ninja

# Qt (Windows: open in Qt Creator, or use cmake)
cd /path/to/OS_Simulator_Qt && mkdir -p build && cd build && cmake -G Ninja .. && ninja
```

### Run
```bash
# Terminal 1: Engine
cd /home/msaad/OS_Simulator1/OS_Kernel
./build/simulator

# Terminal 2: Qt Client
./path/to/OS_Simulator_Qt/build/OS_Simulator_Qt
```

---

## Project Structure

```
OS_Simulator1/
├── OS_Kernel/              # Engine (C++, WSL/Linux)
│   ├── CMakeLists.txt
│   ├── config.json
│   ├── src/
│   │   ├── main.cpp        # Main entry point with TCP server
│   │   ├── process/        # PCB, ProcessManager, ThreadManager, TCB
│   │   ├── scheduler/      # FCFS, RoundRobin, Priority, Metrics
│   │   ├── network/        # TCPServer, SchedulerStateJSON
│   │   └── utils/          # Logger, Config, ThreadSafeQueue
│   └── build/              # Build directory
│       └── simulator       # Final executable
│
└── OS_Simulator_Qt/        # Client UI (Qt6, Windows/Linux/macOS)
    ├── CMakeLists.txt
    ├── src/
    │   ├── main.cpp        # Qt app entry point
    │   ├── main_window.*    # Main window with 5 tabs
    │   ├── simulator_client.* # TCP client code
    │   ├── gantt_widget.*   # Gantt chart visualization
    │   └── gantt_widget.h
    └── build/              # Build directory
        └── OS_Simulator_Qt # Final executable
```

---

## Next Steps (Phase 2+)

Possible enhancements:
- Add process arrival simulation (not all at time 0)
- Real-time preemption simulation while UI updates
- Thread-per-scheduler for parallel execution
- Context switch visualization
- Memory management simulation
- I/O block/unblock events

---

## Support

For issues building or running:
1. Check all prerequisites are installed
2. Verify paths are correct
3. Check firewall/network settings for remote connections
4. Review console output for specific error messages

---

**Phase 1 is complete and ready for deployment!**
