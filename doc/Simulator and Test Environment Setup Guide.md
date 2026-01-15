# InkClock Simulator and Test Environment Setup Guide

[中文版本](模拟器和测试环境设置指南.md)

## Overview

This guide provides a complete setup method for previewing the InkClock firmware interface and testing functions on a PC. We offer the following simulation solutions:

1. **PC Simulator**: C++-based local simulator, requires CMake and compiler

## 1. PC Simulator

### Features
- ✅ Closer to actual firmware runtime environment
- ✅ Supports exporting HTML and SVG preview files
- ✅ Command-line interface control

### System Requirements
- **CMake 3.10+**: For build system
- **C++ Compiler**: Supports C++11 or higher
- **PlatformIO** (optional): For firmware compilation

### Installing Dependencies

#### Windows
1. Download and install [CMake](https://cmake.org/download/)
2. Install [Visual Studio](https://visualstudio.microsoft.com/) or [MinGW](https://www.mingw-w64.org/)
3. (Optional) Install [PlatformIO](https://platformio.org/install)

#### macOS
1. Install using Homebrew: `brew install cmake`
2. Install Xcode command line tools: `xcode-select --install`
3. (Optional) Install PlatformIO: `pip install platformio`

#### Linux
1. Install using package manager: `sudo apt-get install cmake build-essential`
2. (Optional) Install PlatformIO: `pip install platformio`

### Building and Running

1. Navigate to the `code/` directory
2. Run the build script: `build_simulator.bat`
3. After build completion, run the simulator: `inkclock_simulator.exe`

### Usage Instructions

After launching the simulator, you can use the command-line menu:

1. Update display
2. Show splash screen
3. Switch clock mode
4. Switch to calendar page
5. Switch to stock page
6. Switch to message page
7. Export display content
8. Exit

### Output Files

The simulator generates the following preview files:
- `inkclock_preview.html`: HTML format display preview
- `inkclock_preview.svg`: SVG format display preview

## 2. Firmware Compilation (Optional)

If you have PlatformIO installed, you can compile the actual firmware:

1. Navigate to the `code/` directory
2. Run the automatic build script: `auto_build_run.bat`
3. The script will compile firmware for multiple platforms and run the simulator

### Supported Platforms

- ESP32 series (esp32-c3-devkitc-02, esp32-s3-devkitc-1, esp32-c6-n4, esp32-s2-devkitc-1, esp32-wroom-32, esp32-c3-supermini)
- ESP8266 series (nodemcuv2)
- NRF52 series (nrf52840dk)
- STM32 series (bluepill_f103c8)
- RP2040 series (raspberrypi_pico)

## 3. Automatic Run Process

### Quick Start Guide

1. **Advanced Solution**: Using PC Simulator
   - Install CMake and compiler
   - Run `build_simulator.bat`
   - Run `inkclock_simulator.exe`

2. **Complete Solution**: Compile and Simulate
   - Install PlatformIO and CMake
   - Run `auto_build_run.bat`
   - Automatically compile firmware and run simulator

## 4. Troubleshooting

### PC Simulator Issues

**Problem**: CMake configuration failed
**Solution**:
- Ensure CMake is correctly installed
- Ensure CMake version is 3.10 or higher
- Check system environment variable settings

**Problem**: Compilation failed
**Solution**:
- Ensure C++ compiler is correctly installed
- Check for syntax errors in code
- Try updating CMake and compiler to latest versions

### Firmware Compilation Issues

**Problem**: PlatformIO command not found
**Solution**:
- Ensure PlatformIO is correctly installed
- Check system environment variable settings
- Try reinstalling PlatformIO

**Problem**: Dependency library installation failed
**Solution**:
- Ensure network connection is normal
- Try running `pio lib update` to update libraries
- Check dependency configuration in `platformio.ini` file

## 5. Testing Best Practices

### Function Testing

1. **Page Switching Test**: Test switching between all pages (calendar, stock, message)
2. **Clock Mode Test**: Switch between digital/analog clock modes
3. **Battery Display Test**: Test display effects at different battery levels
4. **Weather Display Test**: Test display for different weather conditions
5. **Interface Response Test**: Test response speed of control operations

### Performance Testing

1. **Startup Time**: Measure time from simulator startup to display readiness
2. **Page Switching Speed**: Measure response time for page switching
3. **Display Update Speed**: Measure processing time for display updates

### Compatibility Testing

1. **Operating System Compatibility**: Test in different operating systems

## 6. Technical Support

### Frequently Asked Questions

**Q: Can I test all functions without hardware?**
A: Yes, the PC simulator provides complete interface preview and function testing.

**Q: Is there a difference between the PC simulator and actual hardware display effects?**
A: The PC simulator provides a visual approximation, actual hardware display may have slight differences, especially in color and refresh rate.

**Q: Can I test sensor data and network functions?**
A: The PC simulator provides simulated sensor data and network status, you can test interface display effects, but cannot test actual network connections.

**Q: How to apply settings from the simulator to actual hardware?**
A: The simulator is mainly used for preview and testing, actual hardware configuration needs to be done through firmware settings or web interface.

### Contact Support

If you encounter any issues, please refer to the following resources:

- Check the `README.md` file for more information
- View code comments to understand implementation details
- Contact technical support for further assistance

## 7. Conclusion

The PC simulator is the best choice for previewing the InkClock firmware interface on a PC, as it is closer to the actual hardware runtime environment. If you need a complete test environment, you can set up a firmware compilation environment.

Regardless of which method you choose, this guide provides detailed setup and usage instructions to help you quickly start testing and development work.

---

**Version**: 1.2
**Update Date**: 2026-01-14
**Supported Platforms**: Windows, macOS, Linux