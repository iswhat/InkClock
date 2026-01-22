cmake_minimum_required(VERSION 3.16)

project(InkClockSimulator VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Qt packages
find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Network)
qt_standard_project_setup()

# Add source files
set(SOURCES
    main.cpp
    simulator.cpp
    displaywidget.cpp
    inkclockemulator.cpp
    mockhardware.cpp
    mockdisplay.cpp
    mocksensors.cpp
    mocknetwork.cpp
)

# Add header files
set(HEADERS
    simulator.h
    displaywidget.h
    inkclockemulator.h
    mockhardware.h
    mockdisplay.h
    mocksensors.h
    mocknetwork.h
    inkclocktypes.h
)

# Add resources
set(RESOURCES
    resources.qrc
)

# Create the application executable
qt_add_executable(inkclock_simulator ${SOURCES} ${HEADERS} ${RESOURCES})

# Link Qt libraries
target_link_libraries(inkclock_simulator PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Network)

# Install settings
install(TARGETS inkclock_simulator DESTINATION bin)
