#include "ipv6_server.h"
#include <ArduinoJson.h>
#include "services/wifi_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;

IPv6Server::IPv6Server() {
  server = nullptr;
  running = false;
}

IPv6Server::~IPv6Server() {
  if (server != nullptr) {
    delete server;
    server = nullptr;
  }
}

void IPv6Server::init() {
  DEBUG_PRINTLN("初始化IPv6服务器...");
  
  // 创建Web服务器实例，监听端口80
  server = new WebServer(80);
  
  // 注册路由处理函数
  server->on("/", HTTP_GET, std::bind(&IPv6Server::handleRoot, this));
  server->on("/api/push", HTTP_POST, std::bind(&IPv6Server::handleMessagePush, this));
  server->on("/api/status", HTTP_GET, std::bind(&IPv6Server::handleStatus, this));
  server->onNotFound(std::bind(&IPv6Server::handleNotFound, this));
  
  // 启动服务器
  server->begin();
  running = true;
  
  DEBUG_PRINTLN("IPv6服务器启动成功，监听端口80");
}

void IPv6Server::loop() {
  if (running && server != nullptr) {
    server->handleClient();
  }
}

bool IPv6Server::isRunning() {
  return running;
}

void IPv6Server::handleRoot() {
  String response = "<!DOCTYPE html><html><head><title>InkClock Device</title></head><body>";
  response += "<h1>InkClock Device</h1>";
  response += "<p>设备在线，可以通过API推送消息</p>";
  response += "<p>API地址: /api/push (POST)</p>";
  response += "<p>状态地址: /api/status (GET)</p>";
  response += "</body></html>";
  
  server->send(200, "text/html", response);
}

void IPv6Server::handleMessagePush() {
  // 检查Content-Type
  String contentType = server->header("Content-Type");
  if (contentType != "application/json") {
    sendJsonResponse("{\"error\": \"Invalid Content-Type, application/json required\"}", 400);
    return;
  }
  
  // 读取请求体
  String body = server->arg("plain");
  if (body.length() == 0) {
    sendJsonResponse("{\"error\": \"Empty request body\"}", 400);
    return;
  }
  
  // 解析JSON
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, body);
  if (error) {
    String errorStr = "{\"error\": \"Invalid JSON: ";
    errorStr += error.c_str();
    errorStr += "\"}";
    sendJsonResponse(errorStr, 400);
    return;
  }
  
  // 检查必填字段
  if (!doc.containsKey("content")) {
    sendJsonResponse("{\"error\": \"Missing required field: content\"}", 400);
    return;
  }
  
  // 提取消息内容
  String content = doc["content"].as<String>();
  String sender = doc.containsKey("sender") ? doc["sender"].as<String>() : "Direct Push";
  String type = doc.containsKey("type") ? doc["type"].as<String>() : "text";
  
  // 转换消息类型
  MessageType messageType = MESSAGE_TEXT;
  if (type == "image") {
    messageType = MESSAGE_IMAGE;
  } else if (type == "audio") {
    messageType = MESSAGE_AUDIO;
  }
  
  // 添加消息到消息管理器
  bool success = messageManager.addMessage(sender, content, messageType);
  
  if (success) {
    sendJsonResponse("{\"success\": true, \"message\": \"Message pushed successfully\"}");
    DEBUG_PRINTLN("收到直接推送消息: " + content);
  } else {
    sendJsonResponse("{\"error\": \"Failed to push message\"}", 500);
    DEBUG_PRINTLN("消息推送失败: " + content);
  }
}

void IPv6Server::handleStatus() {
  DynamicJsonDocument doc(1024);
  
  doc["status"] = "online";
  doc["ip_address"] = wifiManager.getIPAddress();
  doc["ipv6_address"] = wifiManager.getIPv6Address();
  doc["mac_address"] = WiFi.macAddress();
  doc["time"] = getCurrentTime();
  
  String json;
  serializeJson(doc, json);
  
  sendJsonResponse(json);
}

void IPv6Server::handleNotFound() {
  sendJsonResponse("{\"error\": \"Not Found\"}", 404);
}

void IPv6Server::sendJsonResponse(const String& json, int statusCode) {
  server->send(statusCode, "application/json", json);
}

String IPv6Server::getCurrentTime() {
  // 获取当前时间（这里需要根据实际情况实现，可能需要NTP或其他时间源）
  return "2025-01-01 00:00:00";
}
