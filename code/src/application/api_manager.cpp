/**
 * @file api_manager.cpp
 * @brief API管理模块实现
 * @author iswhat
 * @date 2025-12-26
 * @version 1.0
 */

#include "api_manager.h"
#include "application/wifi_manager.h"


// 外部全局对象
extern WiFiManager wifiManager;

// 使用config.h中定义的配置参数
#include "../coresystem/config.h"
#include "../coresystem/config_manager.h"

APIManager::APIManager() {
    // 初始化成员变量
    #if PLATFORM_ESP32
    wifiClient = nullptr;
    httpClient = nullptr;
    #elif PLATFORM_ESP8266
    wifiClient = nullptr;
    httpClient = nullptr;
    #endif
    lastCacheCleanup = 0;
    verifyCertificate = false; // 默认禁用证书验证，简化开发
    useProxy = false;
    proxyPort = 0;
    maxCacheSize = 100; // 默认最大缓存大小为100项
    connectionTimeout = 10000; // 连接超时时间（毫秒）
    maxRetries = 3; // 最大重试次数
    retryDelay = 1000; // 重试延迟（毫秒）
    requestQueueSize = 10; // 请求队列大小
    
    // 初始化缓存时间配置
    defaultCacheTime = 3600000; // 默认1小时
    weatherCacheTime = 600000; // 默认10分钟
    stockCacheTime = 300000; // 默认5分钟
    lunarCacheTime = 86400000; // 默认1天
    
    // 初始化统计信息
    totalRequests = 0;
    successfulRequests = 0;
    failedRequests = 0;
    cachedRequests = 0;
    totalResponseTime = 0;
    cacheHits = 0;
    cacheMisses = 0;
    
    // 初始化请求队列
    requestQueue.clear();
}

APIManager::~APIManager() {
    // 清理资源
    #if PLATFORM_ESP32 || PLATFORM_ESP8266
    if (wifiClient) {
        delete wifiClient;
        wifiClient = nullptr;
    }
    
    if (httpClient) {
        delete httpClient;
        httpClient = nullptr;
    }
    #endif
    
    // 清理缓存
    cache.clear();
}

void APIManager::init() {
    DEBUG_PRINTLN("初始化API管理器...");
    
    // 初始化HTTPS客户端
    #if PLATFORM_ESP32
    wifiClient = new WiFiClientSecure();
    httpClient = new HTTPClient();
    #elif PLATFORM_ESP8266
    wifiClient = new WiFiClientSecure();
    httpClient = new HTTPClient();
    #endif
    
    // 配置客户端
    wifiClient->setInsecure(); // 禁用证书验证
    
    // 从配置文件读取缓存配置
    defaultCacheTime = CONFIG_GET_INT("api.cache_time", 3600000); // 默认1小时
    weatherCacheTime = CONFIG_GET_INT("api.weather_cache_time", 600000); // 默认10分钟
    stockCacheTime = CONFIG_GET_INT("api.stock_cache_time", 300000); // 默认5分钟
    lunarCacheTime = CONFIG_GET_INT("api.lunar_cache_time", 86400000); // 默认1天
    
    // 记录初始时间
    lastCacheCleanup = millis();
    
    DEBUG_PRINT("API管理器初始化完成，默认缓存时间：");
    DEBUG_PRINTLN(defaultCacheTime / 1000);
    DEBUG_PRINT("天气缓存时间：");
    DEBUG_PRINTLN(weatherCacheTime / 1000);
    DEBUG_PRINT("股票缓存时间：");
    DEBUG_PRINTLN(stockCacheTime / 1000);
    DEBUG_PRINT("农历缓存时间：");
    DEBUG_PRINTLN(lunarCacheTime / 1000);
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
            cacheHits++;
            DEBUG_PRINTLN("使用缓存数据：" + cacheKey);
            return response;
        }
    } else {
        cacheMisses++;
    }
    
    // 清理过期缓存
    unsigned long now = millis();
    if (now - lastCacheCleanup > API_CACHE_CLEANUP_INTERVAL) {
        cleanupExpiredCache();
        lastCacheCleanup = now;
    }
    
    // 准备HTTP请求
    String fullUrl = request.url;
    
    DEBUG_PRINTLN("发送API请求：" + fullUrl);
    
    // 设置请求超时
    unsigned long requestTimeout = request.timeout > 0 ? request.timeout : API_DEFAULT_TIMEOUT;
    
    // 重试机制
    int retryCount = 0;
    bool requestSuccess = false;
    
    while (retryCount < maxRetries && !requestSuccess) {
        // 开始HTTP请求
        #if PLATFORM_ESP32 || PLATFORM_ESP8266
        httpClient->setTimeout(requestTimeout);
        #if PLATFORM_ESP32
        httpClient->setReuse(true); // 启用连接重用，减少连接建立开销
        #endif
        
        // 设置证书验证
        if (!verifyCertificate) {
            wifiClient->setInsecure(); // 禁用证书验证
        }
        
        // 发送请求
        int httpCode = -1;
        if (request.method.equalsIgnoreCase("GET")) {
            if (!httpClient->begin(*wifiClient, fullUrl)) {
                response.error = "初始化HTTP请求失败";
                response.status = API_STATUS_ERROR;
                retryCount++;
                DEBUG_PRINTLN("API请求失败：初始化HTTP请求失败，重试 " + String(retryCount) + "/" + String(maxRetries));
                delay(retryDelay);
                continue;
            }
            httpCode = httpClient->GET();
        } else if (request.method.equalsIgnoreCase("POST")) {
            if (!httpClient->begin(*wifiClient, fullUrl)) {
                response.error = "初始化HTTP请求失败";
                response.status = API_STATUS_ERROR;
                retryCount++;
                DEBUG_PRINTLN("API请求失败：初始化HTTP请求失败，重试 " + String(retryCount) + "/" + String(maxRetries));
                delay(retryDelay);
                continue;
            }
            httpClient->addHeader("Content-Type", "application/json");
            httpCode = httpClient->POST(request.body);
        } else {
            response.error = "不支持的请求方法：" + request.method;
            DEBUG_PRINTLN(response.error);
            failedRequests++;
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
                DEBUG_PRINTLN("API请求成功：" + String(httpCode) + "，URL：" + fullUrl);
                
                // 保存缓存
                if (request.cacheTime > 0) {
                    saveCache(cacheKey, response, request.cacheTime);
                }
                requestSuccess = true;
            } else {
                // 请求失败
                response.status = API_STATUS_ERROR;
                response.error = "HTTP错误：" + String(httpCode) + "，URL：" + fullUrl;
                retryCount++;
                DEBUG_PRINTLN("API请求失败：" + String(httpCode) + "，URL：" + fullUrl + "，重试 " + String(retryCount) + "/" + String(maxRetries));
                delay(retryDelay);
            }
        } else {
            // 请求失败
            response.status = API_STATUS_TIMEOUT;
            response.error = "请求超时：" + httpClient->errorToString(httpCode) + "，URL：" + fullUrl;
            retryCount++;
            DEBUG_PRINTLN("API请求超时：" + httpClient->errorToString(httpCode) + "，URL：" + fullUrl + "，重试 " + String(retryCount) + "/" + String(maxRetries));
            delay(retryDelay);
        }
        
        // 结束HTTP请求
        httpClient->end();
        #endif
    }
    
    // 如果所有重试都失败
    if (!requestSuccess) {
        failedRequests++;
    }
    
    // 计算响应时间
    unsigned long responseTime = millis() - response.timestamp;
    totalResponseTime += responseTime;
    
    return response;
}

ApiResponse APIManager::sendQueuedRequest(const ApiRequest& request) {
    // 检查请求队列大小
    if (requestQueue.size() >= requestQueueSize) {
        // 队列已满，移除最早的请求
        requestQueue.erase(requestQueue.begin());
    }
    
    // 将请求添加到队列
    requestQueue.push_back(request);
    
    // 处理队列中的请求
    return processRequestQueue();
}

ApiResponse APIManager::processRequestQueue() {
    if (requestQueue.empty()) {
        ApiResponse response;
        response.status = API_STATUS_ERROR;
        response.error = "请求队列为空";
        return response;
    }
    
    // 获取队列中的第一个请求
    ApiRequest request = requestQueue.front();
    requestQueue.erase(requestQueue.begin());
    
    // 处理请求
    return sendRequest(request);
}

void APIManager::optimizeCache() {
    // 优化缓存，使用LRU策略
    // 这里可以实现LRU缓存淘汰算法
    DEBUG_PRINTLN("优化缓存，当前缓存大小：" + String(getCacheSize()));
}

ApiResponse APIManager::get(const String& url, ApiType type, unsigned long cacheTime) {
    // 根据API类型设置默认缓存时间
    if (cacheTime == 0) {
        switch (type) {
            case API_TYPE_WEATHER:
                cacheTime = weatherCacheTime;
                break;
            case API_TYPE_STOCK:
                cacheTime = stockCacheTime;
                break;
            case API_TYPE_LUNAR:
                cacheTime = lunarCacheTime;
                break;
            default:
                cacheTime = defaultCacheTime;
                break;
        }
    }
    
    // 创建GET请求
    ApiRequest request;
    request.url = url;
    request.method = "GET";
    request.timeout = API_DEFAULT_TIMEOUT;
    request.type = type;
    request.cacheTime = cacheTime;
    
    // 发送请求
    return sendRequest(request);
}

ApiResponse APIManager::post(const String& url, const String& body, ApiType type, unsigned long cacheTime) {
    // 根据API类型设置默认缓存时间
    if (cacheTime == 0) {
        switch (type) {
            case API_TYPE_WEATHER:
                cacheTime = weatherCacheTime;
                break;
            case API_TYPE_STOCK:
                cacheTime = stockCacheTime;
                break;
            case API_TYPE_LUNAR:
                cacheTime = lunarCacheTime;
                break;
            default:
                cacheTime = defaultCacheTime;
                break;
        }
    }
    
    // 创建POST请求
    ApiRequest request;
    request.url = url;
    request.method = "POST";
    request.body = body;
    request.timeout = API_DEFAULT_TIMEOUT;
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
    
    // 检查缓存中是否存在该键且未过期
    auto it = cache.find(key);
    if (it != cache.end()) {
        return now < it->second.expireTime;
    }
    
    return false;
}

bool APIManager::getCachedResponse(const String& key, ApiResponse& response) {
    unsigned long now = millis();
    
    // 查找缓存
    auto it = cache.find(key);
    if (it != cache.end() && now < it->second.expireTime) {
        // 找到有效缓存
        response.response = it->second.value;
        response.httpCode = 200;
        response.status = API_STATUS_CACHED;
        response.timestamp = now;
        return true;
    }
    
    return false;
}

void APIManager::saveCache(const String& key, const ApiResponse& response, unsigned long cacheTime) {
    // 检查缓存大小是否超过限制
    if (cache.size() >= maxCacheSize) {
        // 清理过期缓存
        cleanupExpiredCache();
        
        // 如果仍然超过限制，移除最早的缓存项
        if (cache.size() >= maxCacheSize) {
            auto it = cache.begin();
            cache.erase(it);
        }
    }
    
    // 创建新缓存项
    CacheItem item;
    item.value = response.response;
    item.expireTime = millis() + cacheTime;
    item.type = response.status == API_STATUS_SUCCESS ? API_TYPE_CUSTOM : API_TYPE_CUSTOM;
    
    // 添加到缓存（会自动覆盖已存在的键）
    cache[key] = item;
    
    DEBUG_PRINTLN("缓存数据：" + key + "，过期时间：" + String(item.expireTime));
}

void APIManager::cleanupExpiredCache() {
    unsigned long now = millis();
    size_t initialSize = cache.size();
    
    // 遍历缓存，移除过期项
    for (auto it = cache.begin(); it != cache.end();) {
        if (it->second.expireTime < now) {
            it = cache.erase(it);
        } else {
            ++it;
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
        for (auto it = cache.begin(); it != cache.end();) {
            if (it->second.type == type) {
                it = cache.erase(it);
            } else {
                ++it;
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
    
    #if PLATFORM_ESP32 || PLATFORM_ESP8266
    if (!verify) {
        wifiClient->setInsecure(); // 禁用证书验证
        DEBUG_PRINTLN("禁用证书验证");
    } else {
        DEBUG_PRINTLN("启用证书验证");
        // 启用证书验证时不需要调用setInsecure()，这是默认行为
    }
    #endif
}

String APIManager::getStats() {
    // 生成统计信息
    String stats = "API请求统计：\n";
    stats += "总请求数：" + String(totalRequests) + "\n";
    stats += "成功请求：" + String(successfulRequests) + "\n";
    stats += "失败请求：" + String(failedRequests) + "\n";
    stats += "缓存请求：" + String(cachedRequests) + "\n";
    stats += "缓存大小：" + String(getCacheSize()) + "\n";
    stats += "缓存命中：" + String(cacheHits) + "\n";
    stats += "缓存未命中：" + String(cacheMisses) + "\n";
    
    if (totalRequests > 0) {
        unsigned long avgResponseTime = totalResponseTime / totalRequests;
        stats += "平均响应时间：" + String(avgResponseTime) + "ms\n";
    }
    
    if (cacheHits + cacheMisses > 0) {
        float cacheHitRate = (float)cacheHits / (cacheHits + cacheMisses) * 100;
        stats += "缓存命中率：" + String(cacheHitRate, 2) + "%\n";
    }
    
    return stats;
}
