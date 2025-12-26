/**
 * @file main.cpp
 * @brief 家用网络智能墨水屏万年历主程序
 * @author iswhat
 * @date 2025-12-26
 * @version 1.1
 * 
 * 该文件是家用网络智能墨水屏万年历的主入口文件，负责初始化所有模块、
 * 调度各模块的循环任务，并处理模块间的交互。
 * 
 * 主要功能：
 * 1. 系统初始化和模块管理
 * 2. 任务调度和循环执行
 * 3. 异常处理和系统容错
 * 4. 系统安全模式保障
 */

#include <Arduino.h>
#include "config.h"
#include "display_manager.h"
#include "display_driver.h"
#include "eink_driver.h"
#include "wifi_manager.h"
#include "time_manager.h"
#include "weather_manager.h"
#include "sensor_manager.h"
#include "audio_manager.h"
#include "button_manager.h"
#include "message_manager.h"
#include "stock_manager.h"
#include "plugin_manager.h"
#include "bluetooth_manager.h"
#include "web_server.h"
#include "power_manager.h"
#include "touch_manager.h"
#include "camera_manager.h"
#include "web_client.h"
#include "ipv6_server.h"
#include "firmware_manager.h"
#include "lunar_manager.h"
#include "api_manager.h"
#include "geo_manager.h"
#include "driver_registry.h"

/**
 * @brief 全局对象实例
 * 
 * 所有功能模块的全局实例，用于在setup和loop函数中访问各个模块
 */// 全局对象实例
DisplayManager displayManager;       // 显示管理模块
WiFiManager wifiManager;             // WiFi管理模块
TimeManager timeManager;             // 时间管理模块
LunarManager lunarManager;           // 农历管理模块
WeatherManager weatherManager;       // 天气管理模块
SensorManager sensorManager;         // 传感器管理模块
AudioManager audioManager;           // 音频管理模块
ButtonManager buttonManager;         // 按键管理模块
MessageManager messageManager;       // 消息管理模块
StockManager stockManager;           // 股票管理模块
PluginManager pluginManager;         // 插件管理模块
PowerManager powerManager;           // 电源管理模块
BluetoothManager bluetoothManager;   // 蓝牙管理模块
WebServerManager webServerManager;   // Web服务器模块（合并了IPv6Server功能）
TouchManager touchManager;           // 触摸管理模块
CameraManager cameraManager;         // 摄像头管理模块
WebClient webClient;                 // Web客户端模块
FirmwareManager firmwareManager;     // 固件管理模块
APIManager apiManager;               // API管理模块
GeoManager geoManager;               // 地理位置管理模块

/**
 * @brief 初始化函数
 * 
 * 设备启动时执行的初始化函数，负责初始化所有功能模块
 * 执行顺序：
 * 1. 串口初始化
 * 2. 固件管理器初始化（硬件自动检测）
 * 3. 显示驱动初始化
 * 4. 蓝牙管理模块初始化
 * 5. 传感器和其他不需要WiFi的模块初始化
 * 6. Web客户端初始化
 * 7. WiFi连接（如果未配置则等待蓝牙配置）
 * 8. 需要WiFi的模块初始化
 * 9. Web服务器和IPv6服务器初始化
 * 10. 初始数据更新
 * 11. 初始页面显示
 */
void setup() {
  // 初始化串口通信，用于调试输出
  Serial.begin(115200);
  delay(1000);
  
  // 打印启动信息
  Serial.println("===== 家用网络智能墨水屏万年历 ====");
  
  try {
    // 初始化固件管理器，进行硬件自动检测和驱动适配
    firmwareManager.init();
    
    // 初始化电源管理，影响系统运行模式
    powerManager.init();        // 电源管理和低功耗控制
    
    // 注册所有可用的驱动到驱动注册表
    Serial.println("注册硬件驱动...");
    
    // 注册墨水屏驱动
    registerDisplayDriver<EinkDriver>();
    
    // 注册所有传感器驱动到驱动注册表
    Serial.println("注册传感器驱动...");
    
    // 温湿度传感器驱动
    registerSensorDriver<DHT22Driver>();
    registerSensorDriver<SHT30Driver>();
    registerSensorDriver<AM2302Driver>();
    registerSensorDriver<SHT20Driver>();
    registerSensorDriver<SHT21Driver>();
    registerSensorDriver<SHT40Driver>();
    registerSensorDriver<HDC1080Driver>();
    registerSensorDriver<HTU21DDriver>();
    registerSensorDriver<SI7021Driver>();
    registerSensorDriver<BME280Driver>();
    registerSensorDriver<BME680Driver>();
    registerSensorDriver<LPS25HBDriver>();
    registerSensorDriver<BMP388Driver>();
    
    // 人体感应传感器驱动
    registerSensorDriver<HC_SR501Driver>();
    registerSensorDriver<HC_SR505Driver>();
    registerSensorDriver<RE200BDriver>();
    registerSensorDriver<LD2410Driver>();
    
    // 光照传感器驱动
    registerSensorDriver<BH1750Driver>();
    registerSensorDriver<TSL2561Driver>();
    registerSensorDriver<GY30Driver>();
    registerSensorDriver<SI1145Driver>();
    
    // 气体传感器驱动
    registerSensorDriver<MQ2Driver>();
    registerSensorDriver<MQ5Driver>();
    registerSensorDriver<MQ7Driver>();
    registerSensorDriver<MQ135Driver>();
    
    // 火焰传感器驱动
    registerSensorDriver<IRFlameDriver>();
    
    Serial.println("传感器驱动注册完成");
    
    // 使用驱动注册表自动检测并获取显示驱动
    DriverRegistry* registry = DriverRegistry::getInstance();
    IDisplayDriver* displayDriver = nullptr;
    
    // 如果配置了固定的显示类型，则直接获取对应驱动
    #if DISPLAY_TYPE != EINK_75_INCH
      displayDriver = registry->getDisplayDriver(DISPLAY_TYPE);
      Serial.print("使用配置的显示驱动: ");
    #else
      // 否则自动检测显示驱动
      displayDriver = registry->autoDetectDisplayDriver();
      Serial.print("自动检测显示驱动: ");
    #endif
    
    if (displayDriver == nullptr) {
      // 如果无法获取显示驱动，使用默认的墨水屏驱动
      Serial.println("未找到匹配的显示驱动，使用默认墨水屏驱动");
      displayDriver = new EinkDriver();
    } else {
      Serial.println("成功获取显示驱动");
    }
    
    // 设置显示驱动
    displayManager.setDisplayDriver(displayDriver);
    
    // 初始化显示管理器，显示启动画面
    displayManager.init();
    displayManager.showSplashScreen();
    
    // 初始化输入设备，用于用户交互
    buttonManager.init();       // 按键事件处理
    touchManager.init();        // 触摸事件处理（仅支持ESP32-S3系列）
    
    // 初始化蓝牙管理模块，用于首次WiFi配置
    bluetoothManager.init();
    
    // 初始化本地传感器和不需要网络的模块
    sensorManager.init();       // 温湿度传感器管理
    audioManager.init();        // 音频录制和播放管理
    messageManager.init();      // 消息接收和存储
    pluginManager.init();       // 插件加载和管理
    cameraManager.init();       // 摄像头管理（仅支持ESP32-S3系列，资源消耗大）
    
    // 初始化网络通信相关模块
    webClient.init();           // Web客户端，用于与云端服务器通信
    apiManager.init();          // API管理器，用于统一处理所有外部API请求
    geoManager.init();          // 地理位置管理器，用于自动检测和管理地理位置
    
    // 检查是否已经配置了WiFi，如果没有，等待蓝牙配置
    if (!wifiManager.isConnected()) {
      Serial.println("等待蓝牙WiFi配置...");
      
      // 在墨水屏上显示蓝牙配置提示
      displayManager.showMessage("等待蓝牙WiFi配置...", 10000);
      
      // 等待蓝牙配置WiFi信息
      while (!bluetoothManager.isWiFiConfigured()) {
        bluetoothManager.loop();
        delay(1000);
      }
      
      // 使用蓝牙配置的WiFi信息连接网络
      String ssid = bluetoothManager.getWiFiSSID();
      String password = bluetoothManager.getWiFiPassword();
      wifiManager.init();
      wifiManager.connect(ssid, password);
    } else {
      // 已经连接了WiFi，直接初始化WiFi模块
      wifiManager.init();
    }
    
    // 初始化需要WiFi的模块
    timeManager.init();         // NTP时间同步
    weatherManager.init();      // 天气数据获取
    stockManager.init();        // 股票数据获取和显示（依赖网络）
    lunarManager.init();        // 农历管理模块（依赖网络）
    
    // 初始化Web服务器，提供设备管理API和IPv6推送功能
    webServerManager.init();
    
    // 更新初始数据
    timeManager.update();       // 更新时间
    weatherManager.update();    // 更新天气
    sensorManager.update();     // 更新传感器数据
    stockManager.update();      // 更新股票数据
    lunarManager.update();      // 更新农历数据
    
    // 显示初始页面
    displayManager.updateDisplay();
    
    // 打印初始化完成信息
    Serial.println("初始化完成");
  } catch (const std::exception& e) {
    Serial.print("初始化异常: ");
    Serial.println(e.what());
    
    // 显示错误信息
    displayManager.showMessage("初始化失败，进入安全模式", 5000);
    
    // 仅保留基本功能，确保可以进行刷机或更新
    firmwareManager.init();
    webServerManager.init();
  }
}

/**
 * @brief 主循环函数
 * 
 * 设备启动后持续执行的主循环，负责调度各模块的循环任务
 * 执行顺序：
 * 1. 各模块的循环任务处理
 * 2. 根据电源管理建议更新显示
 * 3. 定期更新数据
 */
/**
 * @brief 主循环函数
 * 
 * 设备启动后持续执行的主循环，负责调度各模块的循环任务
 * 执行顺序：
 * 1. 各模块的循环任务处理
 * 2. 根据电源管理建议更新显示
 * 3. 定期更新数据
 */
void loop() {
  static unsigned long lastWatchdogReset = 0;
  static const unsigned long WATCHDOG_TIMEOUT = 60000; // 看门狗超时时间，60秒
  
  try {
    // 重置软件看门狗
    lastWatchdogReset = millis();
    
    // 处理各个模块的循环任务，每个模块都用try-catch包裹，确保单个模块崩溃不会影响整个系统
    // 核心功能模块 - 优先执行
    try {
      wifiManager.loop();           // WiFi状态监测和重连（核心网络功能）
    } catch (const std::exception& e) {
      Serial.print("WiFi模块异常: ");
      Serial.println(e.what());
      Serial.flush();
    }
    
    try {
      powerManager.loop();          // 电源状态监测和低功耗控制（影响系统运行模式）
    } catch (const std::exception& e) {
      Serial.print("电源模块异常: ");
      Serial.println(e.what());
    }
    
    // 实时交互模块 - 需要快速响应
    try {
      buttonManager.loop();         // 按键事件检测（实时交互）
    } catch (const std::exception& e) {
      Serial.print("按键模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      touchManager.loop();          // 触摸事件检测（实时交互）
    } catch (const std::exception& e) {
      Serial.print("触摸模块异常: ");
      Serial.println(e.what());
    }
    
    // 时间基准模块 - 影响其他模块的时间戳
    try {
      timeManager.loop();           // 时间更新和同步（系统时间基准）
    } catch (const std::exception& e) {
      Serial.print("时间模块异常: ");
      Serial.println(e.what());
      Serial.flush();
    }
    
    // 地理位置模块 - 影响天气、股票等数据
    try {
      geoManager.loop();            // 地理位置更新和管理
    } catch (const std::exception& e) {
      Serial.print("地理位置模块异常: ");
      Serial.println(e.what());
    }
    
    // 数据获取模块 - 顺序执行，避免并发网络请求
    try {
      sensorManager.loop();         // 传感器数据采集（本地传感器，响应快）
    } catch (const std::exception& e) {
      Serial.print("传感器模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      lunarManager.loop();          // 农历管理模块循环
    } catch (const std::exception& e) {
      Serial.print("农历模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      weatherManager.loop();        // 天气数据管理
    } catch (const std::exception& e) {
      Serial.print("天气模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      stockManager.loop();          // 股票数据管理
    } catch (const std::exception& e) {
      Serial.print("股票模块异常: ");
      Serial.println(e.what());
    }
    
    // 扩展功能模块 - 资源消耗较大，放在后面
    try {
      audioManager.loop();          // 音频播放控制
    } catch (const std::exception& e) {
      Serial.print("音频模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      bluetoothManager.loop();      // 蓝牙连接管理
    } catch (const std::exception& e) {
      Serial.print("蓝牙模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      cameraManager.loop();         // 摄像头状态管理（资源消耗大）
    } catch (const std::exception& e) {
      Serial.print("摄像头模块异常: ");
      Serial.println(e.what());
    }
    
    // 网络服务模块
    try {
      webServerManager.loop();      // Web服务器请求处理（包括IPv6推送功能）
    } catch (const std::exception& e) {
      Serial.print("Web服务器模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      webClient.loop();             // 与云端服务器通信
    } catch (const std::exception& e) {
      Serial.print("Web客户端模块异常: ");
      Serial.println(e.what());
    }
    
    // 后台功能模块
    try {
      messageManager.loop();        // 消息接收和处理
    } catch (const std::exception& e) {
      Serial.print("消息模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      pluginManager.loop();         // 插件更新和显示
    } catch (const std::exception& e) {
      Serial.print("插件模块异常: ");
      Serial.println(e.what());
    }
    
    try {
      firmwareManager.loop();       // 固件更新检查（后台功能）
    } catch (const std::exception& e) {
      Serial.print("固件管理模块异常: ");
      Serial.println(e.what());
    }
    
    // 根据power_manager的建议更新显示，减少不必要的刷新
    try {
      if (powerManager.shouldUpdateDisplay()) {
        // 使用局部刷新更新显示，降低功耗
        displayManager.updateDisplayPartial();
      }
    } catch (const std::exception& e) {
      Serial.print("显示更新异常: ");
      Serial.println(e.what());
    }
    
    // 定期更新数据 - 为每个模块使用独立的更新间隔，避免不必要的数据刷新
    static unsigned long lastTimeUpdate = 0;
    static unsigned long lastWeatherUpdate = 0;
    static unsigned long lastSensorUpdate = 0;
    static unsigned long lastStockUpdate = 0;
    static unsigned long lastLunarUpdate = 0;
    
    unsigned long now = millis();
    
    // 更新时间（根据time_manager的配置）
    try {
      if (now - lastTimeUpdate > CLOCK_REFRESH_INTERVAL) {
        lastTimeUpdate = now;
        timeManager.update();       // 更新时间
      }
    } catch (const std::exception& e) {
      Serial.print("时间更新异常: ");
      Serial.println(e.what());
    }
    
    // 更新天气（根据weather_manager的配置）
    try {
      if (now - lastWeatherUpdate > WEATHER_REFRESH_INTERVAL) {
        lastWeatherUpdate = now;
        weatherManager.update();    // 更新天气
      }
    } catch (const std::exception& e) {
      Serial.print("天气更新异常: ");
      Serial.println(e.what());
    }
    
    // 更新传感器数据（根据sensor_manager的配置）
    try {
      if (now - lastSensorUpdate > SENSOR_REFRESH_INTERVAL) {
        lastSensorUpdate = now;
        sensorManager.update();     // 更新传感器数据
      }
    } catch (const std::exception& e) {
      Serial.print("传感器更新异常: ");
      Serial.println(e.what());
    }
    
    // 更新股票数据（根据stock_manager的配置）
    try {
      if (now - lastStockUpdate > STOCK_REFRESH_INTERVAL) {
        lastStockUpdate = now;
        stockManager.update();      // 更新股票数据
      }
    } catch (const std::exception& e) {
      Serial.print("股票更新异常: ");
      Serial.println(e.what());
    }
    
    // 更新农历数据（根据lunar_manager的配置，每天更新一次）
    try {
      if (now - lastLunarUpdate > 86400000) { // 24小时
        lastLunarUpdate = now;
        lunarManager.update();      // 更新农历数据
      }
    } catch (const std::exception& e) {
      Serial.print("农历更新异常: ");
      Serial.println(e.what());
    }
  } catch (const std::exception& e) {
    // 捕获所有未处理的异常，确保系统不会完全崩溃
    Serial.print("主循环异常: ");
    Serial.println(e.what());
    Serial.flush();
    
    // 尝试恢复基本功能
    try {
      // 仅保留核心功能运行
      webServerManager.loop();
      firmwareManager.loop();
    } catch (...) {
      // 忽略恢复过程中的异常
    }
  } catch (...) {
    // 捕获所有其他类型的异常
    Serial.println("捕获到未知异常");
    Serial.flush();
  }
  
  // 软件看门狗检查
  if (millis() - lastWatchdogReset > WATCHDOG_TIMEOUT) {
    Serial.println("软件看门狗超时，系统将重启");
    Serial.flush();
    delay(1000);
    ESP.restart(); // 重启系统
  }
  
  // 短暂延迟，降低CPU占用
  delay(10);
}