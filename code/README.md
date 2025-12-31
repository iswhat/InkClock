# InkClock - 智能墨水屏时钟系统

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
   - 设备会自动下载并更新固件
   - 更新完成后，设备会自动重启

2. **GitHub更新**：
   - 设备支持从GitHub仓库自动更新固件
   - 在配置页面设置GitHub仓库地址和分支
   - 设备会定期检查更新并自动下载安装

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
│   └── api_manager.cpp/h
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

### v1.4.0 (2026-03-01)
- **增强蓝牙配网和热点方式配置WiFi网络**：
  - 优化初始状态蓝牙配网过程，增强易用性、稳定性和容错性
  - 实现AP模式自动切换，当WiFi连接失败时自动进入AP模式
  - 添加WiFi配置保存和加载功能，实现一键连接
  - 增强蓝牙配网后的WiFi连接逻辑，确保配置后能正确连接
  - 优化AP模式的SSID命名，包含设备MAC地址后4位，便于区分不同设备
  - 为AP模式添加固定密码，方便用户连接
  - 实现AP模式下的Web配置页面提示
  - 增强蓝牙配网的状态通知，实时告知配置结果

- **远程控制和数据同步功能增强**：
  - 优化远程控制API，增强错误处理和稳定性
  - 扩展数据同步API，提供更全面的数据
  - 为刷新显示API添加请求频率限制，防止系统过载
  - 增加对POST请求JSON参数的支持，提高API的灵活性和兼容性
  - 增强参数验证，防止恶意请求

### v1.3.0 (2026-02-15)
- **跨平台兼容性优化**：
  - 将所有`millis()`调用替换为`platformGetMillis()`
  - 将所有`delay()`调用替换为`platformDelay()`
  - 将`ESP.restart()`替换为`platformReset()`
  - 为ESP32特定功能（蓝牙、WiFi、rtc_gpio）添加条件编译
  - 统一时间和延迟API，提高代码跨平台兼容性

- **低功耗优化增强**：
  - 统一电源管理API调用
  - 使用平台无关的CPU频率控制
  - 为平台特定低功耗功能添加条件编译
  - 优化电池管理和低功耗策略

- **远程控制和数据同步功能**：
  - 添加远程控制API（`/api/control`）
  - 实现数据同步API（`/api/sync`）
  - 添加显示刷新API（`/api/refresh`）
  - 支持电源控制、低功耗模式切换、刷新间隔调整等远程命令

### v1.2.0 (2026-02-01)
- 添加音频播放功能
- 支持蓝牙/WiFi远程控制
- 实现自定义显示布局
- 优化系统稳定性

### v1.1.0 (2026-01-15)
- 添加股票行情显示功能
- 支持农历信息显示
- 实现人体感应和光照感应
- 优化电池管理

### v1.0.0 (2025-12-30)
- 初始版本发布
- 支持基本的时间、日期、天气显示
- 支持多种传感器和外设
- 实现低功耗优化
- 支持OTA远程更新

## 未来计划

- [ ] 支持更多硬件平台
- [ ] 实现更丰富的显示内容
- [ ] 优化低功耗算法
- [ ] 开发手机APP
- [ ] 支持更多传感器和外设
- [ ] 实现机器学习功能，如行为识别、预测分析

---

**InkClock - 让时间更智能，让生活更美好！** 🚀
