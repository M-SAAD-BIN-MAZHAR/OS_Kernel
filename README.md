# OS_Simulator Engine (OS_Kernel)

C++ backend engine for the OS Process Scheduler Simulator.

## Status

- Phase 0 complete
- Phase 1 complete
- Phase 2 complete

## Features

- Process + thread management:
  - PCB + TCB models
  - 5-state lifecycle transitions
- CPU scheduling:
  - FCFS
  - Round Robin (quantum)
  - Priority
- Metrics:
  - Wait time
  - Turnaround time
  - CPU utilization
  - Throughput
- Synchronization demos:
  - OsMutex (lock/unlock/tryLock with event logging)
  - OsSemaphore (wait/signal)
  - Producer-Consumer
  - Dining Philosophers (deadlock + safe)
  - Race condition (with/without mutex)
- Networking:
  - TCP server on port 9000
  - Newline-delimited JSON stream for scheduler and sync state

## Build

```bash
cd OS_Kernel
mkdir -p build
cd build
cmake -G Ninja ..
ninja
```

Executable:

- `build/simulator` (from inside `OS_Kernel`)

Depending on your top-level build setup, you may also have:

- `../build/simulator` (from workspace root)

## Run

```bash
# recommended from workspace root
cd /home/msaad/OS_Simulator1
./build/simulator
```

Behavior:

- Starts TCP server on `localhost:9000`
- Streams scheduler and sync JSON every ~200ms
- Keeps running until interrupted (`Ctrl+C`)

## Runtime Commands (from UI client)

- Select scheduler:
  - `{"command":"select_scheduler","algorithm":"FCFS"}`
  - `{"command":"select_scheduler","algorithm":"RoundRobin"}`
  - `{"command":"select_scheduler","algorithm":"Priority"}`
- Start scheduler simulation:
  - `{"command":"start_simulation"}`
- Start synchronization demo:
  - `{"command":"start_sync_demo","demo":"producer_consumer"}`
  - `{"command":"start_sync_demo","demo":"philosophers_deadlock"}`
  - `{"command":"start_sync_demo","demo":"philosophers_safe"}`
  - `{"command":"start_sync_demo","demo":"race"}`

## Notes for Windows Qt + WSL Engine

- Run engine in WSL.
- Run Qt UI in Windows Qt Creator.
- Connect UI client to `localhost:9000`.
- If connection drops unexpectedly, verify engine terminal is still running.

## Config

Edit `config.json` in `OS_Kernel` as needed for process defaults.

