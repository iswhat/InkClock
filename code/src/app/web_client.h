#ifndef WEB_CLIENT_H
#define WEB_CLIENT_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include "../core/config.h"
#include "../modules/message_manager.h"
#include "../services/api_manager.h"

class WebClient {
public:
  WebClient();
  ~WebClient();
  
  void init();
  void loop();
  
  // 设备注册
  bool registerDevice();
  
  // 获取设备ID
  String getDeviceId() { return deviceId; }
  
  // 设置设备ID
  void setDeviceId(String id) { deviceId = id; }
  
  // 检查是否已注册
  bool isRegistered() { return deviceId.length() > 0; }
  
  // 手动获取消息
  bool fetchMessages();
  
  // 发送消息到服务器（用于测试）
  bool sendMessage(String content, String type = "text");
  
private:
  WiFiClientSecure client;
  String deviceId;
  unsigned long lastRegisterAttempt;
  unsigned long lastMessageFetch;
  String webServerUrls[3]; // 主、备用、次备用Web服务器URL
  int currentWebServerIndex; // 当前使用的Web服务器索引
  String apiKey;
  
  // 设备信息
  String getDeviceInfo();
  
  // 读取存储的设备ID
  String readDeviceId();
  
  // 保存设备ID到Flash
  void saveDeviceId(String id);
  
  // 解析JSON响应
  bool parseJsonResponse(String response, DynamicJsonDocument& doc);
  
  // 处理获取到的消息
  void processMessages(JsonArray messages);
};

#endif // WEB_CLIENT_H