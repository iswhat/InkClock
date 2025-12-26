/**
 * @file api_manager.h
 * @brief API管理模块
 * @author iswhat
 * @date 2025-12-26
 * @version 1.0
 * 
 * 该模块负责统一管理所有外部API请求，包括：
 * 1. 统一的HTTP/HTTPS请求处理
 * 2. 请求缓存机制
 * 3. 错误处理和重试逻辑
 * 4. API配置管理
 * 5. 接口映射和中转
 */

#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <time.h>

// API类型枚举
enum ApiType {
    API_TYPE_LUNAR = 0,      // 农历API
    API_TYPE_WEATHER = 1,    // 天气API
    API_TYPE_NTP = 2,        // NTP时间API
    API_TYPE_STOCK = 3,      // 股票API
    API_TYPE_CUSTOM = 4      // 自定义API
};

// API状态枚举
enum ApiStatus {
    API_STATUS_SUCCESS = 0,  // 请求成功
    API_STATUS_ERROR = 1,    // 请求失败
    API_STATUS_CACHED = 2,   // 使用缓存数据
    API_STATUS_TIMEOUT = 3   // 请求超时
};

// API请求结构体
typedef struct {
    String url;              // 请求URL
    String method;           // 请求方法（GET/POST等）
    String headers;          // 请求头
    String body;             // 请求体
    unsigned long timeout;   // 请求超时时间（毫秒）
    ApiType type;            // API类型
    unsigned long cacheTime; // 缓存时间（毫秒，0表示不缓存）
} ApiRequest;

// API响应结构体
typedef struct {
    int httpCode;            // HTTP响应码
    String response;         // 响应内容
    ApiStatus status;        // API请求状态
    String error;            // 错误信息
    unsigned long timestamp; // 响应时间戳
} ApiResponse;

// 缓存项结构体
typedef struct {
    String value;            // 缓存值
    unsigned long expireTime;// 过期时间
    ApiType type;            // API类型
} CacheItem;

/**
 * @class APIManager
 * @brief API管理类，负责统一处理所有外部API请求
 */
class APIManager {
public:
    /**
     * @brief 构造函数
     */
    APIManager();
    
    /**
     * @brief 析构函数
     */
    ~APIManager();
    
    /**
     * @brief 初始化API管理器
     */
    void init();
    
    /**
     * @brief 发送API请求
     * @param request API请求结构体
     * @return ApiResponse API响应结构体
     */
    ApiResponse sendRequest(const ApiRequest& request);
    
    /**
     * @brief 简化的GET请求
     * @param url 请求URL
     * @param type API类型
     * @param cacheTime 缓存时间（毫秒）
     * @return ApiResponse API响应结构体
     */
    ApiResponse get(const String& url, ApiType type, unsigned long cacheTime = 3600000);
    
    /**
     * @brief 简化的POST请求
     * @param url 请求URL
     * @param body 请求体
     * @param type API类型
     * @param cacheTime 缓存时间（毫秒）
     * @return ApiResponse API响应结构体
     */
    ApiResponse post(const String& url, const String& body, ApiType type, unsigned long cacheTime = 3600000);
    
    /**
     * @brief 清除指定类型的缓存
     * @param type API类型
     */
    void clearCache(ApiType type = API_TYPE_CUSTOM);
    
    /**
     * @brief 清除所有缓存
     */
    void clearAllCache();
    
    /**
     * @brief 获取缓存大小
     * @return int 缓存项数量
     */
    int getCacheSize();
    
    /**
     * @brief 设置代理服务器
     * @param proxyHost 代理服务器主机
     * @param proxyPort 代理服务器端口
     */
    void setProxy(const String& proxyHost, uint16_t proxyPort);
    
    /**
     * @brief 启用/禁用证书验证
     * @param verify 是否验证证书
     */
    void setCertificateVerify(bool verify);
    
    /**
     * @brief 获取API请求统计信息
     * @return String 统计信息
     */
    String getStats();
    
private:
    /**
     * @brief 生成缓存键
     * @param request API请求结构体
     * @return String 缓存键
     */
    String generateCacheKey(const ApiRequest& request);
    
    /**
     * @brief 检查缓存是否有效
     * @param key 缓存键
     * @return bool 是否有效
     */
    bool isCacheValid(const String& key);
    
    /**
     * @brief 获取缓存数据
     * @param key 缓存键
     * @param response 响应结构体，用于存储缓存数据
     * @return bool 是否成功获取缓存
     */
    bool getCachedResponse(const String& key, ApiResponse& response);
    
    /**
     * @brief 保存缓存数据
     * @param key 缓存键
     * @param response API响应结构体
     * @param cacheTime 缓存时间（毫秒）
     */
    void saveCache(const String& key, const ApiResponse& response, unsigned long cacheTime);
    
    /**
     * @brief 清理过期缓存
     */
    void cleanupExpiredCache();
    
    // HTTPS客户端
    WiFiClientSecure *wifiClient;
    HTTPClient *httpClient;
    
    // 缓存相关
    std::unordered_map<String, CacheItem> cache; // 使用unordered_map提高查找效率
    unsigned long lastCacheCleanup;
    unsigned long maxCacheSize; // 最大缓存大小
    
    // 配置参数
    bool verifyCertificate;
    String proxyHost;
    uint16_t proxyPort;
    bool useProxy;
    
    // 统计信息
    unsigned long totalRequests;
    unsigned long successfulRequests;
    unsigned long failedRequests;
    unsigned long cachedRequests;
    unsigned long totalResponseTime;
};

#endif // API_MANAGER_H
