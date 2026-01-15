# InkClock - 智能墨水屏时钟系统

[English Version](README_EN.md)

## 项目简介

InkClock是一款基于多种开发板的智能墨水屏时钟系统，支持ESP32、ESP8266、NRF52、STM32、RP2040等多种微控制器平台，具有低功耗、高清晰度、可远程配置等特点。系统支持显示时间、日期、天气、空气质量、股票行情等信息，并具备人体感应、光照感应、低功耗优化等功能。

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
- 📱 **手机控制**：支持蓝牙/WiFi远程控制
- 🎵 **音频播放**：支持MP3播放、语音播报
- 🔔 **智能提醒**：定时提醒、事件提醒
- 🌐 **网络同步**：NTP时间同步、网络天气更新
- 📝 **自定义显示**：支持自定义显示内容和布局

### 硬件支持

#### 微控制器
- ESP32 (推荐)
- ESP8266
- NRF52
- STM32
- RP2040

#### 墨水屏显示
- 支持多种尺寸的墨水屏：2.13"、2.9"、4.2"、5.83"、7.5"等
- 支持黑白、黑白红、黑白黄等多种颜色模式

#### 传感器支持
- **温度湿度**：DHT11/22、AM2302、SHT30/31/40、HTU21D、HDC1080、BME280、BME680
- **气压**：BMP180、BMP280、BME280、BME680、LPS25HB
- **空气质量**：SGP30、MQ系列(MQ-2/5/7/135)、TGS2600、RE200B
- **光照**：BH1750、GY30、TSL2561、SI1145
- **人体感应**：HC-SR501、HC-SR505、RE200B、LD2410
- **火焰检测**：IR火焰传感器

## 文档目录

以下是项目的详细文档，存放于 `doc/` 目录下：

| 文档名称 | 描述 | 链接 |
|---------|------|------|
| 项目说明文档 | 项目总体说明和功能介绍 | [doc/项目说明文档.md](doc/项目说明文档.md) |
| 产品需求文档 | 产品需求分析和功能规划 | [doc/产品需求文档.md](doc/产品需求文档.md) |
| 系统二次开发说明 | 系统架构和二次开发指南 | [doc/系统二次开发说明.md](doc/系统二次开发说明.md) |
| 驱动程序说明 | 硬件驱动程序开发和使用说明 | [doc/驱动程序说明.md](doc/驱动程序说明.md) |
| 模拟器和测试环境设置指南 | 模拟器和测试环境的设置方法 | [doc/模拟器和测试环境设置指南.md](doc/模拟器和测试环境设置指南.md) |
| DIY制作指南 | 设备DIY制作和组装指南 | [doc/DIY制作指南.md](doc/DIY制作指南.md) |
| 硬件物料选型指南 | 硬件组件选型参考 | [doc/硬件物料选型指南.md](doc/硬件物料选型指南.md) |
| 电路设计指南 | 电路设计和PCB布局指南 | [doc/电路设计指南.md](doc/电路设计指南.md) |
| 开发硬件说明 | 开发板和硬件组件详细说明 | [doc/开发硬件说明.md](doc/开发硬件说明.md) |

## 项目结构

```
InkClock/
├── code/           # 源代码目录
│   ├── src/        # 固件源代码
│   │   ├── application/    # 应用模块
│   │   │   ├── api_manager.cpp/h             # API请求管理
│   │   │   ├── display_manager.cpp/h          # 显示内容管理
│   │   │   ├── feedback_manager.cpp/h         # 操作反馈管理
│   │   │   ├── geo_manager.cpp/h              # 地理位置管理
│   │   │   ├── lunar_manager.cpp/h            # 农历信息管理
│   │   │   ├── message_manager.cpp/h          # 消息管理
│   │   │   ├── power_manager.cpp/h            # 电源管理和低功耗控制
│   │   │   ├── scene_manager.cpp/h            # 场景模式管理
│   │   │   ├── sensor_manager.cpp/h           # 传感器数据管理
│   │   │   ├── stock_manager.cpp/h            # 股票数据管理
│   │   │   ├── time_manager.cpp/h             # 时间同步管理
│   │   │   ├── weather_manager.cpp/h          # 天气数据管理
│   │   │   ├── web_client.cpp/h               # Web客户端通信
│   │   │   ├── web_server.cpp/h               # 本地Web服务器
│   │   │   └── wifi_manager.cpp/h             # WiFi连接管理
│   │   ├── coresystem/     # 核心系统
│   │   │   ├── arduino_compat.cpp/h           # Arduino兼容性支持
│   │   │   ├── base_plugin.h                  # 插件基类
│   │   │   ├── config.h                       # 系统配置
│   │   │   ├── config_manager.cpp/h           # 配置管理
│   │   │   ├── core_system.cpp/h              # 核心系统管理
│   │   │   ├── dependency_injection.cpp/h     # 依赖注入
│   │   │   ├── driver_registry.h              # 驱动注册表
│   │   │   ├── error_handling.cpp/h           # 错误处理
│   │   │   ├── event_bus.h                    # 事件总线
│   │   │   ├── feature_manager.cpp/h          # 功能管理
│   │   │   ├── font_manager.cpp/h             # 字体管理
│   │   │   ├── hardware_detector.cpp/h        # 硬件检测
│   │   │   ├── i18n_manager.cpp/h             # 国际化支持
│   │   │   ├── icore_system.h                 # 核心系统接口
│   │   │   ├── memory_pool.cpp/h              # 内存池管理
│   │   │   ├── module_registry.cpp/h          # 模块注册表
│   │   │   ├── network_manager.cpp/h          # 网络管理
│   │   │   ├── performance_monitor.cpp/h      # 性能监控
│   │   │   ├── platform_abstraction.cpp/h     # 平台抽象层
│   │   │   ├── plugin_manager.cpp/h           # 插件管理
│   │   │   ├── spiffs_manager.h               # SPIFFS文件系统管理
│   │   │   ├── storage_manager.cpp/h          # 存储管理
│   │   │   └── tf_card_manager.h              # TF卡管理
│   │   ├── drivers/        # 驱动模块
│   │   │   ├── peripherals/    # 外设驱动
│   │   │   │   ├── am2302_driver.cpp/h        # AM2302温湿度传感器驱动
│   │   │   │   ├── base_mq_sensor_driver.h    # MQ气体传感器基类
│   │   │   │   ├── base_sensor_driver.cpp/h   # 传感器驱动基类
│   │   │   │   ├── bh1750_driver.cpp/h        # BH1750光照传感器驱动
│   │   │   │   ├── bme280_driver.cpp/h        # BME280温湿度气压传感器驱动
│   │   │   │   ├── bme680_driver.cpp/h        # BME680温湿度气压气体传感器驱动
│   │   │   │   ├── bmp388_driver.cpp/h        # BMP388气压传感器驱动
│   │   │   │   ├── dht22_driver.cpp/h         # DHT22温湿度传感器驱动
│   │   │   │   ├── display_driver.h           # 显示驱动接口
│   │   │   │   ├── eink_display.cpp/h         # 墨水屏显示管理
│   │   │   │   ├── eink_driver.cpp/h          # 墨水屏驱动
│   │   │   │   ├── gy30_driver.cpp/h          # GY30光照传感器驱动
│   │   │   │   ├── hc_sr501_driver.cpp/h      # HC-SR501人体感应传感器驱动
│   │   │   │   ├── hc_sr505_driver.cpp/h      # HC-SR505人体感应传感器驱动
│   │   │   │   ├── hdc1080_driver.cpp/h       # HDC1080温湿度传感器驱动
│   │   │   │   ├── htu21d_driver.cpp/h        # HTU21D温湿度传感器驱动
│   │   │   │   ├── ir_flame_driver.cpp/h      # IR火焰传感器驱动
│   │   │   │   ├── ld2410_driver.cpp/h        # LD2410人体存在传感器驱动
│   │   │   │   ├── lps25hb_driver.cpp/h       # LPS25HB气压传感器驱动
│   │   │   │   ├── mq135_driver.cpp/h         # MQ135空气质量传感器驱动
│   │   │   │   ├── mq2_driver.cpp/h           # MQ2烟雾传感器驱动
│   │   │   │   ├── mq5_driver.cpp/h           # MQ5燃气传感器驱动
│   │   │   │   ├── mq7_driver.cpp/h           # MQ7一氧化碳传感器驱动
│   │   │   │   ├── re200b_driver.cpp/h        # RE200B人体感应传感器驱动
│   │   │   │   ├── sensor_driver.h            # 传感器驱动接口
│   │   │   │   ├── sgp30_driver.cpp/h         # SGP30空气质量传感器驱动
│   │   │   │   ├── sht20_driver.cpp/h         # SHT20温湿度传感器驱动
│   │   │   │   ├── sht21_driver.cpp/h         # SHT21温湿度传感器驱动
│   │   │   │   ├── sht30_driver.cpp/h         # SHT30温湿度传感器驱动
│   │   │   │   ├── sht40_driver.cpp/h         # SHT40温湿度传感器驱动
│   │   │   │   ├── si1145_driver.cpp/h        # SI1145光照传感器驱动
│   │   │   │   ├── si7021_driver.cpp/h        # SI7021温湿度传感器驱动
│   │   │   │   ├── tgs2600_driver.cpp/h       # TGS2600气体传感器驱动
│   │   │   │   └── tsl2561_driver.cpp/h       # TSL2561光照传感器驱动
│   │   │   ├── audio_driver.cpp/h             # 音频驱动
│   │   ├── extensions/     # 扩展模块
│   │   │   └── plugin_manager.cpp/h           # 扩展插件管理
│   │   ├── plugins/        # 插件模块
│   │   │   └── example_plugin.cpp/h           # 示例插件
│   │   ├── audio_manager.cpp/h                # 音频管理
│   │   ├── bluetooth_manager.cpp/h            # 蓝牙管理
│   │   ├── button_manager.cpp/h               # 按键管理
│   │   ├── camera_manager.cpp/h               # 摄像头管理
│   │   ├── firmware_manager.cpp/h             # 固件更新管理
│   │   ├── ipv6_server.cpp/h                  # IPv6服务器
│   │   ├── touch_manager.cpp/h                # 触摸管理
│   │   └── main.cpp                           # 主程序入口
│   ├── config/         # 配置文件目录
│   ├── doc/            # 文档目录
│   ├── hardware/       # 硬件相关文件
│   ├── tool/           # 工具脚本
│   ├── webserver/      # Web服务器代码
│   │   ├── config/     # Web服务器配置
│   │   ├── public/     # Web服务器静态文件
│   │   ├── src/        # Web服务器源代码
│   │   │   ├── Controller/     # 控制器
│   │   │   ├── Model/          # 模型
│   │   │   ├── Service/        # 服务
│   │   │   └── Utils/          # 工具
│   │   └── index.php   # Web服务器入口
│   └── README.md       # 项目说明文档
```

## 快速开始

### 1. 硬件准备

1. **开发板选择**：选择支持的开发板（推荐ESP32-S3）
2. **墨水屏选择**：选择合适尺寸的墨水屏
3. **传感器选择**：根据需要选择传感器
4. **其他组件**：电源模块、外壳等

### 2. 软件设置

1. **安装开发环境**：
   - Arduino IDE 2.0+ 或 PlatformIO
   - 相关依赖库

2. **配置固件**：
   - 修改 `code/src/coresystem/config.h` 中的配置项
   - 设置WiFi信息和其他参数

3. **编译上传**：
   - 编译固件
   - 上传到开发板

### 3. 首次使用

1. 通电后，设备会进入WiFi配置模式
2. 使用手机连接设备创建的WiFi热点（名称：InkClock-XXXX）
3. 在浏览器中访问 `192.168.4.1`，进入配置页面
4. 配置WiFi网络和其他参数
5. 保存配置后，设备会重启并连接到互联网
6. 设备会自动获取时间、天气等信息，并显示在屏幕上

### 4. 日常使用

- **查看信息**：设备会自动循环显示不同的信息页面
- **手动切换**：按下设备上的按钮可以手动切换页面
- **人体感应**：当有人靠近时，设备会自动唤醒并显示当前时间
- **低功耗模式**：无人时，设备会自动进入低功耗模式，减少刷新频率
- **远程控制**：使用手机APP或Web界面可以远程控制设备

## 固件更新

### 1. OTA更新

1. 在Web配置页面上传新的固件文件
2. 设备会自动下载并更新固件
3. 更新完成后，设备会自动重启

### 2. GitHub更新

1. 在配置页面设置GitHub仓库地址和分支
2. 设备会定期检查更新并自动下载安装

### 3. 固件更新安全机制

- SHA-256哈希验证，确保固件完整性
- 固件签名验证，防止恶意固件
- 双分区机制，支持更新失败回滚
- 自动备份和恢复关键配置
- 电源稳定性检查，防止低电压更新
- 内存不足检查，防止内存溢出

## 开发环境设置

### 1. Arduino IDE

1. 安装Arduino IDE 2.0+
2. 安装ESP32/ESP8266板支持
3. 安装依赖库：
   - Adafruit_GFX_Library
   - GxEPD2
   - DHT-sensor-library
   - Adafruit_BME280_Library
   - ArduinoJson
   - WiFiManager
   - NTPClient

### 2. PlatformIO

1. 安装Visual Studio Code
2. 安装PlatformIO插件
3. 打开项目文件夹
4. PlatformIO会自动安装依赖库

## 故障排除

### 1. 常见问题

| 问题 | 可能原因 | 解决方案 |
|------|---------|----------|
| 设备无法启动 | 电源问题 | 检查电源连接和电压 |
| WiFi无法连接 | WiFi密码错误 | 重新配置WiFi信息 |
| 屏幕不显示 | 墨水屏连接错误 | 检查墨水屏连接 |
| 传感器无数据 | 传感器连接错误 | 检查传感器连接 |
| 固件更新失败 | 网络问题 | 检查网络连接并重试 |

### 2. 恢复出厂设置

- **软恢复**：通过Web页面触发
- **硬恢复**：长按按键5秒以上触发

## 技术支持

如有问题，请参考相关文档或联系项目维护者。

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
   - 方法名：使用小驼峰命名法，如`updateDisplay`
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

## 致谢

感谢所有为InkClock项目做出贡献的开发者和用户！
