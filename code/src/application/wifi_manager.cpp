#include "wifi_manager.h"

WiFiManager::WiFiManager() {
  connected = false;
  lastReconnectAttempt = 0;
  connectionAttempts = 0;
  maxConnectionAttempts = 20;
  currentSSID = WIFI_SSID;
  currentPassword = WIFI_PASSWORD;
}

WiFiManager::~WiFiManager() {
  // 清理资源
  disconnect();
}

void WiFiManager::init() {
  DEBUG_PRINTLN("初始化WiFi管理器...");
  
  // 设置WiFi模式为STA
  WiFi.mode(WIFI_STA);
  
  // 启用IPv6支持
  WiFi.enableIpV6();
  
  // 禁用WiFi自动连接
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  
  DEBUG_PRINTLN("WiFi管理器初始化完成");
}

void WiFiManager::connect() {
  DEBUG_PRINT("连接到WiFi: ");
  DEBUG_PRINTLN(WIFI_SSID);
  
  setupWiFi(WIFI_SSID, WIFI_PASSWORD);
}

void WiFiManager::connect(String ssid, String password) {
  DEBUG_PRINT("连接到WiFi: ");
  DEBUG_PRINTLN(ssid);
  
  setupWiFi(ssid, password);
}

void WiFiManager::disconnect() {
  DEBUG_PRINTLN("断开WiFi连接...");
  
  WiFi.disconnect();
  connected = false;
  
  DEBUG_PRINTLN("WiFi已断开");
}

void WiFiManager::loop() {
  // 检查WiFi连接状态
  wl_status_t status = WiFi.status();
  
  if (status != WL_CONNECTED) {
    if (connected) {
      // 连接断开，标记为未连接
      connected = false;
      connectionAttempts = 0;
      DEBUG_PRINTLN("WiFi连接已断开");
    }
    
    // 尝试重连，但限制最大尝试次数
    if (connectionAttempts < maxConnectionAttempts) {
      reconnect();
    } else {
      // 超过最大尝试次数，暂停一段时间后再尝试
      static unsigned long lastPauseTime = 0;
      if (millis() - lastPauseTime > 60000) { // 暂停60秒
        lastPauseTime = millis();
        connectionAttempts = 0; // 重置尝试次数
        DEBUG_PRINTLN("暂停重连，60秒后重试");
      }
    }
  } else {
    if (!connected) {
      // 连接成功
      connected = true;
      connectionAttempts = 0;
      printWiFiStatus();
    }
  }
}

void WiFiManager::setupWiFi(String ssid, String password) {
  // 连接WiFi
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // 等待连接
  DEBUG_PRINTLN("正在连接WiFi...");
  int attempts = 0;
  const int maxAttempts = 10;
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    DEBUG_PRINT(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    printWiFiStatus();
  } else {
    connected = false;
    DEBUG_PRINTLN("\nWiFi连接失败");
  }
}

void WiFiManager::reconnect() {
  // 定期尝试重连
  if (millis() - lastReconnectAttempt > WIFI_RECONNECT_INTERVAL) {
    lastReconnectAttempt = millis();
    
    DEBUG_PRINTLN("尝试重连WiFi...");
    // 使用当前连接的SSID和密码进行重连
    // 如果当前没有连接，使用默认配置
    String currentSSID = WiFi.SSID();
    if (currentSSID == "") {
      currentSSID = WIFI_SSID;
    }
    // 注意：由于WiFi库不存储密码，我们无法获取当前连接的密码
    // 因此重连时使用默认密码或上次成功连接的密码
    // 这里简化处理，使用默认密码
    setupWiFi(currentSSID, WIFI_PASSWORD);
  }
}

void WiFiManager::printWiFiStatus() {
  DEBUG_PRINTLN("\nWiFi连接成功");
  DEBUG_PRINT("SSID: ");
  DEBUG_PRINTLN(WiFi.SSID());
  DEBUG_PRINT("IP地址: ");
  DEBUG_PRINTLN(WiFi.localIP());
  // 显示IPv6地址
  DEBUG_PRINT("IPv6地址: ");
  DEBUG_PRINTLN(WiFi.localIPv6());
  DEBUG_PRINT("信号强度: ");
  DEBUG_PRINT(WiFi.RSSI());
  DEBUG_PRINTLN(" dBm");
  DEBUG_PRINT("MAC地址: ");
  DEBUG_PRINTLN(WiFi.macAddress());
}