/**
 * @file geo_manager.h
 * @brief 地理位置管理模块
 * @author iswhat
 * @date 2025-12-26
 * @version 1.0
 * 
 * 该模块负责：
 * 1. 实现IP地理位置自动识别
 * 2. 提供地理位置配置的存储和读取
 * 3. 提供地理位置信息的获取接口
 */

#ifndef GEO_MANAGER_H
#define GEO_MANAGER_H

#include <Arduino.h>
#include "../coresystem/config.h"

// 地理位置信息结构体
typedef struct {
    String cityId;        // 城市ID
    String cityName;      // 城市名称
    float latitude;       // 纬度
    float longitude;      // 经度
    String country;       // 国家
    String region;        // 地区/省份
    bool autoDetected;     // 是否自动检测
} GeoLocation;

/**
 * @class GeoManager
 * @brief 地理位置管理类，负责地理位置的自动识别和手动配置
 */
class GeoManager {
public:
    /**
     * @brief 构造函数
     */
    GeoManager();
    
    /**
     * @brief 析构函数
     */
    ~GeoManager();
    
    /**
     * @brief 初始化地理位置管理器
     */
    void init();
    
    /**
     * @brief 更新地理位置信息
     * @param forceAutoDetect 是否强制重新检测
     */
    void update(bool forceAutoDetect = false);
    
    /**
     * @brief 获取当前地理位置信息
     * @return GeoLocation 当前地理位置信息
     */
    GeoLocation getLocation();
    
    /**
     * @brief 设置地理位置信息
     * @param location 地理位置信息
     */
    void setLocation(const GeoLocation& location);
    
    /**
     * @brief 获取城市ID
     * @return String 城市ID
     */
    String getCityId();
    
    /**
     * @brief 获取城市名称
     * @return String 城市名称
     */
    String getCityName();
    
    /**
     * @brief 获取纬度
     * @return float 纬度
     */
    float getLatitude();
    
    /**
     * @brief 获取经度
     * @return float 经度
     */
    float getLongitude();
    
    /**
     * @brief 是否自动检测地理位置
     * @return bool 是否自动检测
     */
    bool isAutoDetect();
    
    /**
     * @brief 设置自动检测开关
     * @param autoDetect 是否自动检测
     */
    void setAutoDetect(bool autoDetect);
    
    /**
     * @brief 循环处理函数，定期更新地理位置信息
     */
    void loop();
    
private:
    /**
     * @brief 从IP自动检测地理位置
     * @return bool 检测是否成功
     */
    bool autoDetectLocation();
    
    /**
     * @brief 保存地理位置信息到配置
     */
    void saveLocation();
    
    /**
     * @brief 从配置加载地理位置信息
     */
    void loadLocation();
    
    // 地理位置信息
    GeoLocation currentLocation;
    
    // 是否自动检测
    bool autoDetect;
    
    // 上次更新时间
    unsigned long lastUpdate;
    
    // IP地理位置检测API
    static const char* GEO_API_URL; // 主地理API URL
    static const char* GEO_API_URL_BACKUP; // 备用地理API URL
    static const char* GEO_API_URL_SECONDARY_BACKUP; // 次备用地理API URL
    
    // 检测间隔（毫秒）
    static const unsigned long DETECTION_INTERVAL;
    
    /**
     * @brief 尝试从指定API检测地理位置
     * @param apiUrl API URL
     * @return bool 检测是否成功
     */
    bool tryDetectLocation(const char* apiUrl);
};

#endif // GEO_MANAGER_H
