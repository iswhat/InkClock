@echo off

set "VCVARS_BAT=C:\Program Files (x86)\Microsoft Visual Studio\2019\BuildTools\VC\Auxiliary\Build\vcvars64.bat"
set "SRC_DIR=d:\InkClock\code\src"
set "OUTPUT_DIR=d:\InkClock\code"

REM 设置Visual Studio环境
call "%VCVARS_BAT"

REM 编译PC模拟器
cl.exe /EHsc /I"%SRC_DIR%\simulator" /I"%SRC_DIR%\coresystem" /I"%SRC_DIR%\application" /I"%SRC_DIR%\drivers\peripherals" "%SRC_DIR%\simulator\pc_simulator.cpp" /Fe"%OUTPUT_DIR%\inkclock_pc_simulator.exe"

REM 检查编译结果
if exist "%OUTPUT_DIR%\inkclock_pc_simulator.exe" (
    echo Compilation successful!
) else (
    echo Compilation failed!
)

pause