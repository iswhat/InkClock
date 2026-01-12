#include "wifi_manager.h"
#include <Preferences.h>
#include <esp_system.h>

Preferences preferences;

WiFiManager::WiFiManager() {
  connected = false;
  apMode = false;
  lastReconnectAttempt = 0;
  connectionAttempts = 0;
  maxConnectionAttempts = 20;
  currentSSID = WIFI_SSID;
  currentPassword = WIFI_PASSWORD;
  configuredSSID = "";
  configuredPassword = "";
  lastSignalStrength = -100;
  signalStrengthCheckInterval = 0;
}

WiFiManager::~WiFiManager() {
  // 清理资源
  disconnect();
  stopAP();
}

void WiFiManager::init() {
  DEBUG_PRINTLN("初始化WiFi管理器...");
  
  // 加载配置的WiFi信息
  loadConfiguredWiFi();
  
  // 设置WiFi模式为STA
  WiFi.mode(WIFI_STA);
  
  // 启用IPv6支持
  WiFi.enableIpV6();
  
  // 禁用WiFi自动连接
  WiFi.setAutoConnect(false);
  WiFi.setAutoReconnect(false);
  
  // 配置WiFi电源管理，平衡功耗和性能
  WiFi.setSleep(WIFI_PS_MIN_MODEM);
  
  // 如果有配置的WiFi信息，尝试连接
  if (hasConfiguredWiFi()) {
    DEBUG_PRINT("使用配置的WiFi信息连接: ");
    DEBUG_PRINTLN(configuredSSID);
    setupWiFi(configuredSSID, configuredPassword);
  } else {
    // 没有配置的WiFi信息，进入AP模式
    DEBUG_PRINTLN("没有配置的WiFi信息，进入AP模式");
    startAP();
  }
  
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
      // 超过最大尝试次数，进入AP模式
      if (!apMode) {
        DEBUG_PRINTLN("超过最大尝试次数，进入AP模式");
        startAP();
      }
      
      // 定期尝试退出AP模式并重新连接
      static unsigned long lastAPModeTime = 0;
      if (millis() - lastAPModeTime > 300000) { // 5分钟后尝试重新连接
        lastAPModeTime = millis();
        DEBUG_PRINTLN("尝试退出AP模式，重新连接WiFi...");
        stopAP();
        connectionAttempts = 0;
        if (hasConfiguredWiFi()) {
          setupWiFi(configuredSSID, configuredPassword);
        }
      }
    }
  } else {
    if (!connected) {
      // 连接成功，退出AP模式
      if (apMode) {
        stopAP();
      }
      
      // 连接成功
      connected = true;
      connectionAttempts = 0;
      printWiFiStatus();
    } else {
      // 定期检查信号强度
      checkSignalStrength();
    }
  }
}

void WiFiManager::setupWiFi(String ssid, String password) {
  // 确保不是在AP模式下
  if (apMode) {
    stopAP();
  }
  
  // 连接WiFi
  WiFi.begin(ssid.c_str(), password.c_str());
  
  // 等待连接
  DEBUG_PRINTLN("正在连接WiFi...");
  int attempts = 0;
  const int maxAttempts = 15; // 增加连接尝试次数
  
  while (WiFi.status() != WL_CONNECTED && attempts < maxAttempts) {
    delay(500);
    DEBUG_PRINT(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    connected = true;
    currentSSID = ssid;
    currentPassword = password;
    printWiFiStatus();
    // 保存配置的WiFi信息
    saveConfiguredWiFi(ssid, password);
  } else {
    connected = false;
    DEBUG_PRINTLN("\nWiFi连接失败");
    
    // 记录错误状态
    wl_status_t errorStatus = WiFi.status();
    DEBUG_PRINT("错误状态: ");
    DEBUG_PRINTLN(getWiFiStatusString(errorStatus));
    
    // 如果连接失败，尝试进入AP模式
    if (!apMode) {
      DEBUG_PRINTLN("尝试进入AP模式...");
      startAP();
    }
  }
}

void WiFiManager::reconnect() {
  // 定期尝试重连
  if (millis() - lastReconnectAttempt > WIFI_RECONNECT_INTERVAL) {
    lastReconnectAttempt = millis();
    
    DEBUG_PRINTLN("尝试重连WiFi...");
    // 使用当前连接的SSID和密码进行重连
    // 如果当前没有连接，使用配置的WiFi信息
    String ssid = hasConfiguredWiFi() ? configuredSSID : WIFI_SSID;
    String password = hasConfiguredWiFi() ? configuredPassword : WIFI_PASSWORD;
    
    setupWiFi(ssid, password);
    connectionAttempts++;
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
  int rssi = WiFi.RSSI();
  DEBUG_PRINT(rssi);
  DEBUG_PRINT(" dBm (质量: ");
  DEBUG_PRINT(getSignalQuality(rssi));
  DEBUG_PRINTLN(")");
  DEBUG_PRINT("MAC地址: ");
  DEBUG_PRINTLN(WiFi.macAddress());
  DEBUG_PRINT("WiFi模式: ");
  DEBUG_PRINTLN(WiFi.getMode() == WIFI_STA ? "STA" : "AP");
  lastSignalStrength = rssi;
}

void WiFiManager::startAP() {
  if (apMode) {
    return; // 已经在AP模式
  }
  
  DEBUG_PRINTLN("启动AP模式...");
  
  // 设置WiFi模式为AP
  WiFi.mode(WIFI_AP);
  
  // 生成AP名称，包含设备MAC地址后4位
  String mac = WiFi.macAddress();
  String apName = "InkClock-" + mac.substring(mac.length() - 5, mac.length() - 1);
  
  // 启动AP
  bool result = WiFi.softAP(apName.c_str(), "inkclock123");
  
  if (result) {
    apMode = true;
    DEBUG_PRINT("AP模式启动成功，名称: ");
    DEBUG_PRINTLN(apName);
    DEBUG_PRINT("AP IP地址: ");
    DEBUG_PRINTLN(WiFi.softAPIP());
    DEBUG_PRINTLN("请使用手机连接此WiFi，然后在浏览器中访问 192.168.4.1 进行配置");
  } else {
    DEBUG_PRINTLN("AP模式启动失败");
  }
}

void WiFiManager::stopAP() {
  if (!apMode) {
    return; // 不在AP模式
  }
  
  DEBUG_PRINTLN("停止AP模式...");
  
  WiFi.softAPdisconnect(true);
  apMode = false;
  
  // 切换回STA模式
  WiFi.mode(WIFI_STA);
  
  DEBUG_PRINTLN("AP模式已停止");
}

void WiFiManager::loadConfiguredWiFi() {
  DEBUG_PRINTLN("加载配置的WiFi信息...");
  
  preferences.begin("wifi", true);
  configuredSSID = preferences.getString("ssid", "");
  configuredPassword = preferences.getString("password", "");
  preferences.end();
  
  if (hasConfiguredWiFi()) {
    DEBUG_PRINT("加载的WiFi配置: ");
    DEBUG_PRINTLN(configuredSSID);
  } else {
    DEBUG_PRINTLN("没有找到配置的WiFi信息");
  }
}

void WiFiManager::saveConfiguredWiFi(String ssid, String password) {
  DEBUG_PRINT("保存WiFi配置: ");
  DEBUG_PRINTLN(ssid);
  
  preferences.begin("wifi", false);
  preferences.putString("ssid", ssid);
  preferences.putString("password", password);
  preferences.end();
  
  // 更新配置的WiFi信息
  configuredSSID = ssid;
  configuredPassword = password;
}

void WiFiManager::setConfiguredWiFi(String ssid, String password) {
  // 设置配置的WiFi信息
  configuredSSID = ssid;
  configuredPassword = password;
  
  // 保存到存储
  saveConfiguredWiFi(ssid, password);
  
  // 尝试使用新配置连接
  setupWiFi(ssid, password);
}

void WiFiManager::checkSignalStrength() {
  // 每10秒检查一次信号强度
  if (millis() - signalStrengthCheckInterval > 10000) {
    signalStrengthCheckInterval = millis();
    
    int currentRSSI = WiFi.RSSI();
    int rssiDiff = abs(currentRSSI - lastSignalStrength);
    
    // 如果信号强度变化超过10dBm，打印信息
    if (rssiDiff > 10 || lastSignalStrength == -100) {
      DEBUG_PRINT("WiFi信号强度变化: ");
      DEBUG_PRINT(currentRSSI);
      DEBUG_PRINT(" dBm (质量: ");
      DEBUG_PRINT(getSignalQuality(currentRSSI));
      DEBUG_PRINTLN(")");
      lastSignalStrength = currentRSSI;
    }
  }
}

String WiFiManager::getWiFiStatusString(wl_status_t status) {
  switch (status) {
    case WL_CONNECTED:
      return "WL_CONNECTED";
    case WL_IDLE_STATUS:
      return "WL_IDLE_STATUS";
    case WL_NO_SSID_AVAIL:
      return "WL_NO_SSID_AVAIL";
    case WL_SCAN_COMPLETED:
      return "WL_SCAN_COMPLETED";
    case WL_CONNECT_FAILED:
      return "WL_CONNECT_FAILED";
    case WL_CONNECTION_LOST:
      return "WL_CONNECTION_LOST";
    case WL_DISCONNECTED:
      return "WL_DISCONNECTED";
    default:
      return "UNKNOWN_STATUS";
  }
}

String WiFiManager::getSignalQuality(int rssi) {
  if (rssi >= -50) {
    return "优秀";
  } else if (rssi >= -60) {
    return "良好";
  } else if (rssi >= -70) {
    return "一般";
  } else if (rssi >= -80) {
    return "较差";
  } else {
    return "很差";
  }
}