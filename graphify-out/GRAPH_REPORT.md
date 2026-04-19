# Graph Report - .  (2026-04-19)

## Corpus Check
- 65 files · ~106,637 words
- Verdict: corpus is large enough that graph structure adds value.

## Summary
- 389 nodes · 963 edges · 25 communities detected
- Extraction: 74% EXTRACTED · 26% INFERRED · 0% AMBIGUOUS · INFERRED: 251 edges (avg confidence: 0.8)
- Token cost: 0 input · 0 output

## Community Hubs (Navigation)
- [Get Number](#community-0---get-number)
- [Widget Setupui](#community-1---widget-setupui)
- [Memorysimulator Roundrobin](#community-2---memorysimulator-roundrobin)
- [Main Tcpserver](#community-3---main-tcpserver)
- [Back Value](#community-4---back-value)
- [Deadlockdetector Deadlocksimulator](#community-5---deadlockdetector-deadlocksimulator)
- [Syncdemorunner Gantt](#community-6---syncdemorunner-gantt)
- [Completion Tcp](#community-7---completion-tcp)
- [Ossemaphore Signal](#community-8---ossemaphore-signal)
- [Module Sources](#community-9---module-sources)
- [Path Qt6](#community-10---path-qt6)
- [Philosophers Widget](#community-11---philosophers-widget)
- [Main Cmakecxxcompilerid](#community-12---main-cmakecxxcompilerid)
- [Osmutex](#community-13---osmutex)
- [Config](#community-14---config)
- [Logger](#community-15---logger)
- [Schedulerstatejson](#community-16---schedulerstatejson)
- [Resourcetype Resource](#community-17---resourcetype-resource)
- [Processmanager](#community-18---processmanager)
- [Processstate Pcb](#community-19---processstate-pcb)
- [Scheduler](#community-20---scheduler)
- [Metricsengine Metrics](#community-21---metricsengine-metrics)
- [Synctypes](#community-22---synctypes)
- [Tcb](#community-23---tcb)
- [Statetransition](#community-24---statetransition)

## God Nodes (most connected - your core abstractions)
1. `push_back()` - 47 edges
2. `size()` - 38 edges
3. `begin()` - 33 edges
4. `end()` - 33 edges
5. `main()` - 23 edges
6. `erase()` - 20 edges
7. `basic_json()` - 20 edges
8. `value()` - 20 edges
9. `empty()` - 19 edges
10. `to_string()` - 18 edges

## Surprising Connections (you probably didn't know these)
- `project phase status summary` --semantically_similar_to--> `overall project completion summary`  [INFERRED] [semantically similar]
  README.md → PROJECT_COMPLETION_SUMMARY.md
- `phase 5 presentation outline` --semantically_similar_to--> `phase 5 integration and evaluation report`  [INFERRED] [semantically similar]
  PHASE_5_PRESENTATION_OUTLINE.md → PHASE_5_COMPLETE.md
- `setupUI()` --calls--> `size()`  [INFERRED]
  OS_Simulator_Qt/src/deadlock_widget.cpp → src/utils/json.hpp
- `updateDeadlockData()` --calls--> `size()`  [INFERRED]
  OS_Simulator_Qt/src/deadlock_widget.cpp → src/utils/json.hpp
- `updateDeadlockData()` --calls--> `clear()`  [INFERRED]
  OS_Simulator_Qt/src/deadlock_widget.cpp → src/utils/json.hpp

## Hyperedges (group relationships)
- **Integrated Runtime Stack** — cmakelists_simulator_target, qt_cmakelists_qt_target, readme_transport_newline_json, concept_tcp_port_9000 [EXTRACTED 1.00]
- **Core OS Curriculum Coverage** — concept_scheduler_algorithms, concept_sync_demos, concept_memory_paging, phase5_report_integration [INFERRED 0.82]

## Communities

### Community 0 - "Get Number"
Cohesion: 0.04
Nodes (51): add(), basic_json(), binary(), crbegin(), data(), decode(), dump_integer(), from_bjdata() (+43 more)

### Community 1 - "Widget Setupui"
Cohesion: 0.07
Nodes (53): DeadlockWidget(), setupUI(), updateDeadlockData(), contains(), convert(), type(), value(), addLog() (+45 more)

### Community 2 - "Memorysimulator Roundrobin"
Cohesion: 0.08
Nodes (44): schedule(), Scheduler(), begin(), cbegin(), cend(), count(), crend(), diff() (+36 more)

### Community 3 - "Main Tcpserver"
Cohesion: 0.09
Nodes (27): load(), stop(), accept(), parse(), to_string(), error(), getTime(), info() (+19 more)

### Community 4 - "Back Value"
Cohesion: 0.17
Nodes (35): at(), back(), create(), emplace(), emplace_back(), empty(), erase(), erase_internal() (+27 more)

### Community 5 - "Deadlockdetector Deadlocksimulator"
Cohesion: 0.15
Nodes (26): addResource(), buildWaitForGraph(), canAllocate(), DeadlockDetector(), detectCycleInGraph(), detectDeadlock(), findCycle(), findSafeSequence() (+18 more)

### Community 6 - "Syncdemorunner Gantt"
Cohesion: 0.16
Nodes (13): animateAxisMax(), clearChart(), GanttWidget(), setupChart(), updateGanttChart(), array(), clear(), reset() (+5 more)

### Community 7 - "Completion Tcp"
Cohesion: 0.18
Nodes (14): paging TLB and page replacement simulation, FCFS Round Robin Priority scheduling, producer-consumer philosophers race-condition demos, TCP port 9000 runtime interface, phase 0 foundation completion report, phase 1 build and run guide, phase 4 memory management completion report, phase 5 presentation outline (+6 more)

### Community 8 - "Ossemaphore Signal"
Cohesion: 0.25
Nodes (5): signal(), waitFor(), runProducerConsumer(), push(), tryPop()

### Community 9 - "Module Sources"
Cohesion: 0.25
Nodes (8): C++17 language standard, memory module sources, network module sources, process module sources, pthread linkage, scheduler module sources, simulator executable target, synchronization module sources

### Community 10 - "Path Qt6"
Cohesion: 0.29
Nodes (7): C++ compiler path /usr/bin/c++, Ninja generator program path, Qt6_DIR unresolved in linux build cache, deadlock widget source set, Qt6 Core/Gui/Widgets/Network/Charts components, OS_Simulator_Qt executable target, sync timeline and philosophers widgets

### Community 11 - "Philosophers Widget"
Cohesion: 0.5
Nodes (3): colorForState(), paintEvent(), PhilosophersWidget()

### Community 12 - "Main Cmakecxxcompilerid"
Cohesion: 1.0
Nodes (0): 

### Community 13 - "Osmutex"
Cohesion: 1.0
Nodes (0): 

### Community 14 - "Config"
Cohesion: 1.0
Nodes (0): 

### Community 15 - "Logger"
Cohesion: 1.0
Nodes (0): 

### Community 16 - "Schedulerstatejson"
Cohesion: 1.0
Nodes (0): 

### Community 17 - "Resourcetype Resource"
Cohesion: 1.0
Nodes (0): 

### Community 18 - "Processmanager"
Cohesion: 1.0
Nodes (0): 

### Community 19 - "Processstate Pcb"
Cohesion: 1.0
Nodes (0): 

### Community 20 - "Scheduler"
Cohesion: 1.0
Nodes (0): 

### Community 21 - "Metricsengine Metrics"
Cohesion: 1.0
Nodes (0): 

### Community 22 - "Synctypes"
Cohesion: 1.0
Nodes (0): 

### Community 23 - "Tcb"
Cohesion: 1.0
Nodes (0): 

### Community 24 - "Statetransition"
Cohesion: 1.0
Nodes (0): 

## Ambiguous Edges - Review These
- `Qt6 Core/Gui/Widgets/Network/Charts components` → `Qt6_DIR unresolved in linux build cache`  [AMBIGUOUS]
  OS_Simulator_Qt/build-linux/CMakeCache.txt · relation: references

## Knowledge Gaps
- **22 isolated node(s):** `invalid_iterator`, `type_error`, `out_of_range`, `other_error`, `json_sax_acceptor` (+17 more)
  These have ≤1 connection - possible missing edges or undocumented components.
- **Thin community `Main Cmakecxxcompilerid`** (2 nodes): `main()`, `CMakeCXXCompilerId.cpp`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Osmutex`** (2 nodes): `OsMutex()`, `OsMutex.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Config`** (2 nodes): `Config()`, `Config.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Logger`** (2 nodes): `Logger()`, `Logger.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Schedulerstatejson`** (2 nodes): `SchedulerStateJSON()`, `SchedulerStateJSON.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Resourcetype Resource`** (2 nodes): `ResourceType()`, `Resource.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Processmanager`** (2 nodes): `ProcessManager()`, `ProcessManager.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Processstate Pcb`** (2 nodes): `ProcessState()`, `PCB.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Scheduler`** (2 nodes): `Scheduler()`, `Scheduler.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Metricsengine Metrics`** (2 nodes): `MetricsEngine()`, `Metrics.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Synctypes`** (1 nodes): `SyncTypes.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Tcb`** (1 nodes): `TCB.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.
- **Thin community `Statetransition`** (1 nodes): `StateTransition.h`
  Too small to be a meaningful cluster - may be noise or needs more connections extracted.

## Suggested Questions
_Questions this graph is uniquely positioned to answer:_

- **What is the exact relationship between `Qt6 Core/Gui/Widgets/Network/Charts components` and `Qt6_DIR unresolved in linux build cache`?**
  _Edge tagged AMBIGUOUS (relation: references) - confidence is low._
- **Why does `push_back()` connect `Back Value` to `Get Number`, `Widget Setupui`, `Memorysimulator Roundrobin`, `Main Tcpserver`, `Deadlockdetector Deadlocksimulator`, `Syncdemorunner Gantt`?**
  _High betweenness centrality (0.136) - this node is a cross-community bridge._
- **Why does `size()` connect `Memorysimulator Roundrobin` to `Get Number`, `Widget Setupui`, `Back Value`, `Deadlockdetector Deadlocksimulator`, `Syncdemorunner Gantt`?**
  _High betweenness centrality (0.084) - this node is a cross-community bridge._
- **Why does `main()` connect `Main Tcpserver` to `Widget Setupui`, `Memorysimulator Roundrobin`, `Deadlockdetector Deadlocksimulator`, `Syncdemorunner Gantt`, `Ossemaphore Signal`?**
  _High betweenness centrality (0.068) - this node is a cross-community bridge._
- **Are the 26 inferred relationships involving `push_back()` (e.g. with `updateGanttChart()` and `updateSynchronizationTab()`) actually correct?**
  _`push_back()` has 26 INFERRED edges - model-reasoned connections that need verification._
- **Are the 21 inferred relationships involving `size()` (e.g. with `setupUI()` and `updateDeadlockData()`) actually correct?**
  _`size()` has 21 INFERRED edges - model-reasoned connections that need verification._
- **Are the 11 inferred relationships involving `begin()` (e.g. with `updateTimelineLocked()` and `buildJSONSnapshot()`) actually correct?**
  _`begin()` has 11 INFERRED edges - model-reasoned connections that need verification._