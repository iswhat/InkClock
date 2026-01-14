# InkClock - 智能墨水屏时钟系统 / Intelligent E-Ink Clock System

## 语言 / Language

- [中文](#中文)
- [English](#english)

## 中文



## 项目简介

InkClock是一款基于低功耗微控制器的智能墨水屏时钟系统，具有丰富的功能和良好的扩展性。它采用模块化设计，支持多种传感器和外设，能够显示时间、日期、天气、空气质量、股票行情等信息，并具备低功耗优化和远程更新功能。

## 功能特性

### 核心功能
- 🌍 **时间与日期**：显示公历时间、日期、星期
- 🌙 **农历信息**：显示农历日期、节气、节日
- 🌡️ **天气信息**：实时天气、温度、湿度、气压
- 💨 **空气质量**：PM2.5、CO2、VOC等环境指标
- 📈 **股票行情**：实时股票价格、涨跌幅、K线图
- 🌞 **光照感应**：自动调节屏幕亮度和刷新频率
- 👤 **人体感应**：有人时自动亮屏，无人时进入低功耗模式
- 🔋 **电池管理**：实时显示电池电量，低电量提醒
- 🔄 **自动更新**：支持OTA远程固件更新

### 扩展功能
- 📱 **手机控制**：支持蓝牙/WiFi远程控制
- 🎵 **音频播放**：支持MP3播放、语音播报
- 📷 **摄像头**：支持人脸识别、远程监控
- 🔔 **智能提醒**：定时提醒、事件提醒
- 🌐 **网络同步**：NTP时间同步、网络天气更新
- 📝 **自定义显示**：支持自定义显示内容和布局

## 硬件支持

### 微控制器
- ESP32 (推荐)
- ESP8266
- NRF52
- STM32
- RP2040

### 墨水屏显示
- 支持多种尺寸的墨水屏：2.13"、2.9"、4.2"、5.83"、7.5"等
- 支持黑白、黑白红、黑白黄等多种颜色模式

### 传感器支持
- **温度湿度**：DHT11/22、AM2302、SHT30/31/40、HTU21D、HDC1080
- **气压**：BMP180、BMP280、BME280、BME680、LPS25HB
- **空气质量**：SGP30、MQ系列(MQ-2/5/7/135)、TGS2600、RE200B
- **光照**：BH1750、GY30、TSL2561、SI1145
- **人体感应**：HC-SR501、HC-SR505
- **火焰检测**：IR火焰传感器

### 通信接口
- WiFi (ESP32/ESP8266)
- Bluetooth (ESP32/NRF52)
- BLE (ESP32/NRF52)
- I2C
- SPI
- UART

## 软件架构

### 系统架构

```
┌─────────────────────────────────────────────────────────────────────┐
│                         InkClock System                            │
├─────────┬─────────────┬─────────────┬─────────────┬─────────────────┤
│         │             │             │             │                 │
│  应用层  │    服务层   │    驱动层    │    核心层    │    硬件抽象层   │
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

### 核心模块

1. **CoreSystem**：系统核心，负责初始化、调度和管理各个模块
2. **EventBus**：事件总线，实现模块间的解耦通信
3. **DriverRegistry**：驱动注册表，负责传感器和外设的动态检测和管理
4. **PlatformAbstraction**：平台抽象层，屏蔽不同硬件平台的差异
5. **Config**：配置管理，负责系统配置的加载和保存

### 应用模块

1. **DisplayManager**：显示管理，负责墨水屏的初始化、刷新和内容更新
2. **PowerManager**：电源管理，负责电池检测、充电管理和低功耗优化
3. **SensorManager**：传感器管理，负责各种传感器的数据采集和处理
4. **WeatherManager**：天气管理，负责获取和显示天气信息
5. **StockManager**：股票管理，负责获取和显示股票行情
6. **TimeManager**：时间管理，负责NTP同步和时间显示
7. **LunarManager**：农历管理，负责农历信息的获取和显示
8. **WiFiManager**：WiFi管理，负责网络连接和配置

## 安装和配置

### 开发环境

1. **Arduino IDE**：推荐使用Arduino IDE 2.0及以上版本
2. **PlatformIO**：支持PlatformIO开发环境
3. **依赖库**：
   - Adafruit_GFX_Library
   - GxEPD2
   - DHT-sensor-library
   - Adafruit_BME280_Library
   - Adafruit_SGP30
   - ArduinoJson
   - WiFiManager
   - NTPClient

### 硬件连接

1. **墨水屏连接**：
   - VCC -> 3.3V
   - GND -> GND
   - DIN -> MOSI (GPIO 23 for ESP32)
   - CLK -> SCK (GPIO 18 for ESP32)
   - CS -> SS (GPIO 5 for ESP32)
   - DC -> GPIO 17 for ESP32
   - RST -> GPIO 16 for ESP32
   - BUSY -> GPIO 4 for ESP32

2. **传感器连接**：
   - 大多数传感器使用I2C接口，连接到SDA (GPIO 21)和SCL (GPIO 22)引脚
   - 模拟传感器连接到ADC引脚
   - 数字传感器连接到任意GPIO引脚

### 软件配置

1. **配置文件**：
   - `src/coresystem/config.h`：系统核心配置
   - `platformio.ini`：PlatformIO配置
   - `arduino_secrets.h`：WiFi密码等敏感信息

2. **主要配置项**：
   ```cpp
   // 屏幕配置
   #define DISPLAY_TYPE GxEPD2_583_T8
   #define DISPLAY_WIDTH 600
   #define DISPLAY_HEIGHT 448
   
   // 刷新间隔配置
   #define NORMAL_REFRESH_INTERVAL 60000  // 正常模式下每分钟刷新一次
   #define LOW_POWER_REFRESH_INTERVAL 300000  // 低功耗模式下每5分钟刷新一次
   
   // 传感器配置
   #define ENABLE_TEMPERATURE_SENSOR true
   #define ENABLE_HUMIDITY_SENSOR true
   #define ENABLE_PRESSURE_SENSOR true
   #define ENABLE_GAS_SENSOR true
   
   // WiFi配置
   #define WIFI_SSID "your_wifi_ssid"
   #define WIFI_PASSWORD "your_wifi_password"
   ```

## 使用方法

### 首次使用

1. 将固件烧录到开发板
2. 连接硬件设备
3. 通电后，设备会进入WiFi配置模式
4. 使用手机连接设备创建的WiFi热点（名称：InkClock-XXXX）
5. 在浏览器中访问 `192.168.4.1`，进入配置页面
6. 配置WiFi网络和其他参数
7. 保存配置后，设备会重启并连接到互联网
8. 设备会自动获取时间、天气等信息，并显示在屏幕上

### 日常使用

- **查看信息**：设备会自动循环显示不同的信息页面
- **手动切换**：按下设备上的按钮可以手动切换页面
- **人体感应**：当有人靠近时，设备会自动唤醒并显示当前时间
- **低功耗模式**：无人时，设备会自动进入低功耗模式，减少刷新频率
- **远程控制**：使用手机APP或Web界面可以远程控制设备

### 远程更新

1. **OTA更新**：
   - 在Web配置页面上传新的固件文件
   - 设备会自动下载并更新固件，支持下载和更新重试机制
   - 更新过程中进行电源稳定性、内存可用性检查
   - 更新前自动备份当前分区和关键配置
   - 更新过程中有看门狗保护，防止设备死机
   - 更新完成后，设备会自动重启
   - 支持固件完整性验证和签名验证，防止恶意固件

2. **GitHub更新**：
   - 设备支持从GitHub仓库自动更新固件
   - 在配置页面设置GitHub仓库地址和分支
   - 设备会定期检查更新并自动下载安装
   - 支持API密钥授权，只有授权设备才能进行远程更新
   - 支持更新状态实时监控和错误报告

3. **固件更新安全机制**：
   - SHA-256哈希验证，确保固件完整性
   - 固件签名验证，防止恶意固件
   - 双分区机制，支持更新失败回滚
   - 关键配置自动备份和恢复
   - 电源稳定性检查，防止低电压更新
   - 内存不足检查，防止内存溢出
   - 看门狗保护，防止更新过程中死机
   - 更新授权机制，防止未授权更新

## 开发指南

### 代码结构

```
src/
├── application/        # 应用模块
│   ├── display_manager.cpp/h
│   ├── power_manager.cpp/h
│   ├── sensor_manager.cpp/h
│   ├── weather_manager.cpp/h
│   ├── stock_manager.cpp/h
│   ├── time_manager.cpp/h
│   ├── lunar_manager.cpp/h
│   ├── wifi_manager.cpp/h
│   ├── api_manager.cpp/h
│   └── firmware_manager.cpp/h  # 固件管理模块
├── coresystem/         # 核心系统
│   ├── core_system.cpp/h
│   ├── event_bus.h
│   ├── driver_registry.h
│   ├── platform_abstraction.cpp/h
│   └── config.h
├── drivers/            # 驱动模块
│   ├── peripherals/    # 外设驱动
│   │   ├── bme280_driver.cpp/h
│   │   ├── sht30_driver.cpp/h
│   │   ├── bh1750_driver.cpp/h
│   │   ├── hc_sr501_driver.cpp/h
│   │   └── ...
│   └── audio_driver.cpp/h
├── services/           # 服务模块
│   ├── web_client.cpp/h
│   ├── message_manager.cpp/h
│   └── ...
├── extensions/         # 扩展模块
│   └── plugin_manager.cpp/h
└── main.cpp            # 主程序入口
```

### 开发流程

1. **创建新模块**：
   - 在对应的目录下创建新的.cpp和.h文件
   - 实现模块的初始化、更新和循环方法
   - 在main.cpp中添加模块的初始化和调用

2. **添加新传感器**：
   - 在drivers/peripherals目录下创建新的驱动文件
   - 继承BaseSensorDriver类
   - 实现init()、readData()、getType()等方法
   - 在DriverRegistry中注册新的传感器驱动

3. **添加新功能**：
   - 实现新功能的应用模块
   - 在EventBus中注册新的事件类型
   - 在CoreSystem中添加新模块的初始化和调度

### 低功耗优化

1. **硬件优化**：
   - 使用低功耗微控制器
   - 选择低功耗传感器
   - 优化电路设计，减少待机电流

2. **软件优化**：
   - 实现动态刷新率：根据电池电量和使用场景调整刷新率
   - 支持深度睡眠：无人时进入深度睡眠模式
   - 关闭不必要的外设：在低功耗模式下关闭WiFi、蓝牙等外设
   - 优化代码：减少CPU占用和内存使用

## 贡献指南

欢迎大家参与InkClock项目的开发和贡献！

### 贡献方式

1. **提交Issue**：报告bug、提出新功能建议
2. **提交Pull Request**：修复bug、实现新功能
3. **完善文档**：更新README、添加注释、编写教程
4. **测试反馈**：测试硬件兼容性、报告测试结果

### 代码规范

1. **命名规范**：
   - 类名：使用大驼峰命名法，如`DisplayManager`
   - 方法名：使用小驼峰命名法，如`initDisplay()`
   - 变量名：使用小驼峰命名法，如`screenWidth`
   - 常量名：使用全大写，下划线分隔，如`MAX_REFRESH_INTERVAL`

2. **注释规范**：
   - 类和方法使用Doxygen风格注释
   - 复杂代码段添加详细注释
   - 关键变量和常量添加注释

3. **代码结构**：
   - 每个文件只包含一个主要类
   - 方法长度不超过100行
   - 合理使用命名空间
   - 避免全局变量，使用单例模式或依赖注入

## 许可证

InkClock项目采用MIT许可证，详见LICENSE文件。

## 联系方式

- **GitHub**：[https://github.com/yourusername/InkClock](https://github.com/yourusername/InkClock)
- **邮箱**：your.email@example.com
- **论坛**：[https://forum.inkclock.com](https://forum.inkclock.com)

## 致谢

感谢所有为InkClock项目做出贡献的开发者和用户！

## 版本历史

### v1.4.0 (2025-12-31)

**固件管理系统优化**：
- **固件完整性验证**：
  - 添加SHA-256哈希计算和验证功能
  - 从firmware_info.json中读取预期哈希值并验证
  - 确保固件文件在传输和存储过程中未被篡改

- **固件签名验证机制**：
  - 实现固件签名验证框架
  - 从固件信息中获取签名和公钥
  - 支持ECDSA/RSA等加密算法
  - 防止恶意固件的安装

- **双分区更新和回滚机制**：
  - 实现分区管理功能：获取当前分区、切换分区、备份分区和恢复分区
  - 添加固件回滚功能，支持自动检测回滚需求
  - 更新前自动备份当前分区
  - 更新后切换到新分区，失败时可回滚到旧分区

- **更新过程中的看门狗保护**：
  - 初始化看门狗定时器（30秒超时）
  - 在更新关键步骤中定期重置看门狗
  - 防止更新过程中设备死机导致变砖
  - 更新完成后禁用看门狗

- **关键配置备份和恢复机制**：
  - 在更新前自动备份关键配置（WiFi配置、设备ID等）
  - 设备启动时检查配置有效性，无效时自动恢复
  - 确保固件更新不会丢失重要配置数据

- **电源稳定性检查**：
  - 实现电源稳定性检查，确保在安全电压下进行更新
  - 多次检查（5次）确保电源稳定
  - 拒绝在低电压或不稳定电源下进行固件更新

- **内存管理优化和内存不足检查**：
  - 添加内存使用优化功能
  - 在更新前检查可用内存，确保有足够内存进行更新
  - 防止因内存不足导致更新失败

- **增强的更新状态报告和错误处理**：
  - 添加详细的错误代码枚举
  - 实现状态回调机制，支持实时监控更新进度
  - 为每个错误情况设置唯一的错误代码
  - 提供获取最后错误代码的公共方法

- **更新授权机制**：
  - 添加API密钥验证功能
  - 支持设置和获取API密钥
  - 只有授权的请求才能进行远程OTA更新
  - 支持无API密钥模式（方便本地测试）

- **OTA更新重试机制**：
  - 为固件下载添加重试逻辑（最多3次）
  - 为固件更新添加重试逻辑（最多2次）
  - 实现超时处理，防止无限等待
  - 智能重试机制，只在特定错误情况下重试
  - 提高OTA更新的成功率

**增强蓝牙配网和热点方式配置WiFi网络**：
  - 优化初始状态蓝牙配网过程，增强易用性、稳定性和容错性
  - 实现AP模式自动切换，当WiFi连接失败时自动进入AP模式
  - 添加WiFi配置保存和加载功能，实现一键连接
  - 增强蓝牙配网后的WiFi连接逻辑，确保配置后能正确连接
  - 优化AP模式的SSID命名，包含设备MAC地址后4位，便于区分不同设备
  - 为AP模式添加固定密码，方便用户连接
  - 实现AP模式下的Web配置页面提示
  - 增强蓝牙配网的状态通知，实时告知配置结果

**远程控制和数据同步功能增强**：
  - 优化远程控制API，增强错误处理和稳定性
  - 扩展数据同步API，提供更全面的数据
  - 为刷新显示API添加请求频率限制，防止系统过载
  - 增加对POST请求JSON参数的支持，提高API的灵活性和兼容性
  - 增强参数验证，防止恶意请求

### v1.5.0 (2026-01-13)

**场景管理系统优化**：
- **简化场景模式**：将场景系统简化为三种模式：normal（正常）、interactive（交互）、sleep（睡眠）
- **场景配置持久化**：实现saveScenes/loadScenes函数，使用SPIFFS和ArduinoJson持久化场景配置
- **场景配置应用修复**：修复applySceneConfig函数，实际应用场景配置到各个模块
- **用户活动检测优化**：优化recordUserActivity函数，正确处理活动计数和场景切换

**存储管理系统增强**：
- **跨介质数据备份**：实现backupData函数，支持跨存储介质的数据备份
- **数据压缩功能**：实现compressData/decompressData函数，使用Run-Length Encoding (RLE)算法压缩数据

**硬件检测器优化**：
- **CPU使用率检测**：更新CpuDetector，在ESP32上使用esp_cpu_utilization_get()获取真实CPU使用率数据
- **存储使用检测**：更新StorageDetector，使用SPIFFS.info()获取真实存储使用数据
- **网络信号检测**：更新NetworkDetector，使用WiFi.RSSI()获取真实网络信号强度
- **电池电压检测**：更新PowerDetector，在ESP32上使用ADC获取真实电池电压

**性能监控系统增强**：
- **性能数据发布**：实现publishPerformanceDataEvent函数，使用EventBus发布性能数据
- **警报事件发布**：实现publishAlertEvent函数，使用EventBus发布警报事件

## English

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
│   ├── display_manager.cpp/h
│   ├── power_manager.cpp/h
│   ├── sensor_manager.cpp/h
│   ├── weather_manager.cpp/h
│   ├── stock_manager.cpp/h
│   ├── time_manager.cpp/h
│   ├── lunar_manager.cpp/h
│   ├── wifi_manager.cpp/h
│   ├── api_manager.cpp/h
│   └── firmware_manager.cpp/h  # Firmware management module
├── coresystem/         # Core system
│   ├── core_system.cpp/h
│   ├── event_bus.h
│   ├── driver_registry.h
│   ├── platform_abstraction.cpp/h
│   └── config.h
├── drivers/            # Driver modules
│   ├── peripherals/    # Peripheral drivers
│   │   ├── bme280_driver.cpp/h
│   │   ├── sht30_driver.cpp/h
│   │   ├── bh1750_driver.cpp/h
│   │   ├── hc_sr501_driver.cpp/h
│   │   └── ...
│   └── audio_driver.cpp/h
├── services/           # Service modules
│   ├── web_client.cpp/h
│   ├── message_manager.cpp/h
│   └── ...
├── extensions/         # Extension modules
│   └── plugin_manager.cpp/h
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
- [ ] 实现更丰富的显示内容
- [ ] 优化低功耗算法
- [ ] 开发手机APP
- [ ] 支持更多传感器和外设
