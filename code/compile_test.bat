@echo off
REM InkClock 编译测试脚本

echo ====================================
echo InkClock 编译测试
echo ====================================
echo.

REM 设置 Python 路径
set PYTHON=C:\Python314\python.exe

echo 检查 PlatformIO...
%PYTHON% -m platformio --version
if %errorlevel% neq 0 (
    echo PlatformIO 未正确安装！
    pause
    exit /b 1
)
echo PlatformIO 已就绪
echo.

echo ====================================
echo 开始编译 ESP32-WROOM-32 固件...
echo ====================================
%PYTHON% -m platformio run -e esp32-wroom-32
if %errorlevel% neq 0 (
    echo 编译 ESP32-WROOM-32 失败！
    pause
    exit /b 1
)
echo 编译成功！
echo.

echo ====================================
echo 开始编译 ESP32-C3 固件...
echo ====================================
%PYTHON% -m platformio run -e esp32-c3-devkitc-02
if %errorlevel% neq 0 (
    echo 编译 ESP32-C3 失败！
    pause
    exit /b 1
)
echo 编译成功！
echo.

echo ====================================
echo 所有编译完成！
echo ====================================
pause
