@echo off

REM InkClock Simulator Build Script
REM This script builds the InkClock simulator using CMake

setlocal enabledelayedexpansion

REM Check if CMake is installed
where cmake >nul 2>&1
if %errorlevel% neq 0 (
    echo Error: CMake is not installed. Please install CMake 3.16 or later.
    echo You can download CMake from: https://cmake.org/download/
    pause
    exit /b 1
)

REM Create build directory if it doesn't exist
if not exist "build" (
    mkdir build
)

REM Change to build directory
cd build

REM Run CMake to configure the project
echo Configuring project with CMake...
cmake .. -G "Visual Studio 17 2022" -A Win32

if %errorlevel% neq 0 (
    echo Error: CMake configuration failed.
    pause
    exit /b 1
)

REM Build the project
echo Building project...
cmake --build . --config Release

if %errorlevel% neq 0 (
    echo Error: Build failed.
    pause
    exit /b 1
)

REM Run the simulator
echo Running InkClock Simulator...
Release\inkclock_simulator.exe

endlocal