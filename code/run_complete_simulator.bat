@echo off

REM InkClock 完整模拟器启动脚本
REM 这个脚本会编译并运行简化的完整模拟器

setlocal

echo =========================================
echo InkClock Complete Simulator Launcher
echo =========================================
echo 

REM 检查是否存在编译好的模拟器
if exist "src\simulator_simple_complete\inkclock_simulator.exe" (
    echo 发现已编译的模拟器，直接运行...
    goto run_simulator
)

echo 未发现编译好的模拟器，开始编译...
echo 

REM 检查是否有Visual Studio编译器
set "VSWHERE=%ProgramFiles(x86)%\Microsoft Visual Studio\Installer\vswhere.exe"
if exist "%VSWHERE%" (
    for /f "usebackq tokens=*" %%i in (`"%VSWHERE%" -latest -property installationPath`) do set "VS_PATH=%%i"
    
    if exist "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat" (
        echo 发现Visual Studio，设置编译环境...
        call "%VS_PATH%\VC\Auxiliary\Build\vcvars64.bat"
        
        echo 编译模拟器...
        cl.exe "src\simulator_simple_complete\main.cpp" /Fe"src\simulator_simple_complete\inkclock_simulator.exe" /EHsc
        
        if %errorlevel% equ 0 (
            echo 编译成功！
            goto run_simulator
        ) else (
            echo 编译失败，尝试使用MinGW...
            goto try_mingw
        )
    )
)

try_mingw:

REM 检查是否有MinGW编译器
echo 检查MinGW编译器...
where g++ >nul 2>nul
if %errorlevel% equ 0 (
    echo 发现MinGW，编译模拟器...
    g++ "src\simulator_simple_complete\main.cpp" -o "src\simulator_simple_complete\inkclock_simulator.exe"
    
    if %errorlevel% equ 0 (
        echo 编译成功！
        goto run_simulator
    ) else (
        echo 编译失败，请安装C++编译器...
        goto install_compiler
    )
)

install_compiler:
echo 
echo =========================================
echo 编译器安装指南
echo =========================================
echo 1. 安装 Visual Studio Community 2019 或更高版本
echo    下载地址：https://visualstudio.microsoft.com/zh-hans/downloads/
echo    安装时选择 "使用C++的桌面开发"
echo
echo 2. 或安装 MinGW-w64
echo    下载地址：https://www.mingw-w64.org/downloads/
echo    安装后将bin目录添加到系统环境变量
echo
echo 安装完成后，重新运行此脚本。
echo =========================================
echo 
goto end

run_simulator:
echo 
echo 启动模拟器...
echo =========================================
"src\simulator_simple_complete\inkclock_simulator.exe"

end:
echo 
echo 模拟器退出。
echo =========================================
pause
endlocal
