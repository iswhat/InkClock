#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "../core/config.h"

class WebServerManager {
private:
  WebServer server;
  bool initialized;
  
  // 网页内容
  static const char* index_html;
  static const char* settings_html;
  static const char* plugin_html;
  static const char* plugin_list_html;
  static const char* style_css;
  
  // 处理函数
  void handleRoot();
  void handleSettings();
  void handlePlugins();
  void handlePluginList();
  void handleUpdateSettings();
  void handleAddPlugin();
  void handleUpdatePlugin();
  void handleDeletePlugin();
  void handleEnablePlugin();
  void handleDisablePlugin();
  void handleNotFound();
  void handleCSS();
  // 传感器数据接口
  void handleSensorData();
  void handleApi();
  // IPv6推送功能API（合并自IPv6Server）
  void handleMessagePush();
  void handleDeviceStatus();
  // 辅助函数
  void sendJsonResponse(const String& json, int statusCode = 200);
  String getCurrentTime();
  
  // 辅助函数
  String getIPAddress();
  String generateQRCodeURL();
  
public:
  WebServerManager();
  ~WebServerManager();
  
  void init();
  void loop();
  
  // 获取Web服务器是否已初始化
  bool isInitialized() { return initialized; }
};

#endif // WEB_SERVER_H