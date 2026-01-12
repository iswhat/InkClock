@echo off

set "CURRENT_DIR=%~dp0"
set "BUILD_DIR=%CURRENT_DIR%build"
set "SIMULATOR_DIR=%CURRENT_DIR%src\simulator"

 echo ========================================
 echo InkClock Simulator Build Script
 echo ========================================

echo Current directory: %CURRENT_DIR%
echo Build directory: %BUILD_DIR%
echo Simulator directory: %SIMULATOR_DIR%

REM 检查CMake是否安装
cmake --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: CMake is not installed or not in PATH
    echo Please install CMake 3.10 or later
    pause
    exit /b 1
)

REM 创建构建目录
if not exist "%BUILD_DIR%" (
    echo Creating build directory...
    mkdir "%BUILD_DIR"
    if %errorlevel% neq 0 (
        echo Error: Failed to create build directory
        pause
        exit /b 1
    )
)

REM 进入构建目录
cd "%BUILD_DIR%"

REM 运行CMake配置
echo Running CMake configuration...
cmake "%SIMULATOR_DIR%"
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
cd "%CURRENT_DIR%"

REM 检查编译结果
if exist "inkclock_simulator.exe" (
    echo ========================================
    echo Build successful!
    echo ========================================
    echo Simulator executable created: inkclock_simulator.exe
    echo Run the simulator with: inkclock_simulator.exe
    echo ========================================
    echo Preview files will be generated:
    echo - inkclock_preview.html
    echo - inkclock_preview.svg
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
