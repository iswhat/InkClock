#ifndef TF_CARD_MANAGER_H
#define TF_CARD_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <SD.h>

class TFCardManager {
private:
    static TFCardManager* instance;
    bool initialized;
    bool mounted;
    uint8_t chipSelectPin;
    
    TFCardManager() : initialized(false), mounted(false), chipSelectPin(5) {} // 默认CS引脚为5
    
public:
    static TFCardManager* getInstance() {
        if (instance == nullptr) {
            instance = new TFCardManager();
        }
        return instance;
    }
    
    bool init(uint8_t csPin = 5) {
        if (initialized) {
            return mounted;
        }
        
        DEBUG_PRINTLN("初始化TF卡...");
        this->chipSelectPin = csPin;
        
        // 尝试挂载TF卡
        mounted = SD.begin(chipSelectPin);
        
        if (mounted) {
            DEBUG_PRINTLN("TF卡初始化成功");
            
            // 打印TF卡信息
            uint64_t cardSize = SD.cardSize() / (1024 * 1024); // MB
            uint64_t totalBytes = SD.totalBytes() / (1024 * 1024); // MB
            uint64_t usedBytes = SD.usedBytes() / (1024 * 1024); // MB
            
            DEBUG_PRINTF("TF卡容量: %llu MB\n", cardSize);
            DEBUG_PRINTF("可用空间: %llu MB\n", totalBytes - usedBytes);
            DEBUG_PRINTF("已用空间: %llu MB\n", usedBytes);
        } else {
            DEBUG_PRINTLN("TF卡初始化失败");
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
        return SD;
    }
    
    // 获取TF卡信息
    void getInfo(uint64_t& total, uint64_t& used, uint64_t& free) {
        if (!isMounted()) {
            total = 0;
            used = 0;
            free = 0;
            return;
        }
        
        total = SD.totalBytes() / (1024 * 1024); // MB
        used = SD.usedBytes() / (1024 * 1024); // MB
        free = total - used;
    }
    
    // 格式化TF卡（谨慎使用）
    bool format() {
        if (!initialized) {
            init();
        }
        
        DEBUG_PRINTLN("格式化TF卡...");
        
        if (SD.begin(chipSelectPin)) {
            DEBUG_PRINTLN("TF卡格式化功能暂不支持");
            mounted = false;
            return false;
        }
        return false;
    }
    
    // 创建目录
    bool mkdir(String path) {
        if (!isMounted()) {
            DEBUG_PRINTLN("TF卡未挂载，无法创建目录");
            return false;
        }
        
        if (SD.exists(path)) {
            DEBUG_PRINTF("目录已存在: %s\n", path.c_str());
            return true;
        }
        
        if (SD.mkdir(path)) {
            DEBUG_PRINTF("创建目录成功: %s\n", path.c_str());
            return true;
        } else {
            DEBUG_PRINTF("创建目录失败: %s\n", path.c_str());
            return false;
        }
    }
    
    // 删除目录
    bool rmdir(String path) {
        if (!isMounted()) {
            DEBUG_PRINTLN("TF卡未挂载，无法删除目录");
            return false;
        }
        
        if (!SD.exists(path)) {
            DEBUG_PRINTF("目录不存在: %s\n", path.c_str());
            return true;
        }
        
        if (SD.rmdir(path)) {
            DEBUG_PRINTF("删除目录成功: %s\n", path.c_str());
            return true;
        } else {
            DEBUG_PRINTF("删除目录失败: %s\n", path.c_str());
            return false;
        }
    }
    
    // 删除文件
    bool remove(String path) {
        if (!isMounted()) {
            DEBUG_PRINTLN("TF卡未挂载，无法删除文件");
            return false;
        }
        
        if (!SD.exists(path)) {
            DEBUG_PRINTF("文件不存在: %s\n", path.c_str());
            return true;
        }
        
        if (SD.remove(path)) {
            DEBUG_PRINTF("删除文件成功: %s\n", path.c_str());
            return true;
        } else {
            DEBUG_PRINTF("删除文件失败: %s\n", path.c_str());
            return false;
        }
    }
    
    // 重命名文件
    bool rename(String oldPath, String newPath) {
        if (!isMounted()) {
            DEBUG_PRINTLN("TF卡未挂载，无法重命名文件");
            return false;
        }
        
        if (!SD.exists(oldPath)) {
            DEBUG_PRINTF("原文件不存在: %s\n", oldPath.c_str());
            return false;
        }
        
        if (SD.exists(newPath)) {
            DEBUG_PRINTF("新文件已存在: %s\n", newPath.c_str());
            return false;
        }
        
        if (SD.rename(oldPath, newPath)) {
            DEBUG_PRINTF("重命名文件成功: %s -> %s\n", oldPath.c_str(), newPath.c_str());
            return true;
        } else {
            DEBUG_PRINTF("重命名文件失败: %s -> %s\n", oldPath.c_str(), newPath.c_str());
            return false;
        }
    }
};

// 初始化单例实例
TFCardManager* TFCardManager::instance = nullptr;

// 方便使用的全局函数
inline bool initTFCard(uint8_t csPin = 5) {
    return TFCardManager::getInstance()->init(csPin);
}

inline bool isTFCardMounted() {
    return TFCardManager::getInstance()->isMounted();
}

inline FS& getTFCard() {
    return TFCardManager::getInstance()->getFS();
}

inline bool tfCardMkdir(String path) {
    return TFCardManager::getInstance()->mkdir(path);
}

inline bool tfCardRmdir(String path) {
    return TFCardManager::getInstance()->rmdir(path);
}

inline bool tfCardRemove(String path) {
    return TFCardManager::getInstance()->remove(path);
}

inline bool tfCardRename(String oldPath, String newPath) {
    return TFCardManager::getInstance()->rename(oldPath, newPath);
}

inline void getTFCardInfo(uint64_t& total, uint64_t& used, uint64_t& free) {
    TFCardManager::getInstance()->getInfo(total, used, free);
}

#endif // TF_CARD_MANAGER_H