# InkClock - Smart E-Paper Clock System

[中文版本](README.md)

## Project Introduction

InkClock is a smart e-paper clock system based on multiple development boards, supporting ESP32, ESP8266, NRF52, STM32, RP2040 and other microcontroller platforms. It features low power consumption, high resolution, and remote configuration capabilities. The system supports displaying time, date, weather, air quality, stock quotes and other information, and has human presence detection, light sensing, low power optimization and other functions.

## Features

### Core Features
- 🌍 **Time and Date**: Display Gregorian time, date, and day of week
- 🌙 **Lunar Calendar**: Display lunar date, solar terms, and festivals
- 🌡️ **Weather Information**: Real-time weather, temperature, humidity, and pressure
- 💨 **Air Quality**: PM2.5, CO2, VOC and other environmental indicators
- 📈 **Stock Quotes**: Real-time stock prices, price changes, and K-line charts
- 🌞 **Light Sensing**: Automatically adjust screen brightness and refresh rate
- 👤 **Human Presence Detection**: Automatically wake up when someone is present, enter low power mode when no one is around
- 🔋 **Battery Management**: Real-time battery level display, low battery reminder
- 🔄 **Automatic Updates**: Support OTA remote firmware updates
- 📱 **Mobile Control**: Support Bluetooth/WiFi remote control
- 🎵 **Audio Playback**: Support MP3 playback and voice broadcasting
- 🔔 **Smart Reminders**: Timed reminders and event reminders
- 🌐 **Network Synchronization**: NTP time synchronization, network weather updates
- 📝 **Custom Display**: Support custom display content and layout

### Hardware Support

#### Microcontrollers
- ESP32 (Recommended)
- ESP8266
- NRF52
- STM32
- RP2040

#### E-Paper Displays
- Support for multiple sizes: 2.13", 2.9", 4.2", 5.83", 7.5", etc.
- Support for black and white, black and white red, black and white yellow and other color modes

#### Sensor Support
- **Temperature and Humidity**: DHT11/22, AM2302, SHT30/31/40, HTU21D, HDC1080, BME280, BME680
- **Barometric Pressure**: BMP180, BMP280, BME280, BME680, LPS25HB
- **Air Quality**: SGP30, MQ series (MQ-2/5/7/135), TGS2600, RE200B
- **Light**: BH1750, GY30, TSL2561, SI1145
- **Human Presence**: HC-SR501, HC-SR505, RE200B, LD2410
- **Flame Detection**: IR flame sensor

## Document Directory

The following are the detailed documents of the project, stored in the `doc/` directory:

| Document Name | Description | Link |
|--------------|-------------|------|
| Project Description Document | Overall project description and feature introduction | [doc/Project Description Document.md](doc/Project Description Document.md) |
| Product Requirements Document | Product requirements analysis and feature planning | [doc/Product Requirements Document.md](doc/Product Requirements Document.md) |
| System Secondary Development Guide | System architecture and secondary development guide | [doc/System Secondary Development Guide.md](doc/System Secondary Development Guide.md) |
| Driver Documentation | Hardware driver development and usage instructions | [doc/Driver Documentation.md](doc/Driver Documentation.md) |
| Simulator and Test Environment Setup Guide | Setup method for simulator and test environment | [doc/Simulator and Test Environment Setup Guide.md](doc/Simulator and Test Environment Setup Guide.md) |
| DIY Manufacturing Guide | Device DIY manufacturing and assembly guide | [doc/DIY Manufacturing Guide.md](doc/DIY Manufacturing Guide.md) |
| Hardware Material Selection Guide | Hardware component selection reference | [doc/Hardware Material Selection Guide.md](doc/Hardware Material Selection Guide.md) |
| Circuit Design Guide | Circuit design and PCB layout guide | [doc/Circuit Design Guide.md](doc/Circuit Design Guide.md) |
| Development Hardware Description | Detailed description of development boards and hardware components | [doc/Development Hardware Description.md](doc/Development Hardware Description.md) |

## Project Structure

```
InkClock/
├── code/           # Source code directory
│   ├── src/        # Firmware source code
│   │   ├── application/    # Application modules
│   │   │   ├── api_manager.cpp/h             # API request management
│   │   │   ├── display_manager.cpp/h          # Display content management
│   │   │   ├── feedback_manager.cpp/h         # Operation feedback management
│   │   │   ├── geo_manager.cpp/h              # Geographical location management
│   │   │   ├── lunar_manager.cpp/h            # Lunar calendar information management
│   │   │   ├── message_manager.cpp/h          # Message management
│   │   │   ├── power_manager.cpp/h            # Power management and low power control
│   │   │   ├── scene_manager.cpp/h            # Scene mode management
│   │   │   ├── sensor_manager.cpp/h           # Sensor data management
│   │   │   ├── stock_manager.cpp/h            # Stock data management
│   │   │   ├── time_manager.cpp/h             # Time synchronization management
│   │   │   ├── weather_manager.cpp/h          # Weather data management
│   │   │   ├── web_client.cpp/h               # Web client communication
│   │   │   ├── web_server.cpp/h               # Local web server
│   │   │   └── wifi_manager.cpp/h             # WiFi connection management
│   │   ├── coresystem/     # Core system
│   │   │   ├── arduino_compat.cpp/h           # Arduino compatibility support
│   │   │   ├── base_plugin.h                  # Plugin base class
│   │   │   ├── config.h                       # System configuration
│   │   │   ├── config_manager.cpp/h           # Configuration management
│   │   │   ├── core_system.cpp/h              # Core system management
│   │   │   ├── dependency_injection.cpp/h     # Dependency injection
│   │   │   ├── driver_registry.h              # Driver registry
│   │   │   ├── error_handling.cpp/h           # Error handling
│   │   │   ├── event_bus.h                    # Event bus
│   │   │   ├── feature_manager.cpp/h          # Feature management
│   │   │   ├── font_manager.cpp/h             # Font management
│   │   │   ├── hardware_detector.cpp/h        # Hardware detection
│   │   │   ├── i18n_manager.cpp/h             # Internationalization support
│   │   │   ├── icore_system.h                 # Core system interface
│   │   │   ├── memory_pool.cpp/h              # Memory pool management
│   │   │   ├── module_registry.cpp/h          # Module registry
│   │   │   ├── network_manager.cpp/h          # Network management
│   │   │   ├── performance_monitor.cpp/h      # Performance monitoring
│   │   │   ├── platform_abstraction.cpp/h     # Platform abstraction layer
│   │   │   ├── plugin_manager.cpp/h           # Plugin management
│   │   │   ├── spiffs_manager.h               # SPIFFS file system management
│   │   │   ├── storage_manager.cpp/h          # Storage management
│   │   │   └── tf_card_manager.h              # TF card management
│   │   ├── drivers/        # Driver modules
│   │   │   ├── peripherals/    # Peripheral drivers
│   │   │   │   ├── am2302_driver.cpp/h        # AM2302 temperature and humidity sensor driver
│   │   │   │   ├── base_mq_sensor_driver.h    # MQ gas sensor base class
│   │   │   │   ├── base_sensor_driver.cpp/h   # Sensor driver base class
│   │   │   │   ├── bh1750_driver.cpp/h        # BH1750 light sensor driver
│   │   │   │   ├── bme280_driver.cpp/h        # BME280 temperature, humidity and pressure sensor driver
│   │   │   │   ├── bme680_driver.cpp/h        # BME680 temperature, humidity, pressure and gas sensor driver
│   │   │   │   ├── bmp388_driver.cpp/h        # BMP388 pressure sensor driver
│   │   │   │   ├── dht22_driver.cpp/h         # DHT22 temperature and humidity sensor driver
│   │   │   │   ├── display_driver.h           # Display driver interface
│   │   │   │   ├── eink_display.cpp/h         # E-paper display management
│   │   │   │   ├── eink_driver.cpp/h          # E-paper driver
│   │   │   │   ├── gy30_driver.cpp/h          # GY30 light sensor driver
│   │   │   │   ├── hc_sr501_driver.cpp/h      # HC-SR501 human presence sensor driver
│   │   │   │   ├── hc_sr505_driver.cpp/h      # HC-SR505 human presence sensor driver
│   │   │   │   ├── hdc1080_driver.cpp/h       # HDC1080 temperature and humidity sensor driver
│   │   │   │   ├── htu21d_driver.cpp/h        # HTU21D temperature and humidity sensor driver
│   │   │   │   ├── ir_flame_driver.cpp/h      # IR flame sensor driver
│   │   │   │   ├── ld2410_driver.cpp/h        # LD2410 human presence sensor driver
│   │   │   │   ├── lps25hb_driver.cpp/h       # LPS25HB pressure sensor driver
│   │   │   │   ├── mq135_driver.cpp/h         # MQ135 air quality sensor driver
│   │   │   │   ├── mq2_driver.cpp/h           # MQ2 smoke sensor driver
│   │   │   │   ├── mq5_driver.cpp/h           # MQ5 gas sensor driver
│   │   │   │   ├── mq7_driver.cpp/h           # MQ7 carbon monoxide sensor driver
│   │   │   │   ├── re200b_driver.cpp/h        # RE200B human presence sensor driver
│   │   │   │   ├── sensor_driver.h            # Sensor driver interface
│   │   │   │   ├── sgp30_driver.cpp/h         # SGP30 air quality sensor driver
│   │   │   │   ├── sht20_driver.cpp/h         # SHT20 temperature and humidity sensor driver
│   │   │   │   ├── sht21_driver.cpp/h         # SHT21 temperature and humidity sensor driver
│   │   │   │   ├── sht30_driver.cpp/h         # SHT30 temperature and humidity sensor driver
│   │   │   │   ├── sht40_driver.cpp/h         # SHT40 temperature and humidity sensor driver
│   │   │   │   ├── si1145_driver.cpp/h        # SI1145 light sensor driver
│   │   │   │   ├── si7021_driver.cpp/h        # SI7021 temperature and humidity sensor driver
│   │   │   │   ├── tgs2600_driver.cpp/h       # TGS2600 gas sensor driver
│   │   │   │   └── tsl2561_driver.cpp/h       # TSL2561 light sensor driver
│   │   │   ├── audio_driver.cpp/h             # Audio driver
│   │   ├── extensions/     # Extension modules
│   │   │   └── plugin_manager.cpp/h           # Extension plugin management
│   │   ├── plugins/        # Plugin modules
│   │   │   └── example_plugin.cpp/h           # Example plugin
│   │   ├── audio_manager.cpp/h                # Audio management
│   │   ├── bluetooth_manager.cpp/h            # Bluetooth management
│   │   ├── button_manager.cpp/h               # Button management
│   │   ├── camera_manager.cpp/h               # Camera management
│   │   ├── firmware_manager.cpp/h             # Firmware update management
│   │   ├── ipv6_server.cpp/h                  # IPv6 server
│   │   ├── touch_manager.cpp/h                # Touch management
│   │   └── main.cpp                           # Main program entry
│   ├── config/         # Configuration files directory
│   ├── doc/            # Documentation directory
│   ├── hardware/       # Hardware related files
│   ├── tool/           # Tool scripts
│   ├── webserver/      # Web server code
│   │   ├── config/     # Web server configuration
│   │   ├── public/     # Web server static files
│   │   ├── src/        # Web server source code
│   │   │   ├── Controller/     # Controllers
│   │   │   ├── Model/          # Models
│   │   │   ├── Service/        # Services
│   │   │   └── Utils/          # Utilities
│   │   └── index.php   # Web server entry
│   └── README.md       # Project description document
```

## Quick Start

### 1. Hardware Preparation

1. **Development Board Selection**: Choose a supported development board (ESP32-S3 recommended)
2. **E-Paper Selection**: Choose an appropriate size e-paper display
3. **Sensor Selection**: Choose sensors based on needs
4. **Other Components**: Power module, case, etc.

### 2. Software Setup

1. **Install Development Environment**:
   - Arduino IDE 2.0+ or PlatformIO
   - Related dependency libraries

2. **Configure Firmware**:
   - Modify configuration items in `code/src/coresystem/config.h`
   - Set WiFi information and other parameters

3. **Compile and Upload**:
   - Compile firmware
   - Upload to development board

### 3. First Use

1. After powering on, the device will enter WiFi configuration mode
2. Use your phone to connect to the WiFi hotspot created by the device (name: InkClock-XXXX)
3. Access `192.168.4.1` in the browser to enter the configuration page
4. Configure WiFi network and other parameters
5. After saving the configuration, the device will restart and connect to the internet
6. The device will automatically obtain time, weather and other information, and display it on the screen

### 4. Daily Use

- **View Information**: The device will automatically cycle through different information pages
- **Manual Switching**: Press the button on the device to manually switch pages
- **Human Presence**: When someone approaches, the device will automatically wake up and display the current time
- **Low Power Mode**: When no one is around, the device will automatically enter low power mode, reducing refresh frequency
- **Remote Control**: Use mobile APP or Web interface to remotely control the device

## Firmware Updates

### 1. OTA Updates

1. Upload new firmware files through the Web configuration page
2. The device will automatically download and update the firmware
3. After the update is complete, the device will automatically restart

### 2. GitHub Updates

1. Set GitHub repository address and branch in the configuration page
2. The device will periodically check for updates and automatically download and install

### 3. Firmware Update Security Mechanism

- SHA-256 hash verification to ensure firmware integrity
- Firmware signature verification to prevent malicious firmware
- Dual partition mechanism to support update failure rollback
- Automatic backup and recovery of key configurations
- Power stability check to prevent low voltage updates
- Memory check to prevent overflow

## Development Environment Setup

### 1. Arduino IDE

1. Install Arduino IDE 2.0+
2. Install ESP32/ESP8266 board support
3. Install dependency libraries:
   - Adafruit_GFX_Library
   - GxEPD2
   - DHT-sensor-library
   - Adafruit_BME280_Library
   - ArduinoJson
   - WiFiManager
   - NTPClient

### 2. PlatformIO

1. Install Visual Studio Code
2. Install PlatformIO plugin
3. Open project folder
4. PlatformIO will automatically install dependency libraries

## Troubleshooting

### 1. Common Problems

| Problem | Possible Cause | Solution |
|---------|---------------|----------|
| Device cannot start | Power issue | Check power connection and voltage |
| WiFi cannot connect | WiFi password error | Reconfigure WiFi information |
| Screen not displaying | E-paper connection error | Check e-paper connection |
| No sensor data | Sensor connection error | Check sensor connection |
| Firmware update failed | Network issue | Check network connection and retry |

### 2. Factory Reset

- **Soft Reset**: Triggered through the Web page
- **Hard Reset**: Press and hold the button for more than 5 seconds to trigger

## Technical Support

If you have any questions, please refer to the relevant documents or contact the project maintainers.

## Contribution Guidelines

Welcome everyone to participate in the development and contribution of the InkClock project!

### Contribution Methods

1. **Submit Issues**: Report bugs, suggest new features
2. **Submit Pull Requests**: Fix bugs, implement new features
3. **Improve Documentation**: Update README, add comments, write tutorials
4. **Test Feedback**: Test hardware compatibility, report test results

### Code Standards

1. **Naming Conventions**:
   - Class names: Use PascalCase, e.g., `DisplayManager`
   - Method names: Use camelCase, e.g., `updateDisplay`
   - Variable names: Use camelCase, e.g., `screenWidth`
   - Constant names: Use uppercase with underscores, e.g., `MAX_REFRESH_INTERVAL`

2. **Commenting Standards**:
   - Classes and methods use Doxygen-style comments
   - Complex code segments add detailed comments
   - Key variables and constants add comments

3. **Code Structure**:
   - Each file contains only one main class
   - Method length does not exceed 100 lines
   - Use namespaces appropriately
   - Avoid global variables, use singleton pattern or dependency injection

## License

The InkClock project uses the MIT license, see the LICENSE file for details.

## Acknowledgments

Thank you to all developers and users who have contributed to the InkClock project!