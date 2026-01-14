@echo off

echo ========================================
echo InkClock Simple Simulator Build Script
echo ========================================

REM 检查CMake是否安装
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: CMake is not installed or not in PATH
    echo Please install CMake 3.10 or later
    pause
    exit /b 1
)

REM 创建构建目录
if not exist "build" (
    echo Creating build directory...
    mkdir build
    if %errorlevel% neq 0 (
        echo Error: Failed to create build directory
        pause
        exit /b 1
    )
)

REM 进入构建目录
cd build

REM 运行CMake配置
echo Running CMake configuration...
cmake ..\src\simulator
if %errorlevel% neq 0 (
    echo Error: CMake configuration failed
    pause
    exit /b 1
)

REM 编译项目
echo Building simulator...
cmake --build . --config Release
if %errorlevel% neq 0 (
    echo Error: Build failed
    pause
    exit /b 1
)

REM 返回到代码目录
cd ..

REM 检查编译结果
if exist "inkclock_simulator.exe" (
    echo ========================================
    echo Build successful!
    echo ========================================
    echo Simulator executable created: inkclock_simulator.exe
    echo Run the simulator with: inkclock_simulator.exe
    echo ========================================
) else (
    echo Error: Simulator executable not found
    pause
    exit /b 1
)

REM 询问是否运行模拟器
echo.
echo Would you like to run the simulator now? (Y/N)
set /p RUN_SIMULATOR=

if /i "%RUN_SIMULATOR%" equ "Y" (
    echo Running simulator...
    echo ========================================
    inkclock_simulator.exe
) else (
    echo Simulator build completed successfully!
)

pause