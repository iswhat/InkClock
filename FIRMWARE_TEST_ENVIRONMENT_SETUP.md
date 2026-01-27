# InkClock 固件测试环境部署指南

## 概述

本指南提供了在没有实际设备的情况下测试InkClock真实固件的完整方案，包括：
- 解决库依赖问题
- 编写和运行单元测试
- 使用模拟对象测试外部依赖
- 在不同平台上测试固件
- 使用性能监控器测试固件性能

## 1. 环境准备

### 1.1 已安装的软件

- Python 3.14.0
- PlatformIO 6.1.18
- CMake 3.16+（用于构建C++模拟器）

### 1.2 解决库依赖问题

PlatformIO无法找到指定版本的库时，尝试以下解决方案：

```bash
# 进入代码目录
cd d:/InkClock/code

# 清理库缓存
platformio lib uninstall --all

# 重新安装库
platformio lib install

# 或手动指定库版本
platformio lib install adafruit/Adafruit GFX Library
platformio lib install Bodmer/TFT_eSPI
platformio lib install bblanchon/ArduinoJson
```

## 2. 固件编译测试

### 2.1 编译ESP32-WROOM-32固件

```bash
cd d:/InkClock/code
platformio run -e esp32-wroom-32
```

### 2.2 编译其他平台固件

```bash
# ESP32-C3
platformio run -e esp32-c3-devkitc-02

# ESP8266
platformio run -e nodemcuv2

# STM32
platformio run -e bluepill_f103c8

# NRF52
platformio run -e nrf52840dk

# RP2040
platformio run -e raspberrypi_pico
```

### 2.3 查看编译输出

编译成功后，固件文件将生成在：
```
d:/InkClock/code/.pio/build/esp32-wroom-32/firmware.bin
```

## 3. 单元测试框架

### 3.1 创建测试目录结构

```bash
cd d:/InkClock/code/test
mkdir -p src lib ArduinoUnit
```

### 3.2 下载ArduinoUnit测试框架

```bash
cd d:/InkClock/code/test/ArduinoUnit
git clone https://github.com/mmurdoch/arduinounit.git .
```

### 3.3 编写示例单元测试

创建 `d:/InkClock/code/test/src/test_time_manager.cpp` 文件：

```cpp
#include <ArduinoUnit.h>
#include "../../src/application/time_manager.h"

TestSuite timeManagerTests;

test(timeManagerTests, TestInitialize) {
    TimeManager timeManager;
    assertTrue(timeManager.initialize());
}

test(timeManagerTests, TestGetCurrentTime) {
    TimeManager timeManager;
    timeManager.initialize();
    time_t currentTime = timeManager.getCurrentTime();
    assertTrue(currentTime > 0);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
}

void loop() {
    TestSuiteRunner::run(timeManagerTests);
}
```

### 3.4 创建测试配置文件

创建 `d:/InkClock/code/test/platformio.ini` 文件：

```ini
[env:esp32-wroom-32-test]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

build_flags = 
    -DARDUINO_UNIT_TEST
    -I../src
    -I./ArduinoUnit

lib_deps = 
    bblanchon/ArduinoJson@^7.2.0

src_dir = ./src
```

### 3.5 运行单元测试

```bash
cd d:/InkClock/code/test
platformio run -e esp32-wroom-32-test
```

## 4. 模拟测试框架

### 4.1 使用MockNetwork测试网络功能

创建 `d:/InkClock/code/test/src/test_web_client.cpp` 文件：

```cpp
#include <ArduinoUnit.h>
#include "../../src/application/web_client.h"
#include "../../src/drivers/peripherals/mock_network.h"

TestSuite webClientTests;

test(webClientTests, TestNetworkConnection) {
    // 创建模拟网络对象
    MockNetwork mockNetwork;
    mockNetwork.setMockResponse(200, "{\"success\": true}");
    
    // 创建WebClient并注入模拟网络
    WebClient webClient(&mockNetwork);
    
    // 测试网络连接
    assertTrue(webClient.connect());
}

test(webClientTests, TestHttpGet) {
    // 创建模拟网络对象
    MockNetwork mockNetwork;
    mockNetwork.setMockResponse(200, "{\"data\": \"test\"}");
    
    // 创建WebClient并注入模拟网络
    WebClient webClient(&mockNetwork);
    webClient.connect();
    
    // 测试HTTP GET请求
    String response = webClient.get("https://example.com/test");
    assertTrue(response.length() > 0);
    assertTrue(response.indexOf("test") != -1);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
}

void loop() {
    TestSuiteRunner::run(webClientTests);
}
```

### 4.2 使用性能监控器测试固件性能

创建 `d:/InkClock/code/test/src/test_performance_monitor.cpp` 文件：

```cpp
#include <ArduinoUnit.h>
#include "../../src/coresystem/performance_monitor.h"

TestSuite performanceMonitorTests;

test(performanceMonitorTests, TestInitialize) {
    PerformanceMonitor& monitor = PerformanceMonitor::getInstance();
    assertTrue(monitor.initialize());
}

test(performanceMonitorTests, TestMonitoringCycle) {
    PerformanceMonitor& monitor = PerformanceMonitor::getInstance();
    monitor.initialize();
    
    // 运行监控周期
    monitor.runMonitoringCycle();
    
    // 检查是否有异常
    assertFalse(monitor.hasAbnormalities());
}

test(performanceMonitorTests, TestResourceUsage) {
    PerformanceMonitor& monitor = PerformanceMonitor::getInstance();
    monitor.initialize();
    
    // 运行多次监控周期
    for (int i = 0; i < 5; i++) {
        monitor.runMonitoringCycle();
    }
    
    // 获取资源使用率
    float cpuUsage = monitor.getCPUUsage();
    float memoryUsage = monitor.getMemoryUsage();
    float storageUsage = monitor.getStorageUsage();
    
    // 检查资源使用率是否在合理范围内
    assertTrue(cpuUsage >= 0 && cpuUsage <= 100);
    assertTrue(memoryUsage >= 0 && memoryUsage <= 100);
    assertTrue(storageUsage >= 0 && storageUsage <= 100);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
}

void loop() {
    TestSuiteRunner::run(performanceMonitorTests);
}
```

## 5. 跨平台测试

### 5.1 测试不同平台的固件

```bash
# ESP32-C3
platformio run -e esp32-c3-devkitc-02

# ESP8266
platformio run -e nodemcuv2

# STM32
platformio run -e bluepill_f103c8

# NRF52
platformio run -e nrf52840dk

# RP2040
platformio run -e raspberrypi_pico
```

### 5.2 使用平台抽象层测试跨平台兼容性

创建 `d:/InkClock/code/test/src/test_platform_abstraction.cpp` 文件：

```cpp
#include <ArduinoUnit.h>
#include "../../src/coresystem/platform_abstraction.h"

TestSuite platformAbstractionTests;

test(platformAbstractionTests, TestGetPlatformType) {
    PlatformType platformType = getPlatformType();
    assertTrue(platformType != PlatformType::PLATFORM_UNKNOWN);
}

test(platformAbstractionTests, TestGetFreeHeap) {
    size_t freeHeap = platformGetFreeHeap();
    assertTrue(freeHeap > 0);
}

test(platformAbstractionTests, TestGetChipId) {
    String chipId = platformGetChipId();
    assertTrue(chipId.length() > 0);
}

test(platformAbstractionTests, TestGetFlashSize) {
    size_t flashSize = platformGetFlashSize();
    assertTrue(flashSize > 0);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
}

void loop() {
    TestSuiteRunner::run(platformAbstractionTests);
}
```

## 6. 固件静态分析

### 6.1 使用PlatformIO的静态分析工具

```bash
cd d:/InkClock/code
platformio check -e esp32-wroom-32
```

### 6.2 查看分析结果

静态分析会检查：
- 代码质量问题
- 潜在的安全漏洞
- 性能问题
- 代码风格问题

## 7. 固件调试

### 7.1 使用PlatformIO的调试功能

```bash
cd d:/InkClock/code
platformio debug -e esp32-wroom-32
```

### 7.2 使用串行监视器查看调试输出

```bash
cd d:/InkClock/code
platformio device monitor -e esp32-wroom-32
```

## 8. 测试自动化

### 8.1 创建自动化测试脚本

创建 `d:/InkClock/run_tests.bat` 文件：

```batch
@echo off

cd d:/InkClock/code

REM 清理构建文件
platformio run -t clean

REM 编译固件
platformio run -e esp32-wroom-32
platformio run -e esp32-c3-devkitc-02
platformio run -e nodemcuv2

REM 运行静态分析
platformio check -e esp32-wroom-32

REM 运行单元测试
cd test
platformio run -e esp32-wroom-32-test

cd ..
echo 测试完成！
pause
```

### 8.2 运行自动化测试

```bash
cd d:/InkClock
run_tests.bat
```

## 9. 常见问题解决

### 9.1 库依赖问题

```bash
# 清理库缓存
platformio lib uninstall --all

# 重新安装库
platformio lib install

# 或手动指定库版本
platformio lib install adafruit/Adafruit GFX Library@^1.11.9
```

### 9.2 编译错误

```bash
# 查看详细的编译输出
platformio run -e esp32-wroom-32 -v

# 检查代码语法错误
platformio check -e esp32-wroom-32
```

### 9.3 测试失败

```bash
# 查看测试输出
platformio device monitor -e esp32-wroom-32-test

# 检查测试代码
cat d:/InkClock/code/test/src/test_*.cpp
```

## 10. 结论

通过本指南，您可以在没有实际设备的情况下测试InkClock真实固件的运行效果，包括：
- 编译测试：检查固件是否可以在不同平台上编译成功
- 单元测试：测试单个函数或类的功能
- 集成测试：测试多个模块的协作
- 模拟测试：使用模拟对象测试外部依赖
- 性能测试：使用性能监控器测试固件性能
- 跨平台测试：测试固件在不同平台上的兼容性
- 静态分析：检查代码质量和潜在问题

这些测试方法可以帮助您发现真实固件中的bug，提高固件的质量和稳定性。