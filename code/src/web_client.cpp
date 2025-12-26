#include "web_client.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "wifi_manager.h"
#include "message_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;
extern MessageManager messageManager;

// WebServer配置
#define WEB_SERVER_URL "https://your-webserver-url.com/api.php"
#define API_KEY "your_secret_key_here"
#define DEVICE_ID_FILE "/device_id.txt"

WebClient::WebClient() {
  deviceId = "";
  lastRegisterAttempt = 0;
  lastMessageFetch = 0;
  webServerUrl = WEB_SERVER_URL;
  apiKey = API_KEY;
}

WebClient::~WebClient() {
  // 清理资源
}

void WebClient::init() {
  DEBUG_PRINTLN("初始化Web客户端...");
  
  // 初始化SPIFFS文件系统（如果未初始化）
  if (!SPIFFS.begin(false)) {
    DEBUG_PRINTLN("SPIFFS初始化失败");
    return;
  }
  
  // 读取设备ID
  deviceId = readDeviceId();
  if (deviceId.length() > 0) {
    DEBUG_PRINT("已读取设备ID: ");
    DEBUG_PRINTLN(deviceId);
  } else {
    DEBUG_PRINTLN("未找到设备ID，将进行注册");
  }
  
  // 允许不安全连接（用于测试，生产环境应使用证书验证）
  client.setInsecure();
  
  DEBUG_PRINTLN("Web客户端初始化完成");
}

void WebClient::loop() {
  // 检查WiFi连接
  if (!wifiManager.isConnected()) {
    return;
  }
  
  // 设备注册
  if (!isRegistered() && millis() - lastRegisterAttempt > 60000) {
    lastRegisterAttempt = millis();
    registerDevice();
  }
  
  // 定期获取消息
  if (isRegistered() && millis() - lastMessageFetch > 30000) {
    lastMessageFetch = millis();
    fetchMessages();
  }
}

bool WebClient::registerDevice() {
  DEBUG_PRINTLN("注册设备...");
  
  // 构建设备信息
  String deviceInfo = getDeviceInfo();
  
  // 构建HTTP请求
  String url = webServerUrl;
  url += "?path=device";
  
  if (client.connect("your-webserver-url.com", 443)) {
    // 发送HTTP请求
    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: your-webserver-url.com");
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(deviceInfo.length()));
    client.println("api-key: " + apiKey);
    client.println();
    client.println(deviceInfo);
    
    // 等待响应
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        DEBUG_PRINTLN("注册超时");
        client.stop();
        return false;
      }
    }
    
    // 读取响应
    String response = "";
    while (client.available()) {
      response += client.readStringUntil('\r');
      client.read(); // 读取换行符
    }
    
    client.stop();
    
    // 解析响应
    DynamicJsonDocument doc(1024);
    if (parseJsonResponse(response, doc)) {
      if (doc["success"] == true) {
        deviceId = doc["device_id"].as<String>();
        saveDeviceId(deviceId);
        DEBUG_PRINT("设备注册成功，ID: ");
        DEBUG_PRINTLN(deviceId);
        return true;
      }
    }
    
    DEBUG_PRINTLN("设备注册失败");
    return false;
  } else {
    DEBUG_PRINTLN("无法连接到WebServer");
    return false;
  }
}

bool WebClient::fetchMessages() {
  DEBUG_PRINTLN("获取消息...");
  
  // 构建HTTP请求
  String url = webServerUrl;
  url += "?path=message/" + deviceId + "/unread";
  
  if (client.connect("your-webserver-url.com", 443)) {
    // 发送HTTP请求
    client.println("GET " + url + " HTTP/1.1");
    client.println("Host: your-webserver-url.com");
    client.println("api-key: " + apiKey);
    client.println();
    
    // 等待响应
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        DEBUG_PRINTLN("获取消息超时");
        client.stop();
        return false;
      }
    }
    
    // 读取响应
    String response = "";
    while (client.available()) {
      response += client.readStringUntil('\r');
      client.read(); // 读取换行符
    }
    
    client.stop();
    
    // 解析响应
    DynamicJsonDocument doc(2048);
    if (parseJsonResponse(response, doc)) {
      if (doc.containsKey("messages")) {
        JsonArray messages = doc["messages"];
        if (messages.size() > 0) {
          processMessages(messages);
          return true;
        }
      }
    }
    
    DEBUG_PRINTLN("没有新消息或获取失败");
    return true;
  } else {
    DEBUG_PRINTLN("无法连接到WebServer");
    return false;
  }
}

bool WebClient::sendMessage(String content, String type) {
  // 构建消息内容
  DynamicJsonDocument doc(512);
  doc["device_id"] = deviceId;
  doc["content"] = content;
  doc["type"] = type;
  
  String messageJson;
  serializeJson(doc, messageJson);
  
  // 构建HTTP请求
  String url = webServerUrl;
  url += "?path=message";
  
  if (client.connect("your-webserver-url.com", 443)) {
    // 发送HTTP请求
    client.println("POST " + url + " HTTP/1.1");
    client.println("Host: your-webserver-url.com");
    client.println("Content-Type: application/json");
    client.println("Content-Length: " + String(messageJson.length()));
    client.println("api-key: " + apiKey);
    client.println();
    client.println(messageJson);
    
    // 等待响应
    unsigned long timeout = millis();
    while (client.available() == 0) {
      if (millis() - timeout > 5000) {
        DEBUG_PRINTLN("发送消息超时");
        client.stop();
        return false;
      }
    }
    
    // 读取响应
    String response = "";
    while (client.available()) {
      response += client.readStringUntil('\r');
      client.read(); // 读取换行符
    }
    
    client.stop();
    
    // 解析响应
    DynamicJsonDocument respDoc(512);
    if (parseJsonResponse(response, respDoc)) {
      if (respDoc["success"] == true) {
        DEBUG_PRINTLN("消息发送成功");
        return true;
      }
    }
    
    DEBUG_PRINTLN("消息发送失败");
    return false;
  } else {
    DEBUG_PRINTLN("无法连接到WebServer");
    return false;
  }
}

String WebClient::getDeviceInfo() {
  DynamicJsonDocument doc(1024);
  
  // 设备信息
  doc["mac_address"] = WiFi.macAddress();
  doc["ip_address"] = wifiManager.getIPAddress();
  
  // 硬件型号
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_C3_DEFAULT
    doc["model"] = "ESP32-C3-Default";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT
    doc["model"] = "ESP32-S3-Default";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_C6_DEFAULT
    doc["model"] = "ESP32-C6-Default";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_C6_CUSTOM
    doc["model"] = "ESP32-C6-Custom";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S2_DEFAULT
    doc["model"] = "ESP32-S2-Default";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_WROOM_32
    doc["model"] = "ESP32-WROOM-32";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_PRO
    doc["model"] = "ESP32-S3-Pro";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_C3_SUPERMINI
    doc["model"] = "ESP32-C3-Supermini";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_PRO_S3
    doc["model"] = "ESP32-Pro-S3";
  #elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    doc["model"] = "ESP32-S3-WROOM-1";
  #else
    doc["model"] = "Unknown";
  #endif
  
  doc["firmware_version"] = "1.0.0";
  
  String json;
  serializeJson(doc, json);
  return json;
}

String WebClient::readDeviceId() {
  if (!SPIFFS.exists(DEVICE_ID_FILE)) {
    return "";
  }
  
  File file = SPIFFS.open(DEVICE_ID_FILE, FILE_READ);
  if (!file) {
    return "";
  }
  
  String id = file.readString();
  file.close();
  id.trim();
  return id;
}

void WebClient::saveDeviceId(String id) {
  File file = SPIFFS.open(DEVICE_ID_FILE, FILE_WRITE);
  if (file) {
    file.print(id);
    file.close();
    DEBUG_PRINT("设备ID已保存: ");
    DEBUG_PRINTLN(id);
  } else {
    DEBUG_PRINTLN("无法保存设备ID");
  }
}

bool WebClient::parseJsonResponse(String response, DynamicJsonDocument& doc) {
  // 找到JSON响应开始位置
  int jsonStart = response.indexOf('{');
  if (jsonStart == -1) {
    DEBUG_PRINTLN("响应中未找到JSON");
    return false;
  }
  
  // 提取JSON部分
  String json = response.substring(jsonStart);
  
  // 解析JSON
  DeserializationError error = deserializeJson(doc, json);
  if (error) {
    DEBUG_PRINT("JSON解析失败: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  return true;
}

void WebClient::processMessages(JsonArray messages) {
  DEBUG_PRINT("处理消息，共 ");
  DEBUG_PRINT(messages.size());
  DEBUG_PRINTLN(" 条");
  
  for (JsonObject message : messages) {
    String sender = message["sender"].as<String>();
    String content = message["content"].as<String>();
    String type = message["type"].as<String>();
    
    // 添加到消息管理器
    messageManager.addMessage(sender, content, 
      type == "image" ? MESSAGE_IMAGE : (type == "audio" ? MESSAGE_AUDIO : MESSAGE_TEXT));
  }
}