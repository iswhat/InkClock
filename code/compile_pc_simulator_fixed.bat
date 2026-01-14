@echo off

set "VCVARS_BAT=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set "SRC_DIR=d:\InkClock\code\src"
set "OUTPUT_DIR=d:\InkClock\code"

 echo ========================================
 echo Compiling InkClock PC Simulator
 echo ========================================

REM 设置Visual Studio环境
if exist "%VCVARS_BAT%" (
    echo Setting up Visual Studio environment...
    call "%VCVARS_BAT"
) else (
    echo Error: Visual Studio environment script not found
    goto end
)

REM 编译PC模拟器
echo Compiling PC Simulator...
cl.exe /EHsc /I"%SRC_DIR%\simulator" /I"%SRC_DIR%\coresystem" /I"%SRC_DIR%\application" /I"%SRC_DIR%\drivers\peripherals" "%SRC_DIR%\simulator\pc_simulator.cpp" /Fe"%OUTPUT_DIR%\inkclock_pc_simulator.exe"

REM 检查编译结果
if exist "%OUTPUT_DIR%\inkclock_pc_simulator.exe" (
    echo ========================================
    echo Compilation successful!
    echo PC Simulator executable created at:
    echo %OUTPUT_DIR%\inkclock_pc_simulator.exe
    echo ========================================
) else (
    echo ========================================
    echo Compilation failed!
    echo ========================================
    goto end
)

:end
echo ========================================
echo Compilation process completed.
echo ========================================
pause