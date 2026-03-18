cmake_minimum_required(VERSION 3.22)
project(OS_Simulator)

set(CMAKE_CXX_STANDARD 17)

add_executable(simulator
    src/main.cpp

    src/process/ProcessManager.cpp
    src/process/StateTransition.cpp

    src/utils/Config.cpp
    src/utils/Logger.cpp

    src/scheduler/FCFS.cpp
    src/scheduler/RoundRobin.cpp
    src/scheduler/Priority.cpp
    src/scheduler/Metrics.cpp
)

target_include_directories(simulator PRIVATE src)
