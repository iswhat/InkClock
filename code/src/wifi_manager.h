#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include "config.h"

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
  
private:
  // WiFi状态
  bool connected;
  unsigned long lastReconnectAttempt;
  
  // 私有方法
  void setupWiFi();
  void reconnect();
  void printWiFiStatus();
};

#endif // WIFI_MANAGER_H