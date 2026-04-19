cmake_minimum_required(VERSION 3.16)
project(OS_Simulator_Qt VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt6 packages
find_package(Qt6 COMPONENTS 
    Core
    Gui
    Widgets
    Network
    Charts
    REQUIRED
)

# Source files
set(SOURCES
    src/main.cpp
    src/simulator_client.cpp
    src/main_window.cpp
    src/gantt_widget.cpp
    src/sync_timeline_widget.cpp
    src/philosophers_widget.cpp
    src/deadlock_widget.cpp
)

set(HEADERS
    src/simulator_client.h
    src/main_window.h
    src/gantt_widget.h
    src/sync_timeline_widget.h
    src/philosophers_widget.h
    src/deadlock_widget.h
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME}
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Network
    Qt6::Charts
)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
    WIN32_EXECUTABLE ON
    MACOSX_BUNDLE ON
)
