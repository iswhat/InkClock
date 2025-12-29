#include "web_client.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include <WiFi.h>
#include "../services/wifi_manager.h"
#include "../modules/message_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;
extern MessageManager messageManager;
extern APIManager apiManager;

// WebServer配置
#define DEVICE_ID_FILE "/device_id.txt"

WebClient::WebClient() {
  deviceId = "";
  lastRegisterAttempt = 0;
  lastMessageFetch = 0;
  // 从配置文件中获取Web服务器URL
  webServerUrls[0] = String(WEB_SERVER_URL); // 主URL
  webServerUrls[1] = String(WEB_SERVER_URL_BACKUP); // 备用URL
  webServerUrls[2] = String(WEB_SERVER_URL_SECONDARY_BACKUP); // 次备用URL
  apiKey = API_KEY;
  currentWebServerIndex = 0;
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
  
  // 尝试所有可用的Web服务器URL
  for (int i = 0; i < 3; i++) {
    // 构建HTTP请求URL
    String url = webServerUrls[i];
    url += "?path=device";
    
    DEBUG_PRINT("尝试使用Web服务器: ");
    DEBUG_PRINTLN(url);
    
    // 使用API管理器发送POST请求
    ApiResponse apiResponse = apiManager.post(url, deviceInfo, API_TYPE_CUSTOM, 0); // 不缓存
    
    // 检查请求结果
    if (apiResponse.status == API_STATUS_SUCCESS) {
      // 解析响应
      DynamicJsonDocument doc(1024);
      if (parseJsonResponse(apiResponse.response, doc)) {
        if (doc["success"] == true) {
          deviceId = doc["device_id"].as<String>();
          saveDeviceId(deviceId);
          DEBUG_PRINT("设备注册成功，ID: ");
          DEBUG_PRINTLN(deviceId);
          // 更新当前使用的Web服务器索引
          currentWebServerIndex = i;
          return true;
        }
      }
      
      DEBUG_PRINTLN("设备注册失败: " + apiResponse.error);
    }
  }
  
  DEBUG_PRINTLN("所有Web服务器设备注册均失败");
  return false;
}

bool WebClient::fetchMessages() {
  DEBUG_PRINTLN("获取消息...");
  
  // 尝试所有可用的Web服务器URL
  for (int i = 0; i < 3; i++) {
    // 构建HTTP请求URL
    String url = webServerUrls[i];
    url += "?path=message/" + deviceId + "/unread";
    
    DEBUG_PRINT("尝试使用Web服务器: ");
    DEBUG_PRINTLN(url);
    
    // 使用API管理器发送GET请求
    ApiResponse apiResponse = apiManager.get(url, API_TYPE_CUSTOM, 60000); // 缓存1分钟
    
    // 检查请求结果
    if (apiResponse.status == API_STATUS_SUCCESS || apiResponse.status == API_STATUS_CACHED) {
      // 解析响应
      DynamicJsonDocument doc(2048);
      if (parseJsonResponse(apiResponse.response, doc)) {
        if (doc.containsKey("messages")) {
          JsonArray messages = doc["messages"];
          if (messages.size() > 0) {
            processMessages(messages);
          }
          // 更新当前使用的Web服务器索引
          currentWebServerIndex = i;
          DEBUG_PRINTLN("获取消息成功");
          return true;
        }
      }
    }
    
    DEBUG_PRINTLN("获取消息失败: " + apiResponse.error);
  }
  
  DEBUG_PRINTLN("所有Web服务器获取消息均失败");
  return false;
}

bool WebClient::sendMessage(String content, String type) {
  // 构建消息内容
  DynamicJsonDocument doc(512);
  doc["device_id"] = deviceId;
  doc["content"] = content;
  doc["type"] = type;
  
  String messageJson;
  serializeJson(doc, messageJson);
  
  // 尝试所有可用的Web服务器URL
  for (int i = 0; i < 3; i++) {
    // 构建HTTP请求URL
    String url = webServerUrls[i];
    url += "?path=message";
    
    DEBUG_PRINT("尝试使用Web服务器: ");
    DEBUG_PRINTLN(url);
    
    // 使用API管理器发送POST请求
    ApiResponse apiResponse = apiManager.post(url, messageJson, API_TYPE_CUSTOM, 0); // 不缓存
    
    // 检查请求结果
    if (apiResponse.status == API_STATUS_SUCCESS) {
      // 解析响应
      DynamicJsonDocument respDoc(512);
      if (parseJsonResponse(apiResponse.response, respDoc)) {
        if (respDoc["success"] == true) {
          DEBUG_PRINTLN("消息发送成功");
          // 更新当前使用的Web服务器索引
          currentWebServerIndex = i;
          return true;
        }
      }
      
      DEBUG_PRINTLN("消息发送失败: " + apiResponse.error);
    }
  }
  
  DEBUG_PRINTLN("所有Web服务器消息发送均失败");
  return false;
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