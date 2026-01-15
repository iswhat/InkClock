# InkClock - Intelligent E-Ink Clock System

[中文版本](项目说明文档.md)

## Project Introduction

InkClock is an intelligent e-ink clock system based on low-power microcontrollers, featuring rich functionality and good extensibility. It adopts a modular design, supports multiple sensors and peripherals, can display time, date, weather, air quality, stock quotes and other information, and has low-power optimization and remote update capabilities.

## Features

### Core Features
- 🌍 **Time and Date**: Display Gregorian time, date, day of week
- 🌙 **Lunar Information**: Display lunar date, solar terms, holidays
- 🌡️ **Weather Information**: Real-time weather, temperature, humidity, pressure
- 💨 **Air Quality**: PM2.5, CO2, VOC and other environmental indicators
- 📈 **Stock Quotes**: Real-time stock prices, changes, K-line charts
- 🌞 **Light Sensing**: Automatically adjust screen brightness and refresh rate
- 👤 **Human Sensing**: Automatically turn on the screen when someone is present, enter low-power mode when no one is present
- 🔋 **Battery Management**: Real-time display of battery level, low battery reminder
- 🔄 **Auto Update**: Support OTA remote firmware update

### Extended Features
- 📱 **Mobile Control**: Support Bluetooth/WiFi remote control
- 🎵 **Audio Playback**: Support MP3 playback, voice broadcast
- 📷 **Camera**: Support face recognition, remote monitoring
- 🔔 **Smart Reminder**: Timed reminders, event reminders
- 🌐 **Network Synchronization**: NTP time synchronization, network weather updates
- 📝 **Custom Display**: Support custom display content and layout

## Hardware Support

### Microcontrollers
- ESP32 (recommended)
- ESP8266
- NRF52
- STM32
- RP2040

### E-Ink Displays
- Support multiple sizes of e-ink displays: 2.13", 2.9", 4.2", 5.83", 7.5", etc.
- Support black and white, black and white red, black and white yellow and other color modes

### Sensor Support
- **Temperature and Humidity**: DHT11/22, AM2302, SHT30/31/40, HTU21D, HDC1080
- **Pressure**: BMP180, BMP280, BME280, BME680, LPS25HB
- **Air Quality**: SGP30, MQ series(MQ-2/5/7/135), TGS2600, RE200B
- **Light**: BH1750, TSL2561, GY30, SI1145
- **Human Sensing**: HC-SR501, HC-SR505, RE200B, LD2410
- **Flame Detection**: IR flame sensors

## Software Architecture

### System Architecture

```
┌─────────────────────────────────────────────────────────────────────┐
│                         InkClock System                            │
├─────────┬─────────────┬─────────────┬─────────────┬─────────────────┤
│         │             │             │             │                 │
│  Application Layer  │    Service Layer   │    Driver Layer    │    Core Layer    │    Hardware Abstraction Layer   │
│         │             │             │             │                 │
├─────────┼─────────────┼─────────────┼─────────────┼─────────────────┤
│         │             │             │             │                 │
│ Display │ WiFi        │ Sensor      │ CoreSystem  │ Platform        │
│ Manager │ Manager     │ Drivers     │ EventBus    │ Abstraction     │
│ Power   │ API         │ Display     │ Driver      │                 │
│ Manager │ Manager     │ Drivers     │ Registry    │                 │
│ Weather │ Time        │ Audio       │ Config      │                 │
│ Manager │ Manager     │ Drivers     │             │                 │
│ Stock   │ Lunar       │             │             │                 │
│ Manager │ Manager     │             │             │                 │
│         │ Geo         │             │             │                 │
│         │ Manager     │             │             │                 │
│         │             │             │             │                 │
└─────────┴─────────────┴─────────────┴─────────────┴─────────────────┘
```

### Core Modules

1. **CoreSystem**: System core, responsible for initialization, scheduling and management of various modules
2. **EventBus**: Event bus, implementing decoupled communication between modules
3. **DriverRegistry**: Driver registry, responsible for dynamic detection and management of sensors and peripherals
4. **PlatformAbstraction**: Platform abstraction layer, shielding differences between different hardware platforms
5. **Config**: Configuration management, responsible for loading and saving system configuration

### Application Modules

1. **DisplayManager**: Display management, responsible for initialization, refresh and content update of e-ink displays
2. **PowerManager**: Power management, responsible for battery detection, charging management and low-power optimization
3. **SensorManager**: Sensor management, responsible for data collection and processing of various sensors
4. **WeatherManager**: Weather management, responsible for obtaining and displaying weather information
5. **StockManager**: Stock management, responsible for obtaining and displaying stock quotes
6. **TimeManager**: Time management, responsible for NTP synchronization and time display
7. **LunarManager**: Lunar management, responsible for obtaining and displaying lunar information
8. **WiFiManager**: WiFi management, responsible for network connection and configuration

## Installation and Configuration

### Development Environment

1. **Arduino IDE**: Recommended to use Arduino IDE 2.0 and above
2. **PlatformIO**: Support PlatformIO development environment
3. **Dependency Libraries**:
   - Adafruit_GFX_Library
   - GxEPD2
   - DHT-sensor-library
   - Adafruit_BME280_Library
   - Adafruit_SGP30
   - ArduinoJson
   - WiFiManager
   - NTPClient

### Hardware Connection

1. **E-Ink Display Connection**:
   - VCC -> 3.3V
   - GND -> GND
   - DIN -> MOSI (GPIO 23 for ESP32)
   - CLK -> SCK (GPIO 18 for ESP32)
   - CS -> SS (GPIO 5 for ESP32)
   - DC -> GPIO 17 for ESP32
   - RST -> GPIO 16 for ESP32
   - BUSY -> GPIO 4 for ESP32

2. **Sensor Connection**:
   - Most sensors use I2C interface, connected to SDA (GPIO 21) and SCL (GPIO 22) pins
   - Analog sensors connect to ADC pins
   - Digital sensors connect to any GPIO pins

### Software Configuration

1. **Configuration Files**:
   - `src/coresystem/config.h`: System core configuration
   - `platformio.ini`: PlatformIO configuration
   - `arduino_secrets.h`: WiFi passwords and other sensitive information

2. **Main Configuration Items**:
   ```cpp
   // Display configuration
   #define DISPLAY_TYPE GxEPD2_583_T8
   #define DISPLAY_WIDTH 600
   #define DISPLAY_HEIGHT 448
   
   // Refresh interval configuration
   #define NORMAL_REFRESH_INTERVAL 60000  // Normal mode refresh once per minute
   #define LOW_POWER_REFRESH_INTERVAL 300000  // Low power mode refresh once every 5 minutes
   
   // Sensor configuration
   #define ENABLE_TEMPERATURE_SENSOR true
   #define ENABLE_HUMIDITY_SENSOR true
   #define ENABLE_PRESSURE_SENSOR true
   #define ENABLE_GAS_SENSOR true
   
   // WiFi configuration
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASSWORD "your_wifi_password"
   ```

## Usage

### First Use

1. Burn the firmware to the development board
2. Connect hardware devices
3. After power-on, the device will enter WiFi configuration mode
4. Use your phone to connect to the WiFi hotspot created by the device (name: InkClock-XXXX)
5. Access `192.168.4.1` in the browser to enter the configuration page
6. Configure WiFi network and other parameters
7. After saving the configuration, the device will restart and connect to the internet
8. The device will automatically obtain time, weather and other information and display it on the screen

### Daily Use

- **View Information**: The device will automatically cycle through different information pages
- **Manual Switching**: Press the button on the device to manually switch pages
- **Human Sensing**: When someone approaches, the device will automatically wake up and display the current time
- **Low Power Mode**: When no one is present, the device will automatically enter low power mode, reducing refresh frequency
- **Remote Control**: Use mobile APP or Web interface to remotely control the device

### Remote Update

1. **OTA Update**:
   - Upload new firmware files in the Web configuration page
   - The device will automatically download and update the firmware, supporting download and update retry mechanisms
   - Check power stability and memory availability during the update process
   - Automatically back up current partition and key configurations before update
   - Watchdog protection during the update process to prevent device bricking
   - After update completion, the device will automatically restart
   - Support firmware integrity verification and signature verification to prevent malicious firmware

2. **GitHub Update**:
   - The device supports automatic firmware updates from GitHub repositories
   - Set GitHub repository address and branch in the configuration page
   - The device will regularly check for updates and automatically download and install
   - Support API key authorization, only authorized devices can perform remote updates
   - Support real-time monitoring of update status and error reporting

3. **Firmware Update Security Mechanisms**:
   - SHA-256 hash verification to ensure firmware integrity
   - Firmware signature verification to prevent malicious firmware
   - Dual partition mechanism, supporting update failure rollback
   - Automatic backup and recovery of key configurations
   - Power stability check to prevent low-voltage updates
   - Memory shortage check to prevent memory overflow
   - Watchdog protection to prevent device bricking during update
   - Update authorization mechanism to prevent unauthorized updates

## Development Guide

### Code Structure

```
src/
├── application/        # Application modules
│   ├── api_manager.cpp/h
│   ├── display_manager.cpp/h
│   ├── feedback_manager.cpp/h
│   ├── geo_manager.cpp/h
│   ├── lunar_manager.cpp/h
│   ├── message_manager.cpp/h
│   ├── power_manager.cpp/h
│   ├── scene_manager.cpp/h
│   ├── sensor_manager.cpp/h
│   ├── stock_manager.cpp/h
│   ├── time_manager.cpp/h
│   ├── weather_manager.cpp/h
│   ├── web_client.cpp/h
│   ├── web_server.cpp/h
│   └── wifi_manager.cpp/h
├── coresystem/         # Core system
│   ├── arduino_compat.cpp/h
│   ├── base_plugin.h
│   ├── config.h
│   ├── config_manager.cpp/h
│   ├── core_system.cpp/h
│   ├── dependency_injection.cpp/h
│   ├── driver_registry.h
│   ├── error_handling.cpp/h
│   ├── event_bus.h
│   ├── feature_manager.cpp/h
│   ├── font_manager.cpp/h
│   ├── hardware_detector.cpp/h
│   ├── i18n_manager.cpp/h
│   ├── icore_system.h
│   ├── memory_pool.cpp/h
│   ├── module_registry.cpp/h
│   ├── network_manager.cpp/h
│   ├── performance_monitor.cpp/h
│   ├── platform_abstraction.cpp/h
│   ├── plugin_manager.cpp/h
│   ├── spiffs_manager.h
│   ├── storage_manager.cpp/h
│   └── tf_card_manager.h
├── drivers/            # Driver modules
│   ├── peripherals/    # Peripheral drivers
│   │   ├── am2302_driver.cpp/h
│   │   ├── base_mq_sensor_driver.h
│   │   ├── base_sensor_driver.cpp/h
│   │   ├── bh1750_driver.cpp/h
│   │   ├── bme280_driver.cpp/h
│   │   ├── bme680_driver.cpp/h
│   │   ├── bmp388_driver.cpp/h
│   │   ├── dht22_driver.cpp/h
│   │   ├── display_driver.h
│   │   ├── eink_display.cpp/h
│   │   ├── eink_driver.cpp/h
│   │   ├── gy30_driver.cpp/h
│   │   ├── hc_sr501_driver.cpp/h
│   │   ├── hc_sr505_driver.cpp/h
│   │   ├── hdc1080_driver.cpp/h
│   │   ├── htu21d_driver.cpp/h
│   │   ├── ir_flame_driver.cpp/h
│   │   ├── ld2410_driver.cpp/h
│   │   ├── lps25hb_driver.cpp/h
│   │   ├── mq135_driver.cpp/h
│   │   ├── mq2_driver.cpp/h
│   │   ├── mq5_driver.cpp/h
│   │   ├── mq7_driver.cpp/h
│   │   ├── re200b_driver.cpp/h
│   │   ├── sensor_driver.h
│   │   ├── sgp30_driver.cpp/h
│   │   ├── sht20_driver.cpp/h
│   │   ├── sht21_driver.cpp/h
│   │   ├── sht30_driver.cpp/h
│   │   ├── sht40_driver.cpp/h
│   │   ├── si1145_driver.cpp/h
│   │   ├── si7021_driver.cpp/h
│   │   ├── tgs2600_driver.cpp/h
│   │   └── tsl2561_driver.cpp/h
│   ├── audio_driver.cpp/h
├── extensions/         # Extension modules
│   └── plugin_manager.cpp/h
├── plugins/            # Plugin modules
│   ├── example_plugin.cpp/h
├── audio_manager.cpp/h
├── bluetooth_manager.cpp/h
├── button_manager.cpp/h
├── camera_manager.cpp/h
├── firmware_manager.cpp/h
├── ipv6_server.cpp/h
├── touch_manager.cpp/h
└── main.cpp            # Main program entry
```

### Development Process

1. **Create New Module**:
   - Create new .cpp and .h files in the corresponding directory
   - Implement module initialization, update and loop methods
   - Add module initialization and calls in main.cpp

2. **Add New Sensor**:
   - Create new driver files in the drivers/peripherals directory
   - Inherit from BaseSensorDriver class
   - Implement init(), readData(), getType() and other methods
   - Register new sensor driver in DriverRegistry

3. **Add New Feature**:
   - Implement new feature's application module
   - Register new event types in EventBus
   - Add new module initialization and scheduling in CoreSystem

### Low Power Optimization

1. **Hardware Optimization**:
   - Use low-power microcontrollers
   - Choose low-power sensors
   - Optimize circuit design to reduce standby current

2. **Software Optimization**:
   - Implement dynamic refresh rate: adjust refresh rate based on battery level and usage scenario
   - Support deep sleep: enter deep sleep mode when no one is present
   - Turn off unnecessary peripherals: turn off WiFi, Bluetooth and other peripherals in low-power mode
   - Optimize code: reduce CPU usage and memory usage

## Contribution Guide

Welcome everyone to participate in the development and contribution of the InkClock project!

### Contribution Methods

1. **Submit Issue**: Report bugs, suggest new features
2. **Submit Pull Request**: Fix bugs, implement new features
3. **Improve Documentation**: Update README, add comments, write tutorials
4. **Test Feedback**: Test hardware compatibility, report test results

### Code Standards

1. **Naming Conventions**:
   - Class names: Use PascalCase (e.g., DisplayManager)
   - Method names: Use camelCase (e.g., updateDisplay)
   - Variable names: Use camelCase (e.g., screenWidth)
   - Constant names: Use ALL_CAPS with underscores (e.g., MAX_REFRESH_INTERVAL)

2. **Comment Standards**:
   - Use Doxygen style comments for classes and methods
   - Add detailed comments for complex code segments
   - Add comments for key variables and constants

3. **Code Structure**:
   - Each file contains only one main class
   - Method length does not exceed 100 lines
   - Use namespaces reasonably
   - Avoid global variables, use singleton pattern or dependency injection

## License

The InkClock project adopts the MIT license, see the LICENSE file for details.

## Contact Information

- **GitHub**: [https://github.com/yourusername/InkClock](https://github.com/yourusername/InkClock)
- **Email**: your.email@example.com
- **Forum**: [https://forum.inkclock.com](https://forum.inkclock.com)

## Acknowledgments

Thank all developers and users who have contributed to the InkClock project!

## Version History

### v1.5.0 (2026-01-13)

**Scene Management System Optimization**:
- **Simplified Scene Modes**: Simplified scene system to three modes: normal, interactive, sleep
- **Scene Configuration Persistence**: Implemented saveScenes/loadScenes functions to persist scene configurations using SPIFFS and ArduinoJson
- **Scene Configuration Application Fix**: Fixed applySceneConfig function to actually apply scene configurations to various modules
- **User Activity Detection Optimization**: Optimized recordUserActivity function to correctly handle activity counting and scene switching

**Storage Management System Enhancement**:
- **Cross-medium Data Backup**: Implemented backupData function to support cross-storage medium data backups
- **Data Compression Functionality**: Implemented compressData/decompressData functions using Run-Length Encoding (RLE) algorithm

**Hardware Detector Optimization**:
- **CPU Usage Detection**: Updated CpuDetector to use esp_cpu_utilization_get() on ESP32 for real CPU usage data
- **Storage Usage Detection**: Updated StorageDetector to use SPIFFS.info() for real storage usage data
- **Network Signal Detection**: Updated NetworkDetector to use WiFi.RSSI() for real network signal strength
- **Battery Voltage Detection**: Updated PowerDetector to use ADC for real battery voltage on ESP32

**Performance Monitoring System Enhancement**:
- **Performance Data Publishing**: Implemented publishPerformanceDataEvent function to publish performance data using EventBus
- **Alert Event Publishing**: Implemented publishAlertEvent function to publish alert events using EventBus

### v1.4.0 (2025-12-31)

**Firmware Management System Optimization**:
- **Firmware Integrity Verification**:
  - Added SHA-256 hash calculation and verification functionality
  - Read expected hash value from firmware_info.json and verify
  - Ensure firmware files are not tampered with during transmission and storage

- **Firmware Signature Verification Mechanism**:
  - Implemented firmware signature verification framework
  - Obtain signatures and public keys from firmware information
  - Support encryption algorithms such as ECDSA/RSA
  - Prevent installation of malicious firmware

- **Dual Partition Update and Rollback Mechanism**:
  - Implemented partition management functionality: get current partition, switch partition, backup partition and restore partition
  - Added firmware rollback functionality, supporting automatic detection of rollback needs
  - Automatically backup current partition before update
  - Switch to new partition after update, can rollback to old partition if failed

- **Watchdog Protection During Update**:
  - Initialize watchdog timer (30-second timeout)
  - Regularly reset watchdog in key update steps
  - Prevent device bricking due to update process crashes
  - Disable watchdog after update completion

- **Key Configuration Backup and Recovery Mechanism**:
  - Automatically backup key configurations (WiFi configuration, device ID, etc.) before update
  - Check configuration validity during device startup, automatically restore if invalid
  - Ensure firmware updates do not lose important configuration data

- **Power Stability Check**:
  - Implemented power stability check to ensure updates are performed at safe voltage
  - Multiple checks (5 times) to ensure power stability
  - Reject firmware updates at low voltage or unstable power

- **Memory Management Optimization and Memory Shortage Check**:
  - Added memory usage optimization functionality
  - Check available memory before update to ensure sufficient memory for update
  - Prevent update failures due to insufficient memory

- **Enhanced Update Status Reporting and Error Handling**:
  - Added detailed error code enumeration
  - Implemented status callback mechanism to support real-time monitoring of update progress
  - Set unique error codes for each error situation
  - Provide public method to get last error code

- **Update Authorization Mechanism**:
  - Added API key verification functionality
  - Support setting and getting API keys
  - Only authorized requests can perform remote OTA updates
  - Support no API key mode (convenient for local testing)

- **OTA Update Retry Mechanism**:
  - Added retry logic for firmware download (up to 3 times)
  - Added retry logic for firmware update (up to 2 times)
  - Implemented timeout handling to prevent infinite waiting
  - Intelligent retry mechanism, only retry in specific error situations
  - Improve OTA update success rate

**Enhanced Bluetooth Network Configuration and Hotspot WiFi Configuration**:
  - Optimized initial state Bluetooth network configuration process, enhancing ease of use, stability and fault tolerance
  - Implemented AP mode automatic switching, automatically entering AP mode when WiFi connection fails
  - Added WiFi configuration save and load functionality, implementing one-click connection
  - Enhanced WiFi connection logic after Bluetooth network configuration to ensure correct connection after configuration
  - Optimized AP mode SSID naming, including device MAC address last 4 digits for easy differentiation of different devices
  - Added fixed password for AP mode, convenient for users to connect
  - Implemented AP mode Web configuration page prompt
  - Enhanced Bluetooth network configuration status notification, real-time notification of configuration results

**Remote Control and Data Synchronization Functionality Enhancement**:
  - Optimized remote control API, enhanced error handling and stability
  - Extended data synchronization API to provide more comprehensive data
  - Added request frequency limit for refresh display API to prevent system overload
  - Increased support for POST request JSON parameters to improve API flexibility and compatibility
  - Enhanced parameter verification to prevent malicious requests

### v1.3.0 (2025-12-31)
- **Cross-platform Compatibility Optimization**:
  - Replaced all `millis()` calls with `platformGetMillis()`
  - Replaced all `delay()` calls with `platformDelay()`
  - Replaced `ESP.restart()` with `platformReset()`
  - Added conditional compilation for ESP32-specific features (Bluetooth, WiFi, rtc_gpio)
  - Unified time and delay APIs to improve code cross-platform compatibility

- **Low Power Optimization Enhancement**:
  - Unified power management API calls
  - Used platform-independent CPU frequency control
  - Added conditional compilation for platform-specific low-power features
  - Optimized battery management and low-power strategies

- **Remote Control and Data Synchronization Functionality**:
  - Added remote control API (`/api/control`)
  - Implemented data synchronization API (`/api/sync`)
  - Added display refresh API (`/api/refresh`)
  - Support remote commands for power control, low-power mode switching, refresh interval adjustment, etc.

### v1.2.0 (2025-12-30)
- Added audio playback functionality
- Support Bluetooth/WiFi remote control
- Implemented custom display layout
- Optimized system stability

### v1.1.0 (2025-12-29))
- Added stock quote display functionality
- Support lunar information display
- Implemented human sensing and light sensing
- Optimized battery management

### v1.0.0 (2025-12-27)
- Initial version release
- Support basic time, date, weather display
- Support multiple sensors and peripherals
- Implemented low-power optimization
- Support OTA remote update
- [ ] Implement richer display content
- [ ] Optimize low-power algorithms
- [ ] Develop mobile APP
- [ ] Support more sensors and peripherals