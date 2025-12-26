/**
 * @file geo_manager.cpp
 * @brief 地理位置管理模块实现
 * @author iswhat
 * @date 2025-12-26
 * @version 1.0
 */

#include "geo_manager.h"
#include "wifi_manager.h"
#include "api_manager.h"
#include <ArduinoJson.h>

// 外部全局对象
extern WiFiManager wifiManager;
extern APIManager apiManager;

// 静态成员初始化
const char* GeoManager::GEO_API_URL = "http://ip-api.com/json/?lang=zh-CN"; // 主地理API（公共免密钥）
const char* GeoManager::GEO_API_URL_BACKUP = "https://freeipapi.com/api/json/"; // 备用地理API（公共免密钥）
const char* GeoManager::GEO_API_URL_SECONDARY_BACKUP = "https://ipwho.is/"; // 次备用地理API（公共免密钥）
const unsigned long GeoManager::DETECTION_INTERVAL = 86400000; // 24小时

GeoManager::GeoManager() {
    // 初始化成员变量
    autoDetect = AUTO_DETECT_LOCATION;
    lastUpdate = 0;
    
    // 初始化地理位置信息
    currentLocation.cityId = GEO_CITY_ID;
    currentLocation.cityName = GEO_CITY_NAME;
    currentLocation.latitude = GEO_LATITUDE;
    currentLocation.longitude = GEO_LONGITUDE;
    currentLocation.country = "中国";
    currentLocation.region = "";
    currentLocation.autoDetected = false;
}

GeoManager::~GeoManager() {
    // 清理资源
}

void GeoManager::init() {
    DEBUG_PRINTLN("初始化地理位置管理器...");
    
    // 加载保存的地理位置配置
    loadLocation();
    
    // 如果启用自动检测，尝试获取地理位置
    if (autoDetect) {
        update(true);
    }
    
    DEBUG_PRINTLN("地理位置管理器初始化完成");
}

void GeoManager::update(bool forceAutoDetect) {
    // 检查是否需要更新
    unsigned long now = millis();
    if (!forceAutoDetect && now - lastUpdate < DETECTION_INTERVAL) {
        return;
    }
    
    // 如果启用自动检测，尝试获取地理位置
    if (autoDetect || forceAutoDetect) {
        bool success = autoDetectLocation();
        if (success) {
            DEBUG_PRINTLN("自动检测地理位置成功");
            currentLocation.autoDetected = true;
            saveLocation();
        } else {
            DEBUG_PRINTLN("自动检测地理位置失败，使用配置的地理位置");
        }
    }
    
    lastUpdate = now;
}

GeoLocation GeoManager::getLocation() {
    return currentLocation;
}

void GeoManager::setLocation(const GeoLocation& location) {
    currentLocation = location;
    currentLocation.autoDetected = false;
    saveLocation();
    
    DEBUG_PRINTLN("手动设置地理位置成功");
    DEBUG_PRINT("城市: ");
    DEBUG_PRINTLN(currentLocation.cityName);
    DEBUG_PRINT("城市ID: ");
    DEBUG_PRINTLN(currentLocation.cityId);
}

String GeoManager::getCityId() {
    return currentLocation.cityId;
}

String GeoManager::getCityName() {
    return currentLocation.cityName;
}

float GeoManager::getLatitude() {
    return currentLocation.latitude;
}

float GeoManager::getLongitude() {
    return currentLocation.longitude;
}

bool GeoManager::isAutoDetect() {
    return autoDetect;
}

void GeoManager::setAutoDetect(bool autoDetect) {
    this->autoDetect = autoDetect;
    saveLocation();
    
    DEBUG_PRINT("自动检测设置为: ");
    DEBUG_PRINTLN(autoDetect ? "开启" : "关闭");
}

void GeoManager::loop() {
    // 定期更新地理位置信息
    update();
}

bool GeoManager::autoDetectLocation() {
    // 检查WiFi连接
    if (!wifiManager.isConnected()) {
        DEBUG_PRINTLN("WiFi未连接，无法自动检测地理位置");
        return false;
    }
    
    DEBUG_PRINTLN("正在自动检测地理位置...");
    
    // 尝试使用主API
    if (tryDetectLocation(GEO_API_URL)) {
        return true;
    }
    
    // 尝试使用备用API
    if (tryDetectLocation(GEO_API_URL_BACKUP)) {
        return true;
    }
    
    // 尝试使用次备用API
    if (tryDetectLocation(GEO_API_URL_SECONDARY_BACKUP)) {
        return true;
    }
    
    DEBUG_PRINTLN("所有地理位置API都失败了");
    return false;
}

bool GeoManager::tryDetectLocation(const char* apiUrl) {
    // 使用API管理器发送HTTP请求
    ApiResponse apiResponse = apiManager.get(apiUrl, API_TYPE_CUSTOM, DETECTION_INTERVAL);
    
    // 检查请求结果
    if (apiResponse.status != API_STATUS_SUCCESS && apiResponse.status != API_STATUS_CACHED) {
        DEBUG_PRINTLN("获取地理位置失败: " + apiResponse.error);
        return false;
    }
    
    String response = apiResponse.response;
    if (response.isEmpty()) {
        DEBUG_PRINTLN("获取地理位置失败，响应为空");
        return false;
    }
    
    // 解析JSON响应
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, response);
    
    if (error) {
        DEBUG_PRINT("地理位置JSON解析失败: ");
        DEBUG_PRINTLN(error.c_str());
        return false;
    }
    
    // 解析地理位置信息
    GeoLocation geoInfo;
    
    // 根据不同API的响应格式解析数据
    if (String(apiUrl).indexOf("ip-api.com") != -1) {
        // ip-api.com 格式
        if (doc["status"].as<String>() != "success") {
            DEBUG_PRINT("IP地理位置API请求失败: ");
            DEBUG_PRINTLN(doc["message"].as<String>());
            return false;
        }
        
        geoInfo.cityName = doc["city"].as<String>();
        geoInfo.country = doc["country"].as<String>();
        geoInfo.region = doc["regionName"].as<String>();
        geoInfo.latitude = doc["lat"].as<float>();
        geoInfo.longitude = doc["lon"].as<float>();
    } else if (String(apiUrl).indexOf("freeipapi.com") != -1) {
        // freeipapi.com 格式
        geoInfo.cityName = doc["city"].as<String>();
        geoInfo.country = doc["country_name"].as<String>();
        geoInfo.region = doc["region_name"].as<String>();
        geoInfo.latitude = doc["latitude"].as<float>();
        geoInfo.longitude = doc["longitude"].as<float>();
    } else if (String(apiUrl).indexOf("ipwho.is") != -1) {
        // ipwho.is 格式
        if (!doc["success"].as<bool>()) {
            DEBUG_PRINT("IP地理位置API请求失败: ");
            DEBUG_PRINTLN(doc["message"].as<String>());
            return false;
        }
        
        geoInfo.cityName = doc["city"].as<String>();
        geoInfo.country = doc["country"].as<String>();
        geoInfo.region = doc["region"].as<String>();
        geoInfo.latitude = doc["latitude"].as<float>();
        geoInfo.longitude = doc["longitude"].as<float>();
    }
    
    // 注意：大多数IP地理位置API不提供城市ID，需要使用其他API或手动配置
    // 这里使用城市名称作为临时解决方案
    geoInfo.cityId = currentLocation.cityId; // 保持原有城市ID
    geoInfo.autoDetected = true;
    
    // 更新当前地理位置信息
    currentLocation = geoInfo;
    
    DEBUG_PRINT("自动检测到地理位置: ");
    DEBUG_PRINT(currentLocation.cityName);
    DEBUG_PRINT(", ");
    DEBUG_PRINT(currentLocation.region);
    DEBUG_PRINT(", ");
    DEBUG_PRINT(currentLocation.country);
    DEBUG_PRINT(", 坐标: ");
    DEBUG_PRINT(currentLocation.latitude);
    DEBUG_PRINT(", ");
    DEBUG_PRINTLN(currentLocation.longitude);
    
    return true;
}

void GeoManager::saveLocation() {
    // TODO: 保存地理位置配置到EEPROM或SPIFFS
    // 目前暂时只保存在内存中
    DEBUG_PRINTLN("保存地理位置配置");
}

void GeoManager::loadLocation() {
    // TODO: 从EEPROM或SPIFFS加载地理位置配置
    // 目前暂时只使用默认配置
    DEBUG_PRINTLN("加载地理位置配置");
}
