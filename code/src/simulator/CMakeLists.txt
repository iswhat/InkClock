cmake_minimum_required(VERSION 3.10)

project(InkClockSimulator)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 选项配置
option(USE_SDL2 "Use SDL2 for real-time display" OFF)

# 设置编译选项
if(MSVC)
    # MSVC 编译器选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4")
else()
    # GCC/Clang 编译器选项
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wpedantic")
endif()

# 包含头文件目录
include_directories(
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/../coresystem
  ${CMAKE_CURRENT_SOURCE_DIR}/../application
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/peripherals
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers
  ${CMAKE_CURRENT_SOURCE_DIR}/../plugins
)

# 收集源文件
set(SOURCE_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/simulator_main.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../coresystem/arduino_compat.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../coresystem/dependency_injection.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../coresystem/plugin_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../coresystem/network_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../drivers/peripherals/simulator_display.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../coresystem/core_system.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../application/display_manager.cpp
  ${CMAKE_CURRENT_SOURCE_DIR}/../plugins/example_plugin.cpp
)

# 创建可执行文件
add_executable(inkclock_simulator ${SOURCE_FILES})

# 链接库（如果需要）
target_link_libraries(inkclock_simulator
  # 这里可以添加需要的库
)

# SDL2 支持
if(USE_SDL2)
    find_package(SDL2 REQUIRED)
    if(SDL2_FOUND)
        include_directories(${SDL2_INCLUDE_DIRS})
        target_link_libraries(inkclock_simulator ${SDL2_LIBRARIES})
        target_compile_definitions(inkclock_simulator PRIVATE USE_SDL2)
        message(STATUS "SDL2 found: ${SDL2_LIBRARIES}")
    else()
        message(WARNING "SDL2 not found, falling back to basic simulator")
        set(USE_SDL2 OFF)
    endif()
endif()

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

# 配置信息
message(STATUS "=======================================")
message(STATUS "InkClock Simulator Configuration")
message(STATUS "=======================================")
message(STATUS "Source files: ${SOURCE_FILES}")
message(STATUS "Output directory: ${CMAKE_CURRENT_SOURCE_DIR}/../..")
message(STATUS "Use SDL2: ${USE_SDL2}")
if(USE_SDL2 AND SDL2_FOUND)
    message(STATUS "SDL2 libraries: ${SDL2_LIBRARIES}")
endif()
message(STATUS "=======================================")

# 构建指令提示
message(STATUS "To build with SDL2 support, run:")
message(STATUS "  cmake -DUSE_SDL2=ON -B build -S code/src/simulator")
message(STATUS "  cmake --build build")
message(STATUS "")
message(STATUS "To build without SDL2, run:")
message(STATUS "  cmake -B build -S code/src/simulator")
message(STATUS "  cmake --build build")
message(STATUS "=======================================")
