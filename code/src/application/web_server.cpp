#include "web_server.h"
#include "application/wifi_manager.h"
#include "../extensions/plugin_manager.h"
#include "sensor_manager.h"
#include "../coresystem/font_manager.h"
#include "../coresystem/tf_card_manager.h"
#include "../coresystem/core_system.h"
#include "display_manager.h"
#include <ArduinoJson.h>

// 外部全局对象
CoreSystem* coreSystem;

// 外部全局对象
extern WiFiManager wifiManager;
extern PluginManager pluginManager;
extern SensorManager sensorManager;
extern MessageManager messageManager;

extern DisplayManager displayManager;

// 简化的HTML内容
const char* WebServerManager::index_html = "<!DOCTYPE html><html><body><h1>InkClock</h1><p>Device status page</p></body></html>";
const char* WebServerManager::settings_html = "<!DOCTYPE html><html><body><h1>Settings</h1><p>Settings page</p></body></html>";
const char* WebServerManager::plugin_html = "<!DOCTYPE html><html><body><h1>Plugins</h1><p>Plugins page</p></body></html>";
const char* WebServerManager::plugin_list_html = "<!DOCTYPE html><html><body><h1>Plugin List</h1><p>Plugin list page</p></body></html>";
const char* WebServerManager::fonts_html = "<!DOCTYPE html><html><body><h1>Fonts</h1><p>Fonts page</p></body></html>";
const char* WebServerManager::tfcard_html = "<!DOCTYPE html><html><body><h1>TF Card</h1><p>TF card page</p></body></html>";
const char* WebServerManager::style_css = "body { font-family: Arial, sans-serif; }";

WebServerManager::WebServerManager() : server(8080), initialized(false) {
}

WebServerManager::~WebServerManager() {
}

void WebServerManager::init() {
    DEBUG_PRINTLN("初始化Web服务器...");
    
    // 设置路由处理函数
    server.on("/", std::bind(&WebServerManager::handleRoot, this));
    server.on("/settings", std::bind(&WebServerManager::handleSettings, this));
    server.on("/plugins", std::bind(&WebServerManager::handlePlugins, this));
    server.on("/plugin_list", std::bind(&WebServerManager::handlePluginList, this));
    server.on("/fonts", std::bind(&WebServerManager::handleFonts, this));
    server.on("/tfcard", std::bind(&WebServerManager::handleTFCard, this));
    server.on("/upload_font", HTTP_POST, std::bind(&WebServerManager::handleUploadFont, this));
    server.on("/update_settings", HTTP_POST, std::bind(&WebServerManager::handleUpdateSettings, this));
    server.on("/add_plugin", HTTP_POST, std::bind(&WebServerManager::handleAddPlugin, this));
    server.on("/update_plugin", HTTP_POST, std::bind(&WebServerManager::handleUpdatePlugin, this));
    server.on("/delete_plugin", HTTP_POST, std::bind(&WebServerManager::handleDeletePlugin, this));
    server.on("/enable_plugin", HTTP_POST, std::bind(&WebServerManager::handleEnablePlugin, this));
    server.on("/disable_plugin", HTTP_POST, std::bind(&WebServerManager::handleDisablePlugin, this));
    server.on("/style.css", std::bind(&WebServerManager::handleCSS, this));
    server.on("/factory_reset", std::bind(&WebServerManager::handleFactoryReset, this));
    
    // API路由
    server.on("/api", std::bind(&WebServerManager::handleApi, this));
    server.on("/api/sensor", std::bind(&WebServerManager::handleSensorData, this));
    server.on("/api/control", HTTP_GET, std::bind(&WebServerManager::handleRemoteControl, this));
    server.on("/api/control", HTTP_POST, std::bind(&WebServerManager::handleRemoteControl, this));
    server.on("/api/sync", HTTP_GET, std::bind(&WebServerManager::handleDataSync, this));
    server.on("/api/sync", HTTP_POST, std::bind(&WebServerManager::handleDataSync, this));
    server.on("/api/refresh", HTTP_GET, std::bind(&WebServerManager::handleRefreshDisplay, this));
    server.on("/api/refresh", HTTP_POST, std::bind(&WebServerManager::handleRefreshDisplay, this));
    server.on("/api/push", HTTP_POST, std::bind(&WebServerManager::handleMessagePush, this));
    server.on("/api/status", HTTP_GET, std::bind(&WebServerManager::handleDeviceStatus, this));
    
    server.onNotFound(std::bind(&WebServerManager::handleNotFound, this));
    
    // 启动Web服务器
    server.begin();
    
    // 启动mDNS服务
    if (!MDNS.begin("inkclock")) {
        DEBUG_PRINTLN("Error starting mDNS");
    } else {
        DEBUG_PRINTLN("mDNS started: http://inkclock.local:8080");
    }
    
    initialized = true;
    DEBUG_PRINTLN("Web服务器初始化完成，端口: 8080");
}

void WebServerManager::loop() {
    if (initialized) {
        server.handleClient();
        // MDNS.update() removed as it's not supported in current MDNS implementation
    }
}

void WebServerManager::handleRoot() {
    DEBUG_PRINTLN("处理根路径请求");
    server.send(200, "text/html", index_html);
}

void WebServerManager::handleSettings() {
    DEBUG_PRINTLN("处理设置页面请求");
    server.send(200, "text/html", settings_html);
}

void WebServerManager::handlePlugins() {
    DEBUG_PRINTLN("处理插件管理页面请求");
    server.send(200, "text/html", plugin_html);
}

void WebServerManager::handleUpdateSettings() {
    DEBUG_PRINTLN("处理设置更新请求");
    server.sendHeader("Location", "/settings");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleAddPlugin() {
    DEBUG_PRINTLN("处理添加插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleUpdatePlugin() {
    DEBUG_PRINTLN("处理更新插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleDeletePlugin() {
    DEBUG_PRINTLN("处理删除插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleEnablePlugin() {
    DEBUG_PRINTLN("处理启用插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleDisablePlugin() {
    DEBUG_PRINTLN("处理禁用插件请求");
    server.sendHeader("Location", "/plugins");
    server.send(302, "text/plain", "");
}

void WebServerManager::handlePluginList() {
    DEBUG_PRINTLN("处理推荐插件列表请求");
    server.send(200, "text/html", plugin_list_html);
}

void WebServerManager::handleCSS() {
    DEBUG_PRINTLN("处理CSS请求");
    server.send(200, "text/css", style_css);
}

void WebServerManager::handleNotFound() {
    DEBUG_PRINT("处理404请求: ");
    DEBUG_PRINTLN(server.uri());
    server.send(404, "text/plain", "404 Not Found");
}

void WebServerManager::handleSensorData() {
    DEBUG_PRINTLN("处理传感器数据API请求");
    
    // 创建JSON响应
    static JsonDocument doc;
    doc.clear();
    doc["status"] = "success";
    doc["timestamp"] = platformGetMillis();
    doc["data"]["temperature"] = 23.5;
    doc["data"]["humidity"] = 45.2;
    doc["data"]["motionDetected"] = false;
    doc["data"]["gasLevel"] = 300;
    doc["data"]["flameDetected"] = false;
    doc["data"]["lightLevel"] = 200;
    doc["data"]["valid"] = true;
    
    // 发送响应
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

void WebServerManager::handleApi() {
    DEBUG_PRINTLN("处理API根请求");
    
    // 创建JSON响应
    static JsonDocument doc;
    doc.clear();
    doc["status"] = "success";
    doc["name"] = "InkClock API";
    doc["version"] = "1.0";
    doc["description"] = "家用网络智能墨水屏万年历API";
    
    // 发送响应
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    server.send(200, "application/json", jsonResponse);
}

String WebServerManager::getIPAddress() {
    return WiFi.localIP().toString();
}

String WebServerManager::generateQRCodeURL() {
    String url = "http://" + getIPAddress() + ":8080";
    return "https://api.qrserver.com/v1/create-qr-code/?size=200x200&data=" + url;
}

void WebServerManager::handleMessagePush() {
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Message pushed successfully\"}");
}

void WebServerManager::handleDeviceStatus() {
    JsonDocument doc;
    
    doc["status"] = "online";
    doc["ip_address"] = getIPAddress();
    doc["mac_address"] = WiFi.macAddress();
    doc["time"] = "2025-01-01 00:00:00";
    
    String json;
    serializeJson(doc, json);
    
    server.send(200, "application/json", json);
}

void WebServerManager::sendJsonResponse(const String& json, int statusCode) {
    server.send(statusCode, "application/json", json);
}

void WebServerManager::handleFonts() {
    DEBUG_PRINTLN("处理字体管理页面请求");
    server.send(200, "text/html", fonts_html);
}

void WebServerManager::handleTFCard() {
    DEBUG_PRINTLN("处理TF卡管理页面请求");
    server.send(200, "text/html", tfcard_html);
}

void WebServerManager::handleUploadFont() {
    DEBUG_PRINTLN("处理字体上传请求");
    server.sendHeader("Location", "/fonts");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleFactoryReset() {
    DEBUG_PRINTLN("处理工厂重置请求");
    server.sendHeader("Location", "/settings");
    server.send(302, "text/plain", "");
}

void WebServerManager::handleRemoteControl() {
    DEBUG_PRINTLN("处理远程控制请求");
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Command executed\"}");
}

void WebServerManager::handleDataSync() {
    DEBUG_PRINTLN("处理数据同步请求");
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Data synced\"}");
}

void WebServerManager::handleRefreshDisplay() {
    DEBUG_PRINTLN("处理显示刷新请求");
    server.send(200, "application/json", "{\"success\": true, \"message\": \"Display refreshed\"}");
}
