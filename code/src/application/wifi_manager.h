#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "../coresystem/config.h"

class WiFiManager {
public:
  WiFiManager();
  ~WiFiManager();
  
  void init();
  void connect();
  void connect(String ssid, String password);
  void disconnect();
  void loop();
  
  // 获取WiFi状态
  bool isConnected() { return WiFi.status() == WL_CONNECTED; }
  String getSSID() { return WiFi.SSID(); }
  int getSignalStrength() { return WiFi.RSSI(); }
  String getIPAddress() { return WiFi.localIP().toString(); }
  String getIPv6Address() { return WiFi.localIPv6().toString(); }
  bool hasIPv6() { return WiFi.localIPv6() != IPAddress(0, 0, 0, 0, 0, 0, 0, 0); }
  
  // AP模式相关方法
  void startAP();
  void stopAP();
  bool isAPMode() { return apMode; }
  String getAPIPAddress() { return WiFi.softAPIP().toString(); }
  
  // 配置管理
  bool hasConfiguredWiFi() { return !configuredSSID.isEmpty() && !configuredPassword.isEmpty(); }
  void setConfiguredWiFi(String ssid, String password);
  String getConfiguredSSID() { return configuredSSID; }
  String getConfiguredPassword() { return configuredPassword; }
  
private:
  // WiFi状态
  bool connected;
  bool apMode;
  unsigned long lastReconnectAttempt;
  int connectionAttempts;
  int maxConnectionAttempts;
  String currentSSID;
  String currentPassword;
  int lastSignalStrength;
  unsigned long signalStrengthCheckInterval;
  
  // 配置的WiFi信息
  String configuredSSID;
  String configuredPassword;
  
  // 私有方法
  void setupWiFi(String ssid, String password);
  void reconnect();
  void printWiFiStatus();
  void loadConfiguredWiFi();
  void saveConfiguredWiFi(String ssid, String password);
};

#endif // WIFI_MANAGER_H