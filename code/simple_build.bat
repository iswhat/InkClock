@echo off

REM 简化版构建脚本
echo ========================================
echo Simple Build Script for InkClock Simulator
echo ========================================

REM 检查CMake是否安装
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: CMake is not installed
    pause
    exit /b 1
)

REM 创建构建目录
if not exist "build" mkdir "build"

REM 进入构建目录
cd "build"

REM 运行CMake配置
echo Running CMake...
cmake "src\simulator"
if %errorlevel% neq 0 (
    echo CMake failed
    pause
    exit /b 1
)

REM 编译项目
echo Building simulator...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Build failed
    pause
    exit /b 1
)

REM 返回到代码目录
cd ..

REM 检查结果
if exist "inkclock_simulator.exe" (
    echo Build successful!
    echo Run with: inkclock_simulator.exe
) else (
    echo Build failed - no executable found
)

pause
