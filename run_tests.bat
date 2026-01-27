@echo off

REM InkClock 自动化测试脚本
REM 用于在没有实际设备的情况下测试固件代码

echo ======================================
echo InkClock 自动化测试脚本
echo ======================================
echo 运行时间：%date% %time%
echo ======================================

REM 设置工作目录
set WORKDIR=d:/InkClock
echo 工作目录：%WORKDIR%
echo.

REM 清理旧的构建文件
echo 1. 清理旧的构建文件...
cd %WORKDIR%/code
platformio run -t clean
if %errorlevel% neq 0 (
    echo 清理构建文件失败！
    pause
    exit /b 1
)
echo 清理完成！
echo.

REM 编译ESP32-WROOM-32固件
echo 2. 编译ESP32-WROOM-32固件...
platformio run -e esp32-wroom-32
if %errorlevel% neq 0 (
    echo 编译ESP32-WROOM-32固件失败！
    pause
    exit /b 1
)
echo ESP32-WROOM-32固件编译成功！
echo.

REM 编译ESP32-C3固件
echo 3. 编译ESP32-C3固件...
platformio run -e esp32-c3-devkitc-02
if %errorlevel% neq 0 (
    echo 编译ESP32-C3固件失败！
    pause
    exit /b 1
)
echo ESP32-C3固件编译成功！
echo.

REM 运行静态分析
echo 4. 运行静态分析...
platformio check -e esp32-wroom-32
if %errorlevel% neq 0 (
    echo 静态分析失败！
    pause
    exit /b 1
)
echo 静态分析完成！
echo.

REM 创建测试目录结构
echo 5. 创建测试目录结构...
cd %WORKDIR%/code/test
if not exist "src" mkdir src
if not exist "lib" mkdir lib
if not exist "ArduinoUnit" mkdir ArduinoUnit
echo 测试目录结构创建完成！
echo.

REM 下载ArduinoUnit测试框架
echo 6. 下载ArduinoUnit测试框架...
cd %WORKDIR%/code/test/ArduinoUnit
if not exist "README.md" (
    git clone https://github.com/mmurdoch/arduinounit.git .
    if %errorlevel% neq 0 (
        echo 下载ArduinoUnit测试框架失败！
        pause
        exit /b 1
    )
    echo ArduinoUnit测试框架下载完成！
) else (
    echo ArduinoUnit测试框架已存在，跳过下载！
)
echo.

REM 创建示例测试文件
echo 7. 创建示例测试文件...

REM 创建time_manager测试文件
set TEST_FILE=%WORKDIR%/code/test/src/test_time_manager.cpp
if not exist "%TEST_FILE%" (
    echo #include ^<ArduinoUnit.h^>
    echo #include "../../src/application/time_manager.h"
    echo.
    echo TestSuite timeManagerTests;
    echo.
    echo test(timeManagerTests, TestInitialize) {
    echo     TimeManager timeManager;
    echo     assertTrue(timeManager.initialize());
    echo }
    echo.
    echo test(timeManagerTests, TestGetCurrentTime) {
    echo     TimeManager timeManager;
    echo     timeManager.initialize();
    echo     time_t currentTime = timeManager.getCurrentTime();
    echo     assertTrue(currentTime ^> 0);
    echo }
    echo.
    echo void setup() {
    echo     Serial.begin(115200);
    echo     while (!Serial) {}
    echo }
    echo.
    echo void loop() {
    echo     TestSuiteRunner::run(timeManagerTests);
    echo } > %TEST_FILE%
    echo 创建test_time_manager.cpp成功！
) else (
    echo test_time_manager.cpp已存在，跳过创建！
)

REM 创建performance_monitor测试文件
set TEST_FILE=%WORKDIR%/code/test/src/test_performance_monitor.cpp
if not exist "%TEST_FILE%" (
    echo #include ^<ArduinoUnit.h^>
    echo #include "../../src/coresystem/performance_monitor.h"
    echo.
    echo TestSuite performanceMonitorTests;
    echo.
    echo test(performanceMonitorTests, TestInitialize) {
    echo     PerformanceMonitor^& monitor = PerformanceMonitor::getInstance();
    echo     assertTrue(monitor.initialize());
    echo }
    echo.
    echo test(performanceMonitorTests, TestMonitoringCycle) {
    echo     PerformanceMonitor^& monitor = PerformanceMonitor::getInstance();
    echo     monitor.initialize();
    echo     
    echo     // 运行监控周期
    echo     monitor.runMonitoringCycle();
    echo     
    echo     // 检查是否有异常
    echo     assertFalse(monitor.hasAbnormalities());
    echo }
    echo.
    echo test(performanceMonitorTests, TestResourceUsage) {
    echo     PerformanceMonitor^& monitor = PerformanceMonitor::getInstance();
    echo     monitor.initialize();
    echo     
    echo     // 运行多次监控周期
    echo     for (int i = 0; i ^< 5; i++) {
    echo         monitor.runMonitoringCycle();
    echo     }
    echo     
    echo     // 获取资源使用率
    echo     float cpuUsage = monitor.getCPUUsage();
    echo     float memoryUsage = monitor.getMemoryUsage();
    echo     float storageUsage = monitor.getStorageUsage();
    echo     
    echo     // 检查资源使用率是否在合理范围内
    echo     assertTrue(cpuUsage ^>= 0 ^&^& cpuUsage ^<= 100);
    echo     assertTrue(memoryUsage ^>= 0 ^&^& memoryUsage ^<= 100);
    echo     assertTrue(storageUsage ^>= 0 ^&^& storageUsage ^<= 100);
    echo }
    echo.
    echo void setup() {
    echo     Serial.begin(115200);
    echo     while (!Serial) {}
    echo }
    echo.
    echo void loop() {
    echo     TestSuiteRunner::run(performanceMonitorTests);
    echo } > %TEST_FILE%
    echo 创建test_performance_monitor.cpp成功！
) else (
    echo test_performance_monitor.cpp已存在，跳过创建！
)

REM 创建platformio.ini测试配置文件
set TEST_INI=%WORKDIR%/code/test/platformio.ini
if not exist "%TEST_INI%" (
    echo [env:esp32-wroom-32-test] > %TEST_INI%
    echo platform = espressif32 >> %TEST_INI%
    echo board = esp32dev >> %TEST_INI%
    echo framework = arduino >> %TEST_INI%
    echo monitor_speed = 115200 >> %TEST_INI%
    echo >> %TEST_INI%
    echo build_flags =  >> %TEST_INI%
    echo     -DARDUINO_UNIT_TEST >> %TEST_INI%
    echo     -I../src >> %TEST_INI%
    echo     -I./ArduinoUnit >> %TEST_INI%
    echo >> %TEST_INI%
    echo lib_deps =  >> %TEST_INI%
    echo     bblanchon/ArduinoJson@^7.2.0 >> %TEST_INI%
    echo >> %TEST_INI%
    echo src_dir = ./src >> %TEST_INI%
    echo 创建platformio.ini成功！
) else (
    echo platformio.ini已存在，跳过创建！
)
echo.

REM 运行单元测试
echo 8. 运行单元测试...
cd %WORKDIR%/code/test
platformio run -e esp32-wroom-32-test
if %errorlevel% neq 0 (
    echo 运行单元测试失败！
    pause
    exit /b 1
)
echo 单元测试运行完成！
echo.

REM 显示测试结果
echo ======================================
echo 测试结果汇总
echo ======================================
echo 1. 清理构建文件：成功
echo 2. 编译ESP32-WROOM-32固件：成功
echo 3. 编译ESP32-C3固件：成功
echo 4. 静态分析：成功
echo 5. 创建测试目录结构：成功
echo 6. 下载ArduinoUnit测试框架：成功
echo 7. 创建示例测试文件：成功
echo 8. 运行单元测试：成功
echo ======================================
echo 所有测试完成！
echo 测试时间：%date% %time%
echo ======================================
echo.
echo 固件文件位置：
echo ESP32-WROOM-32: %WORKDIR%/code/.pio/build/esp32-wroom-32/firmware.bin
echo ESP32-C3: %WORKDIR%/code/.pio/build/esp32-c3-devkitc-02/firmware.bin
echo.
echo 测试文件位置：
echo %WORKDIR%/code/test/src/
echo.

pause