#ifndef SPIFFS_MANAGER_H
#define SPIFFS_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SPIFFS.h>

class SPIFFSManager {
private:
    static SPIFFSManager* instance;
    bool initialized;
    bool mounted;
    
    SPIFFSManager() : initialized(false), mounted(false) {}
    
public:
    static SPIFFSManager* getInstance() {
        if (instance == nullptr) {
            instance = new SPIFFSManager();
        }
        return instance;
    }
    
    bool init() {
        if (initialized) {
            return mounted;
        }
        
        DEBUG_PRINTLN("初始化SPIFFS...");
        
        mounted = SPIFFS.begin(true); // true表示格式化失败的分区
        
        if (mounted) {
            DEBUG_PRINTLN("SPIFFS初始化成功");
        } else {
            DEBUG_PRINTLN("SPIFFS初始化失败");
        }
        
        initialized = true;
        return mounted;
    }
    
    bool isMounted() {
        if (!initialized) {
            init();
        }
        return mounted;
    }
    
    FS& getFS() {
        return SPIFFS;
    }
    
    // 获取分区信息
    void getInfo(size_t& total, size_t& used, size_t& free) {
        if (!isMounted()) {
            total = 0;
            used = 0;
            free = 0;
            return;
        }
        
        total = SPIFFS.totalBytes();
        used = SPIFFS.usedBytes();
        free = total - used;
    }
};

// 初始化单例实例
SPIFFSManager* SPIFFSManager::instance = nullptr;

// 方便使用的全局函数
inline bool initSPIFFS() {
    return SPIFFSManager::getInstance()->init();
}

inline bool isSPIFFSMounted() {
    return SPIFFSManager::getInstance()->isMounted();
}

inline FS& getSPIFFS() {
    return SPIFFSManager::getInstance()->getFS();
}

#endif // SPIFFS_MANAGER_H