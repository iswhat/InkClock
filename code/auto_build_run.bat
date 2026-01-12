@echo off

set "CURRENT_DIR=%~dp0"
set "BUILD_DIR=%CURRENT_DIR%build"
set "SIMULATOR_DIR=%CURRENT_DIR%src\simulator"

 echo ========================================
 echo InkClock Auto Build and Run Script
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

REM 检查PlatformIO是否安装
pio --version >nul 2>&1
if %errorlevel% neq 0 (
    echo Warning: PlatformIO is not installed or not in PATH
    echo Firmware compilation will be skipped
    set "SKIP_FIRMWARE=1"
) else (
    set "SKIP_FIRMWARE=0"
)

REM 步骤1: 编译固件（如果PlatformIO可用）
if %SKIP_FIRMWARE% equ 0 (
    echo ========================================
    echo Step 1: Compiling firmware...
    echo ========================================
    
    REM 编译ESP32固件
    pio run --environment esp32-wroom-32
    if %errorlevel% neq 0 (
        echo Warning: Failed to compile ESP32 firmware
        echo Continuing with simulator build...
    ) else (
        echo ESP32 firmware compiled successfully!
    )
    
    REM 编译ESP8266固件
    pio run --environment nodemcuv2
    if %errorlevel% neq 0 (
        echo Warning: Failed to compile ESP8266 firmware
        echo Continuing with simulator build...
    ) else (
        echo ESP8266 firmware compiled successfully!
    )
) else (
    echo Skipping firmware compilation (PlatformIO not available)
)

REM 步骤2: 编译模拟器
echo ========================================
echo Step 2: Compiling simulator...
echo ========================================

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
if not exist "inkclock_simulator.exe" (
    echo Error: Simulator executable not found
    pause
    exit /b 1
)

REM 步骤3: 运行模拟器
echo ========================================
echo Step 3: Running simulator...
echo ========================================

echo Running InkClock Simulator...
echo ========================================
echo The simulator will generate preview files:
echo - inkclock_preview.html
echo - inkclock_preview.svg
echo ========================================
echo Press any key to continue...
pause >nul

REM 运行模拟器
inkclock_simulator.exe
if %errorlevel% neq 0 (
    echo Warning: Simulator exited with error code %errorlevel%
    echo However, preview files may still have been generated
) else (
    echo Simulator ran successfully!
)

REM 步骤4: 检查预览文件
echo ========================================
echo Step 4: Checking preview files...
echo ========================================

if exist "inkclock_preview.html" (
    echo ✓ inkclock_preview.html generated successfully
    echo You can open this file in your browser to preview the display
) else (
    echo ✗ inkclock_preview.html not found
)

if exist "inkclock_preview.svg" (
    echo ✓ inkclock_preview.svg generated successfully
    echo You can open this file in an SVG viewer to preview the display
) else (
    echo ✗ inkclock_preview.svg not found
)

REM 步骤5: 清理（可选）
echo ========================================
echo Step 5: Cleanup options
echo ========================================
echo Would you like to clean up the build files? (Y/N)
set /p CLEANUP=

if /i "%CLEANUP%" equ "Y" (
    echo Cleaning up build files...
    if exist "%BUILD_DIR%" (
        rd /s /q "%BUILD_DIR%"
        echo Build directory cleaned
    )
    if exist "inkclock_simulator.exe" (
        del "inkclock_simulator.exe"
        echo Simulator executable removed
    )
    echo Cleanup completed
)

REM 完成
echo ========================================
echo Auto Build and Run Process Complete!
echo ========================================
echo Summary:
echo - Firmware compilation: %SKIP_FIRMWARE% (0=completed, 1=skipped)
echo - Simulator compilation: Completed
echo - Simulator execution: Completed
echo - Preview files: Generated

echo ========================================
echo To run the simulator again, use:
echo   inkclock_simulator.exe
echo ========================================
pause
