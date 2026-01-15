# InkClock 模拟器和测试环境设置指南

## 概述

本指南提供了在PC上预览InkClock固件界面和测试功能的完整设置方法。我们提供了以下模拟方案：

1. **PC 模拟器**：基于C++的本地模拟器，需要CMake和编译器

## 1. PC 模拟器

### 特点
- ✅ 更接近实际固件运行环境
- ✅ 支持导出HTML和SVG预览文件
- ✅ 命令行界面控制

### 系统要求
- **CMake 3.10+**：用于构建系统
- **C++ 编译器**：支持C++11或更高版本
- **PlatformIO**（可选）：用于固件编译

### 安装依赖

#### Windows
1. 下载并安装 [CMake](https://cmake.org/download/)
2. 安装 [Visual Studio](https://visualstudio.microsoft.com/) 或 [MinGW](https://www.mingw-w64.org/)
3. （可选）安装 [PlatformIO](https://platformio.org/install)

#### macOS
1. 使用Homebrew安装：`brew install cmake`
2. 安装Xcode命令行工具：`xcode-select --install`
3. （可选）安装PlatformIO：`pip install platformio`

#### Linux
1. 使用包管理器安装：`sudo apt-get install cmake build-essential`
2. （可选）安装PlatformIO：`pip install platformio`

### 构建和运行

1. 导航到 `code/` 目录
2. 运行构建脚本：`build_simulator.bat`
3. 构建完成后，运行模拟器：`inkclock_simulator.exe`

### 使用说明

模拟器启动后，您可以使用命令行菜单：

1. 更新显示
2. 显示启动画面
3. 切换时钟模式
4. 切换到日历页面
5. 切换到股票页面
6. 切换到消息页面
7. 导出显示内容
8. 退出

### 输出文件

模拟器会生成以下预览文件：
- `inkclock_preview.html`：HTML格式的显示预览
- `inkclock_preview.svg`：SVG格式的显示预览

## 2. 固件编译（可选）

如果安装了PlatformIO，您可以编译实际的固件：

1. 导航到 `code/` 目录
2. 运行自动构建脚本：`auto_build_run.bat`
3. 脚本会编译多个平台的固件并运行模拟器

### 支持的平台

- ESP32系列（esp32-c3-devkitc-02, esp32-s3-devkitc-1, esp32-c6-n4, esp32-s2-devkitc-1, esp32-wroom-32, esp32-c3-supermini）
- ESP8266系列（nodemcuv2）
- NRF52系列（nrf52840dk）
- STM32系列（bluepill_f103c8）
- RP2040系列（raspberrypi_pico）

## 3. 自动运行流程

### 快速启动指南

1. **高级方案**：使用PC模拟器
   - 安装CMake和编译器
   - 运行 `build_simulator.bat`
   - 运行 `inkclock_simulator.exe`

2. **完整方案**：编译和模拟
   - 安装PlatformIO和CMake
   - 运行 `auto_build_run.bat`
   - 自动编译固件并运行模拟器

## 4. 故障排除

### PC模拟器问题

**问题**：CMake配置失败
**解决方案**：
- 确保CMake已正确安装
- 确保CMake版本为3.10或更高
- 检查系统环境变量设置

**问题**：编译失败
**解决方案**：
- 确保C++编译器已正确安装
- 检查代码中是否有语法错误
- 尝试更新CMake和编译器到最新版本

### 固件编译问题

**问题**：PlatformIO命令未找到
**解决方案**：
- 确保PlatformIO已正确安装
- 检查系统环境变量设置
- 尝试重新安装PlatformIO

**问题**：依赖库安装失败
**解决方案**：
- 确保网络连接正常
- 尝试运行 `pio lib update` 更新库
- 检查 `platformio.ini` 文件中的依赖配置

## 5. 测试最佳实践

### 功能测试

1. **页面切换测试**：测试所有页面（日历、股票、消息）的切换
2. **时钟模式测试**：切换数字/模拟时钟模式
3. **电池显示测试**：测试不同电量级别的显示效果
4. **天气显示测试**：测试不同天气条件的显示
5. **界面响应测试**：测试控制操作的响应速度

### 性能测试

1. **启动时间**：测量模拟器启动到显示就绪的时间
2. **页面切换速度**：测量页面切换的响应时间
3. **显示更新速度**：测量显示更新的处理时间

### 兼容性测试

1. **操作系统兼容性**：在不同操作系统中测试

## 6. 技术支持

### 常见问题

**Q: 我可以在没有硬件的情况下测试所有功能吗？**
A: 是的，PC模拟器提供了完整的界面预览和功能测试。

**Q: PC模拟器和实际硬件的显示效果有区别吗？**
A: PC模拟器提供了视觉上的近似效果，实际硬件的显示可能会有细微差别，特别是在颜色和刷新率方面。

**Q: 我可以测试传感器数据和网络功能吗？**
A: PC模拟器提供了模拟的传感器数据和网络状态，您可以测试界面显示效果，但无法测试实际的网络连接。

**Q: 如何将模拟器中的设置应用到实际硬件？**
A: 模拟器主要用于预览和测试，实际硬件的配置需要通过固件设置或Web界面进行。

### 联系支持

如果您遇到任何问题，请参考以下资源：

- 检查 `README.md` 文件获取更多信息
- 查看代码注释了解实现细节
- 联系技术支持获取进一步帮助

## 7. 结论

PC模拟器是在PC上预览InkClock固件界面的最佳选择，因为它更接近实际硬件的运行环境。如果您需要完整的测试环境，可以设置固件编译环境。

无论您选择哪种方法，本指南都提供了详细的设置和使用说明，帮助您快速开始测试和开发工作。

---

**版本**：1.2
**更新日期**：2026-01-14
**适用平台**：Windows、macOS、Linux