#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "arduino_compat.h"
#endif

#include <string>
#include <memory>
#include <map>
#include <vector>

// 配置层级枚举
enum ConfigLevel {
    CONFIG_LEVEL_DEFAULT,   // 默认配置（编译时内置）
    CONFIG_LEVEL_PERSISTENT, // 持久化配置（存储在文件系统）
    CONFIG_LEVEL_RUNTIME     // 运行时配置（内存中，重启后丢失）
};

// 配置存储类型枚举
enum ConfigStorageType {
    STORAGE_TYPE_SPIFFS,
    STORAGE_TYPE_SD_CARD,
    STORAGE_TYPE_EEPROM,
    STORAGE_TYPE_RAM
};

// 配置项类
class ConfigItem {
private:
    String key;
    String value;
    String description;
    ConfigLevel level;
    bool editable;
    String defaultValue;
    String validationPattern;

public:
    ConfigItem(
        const String& k,
        const String& v,
        const String& desc,
        ConfigLevel lvl,
        bool edit = true,
        const String& defVal = "",
        const String& validation = ""
    );

    // 获取配置项信息
    String getKey() const;
    String getValue() const;
    String getDescription() const;
    ConfigLevel getLevel() const;
    bool isEditable() const;
    String getDefaultValue() const;
    String getValidationPattern() const;

    // 设置配置值
    bool setValue(const String& v);

    // 验证配置值
    bool validate() const;
    bool validate(const String& v) const;

    // 重置为默认值
    void resetToDefault();
};

// 配置存储接口
class IConfigStorage {
public:
    virtual ~IConfigStorage() {}
    virtual bool init() = 0;
    virtual bool load(const String& key, String& value) = 0;
    virtual bool save(const String& key, const String& value) = 0;
    virtual bool remove(const String& key) = 0;
    virtual bool clear() = 0;
    virtual bool exists(const String& key) = 0;
    virtual std::vector<String> listKeys() = 0;
    virtual ConfigStorageType getType() = 0;
};

// 配置管理器类
class ConfigManager {
private:
    static ConfigManager* instance;
    std::map<String, std::shared_ptr<ConfigItem>> configItems;
    std::vector<std::shared_ptr<IConfigStorage>> storageBackends;
    std::shared_ptr<IConfigStorage> activeStorage;
    bool initialized;

    ConfigManager();

public:
    static ConfigManager* getInstance();

    // 初始化
    bool init();

    // 注册配置存储后端
    bool registerStorageBackend(std::shared_ptr<IConfigStorage> storage);

    // 设置活动存储后端
    bool setActiveStorage(ConfigStorageType type);

    // 注册配置项
    bool registerConfigItem(
        const String& key,
        const String& value,
        const String& description,
        ConfigLevel level,
        bool editable = true,
        const String& defaultValue = "",
        const String& validationPattern = ""
    );

    // 获取配置值
    String getString(const String& key, const String& defaultValue = "");
    int getInt(const String& key, int defaultValue = 0);
    float getFloat(const String& key, float defaultValue = 0.0f);
    bool getBool(const String& key, bool defaultValue = false);

    // 设置配置值
    bool setString(const String& key, const String& value, ConfigLevel level = CONFIG_LEVEL_RUNTIME);
    bool setInt(const String& key, int value, ConfigLevel level = CONFIG_LEVEL_RUNTIME);
    bool setFloat(const String& key, float value, ConfigLevel level = CONFIG_LEVEL_RUNTIME);
    bool setBool(const String& key, bool value, ConfigLevel level = CONFIG_LEVEL_RUNTIME);

    // 检查配置项是否存在
    bool hasConfig(const String& key) const;

    // 获取配置项信息
    std::shared_ptr<ConfigItem> getConfigItem(const String& key) const;

    // 获取所有配置项
    std::vector<std::shared_ptr<ConfigItem>> getAllConfigItems() const;
    std::vector<std::shared_ptr<ConfigItem>> getConfigItemsByLevel(ConfigLevel level) const;

    // 加载配置
    bool loadConfig();

    // 保存配置
    bool saveConfig();

    // 重置配置
    bool resetConfig(ConfigLevel level = CONFIG_LEVEL_PERSISTENT);

    // 备份配置
    bool backupConfig(const String& backupPath);

    // 恢复配置
    bool restoreConfig(const String& backupPath);

    // 验证所有配置
    bool validateAllConfig() const;

    // 导出配置为JSON
    String exportConfigToJson() const;

    // 从JSON导入配置
    bool importConfigFromJson(const String& json);
};

// SPIFFS配置存储实现
class SPIFFSConfigStorage : public IConfigStorage {
private:
    String configFileName;

public:
    SPIFFSConfigStorage(const String& fileName = "/config.json");
    bool init() override;
    bool load(const String& key, String& value) override;
    bool save(const String& key, const String& value) override;
    bool remove(const String& key) override;
    bool clear() override;
    bool exists(const String& key) override;
    std::vector<String> listKeys() override;
    ConfigStorageType getType() override;
};

// SD卡配置存储实现
class SDCardConfigStorage : public IConfigStorage {
private:
    String configFileName;

public:
    SDCardConfigStorage(const String& fileName = "/config/config.json");
    bool init() override;
    bool load(const String& key, String& value) override;
    bool save(const String& key, const String& value) override;
    bool remove(const String& key) override;
    bool clear() override;
    bool exists(const String& key) override;
    std::vector<String> listKeys() override;
    ConfigStorageType getType() override;
};

// RAM配置存储实现（用于运行时配置）
class RAMConfigStorage : public IConfigStorage {
private:
    std::map<String, String> configMap;

public:
    RAMConfigStorage();
    bool init() override;
    bool load(const String& key, String& value) override;
    bool save(const String& key, const String& value) override;
    bool remove(const String& key) override;
    bool clear() override;
    bool exists(const String& key) override;
    std::vector<String> listKeys() override;
    ConfigStorageType getType() override;
};

// 配置管理宏
#define CONFIG_GET_STRING(key, default) ConfigManager::getInstance()->getString(key, default)
#define CONFIG_GET_INT(key, default) ConfigManager::getInstance()->getInt(key, default)
#define CONFIG_GET_FLOAT(key, default) ConfigManager::getInstance()->getFloat(key, default)
#define CONFIG_GET_BOOL(key, default) ConfigManager::getInstance()->getBool(key, default)

#define CONFIG_SET_STRING(key, value) ConfigManager::getInstance()->setString(key, value)
#define CONFIG_SET_INT(key, value) ConfigManager::getInstance()->setInt(key, value)
#define CONFIG_SET_FLOAT(key, value) ConfigManager::getInstance()->setFloat(key, value)
#define CONFIG_SET_BOOL(key, value) ConfigManager::getInstance()->setBool(key, value)

#define CONFIG_HAS(key) ConfigManager::getInstance()->hasConfig(key)

#endif // CONFIG_MANAGER_H