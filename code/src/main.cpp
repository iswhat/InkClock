/**
 * @file main.cpp
 * @brief 家用网络智能墨水屏万年历主程序
 * @author DIY爱好者团队
 * @date 2025-12-26
 * @version 1.0
 * 
 * 该文件是家用网络智能墨水屏万年历的主入口文件，负责初始化所有模块、
 * 调度各模块的循环任务，并处理模块间的交互。
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

/**
 * @brief 全局对象实例
 * 
 * 所有功能模块的全局实例，用于在setup和loop函数中访问各个模块
 */
DisplayManager displayManager;       // 显示管理模块
WiFiManager wifiManager;             // WiFi管理模块
TimeManager timeManager;             // 时间管理模块
WeatherManager weatherManager;       // 天气管理模块
SensorManager sensorManager;         // 传感器管理模块
AudioManager audioManager;           // 音频管理模块
ButtonManager buttonManager;         // 按键管理模块
MessageManager messageManager;       // 消息管理模块
StockManager stockManager;           // 股票管理模块
PluginManager pluginManager;         // 插件管理模块
PowerManager powerManager;           // 电源管理模块
BluetoothManager bluetoothManager;   // 蓝牙管理模块
WebServerManager webServerManager;   // Web服务器模块
TouchManager touchManager;           // 触摸管理模块
CameraManager cameraManager;         // 摄像头管理模块
WebClient webClient;                 // Web客户端模块
IPv6Server ipv6Server;               // IPv6服务器模块
FirmwareManager firmwareManager;     // 固件管理模块

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
  
  // 初始化固件管理器，进行硬件自动检测和驱动适配
  firmwareManager.init();
  
  // 创建并设置显示驱动，使用墨水屏驱动
  IDisplayDriver* einkDriver = new EinkDriver();
  displayManager.setDisplayDriver(einkDriver);
  
  // 初始化显示管理器，显示启动画面
  displayManager.init();
  displayManager.showSplashScreen();
  
  // 初始化蓝牙管理模块，用于首次WiFi配置
  bluetoothManager.init();
  
  // 初始化传感器和其他不需要WiFi的模块
  sensorManager.init();       // 温湿度传感器管理
  audioManager.init();        // 音频录制和播放管理
  buttonManager.init();       // 按键事件处理
  messageManager.init();      // 消息接收和存储
  stockManager.init();        // 股票数据获取和显示
  pluginManager.init();       // 插件加载和管理
  powerManager.init();        // 电源管理和低功耗控制
  
  // 初始化触摸管理器和摄像头管理器（仅支持ESP32-S3系列）
  touchManager.init();
  cameraManager.init();
  
  // 初始化Web客户端，用于与云端服务器通信
  webClient.init();
  
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
  
  // 初始化Web服务器，提供设备管理API
  webServerManager.init();
  
  // 初始化IPv6服务器，支持IPv6直接访问
  ipv6Server.init();
  
  // 更新初始数据
  timeManager.update();       // 更新时间
  weatherManager.update();    // 更新天气
  sensorManager.update();     // 更新传感器数据
  stockManager.update();      // 更新股票数据
  
  // 显示初始页面
  displayManager.updateDisplay();
  
  // 打印初始化完成信息
  Serial.println("初始化完成");
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
void loop() {
  // 处理各个模块的循环任务
  wifiManager.loop();           // WiFi状态监测和重连
  timeManager.loop();           // 时间更新和同步
  sensorManager.loop();         // 传感器数据采集
  audioManager.loop();          // 音频播放控制
  buttonManager.loop();         // 按键事件检测
  messageManager.loop();        // 消息接收和处理
  stockManager.loop();          // 股票数据更新
  pluginManager.loop();         // 插件更新和显示
  powerManager.loop();          // 电源状态监测和低功耗控制
  bluetoothManager.loop();      // 蓝牙连接管理
  webServerManager.loop();      // Web服务器请求处理
  touchManager.loop();          // 触摸事件检测
  cameraManager.loop();         // 摄像头状态管理
  webClient.loop();             // 与云端服务器通信
  ipv6Server.loop();            // IPv6服务器请求处理
  firmwareManager.loop();       // 固件更新检查
  
  // 根据power_manager的建议更新显示，减少不必要的刷新
  if (powerManager.shouldUpdateDisplay()) {
    // 使用局部刷新更新显示，降低功耗
    displayManager.updateDisplayPartial();
  }
  
  // 定期更新数据（每DATA_UPDATE_INTERVAL毫秒，默认为5分钟）
  static unsigned long lastDataUpdate = 0;
  if (millis() - lastDataUpdate > DATA_UPDATE_INTERVAL) {
    lastDataUpdate = millis();
    timeManager.update();       // 更新时间
    weatherManager.update();    // 更新天气
    sensorManager.update();     // 更新传感器数据
    stockManager.update();      // 更新股票数据
  }
  
  // 短暂延迟，降低CPU占用
  delay(10);
}