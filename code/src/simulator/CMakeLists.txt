cmake_minimum_required(VERSION 3.10)

project(InkClockSimulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 设置编译选项
if(MSVC)
    # MSVC 编译器选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
else()
    # GCC/Clang 编译器选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# 包含头文件目录
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}/../coresystem
  ${CMAKE_CURRENT_SOURCE_DIR}/../application
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/peripherals
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers
)

# 收集源文件
set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/simulator_main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/peripherals/simulator_display.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../coresystem/core_system.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../application/display_manager.cpp
)

# 创建可执行文件
add_executable(inkclock_simulator ${SOURCE_FILES})

# 链接库（如果需要）
target_link_libraries(inkclock_simulator
  # 这里可以添加需要的库
)

# 设置输出目录
set_target_properties(inkclock_simulator PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

# 复制模拟显示驱动头文件到输出目录
add_custom_command(TARGET inkclock_simulator POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/peripherals/simulator_display.h
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

message(STATUS "========================================")
message(STATUS "InkClock Simulator Configuration")
message(STATUS "========================================")
message(STATUS "Source files: ${SOURCE_FILES}")
message(STATUS "Output directory: ${CMAKE_CURRENT_SOURCE_DIR}/../..")
message(STATUS "========================================")
