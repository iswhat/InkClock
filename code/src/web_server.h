#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "config.h"

class WebServerManager {
private:
  WebServer server;
  bool initialized;
  
  // 网页内容
  static const char* index_html;
  static const char* settings_html;
  static const char* plugin_html;
  static const char* style_css;
  
  // 处理函数
  void handleRoot();
  void handleSettings();
  void handlePlugins();
  void handleUpdateSettings();
  void handleAddPlugin();
  void handleUpdatePlugin();
  void handleDeletePlugin();
  void handleNotFound();
  void handleCSS();
  
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