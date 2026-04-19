# Phase 5 Presentation Outline

## Slide 1: Title

- OS Process Scheduler Simulator
- Integration and Evaluation Summary

## Slide 2: Problem Statement

- Simulate core operating system behaviors in a teaching-focused desktop application.
- Cover scheduling, synchronization, and memory management.

## Slide 3: Architecture

- C++17 backend engine
- Qt6 frontend UI
- TCP JSON protocol on port 9000

## Slide 4: Scheduling

- FCFS
- Round Robin
- Priority
- Metrics: wait time, turnaround time, CPU utilization, throughput

## Slide 5: Synchronization

- Mutex and semaphore primitives
- Producer-consumer
- Dining philosophers
- Race condition demo

## Slide 6: Memory Management

- Paging and address translation
- TLB tracking
- FIFO, LRU, Optimal replacement

## Slide 7: Evaluation

- Show collected metrics and visual output from the integrated simulator.
- Highlight deadlock detection and race-condition correction.

## Slide 8: Conclusion

- All phases are integrated into one runnable simulator.
- Backend runs in WSL and UI runs on Windows.
