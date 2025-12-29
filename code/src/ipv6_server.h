#ifndef IPV6_SERVER_H
#define IPV6_SERVER_H

#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "modules/message_manager.h"

// 外部全局对象
extern MessageManager messageManager;

class IPv6Server {
public:
  IPv6Server();
  ~IPv6Server();
  
  void init();
  void loop();
  bool isRunning();
  
private:
  WebServer *server;
  bool running;
  
  // 处理函数
  void handleRoot();
  void handleMessagePush();
  void handleStatus();
  void handleNotFound();
  
  // 辅助函数
  void sendJsonResponse(const String& json, int statusCode = 200);
  String getCurrentTime();
};

#endif // IPV6_SERVER_H