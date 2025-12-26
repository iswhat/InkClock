/**
 * @file api_manager.cpp
 * @brief API管理模块实现
 * @author DIY爱好者团队
 * @date 2025-12-26
 * @version 1.0
 */

#include "api_manager.h"
#include "wifi_manager.h"
#include "debug_config.h"

// 外部全局对象
extern WiFiManager wifiManager;

// 默认配置
#define DEFAULT_TIMEOUT 10000          // 默认超时时间：10秒
#define DEFAULT_CACHE_TIME 3600000     // 默认缓存时间：1小时
#define CACHE_CLEANUP_INTERVAL 3600000 // 缓存清理间隔：1小时

APIManager::APIManager() {
    // 初始化成员变量
    wifiClient = nullptr;
    httpClient = nullptr;
    lastCacheCleanup = 0;
    verifyCertificate = false; // 默认禁用证书验证，简化开发
    useProxy = false;
    proxyPort = 0;
    
    // 初始化统计信息
    totalRequests = 0;
    successfulRequests = 0;
    failedRequests = 0;
    cachedRequests = 0;
    totalResponseTime = 0;
}

APIManager::~APIManager() {
    // 清理资源
    if (wifiClient) {
        delete wifiClient;
        wifiClient = nullptr;
    }
    
    if (httpClient) {
        delete httpClient;
        httpClient = nullptr;
    }
    
    // 清理缓存
    cache.clear();
}

void APIManager::init() {
    DEBUG_PRINTLN("初始化API管理器...");
    
    // 创建HTTP客户端
    wifiClient = new WiFiClientSecure();
    httpClient = new HTTPClient();
    
    // 配置客户端
    wifiClient->setInsecure(); // 禁用证书验证
    
    // 记录初始时间
    lastCacheCleanup = millis();
    
    DEBUG_PRINTLN("API管理器初始化完成");
}

ApiResponse APIManager::sendRequest(const ApiRequest& request) {
    // 创建响应对象
    ApiResponse response;
    response.timestamp = millis();
    response.status = API_STATUS_ERROR;
    
    // 检查WiFi连接
    if (!wifiManager.isConnected()) {
        response.error = "WiFi未连接";
        DEBUG_PRINTLN("API请求失败：WiFi未连接");
        return response;
    }
    
    // 增加请求计数
    totalRequests++;
    
    // 生成缓存键
    String cacheKey = generateCacheKey(request);
    
    // 检查缓存
    if (request.cacheTime > 0 && isCacheValid(cacheKey)) {
        if (getCachedResponse(cacheKey, response)) {
            // 使用缓存数据
            response.status = API_STATUS_CACHED;
            cachedRequests++;
            DEBUG_PRINTLN("使用缓存数据：" + cacheKey);
            return response;
        }
    }
    
    // 清理过期缓存
    unsigned long now = millis();
    if (now - lastCacheCleanup > CACHE_CLEANUP_INTERVAL) {
        cleanupExpiredCache();
        lastCacheCleanup = now;
    }
    
    // 准备HTTP请求
    String fullUrl = request.url;
    
    DEBUG_PRINTLN("发送API请求：" + fullUrl);
    
    // 设置请求超时
    unsigned long requestTimeout = request.timeout > 0 ? request.timeout : DEFAULT_TIMEOUT;
    
    // 开始HTTP请求
    httpClient->setTimeout(requestTimeout);
    httpClient->setReuse(false);
    
    // 设置证书验证
    if (verifyCertificate) {
        wifiClient->setInsecure(false);
    } else {
        wifiClient->setInsecure(true);
    }
    
    // 发送请求
    int httpCode = -1;
    if (request.method.equalsIgnoreCase("GET")) {
        httpCode = httpClient->begin(*wifiClient, fullUrl);
    } else if (request.method.equalsIgnoreCase("POST")) {
        httpCode = httpClient->begin(*wifiClient, fullUrl);
        httpClient->addHeader("Content-Type", "application/json");
        httpCode = httpClient->POST(request.body);
    } else {
        response.error = "不支持的请求方法：" + request.method;
        DEBUG_PRINTLN(response.error);
        return response;
    }
    
    // 发送请求并获取响应
    if (httpCode > 0) {
        // 请求成功，读取响应内容
        String responseContent = httpClient->getString();
        
        // 填充响应对象
        response.httpCode = httpCode;
        response.response = responseContent;
        
        if (httpCode >= 200 && httpCode < 300) {
            // 请求成功
            response.status = API_STATUS_SUCCESS;
            successfulRequests++;
            DEBUG_PRINTLN("API请求成功：" + String(httpCode));
            
            // 保存缓存
            if (request.cacheTime > 0) {
                saveCache(cacheKey, response, request.cacheTime);
            }
        } else {
            // 请求失败
            response.status = API_STATUS_ERROR;
            response.error = "HTTP错误：" + String(httpCode);
            failedRequests++;
            DEBUG_PRINTLN("API请求失败：" + String(httpCode));
        }
    } else {
        // 请求失败
        response.status = API_STATUS_TIMEOUT;
        response.error = "请求超时：" + httpClient->errorToString(httpCode);
        failedRequests++;
        DEBUG_PRINTLN("API请求超时：" + httpClient->errorToString(httpCode));
    }
    
    // 结束请求
    httpClient->end();
    
    // 计算响应时间
    unsigned long responseTime = millis() - response.timestamp;
    totalResponseTime += responseTime;
    
    return response;
}

ApiResponse APIManager::get(const String& url, ApiType type, unsigned long cacheTime) {
    // 创建GET请求
    ApiRequest request;
    request.url = url;
    request.method = "GET";
    request.timeout = DEFAULT_TIMEOUT;
    request.type = type;
    request.cacheTime = cacheTime;
    
    // 发送请求
    return sendRequest(request);
}

ApiResponse APIManager::post(const String& url, const String& body, ApiType type, unsigned long cacheTime) {
    // 创建POST请求
    ApiRequest request;
    request.url = url;
    request.method = "POST";
    request.body = body;
    request.timeout = DEFAULT_TIMEOUT;
    request.type = type;
    request.cacheTime = cacheTime;
    
    // 发送请求
    return sendRequest(request);
}

String APIManager::generateCacheKey(const ApiRequest& request) {
    // 根据请求URL和类型生成缓存键
    return String(request.type) + "_" + request.url;
}

bool APIManager::isCacheValid(const String& key) {
    unsigned long now = millis();
    
    // 遍历缓存
    for (const CacheItem& item : cache) {
        if (item.key == key && now < item.expireTime) {
            return true;
        }
    }
    
    return false;
}

bool APIManager::getCachedResponse(const String& key, ApiResponse& response) {
    unsigned long now = millis();
    
    // 遍历缓存
    for (const CacheItem& item : cache) {
        if (item.key == key && now < item.expireTime) {
            // 找到有效缓存
            response.response = item.value;
            response.httpCode = 200;
            response.status = API_STATUS_CACHED;
            response.timestamp = now;
            return true;
        }
    }
    
    return false;
}

void APIManager::saveCache(const String& key, const ApiResponse& response, unsigned long cacheTime) {
    // 移除旧缓存
    for (size_t i = 0; i < cache.size(); i++) {
        if (cache[i].key == key) {
            cache.erase(cache.begin() + i);
            break;
        }
    }
    
    // 创建新缓存项
    CacheItem item;
    item.key = key;
    item.value = response.response;
    item.expireTime = millis() + cacheTime;
    item.type = response.status == API_STATUS_SUCCESS ? API_TYPE_CUSTOM : API_TYPE_CUSTOM;
    
    // 添加到缓存
    cache.push_back(item);
    
    DEBUG_PRINTLN("缓存数据：" + key + "，过期时间：" + String(item.expireTime));
}

void APIManager::cleanupExpiredCache() {
    unsigned long now = millis();
    size_t initialSize = cache.size();
    
    // 遍历缓存，移除过期项
    for (size_t i = cache.size(); i > 0; i--) {
        if (cache[i - 1].expireTime < now) {
            cache.erase(cache.begin() + (i - 1));
        }
    }
    
    size_t removed = initialSize - cache.size();
    if (removed > 0) {
        DEBUG_PRINTLN("清理过期缓存：" + String(removed) + "项");
    }
}

void APIManager::clearCache(ApiType type) {
    if (type == API_TYPE_CUSTOM) {
        // 清除所有缓存
        cache.clear();
        DEBUG_PRINTLN("清除所有缓存");
    } else {
        // 清除指定类型的缓存
        for (size_t i = cache.size(); i > 0; i--) {
            if (cache[i - 1].type == type) {
                cache.erase(cache.begin() + (i - 1));
            }
        }
        DEBUG_PRINTLN("清除类型为" + String(type) + "的缓存");
    }
}

void APIManager::clearAllCache() {
    cache.clear();
    DEBUG_PRINTLN("清除所有缓存");
}

int APIManager::getCacheSize() {
    return cache.size();
}

void APIManager::setProxy(const String& proxyHost, uint16_t proxyPort) {
    this->proxyHost = proxyHost;
    this->proxyPort = proxyPort;
    this->useProxy = !proxyHost.isEmpty() && proxyPort > 0;
    
    if (useProxy) {
        DEBUG_PRINTLN("设置代理：" + proxyHost + ":" + String(proxyPort));
    } else {
        DEBUG_PRINTLN("清除代理设置");
    }
}

void APIManager::setCertificateVerify(bool verify) {
    this->verifyCertificate = verify;
    
    if (verify) {
        wifiClient->setInsecure(false);
        DEBUG_PRINTLN("启用证书验证");
    } else {
        wifiClient->setInsecure(true);
        DEBUG_PRINTLN("禁用证书验证");
    }
}

String APIManager::getStats() {
    // 生成统计信息
    String stats = "API请求统计：\n";
    stats += "总请求数：" + String(totalRequests) + "\n";
    stats += "成功请求：" + String(successfulRequests) + "\n";
    stats += "失败请求：" + String(failedRequests) + "\n";
    stats += "缓存请求：" + String(cachedRequests) + "\n";
    stats += "缓存大小：" + String(getCacheSize()) + "\n";
    
    if (totalRequests > 0) {
        unsigned long avgResponseTime = totalResponseTime / totalRequests;
        stats += "平均响应时间：" + String(avgResponseTime) + "ms\n";
    }
    
    return stats;
}
