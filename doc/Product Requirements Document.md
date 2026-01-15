# Home Network Intelligent E-Ink Calendar - Product Requirements Document

[中文版本](产品需求文档.md)

## 1. Introduction

### 1.1 Document Purpose

This requirements document details the functional requirements, non-functional requirements, technical specifications, and design constraints of the Home Network Intelligent E-Ink Calendar, aiming to provide a clear basis for product development, testing, and acceptance. The document is based on a three-layer architecture design, highlighting the system's scalability, maintainability, and flexibility.

### 1.2 Terminology Definition

| Term | Explanation |
|------|-------------|
| ESP32-S3 | A low-power WiFi+Bluetooth dual-mode microcontroller launched by Espressif |
| ESP8266 | A low-cost WiFi microcontroller launched by Espressif |
| NRF52 | A low-power Bluetooth 5.0 microcontroller launched by Nordic Semiconductor |
| STM32 | A high-performance microcontroller launched by STMicroelectronics |
| RP2040 | A dual-core ARM Cortex-M0+ microcontroller launched by Raspberry Pi Foundation |
| E-Ink Display | A low-power, high-definition electronic paper display technology |
| NTP | Network Time Protocol, used to synchronize device time |
| API | Application Programming Interface, used for data interaction between different systems |
| OTA | Over-The-Air, remotely update device firmware through the network |
| IPv6 | Internet Protocol Version 6, used to identify and locate network devices |
| MQTT | Message Queue Telemetry Transport, a lightweight message transmission protocol |
| Event Bus | Core mechanism for inter-module communication, supporting publish/subscribe mode |
| Driver Registry | Dynamically manage drivers, supporting automatic detection and device discovery |
| Three-Layer Architecture | Layered design of underlying operating system, driver layer, and application layer |

## 2. Product Overview

### 2.1 Product Positioning

The Home Network Intelligent E-Ink Calendar is an intelligent device for home users, using e-ink display, featuring low power consumption, high definition, and remote configuration capabilities. The device adopts a three-layer architecture design, can automatically obtain time, weather, lunar calendar, holidays and other information, and supports modular expansion and secondary development.

### 2.2 Product Features

- **Three-Layer Architecture Design**: Layered design of underlying operating system, driver layer, and application layer, with good scalability and maintainability
- **Low Power Design**: Using e-ink display, extremely low power consumption, can use lithium battery power supply, long battery life
- **High Definition Display**: Support multiple sizes of e-ink displays, clear display, wide viewing angle
- **Automatic Data Synchronization**: Automatically obtain time, weather, lunar calendar and other information, no manual setting required
- **Automatic Geographic Location Identification**: Automatically detect geographic location through network IP, can also be manually configured
- **Modular Design**: Support plugin extension, can easily add new features
- **Remote Configuration Management**: Device configuration and management through web pages
- **Multiple Network Support**: Support WiFi connection, support IPv6
- **Bluetooth Configuration**: Support first-time WiFi configuration
- **Sensor Support**: Support temperature and humidity, human presence, light, gas, flame and other sensors

### 2.3 Application Scenarios

- Time and weather display in home living rooms, bedrooms, study rooms and other places
- Schedule and weather reminders in offices
- Time and date reference for the elderly and children
- Information display terminal for smart home systems
- Learning and practice platform for secondary development and DIY enthusiasts

## 3. Three-Layer Architecture Design

### 3.1 Underlying Operating System (CoreSystem)

The underlying operating system is the core of the system, responsible for managing system resources, providing basic services, and directly interacting with hardware.

#### 3.1.1 Core Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Power Management | Low power mode control, battery level monitoring, charging status management | High |
| Configuration Management | Unified configuration storage and access API | High |
| Timer Management | Support periodic and one-time timers | High |
| Event Bus | Core of inter-module communication, supporting 85+ event types | High |
| System Security | Error handling, safe mode guarantee | High |

#### 3.1.2 Design Constraints

- Minimal core operation, maximum stability
- Complete decoupling from application layer
- Support multiple hardware platforms
- Low power design, long battery life

### 3.2 Driver Layer

The driver layer provides a unified device abstraction interface, supporting dynamic registration and management of sensors, display devices and other hardware.

#### 3.2.1 Core Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Sensor Drivers | Support multiple temperature and humidity, human presence, light, gas, flame and other sensors | High |
| Display Drivers | Support multiple sizes and types of e-ink displays | High |
| Driver Registry | Dynamically manage drivers, supporting automatic detection and device discovery | High |

#### 3.2.2 Design Constraints

- Unified driver interface, easy to extend
- Support hot plugging and dynamic loading
- Automatic detection and device discovery
- Support multiple hardware platforms

### 3.3 Application Layer

The application layer contains various functional modules, communicating with the underlying layer through the event bus, implementing specific business logic.

#### 3.3.1 Core Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Time Management | NTP time synchronization, time display | High |
| Weather Management | Weather data acquisition and display | High |
| Lunar Calendar Management | Lunar date, solar terms, holiday display | High |
| Sensor Data Management | Sensor data collection and display | High |
| Display Management | Page rendering and display refresh | High |
| Plugin Management | Dynamic loading and unloading of plugins | Medium |

#### 3.3.2 Design Constraints

- Event-driven programming, low coupling between modules
- Support dynamic plugin extension
- Easy to develop and maintain
- Support multiple application scenarios

## 4. Functional Requirements

### 4.1 Underlying Operating System Functions

#### 4.1.1 Power Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Low Power Mode | Support device entering low power mode | High |
| Power Status Monitoring | Monitor device power status | Medium |
| Battery Level Display | Display battery level (if using lithium battery) | Medium |
| Charging Status Display | Display charging status (if supporting charging) | Low |

#### 4.1.2 Configuration Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Configuration Storage | Unified configuration storage API | High |
| Configuration Access | Unified configuration access API | High |
| Configuration Backup | Support configuration backup and recovery | Medium |

#### 4.1.3 Timer Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Timer Creation | Support creation of periodic and one-time timers | High |
| Timer Deletion | Support deletion of timers | High |
| Timer Callback | Support timer callback functions | High |

#### 4.1.4 Event Bus Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Event Publishing | Support publishing events | High |
| Event Subscription | Support subscribing to events | High |
| Event Processing | Support event processing functions | High |
| Event Queue | Support event queue management | Medium |

#### 4.1.5 System Security Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Error Detection | Detect errors during system operation | High |
| Error Log Recording | Record error information | Medium |
| Error Recovery Mechanism | Automatically try to recover from error states | High |
| Safe Mode | Enter safe mode when errors are severe | High |

### 4.2 Driver Layer Functions

#### 4.2.1 Sensor Driver Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Temperature and Humidity Sensor Support | Support DHT11, DHT12, DHT22, AM2302, SHT20, SHT21, SHT30, SHT40, HDC1080, HTU21D, SI7021, BME280, BME680 and other temperature and humidity sensors | High |
| Human Presence Sensor Support | Support HC-SR501, HC-SR505, RE200B, LD2410 and other human presence sensors | Medium |
| Light Sensor Support | Support BH1750, TSL2561, GY30, SI1145 and other light sensors | Medium |
| Gas Sensor Support | Support MQ2, MQ5, MQ7, MQ8, MQ135, TGS2600, SGP30 and other gas sensors | High |
| Flame Sensor Support | Support IR, UV and other flame sensors | High |
| Barometric Pressure Sensor Support | Support BMP280, BMP388, LPS25HB and other barometric pressure sensors | Medium |

#### 4.2.2 Display Driver Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| E-Ink Display Support | Support 1.54/2.13/2.66/2.7/2.9/3.12/4.2/4.37/5.4/5.83/6.0/7.5 inch e-ink displays | High |
| Electronic Price Tag Support | Support 1.54/2.13/2.66/2.9/3.12 inch dual-color electronic price tags | Medium |
| Second-hand Reader Screen Support | Support 6.0/7.8/10.3 inch monochrome/color reader screens | Medium |

#### 4.2.3 Driver Registry Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Driver Registration | Support dynamic registration of drivers | High |
| Driver Unregistration | Support dynamic unregistration of drivers | High |
| Driver Acquisition | Support acquiring drivers by type | High |
| Automatic Detection | Support automatic detection and device discovery | High |
| Driver Status Management | Support management and query of driver status | Medium |

### 4.3 Application Layer Functions

#### 4.3.1 Time Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Real-time Time Display | Display current time, including hours, minutes, seconds | High |
| Date Display | Display current date, including year, month, day, week | High |
| Lunar Calendar Display | Display lunar date, solar terms, Heavenly Stems and Earthly Branches year | High |
| Time Zone Automatic Adjustment | Automatically handle time zones and daylight saving time | Medium |
| NTP Time Synchronization | Automatically synchronize time through NTP server | High |
| Backup NTP Server | Support multiple NTP servers, improve reliability | Medium |

#### 4.3.2 Weather Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Current Weather Display | Display current weather conditions, temperature, humidity, wind and other information | High |
| Weather Forecast Display | Display 5-day weather forecast | High |
| Weather Icon Display | Use intuitive icons to represent weather conditions | Medium |
| Main and Backup Weather API | Support main weather API and backup weather API, improve reliability | Medium |
| Weather Update Frequency Setting | Can set weather data update frequency | Medium |

#### 4.3.3 Geographic Location Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Automatic Geographic Location Detection | Automatically detect geographic location through network IP | High |
| Manual Geographic Location Configuration | Support manual input of city ID, city name, latitude, longitude | High |
| Geographic Location Information Storage | Save geographic location configuration, not lost after power-off | High |
| Geographic Location Automatic Update | Regularly automatically update geographic location information | Medium |

#### 4.3.4 Sensor Data Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Sensor Data Collection | Collect sensor data | High |
| Sensor Data Display | Display current sensor data on the screen | Medium |
| Sensor Data Update Frequency | Can set sensor data update frequency | Low |
| Sensor Switch Control | Can switch control each sensor in device settings | Medium |
| Sensor Threshold Setting | Can adjust sensor alarm thresholds, adjust sensitivity | Medium |
| Sensor Data Event Publishing | Publish sensor data updates through event bus | High |

#### 4.3.5 Display Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Page Rendering | Render page content | High |
| Display Refresh | Refresh display content | High |
| Partial Refresh | Support partial refresh, reduce power consumption | Medium |
| Startup Screen Display | Display startup screen | Medium |
| Message Display | Display message notifications | Medium |
| Alarm Display | Display alarm information | High |

#### 4.3.6 Plugin Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Plugin Management Interface | Provide web page for plugin management | Medium |
| Plugin Installation and Uninstallation | Support plugin installation and uninstallation | Medium |
| Plugin Enable and Disable | Support plugin enable and disable | Medium |
| Plugin Configuration | Support plugin parameter configuration | Medium |
| Plugin Update | Support plugin online update | Low |
| Online Plugin Acquisition | Obtain online plugin list through message relay service, directly add to device | High |
| Manual Plugin Addition | Can input plugin name, fill in URL and refresh time (minutes, hours) to add | High |
| Plugin Switch Management | Can see all plugin lists, set enable or disable, set sorting (switching order when clicking switch button) | High |
| Online Plugin Management Interface | Provide online plugin management web interface (name, URL, refresh frequency, setting interface URL) | Medium |

#### 4.3.7 Web Configuration Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Device Status Display | Display device basic information and status | High |
| WiFi Configuration | Support WiFi connection configuration | High |
| Time Configuration | Support time zone and daylight saving time configuration | Medium |
| Weather Configuration | Support weather API key and update frequency configuration | Medium |
| Geographic Location Configuration | Support automatic detection and manual configuration of geographic location | High |
| Display Configuration | Support display refresh frequency and display mode configuration | Medium |
| System Configuration | Support system-level configuration and management | Medium |

#### 4.3.8 Message Push Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Message Reception | Support receiving message push from cloud | Medium |
| Message Display | Display pushed messages on the screen | Medium |
| Message Storage | Support message storage and history viewing | Low |

#### 4.3.9 Stock Data Display Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Stock Data Acquisition | Support acquiring real-time stock data | Low |
| Stock Data Display | Display stock data on the screen | Low |
| Stock List Configuration | Support configuring followed stock list | Low |

#### 4.3.10 Font Management Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| Font Upload | Support uploading font files through web interface | Medium |
| Font Selection | Support selecting default font used by system | High |
| Font Management | Support viewing and deleting installed fonts | Medium |
| Built-in Font Support | Provide built-in font options | High |
| Font Preview | Support font effect preview | Low |

#### 4.3.11 Firmware Update Functions

| Function Point | Description | Priority |
|---------------|-------------|----------|
| OTA Firmware Update | Support remote firmware update through network | Medium |
| Firmware Version Check | Automatically check firmware updates | Low |
| Firmware Update Status Display | Display firmware update progress and status | Medium |

## 5. Non-Functional Requirements

### 5.1 Performance Requirements

| Requirement Point | Description | Target Value |
|------------------|-------------|--------------|
| Startup Time | Time from device startup to normal content display | < 10 seconds |
| Data Update Time | Time from requesting data to displaying update | < 5 seconds |
| Response Time | Web page response time | < 2 seconds |
| Battery Life | Battery life when using lithium battery power supply | > 30 days |
| Display Refresh Frequency | Configurable display refresh frequency | 1 minute - 1 hour |

### 5.2 Reliability Requirements

| Requirement Point | Description | Target Value |
|------------------|-------------|--------------|
| System Stability | System continuous operation time | > 365 days |
| Data Reliability | Data update success rate | > 99% |
| Error Recovery Capability | System error automatic recovery success rate | > 95% |
| Backup API Support | When main API fails, backup API switching success rate | > 99% |

### 5.3 Availability Requirements

| Requirement Point | Description | Target Value |
|------------------|-------------|--------------|
| Usability | First-time configuration steps | < 5 steps |
| Maintainability | System maintainability index | > 8/10 |
| Scalability | Number of supported plugins | Unlimited |
| Compatibility | Supported e-ink display sizes | 1.54-12.48 inches |

### 5.4 Security Requirements

| Requirement Point | Description | Target Value |
|------------------|-------------|--------------|
| Network Security | WiFi password encrypted storage | AES-256 |
| API Security | API request authentication mechanism | Token authentication |
| Firmware Security | Firmware signature verification | RSA-2048 |
| Data Security | Sensitive data encrypted transmission | TLS 1.3 |

## 6. Technical Specifications

### 6.1 Hardware Specifications

| Component | Specification | Remarks |
|-----------|---------------|---------|
| Main Control Chip | ESP32 series, ESP8266 series, NRF52 series, STM32 series, RP2040 series | Support multiple low-power WiFi+Bluetooth dual-mode microcontrollers |
| Memory | 512KB SRAM + 8MB PSRAM | Optional |
| Storage | 16MB Flash | For storing firmware and data |
| Display Screen | 1.54/2.13/2.66/2.7/2.9/3.12/4.2/4.37/5.4/5.83/6.0/7.5/7.8/10.3/12.48 inch e-ink display | Support multiple sizes and types |
| Communication Interface | WiFi 802.11 b/g/n, Bluetooth 5.0 | Dual-mode communication |
| Sensor Interface | I2C, SPI, UART | Support multiple sensors |
| Power Input | USB-Type-C (10W-20W), 3.7V soft polymer battery | Only support USB-Type-C charging, not DC power supply, with charging protection |
| Operating Temperature | -20℃ ~ 60℃ | Wide temperature design |
| Operating Humidity | 0% ~ 90% RH | No condensation |

### 6.2 Software Specifications

| Item | Specification | Remarks |
|------|---------------|---------|
| Operating System | FreeRTOS | Real-time operating system |
| Development Framework | Arduino ESP32 | Facilitate development and secondary development |
| Programming Language | C++ | Support C++11 and above standards |
| Event Bus | Custom event bus | Support 85+ event types |
| Driver Framework | Custom driver framework | Unified driver interface |
| Web Server | ESPAsyncWebServer | Asynchronous web server |
| JSON Library | ArduinoJson | For JSON data parsing and generation |
| Network Library | WiFiClientSecure, HTTPClient | For network communication |
| Storage Library | LittleFS | For file storage |
| Encryption Library | mbedtls | For data encryption and secure communication |

### 6.3 Network Specifications

| Item | Specification | Remarks |
|------|---------------|---------|
| WiFi Standard | 802.11 b/g/n | 2.4GHz band |
| Network Protocol | IPv4/IPv6 | Dual stack support |
| API Request | HTTPS/TLS 1.3 | Secure communication |
| NTP Server | Support multiple NTP servers | Improve reliability |
| MQTT Support | Optional | For message push |

## 7. Design Constraints

### 7.1 Hardware Constraints

- E-ink display refresh speed is slow, need to optimize display refresh strategy
- E-ink display color is limited, usually black and white or black, white and red
- ESP32 resources are limited, need to optimize memory and CPU usage
- Power supply is limited, need to optimize power consumption

### 7.2 Software Constraints

- Must use Arduino ESP32 framework, facilitate secondary development
- Must support C++11 and above standards
- Must follow three-layer architecture design principles
- Must support firmware OTA update
- Must use secure communication protocols
- Must communicate between modules through event bus

### 7.3 Interface Constraints

- Must provide unified driver interface
- Must provide unified event bus interface
- Must provide web configuration page
- Must support plugin extension interface

## 8. Acceptance Criteria

### 8.1 Function Acceptance

| Function Point | Acceptance Criteria |
|---------------|---------------------|
| Three-Layer Architecture Implementation | System adopts three-layer architecture design, underlying, driver layer, application layer clearly separated |
| Event Bus Function | Modules communicate through event bus, support 85+ event types |
| Driver Registry Function | Support dynamic driver management, automatic detection and device discovery |
| Time Display | Can correctly display current time, automatically synchronize NTP time |
| Weather Display | Can correctly display current weather and 5-day weather forecast |
| Lunar Calendar Display | Can correctly display lunar date, solar terms, Heavenly Stems and Earthly Branches year |
| Geographic Location Management | Can automatically detect geographic location, can also be manually configured |
| Sensor Data Display | Can correctly display sensor data |
| Web Configuration | Can configure and manage device through web page |
| Firmware Update | Can update firmware through OTA |
| Error Handling | System can automatically recover or enter safe mode when errors occur |
| Plugin Function | Can dynamically load and unload plugins |

### 8.2 Performance Acceptance

| Performance Indicator | Acceptance Criteria |
|-----------------------|---------------------|
| Startup Time | Device startup to normal content display time < 10 seconds |
| Data Update Time | Time from requesting data to displaying update < 5 seconds |
| Response Time | Web page response time < 2 seconds |
| Battery Life | Battery life when using lithium battery power supply > 30 days |
| System Stability | System continuous operation time > 30 days |

### 8.3 Reliability Acceptance

| Reliability Indicator | Acceptance Criteria |
|-----------------------|---------------------|
| Data Update Success Rate | > 99% |
| Error Recovery Capability | System error automatic recovery success rate > 95% |
| Backup API Support | When main API fails, backup API switching success rate > 99% |

## 9. Risk Assessment

| Risk Point | Probability | Impact Level | Mitigation Measures |
|------------|-------------|--------------|---------------------|
| Unstable Network Connection | Medium | High | Increase network reconnection mechanism, optimize network request strategy |
| API Request Failure | Medium | Medium | Implement backup API mechanism, increase request retry times |
| Firmware Update Failure | Low | High | Implement firmware backup and recovery mechanism, increase update verification |
| E-Ink Display Abnormality | Low | Medium | Optimize display driver, increase display self-check mechanism |
| Unstable Power Supply | Low | High | Increase power management and protection mechanism |
| Insufficient Memory | Low | High | Optimize memory usage, increase memory monitoring and management |
| Three-Layer Architecture Implementation Complexity | Medium | High | Detailed design documents, modular development, unit testing |

## 10. Appendix

### 10.1 Reference Documents

- ESP32 Datasheet
- Arduino ESP32 Documentation
- Eink Display Driver Documentation
- NTP Protocol Specification
- HTTP/HTTPS Protocol Specification
- IPv6 Protocol Specification
- FreeRTOS Documentation

### 10.2 Version History

| Version | Date | Author | Description |
|---------|------|--------|-------------|
| v1.0 | 2025-12-01 | iswhat | Initial version |
| v1.1 | 2025-12-26 | iswhat | Added automatic geographic location identification function, optimized API management module |
| v1.2 | 2025-12-29 | iswhat | Implemented three-layer architecture design, improved event bus and driver registry |
| v1.3.0 | 2025-12-31 | iswhat | Cross-platform compatibility optimization, low power optimization enhancement, remote control and data synchronization functions |
| v1.4.0 | 2025-12-31 | iswhat | Firmware management system optimization, enhanced Bluetooth network configuration and hotspot WiFi configuration, remote control and data synchronization functionality enhancement |
| v1.5.0 | 2026-01-13 | iswhat | Scene management system optimization, storage management system enhancement, hardware detector optimization, performance monitoring system enhancement |

### 10.3 Contact Information

- Project Homepage: https://github.com/iswhat/InkClock
- Email: contact@iswhat.com
- Community Communication: https://discord.gg/xxxxxx