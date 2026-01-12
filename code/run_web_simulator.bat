@echo off

echo ========================================
echo InkClock Web Simulator Launcher
echo ========================================
echo.
echo This script launches the web-based simulator
echo for previewing InkClock firmware interface.
echo.
echo Features:
echo - Web-based interface preview
echo - Interactive testing controls
echo - No dependencies required
echo - Cross-platform compatible
echo.
echo ========================================
echo Launching Web Simulator...
echo ========================================

REM 启动Web模拟器
start "InkClock Web Simulator" "%~dp0src\simulator\web_simulator.html"

if %errorlevel% equ 0 (
    echo.
    echo ✓ Web Simulator launched successfully!
    echo.
    echo The simulator is now open in your default browser.
    echo You can use the control panel to test different features:
    echo - Switch between calendar, stock, and message pages
    echo - Toggle clock mode (digital/analog)
    echo - Adjust battery level
    echo - Change weather conditions
    echo - Update and reset display
    echo.
) else (
    echo.
    echo ✗ Failed to launch Web Simulator
    echo Please check if the file exists:
    echo %~dp0src\simulator\web_simulator.html
    echo.
)

echo ========================================
echo InkClock Web Simulator
 echo ========================================
echo.
echo To run the simulator again, double-click this batch file.
echo.
pause