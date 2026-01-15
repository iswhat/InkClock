cmake_minimum_required(VERSION 3.10)

project(InkClockSimulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 添加可执行文件
add_executable(inkclock_simulator simple_simulator.cpp)

# 设置输出目录
set_target_properties(inkclock_simulator PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# 构建指令提示
message(STATUS "=======================================")
message(STATUS "InkClock Simple Simulator Configuration")
message(STATUS "=======================================")
message(STATUS "To build the simulator, run:")
message(STATUS "  cmake --build build")
message(STATUS "")
message(STATUS "To run the simulator, execute:")
message(STATUS "  inkclock_simulator.exe")
message(STATUS "=======================================")
