#include <Arduino.h>
#include "config.h"
#include "eink_display.h"
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

// 全局对象实例
EinkDisplay display;
WiFiManager wifiManager;
TimeManager timeManager;
WeatherManager weatherManager;
SensorManager sensorManager;
AudioManager audioManager;
ButtonManager buttonManager;
MessageManager messageManager;
StockManager stockManager;
PluginManager pluginManager;
PowerManager powerManager;
BluetoothManager bluetoothManager;
WebServerManager webServerManager;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("===== 家用网络智能墨水屏万年历 ====");
  
  // 初始化各个模块
  display.init();
  display.showSplashScreen();
  
  // 初始化蓝牙管理模块
  bluetoothManager.init();
  
  // 初始化传感器和其他不需要WiFi的模块
  sensorManager.init();
  audioManager.init();
  buttonManager.init();
  messageManager.init();
  stockManager.init();
  pluginManager.init();
  powerManager.init();
  
  // 检查是否已经配置了WiFi，如果没有，等待蓝牙配置
  if (!wifiManager.isConnected()) {
    Serial.println("等待蓝牙WiFi配置...");
    
    // 显示蓝牙配置提示
    display.showMessage("等待蓝牙WiFi配置...", 10000);
    
    // 等待蓝牙配置WiFi
    while (!bluetoothManager.isWiFiConfigured()) {
      bluetoothManager.loop();
      delay(1000);
    }
    
    // 使用蓝牙配置的WiFi信息连接
    String ssid = bluetoothManager.getWiFiSSID();
    String password = bluetoothManager.getWiFiPassword();
    wifiManager.init();
    wifiManager.connect(ssid, password);
  } else {
    // 已经连接了WiFi，直接初始化
    wifiManager.init();
  }
  
  // 初始化需要WiFi的模块
  timeManager.init();
  weatherManager.init();
  
  // 初始化Web服务器
  webServerManager.init();
  
  // 更新初始数据
  timeManager.update();
  weatherManager.update();
  sensorManager.update();
  stockManager.update();
  
  // 显示初始页面
  display.updateDisplay();
  
  Serial.println("初始化完成");
}

void loop() {
  // 处理各个模块的循环任务
  wifiManager.loop();
  timeManager.loop();
  sensorManager.loop();
  audioManager.loop();
  buttonManager.loop();
  messageManager.loop();
  stockManager.loop();
  pluginManager.loop();
  powerManager.loop();
  bluetoothManager.loop();
  webServerManager.loop();
  
  // 定期更新显示
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > DISPLAY_UPDATE_INTERVAL) {
    lastDisplayUpdate = millis();
    display.updateDisplay();
  }
  
  // 定期更新时钟区域（使用部分刷新，每分钟更新一次）
  static unsigned long lastClockUpdate = 0;
  if (millis() - lastClockUpdate > 60000) {
    lastClockUpdate = millis();
    display.updateClockArea();
  }
  
  // 定期更新数据
  static unsigned long lastDataUpdate = 0;
  if (millis() - lastDataUpdate > DATA_UPDATE_INTERVAL) {
    lastDataUpdate = millis();
    timeManager.update();
    weatherManager.update();
    sensorManager.update();
    stockManager.update();
  }
  
  delay(10);
}