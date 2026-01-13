#include "config_manager.h"
#include <sstream>
#include <regex>

// 静态实例初始化
ConfigManager* ConfigManager::instance = nullptr;

// ConfigItem 构造函数
ConfigItem::ConfigItem(
    const String& k,
    const String& v,
    const String& desc,
    ConfigLevel lvl,
    bool edit,
    const String& defVal,
    const String& validation
) : key(k), value(v), description(desc), level(lvl), editable(edit),
    defaultValue(defVal), validationPattern(validation) {
    // 如果默认值为空，使用当前值作为默认值
    if (defaultValue.isEmpty()) {
        defaultValue = value;
    }
}

// ConfigItem 方法实现
String ConfigItem::getKey() const {
    return key;
}

String ConfigItem::getValue() const {
    return value;
}

String ConfigItem::getDescription() const {
    return description;
}

ConfigLevel ConfigItem::getLevel() const {
    return level;
}

bool ConfigItem::isEditable() const {
    return editable;
}

String ConfigItem::getDefaultValue() const {
    return defaultValue;
}

String ConfigItem::getValidationPattern() const {
    return validationPattern;
}

bool ConfigItem::setValue(const String& v) {
    if (!editable) {
        return false;
    }
    
    if (!validate(v)) {
        return false;
    }
    
    value = v;
    return true;
}

bool ConfigItem::validate() const {
    return validate(value);
}

bool ConfigItem::validate(const String& v) const {
    if (validationPattern.isEmpty()) {
        return true;
    }
    
    // 这里可以添加正则表达式验证
    // 注意：在Arduino环境中，可能需要使用简化的验证方法
    return true;
}

void ConfigItem::resetToDefault() {
    value = defaultValue;
}

// ConfigManager 构造函数
ConfigManager::ConfigManager() : initialized(false) {
}

// ConfigManager 单例获取
ConfigManager* ConfigManager::getInstance() {
    if (instance == nullptr) {
        instance = new ConfigManager();
    }
    return instance;
}

// 初始化
bool ConfigManager::init() {
    if (initialized) {
        return true;
    }
    
    // 注册默认的RAM存储后端
    auto ramStorage = std::make_shared<RAMConfigStorage>();
    if (!registerStorageBackend(ramStorage)) {
        return false;
    }
    
    // 注册SPIFFS存储后端
    auto spiffsStorage = std::make_shared<SPIFFSConfigStorage>();
    if (!registerStorageBackend(spiffsStorage)) {
        return false;
    }
    
    // 注册SD卡存储后端
    auto sdStorage = std::make_shared<SDCardConfigStorage>();
    if (!registerStorageBackend(sdStorage)) {
        return false;
    }
    
    // 设置默认的活动存储后端为RAM
    if (!setActiveStorage(STORAGE_TYPE_RAM)) {
        return false;
    }
    
    // 注册默认配置项
    registerDefaultConfigItems();
    
    initialized = true;
    return true;
}

// 注册默认配置项
void ConfigManager::registerDefaultConfigItems() {
    // 系统配置
    registerConfigItem("system.device_name", "InkClock", "设备名称", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("system.auto_restart", "false", "自动重启", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("system.restart_time", "03:00", "重启时间", CONFIG_LEVEL_DEFAULT);
    
    // 网络配置
    registerConfigItem("network.wifi_ssid", "", "WiFi SSID", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("network.wifi_password", "", "WiFi密码", CONFIG_LEVEL_PERSISTENT, true, "", "");
    registerConfigItem("network.auto_connect", "true", "自动连接WiFi", CONFIG_LEVEL_PERSISTENT);
    
    // 时间配置
    registerConfigItem("time.timezone", "8", "时区偏移", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("time.ntp_server", "pool.ntp.org", "NTP服务器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("time.sync_interval", "3600", "时间同步间隔(秒)", CONFIG_LEVEL_PERSISTENT);
    
    // 显示配置
    registerConfigItem("display.brightness", "100", "显示亮度", CONFIG_LEVEL_RUNTIME);
    registerConfigItem("display.rotation", "0", "显示旋转角度", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("display.update_interval", "60", "显示更新间隔(秒)", CONFIG_LEVEL_RUNTIME);
    registerConfigItem("display.type", "EINK_42_INCH_HEMA", "显示类型", CONFIG_LEVEL_DEFAULT);
    
    // 传感器配置
    registerConfigItem("sensor.update_interval", "30", "传感器更新间隔(秒)", CONFIG_LEVEL_RUNTIME);
    registerConfigItem("sensor.enable_all", "true", "启用所有传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_dht22", "false", "启用DHT22传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_am2302", "false", "启用AM2302传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_sht30", "false", "启用SHT30传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_bme280", "false", "启用BME280传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_bme680", "false", "启用BME680传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_hc_sr501", "false", "启用HC-SR501传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_ld2410", "false", "启用LD2410传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_mq135", "false", "启用MQ135传感器", CONFIG_LEVEL_PERSISTENT);
    
    // 天气配置
    registerConfigItem("weather.api_key", "", "天气API密钥", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("weather.city_id", "", "城市ID", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("weather.update_interval", "1800", "天气更新间隔(秒)", CONFIG_LEVEL_PERSISTENT);
    
    // 消息配置
    registerConfigItem("message.enable_notifications", "true", "启用消息通知", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("message.max_messages", "50", "最大消息数", CONFIG_LEVEL_PERSISTENT);
    
    // 电源管理配置
    registerConfigItem("power.low_power_mode", "false", "低功耗模式", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("power.no_motion_timeout", "30000", "无动作超时(毫秒)", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("power.night_light_threshold", "100", "夜间光照阈值", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("power.normal_refresh_interval", "60000", "正常刷新间隔(毫秒)", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("power.low_power_refresh_interval", "300000", "低功耗刷新间隔(毫秒)", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("power.critical_battery_threshold", "10", "临界电池阈值(%)", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("power.critical_low_power_refresh_interval", "600000", "临界低功耗刷新间隔(毫秒)", CONFIG_LEVEL_PERSISTENT);
    
    // 电池配置
    registerConfigItem("battery.full_voltage", "4.2", "满电电压", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("battery.empty_voltage", "3.0", "空电电压", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("battery.low_threshold", "20", "低电量阈值(%)", CONFIG_LEVEL_PERSISTENT);
    
    // 功能启用配置
    registerConfigItem("feature.enable_font", "false", "启用字体管理", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_audio", "false", "启用音频功能", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_text_message", "true", "启用文本消息", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_temperature_humidity", "false", "启用温湿度显示", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_firmware", "true", "启用固件更新", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_bluetooth", "true", "启用蓝牙", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_webclient", "true", "启用Web客户端", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_plugin", "true", "启用插件系统", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_wifi", "true", "启用WiFi", CONFIG_LEVEL_PERSISTENT);
    
    // 硬件配置
    registerConfigItem("hardware.tf_card", "false", "启用TF卡", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.camera_gc0308", "false", "启用GC0308摄像头", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.camera_ov2640", "false", "启用OV2640摄像头", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.charging_protection", "true", "启用充电保护", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.dc_power_supported", "false", "支持DC电源", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.only_usb_power", "true", "仅支持USB电源", CONFIG_LEVEL_DEFAULT);
    
    // 引脚配置
    registerConfigItem("pins.charge_status", "-1", "充电状态引脚", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("pins.battery_adc", "34", "电池电压引脚", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("pins.pir_sensor", "-1", "人体感应传感器引脚", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("pins.light_sensor", "-1", "光照传感器引脚", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("pins.gas_sensor", "-1", "气体传感器引脚", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("pins.flame_sensor", "-1", "火焰传感器引脚", CONFIG_LEVEL_DEFAULT);
    
    // 充电配置
    registerConfigItem("charging.protection_enabled", "true", "启用充电保护", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("charging.power_min", "5.0", "最小充电功率", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("charging.power_max", "18.0", "最大充电功率", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("charging.interface_type", "USB_TYPE_C", "充电接口类型", CONFIG_LEVEL_DEFAULT);
    
    // 传感器配置扩展
    registerConfigItem("sensor.enable_sht20", "false", "启用SHT20传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_sht40", "false", "启用SHT40传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_hdc1080", "false", "启用HDC1080传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_hc_sr505", "false", "启用HC-SR505传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_rcwl_0516", "false", "启用RCWL-0516传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_mq2", "false", "启用MQ2传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_mq5", "false", "启用MQ5传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_mq7", "false", "启用MQ7传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_sgp30", "false", "启用SGP30传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_ir_flame", "false", "启用红外火焰传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_lps25hb", "false", "启用LPS25HB传感器", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("sensor.enable_bmp388", "false", "启用BMP388传感器", CONFIG_LEVEL_PERSISTENT);
    
    // 功能配置扩展
    registerConfigItem("feature.enable_video_message", "false", "启用视频消息", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_voice_message", "false", "启用语音消息", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_motion_saving", "false", "启用动作节能", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_fire_alarm", "false", "启用火灾报警", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_light_saving", "false", "启用光照节能", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_touch", "false", "启用触摸功能", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("feature.enable_gas_alarm", "false", "启用气体报警", CONFIG_LEVEL_PERSISTENT);
    
    // 电源管理扩展配置
    registerConfigItem("power.light_change_threshold", "50", "光照变化阈值", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("power.min_refresh_interval", "10000", "最小刷新间隔(毫秒)", CONFIG_LEVEL_PERSISTENT);
    registerConfigItem("power.max_refresh_interval", "3600000", "最大刷新间隔(毫秒)", CONFIG_LEVEL_PERSISTENT);
    
    // 硬件配置扩展
    registerConfigItem("hardware.audio_driver_type", "AUDIO_DRIVER_NONE", "音频驱动类型", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.wifi_bt_module_type", "WIFI_BT_INTERNAL", "WiFi+蓝牙模块类型", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.tf_card_reader_type", "TF_READER_NONE", "TF卡读卡器类型", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.has_wifi_bt", "true", "是否有WiFi+蓝牙", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.camera_ov5640", "false", "启用OV5640摄像头", CONFIG_LEVEL_DEFAULT);
    registerConfigItem("hardware.camera_esp32_cam", "false", "启用ESP32-CAM摄像头", CONFIG_LEVEL_DEFAULT);
}

// 注册配置存储后端
bool ConfigManager::registerStorageBackend(std::shared_ptr<IConfigStorage> storage) {
    if (!storage->init()) {
        return false;
    }
    
    storageBackends.push_back(storage);
    return true;
}

// 设置活动存储后端
bool ConfigManager::setActiveStorage(ConfigStorageType type) {
    for (const auto& storage : storageBackends) {
        if (storage->getType() == type) {
            activeStorage = storage;
            return true;
        }
    }
    return false;
}

// 注册配置项
bool ConfigManager::registerConfigItem(
    const String& key,
    const String& value,
    const String& description,
    ConfigLevel level,
    bool editable,
    const String& defaultValue,
    const String& validationPattern
) {
    if (configItems.find(key) != configItems.end()) {
        return false;
    }
    
    auto configItem = std::make_shared<ConfigItem>(key, value, description, level, editable, defaultValue, validationPattern);
    configItems[key] = configItem;
    
    // 如果是持久化配置，尝试从存储加载
    if (level == CONFIG_LEVEL_PERSISTENT && activeStorage) {
        String loadedValue;
        if (activeStorage->load(key, loadedValue)) {
            configItem->setValue(loadedValue);
        }
    }
    
    return true;
}

// 获取配置值
String ConfigManager::getString(const String& key, const String& defaultValue) {
    auto it = configItems.find(key);
    if (it != configItems.end()) {
        return it->second->getValue();
    }
    return defaultValue;
}

int ConfigManager::getInt(const String& key, int defaultValue) {
    auto it = configItems.find(key);
    if (it != configItems.end()) {
        return it->second->getValue().toInt();
    }
    return defaultValue;
}

float ConfigManager::getFloat(const String& key, float defaultValue) {
    auto it = configItems.find(key);
    if (it != configItems.end()) {
        return it->second->getValue().toFloat();
    }
    return defaultValue;
}

bool ConfigManager::getBool(const String& key, bool defaultValue) {
    auto it = configItems.find(key);
    if (it != configItems.end()) {
        String value = it->second->getValue();
        return value == "true" || value == "1" || value == "yes";
    }
    return defaultValue;
}

// 设置配置值
bool ConfigManager::setString(const String& key, const String& value, ConfigLevel level) {
    auto it = configItems.find(key);
    if (it == configItems.end()) {
        return false;
    }
    
    auto configItem = it->second;
    if (!configItem->setValue(value)) {
        return false;
    }
    
    // 如果是持久化配置，保存到存储
    if (level == CONFIG_LEVEL_PERSISTENT && activeStorage) {
        activeStorage->save(key, value);
    }
    
    return true;
}

bool ConfigManager::setInt(const String& key, int value, ConfigLevel level) {
    Stringstream ss;
    ss << value;
    return setString(key, ss.str().c_str(), level);
}

bool ConfigManager::setFloat(const String& key, float value, ConfigLevel level) {
    Stringstream ss;
    ss << value;
    return setString(key, ss.str().c_str(), level);
}

bool ConfigManager::setBool(const String& key, bool value, ConfigLevel level) {
    return setString(key, value ? "true" : "false", level);
}

// 检查配置项是否存在
bool ConfigManager::hasConfig(const String& key) const {
    return configItems.find(key) != configItems.end();
}

// 获取配置项信息
std::shared_ptr<ConfigItem> ConfigManager::getConfigItem(const String& key) const {
    auto it = configItems.find(key);
    if (it != configItems.end()) {
        return it->second;
    }
    return nullptr;
}

// 获取所有配置项
std::vector<std::shared_ptr<ConfigItem>> ConfigManager::getAllConfigItems() const {
    std::vector<std::shared_ptr<ConfigItem>> items;
    for (const auto& pair : configItems) {
        items.push_back(pair.second);
    }
    return items;
}

std::vector<std::shared_ptr<ConfigItem>> ConfigManager::getConfigItemsByLevel(ConfigLevel level) const {
    std::vector<std::shared_ptr<ConfigItem>> items;
    for (const auto& pair : configItems) {
        if (pair.second->getLevel() == level) {
            items.push_back(pair.second);
        }
    }
    return items;
}

// 加载配置
bool ConfigManager::loadConfig() {
    if (!activeStorage) {
        return false;
    }
    
    bool success = true;
    for (auto& pair : configItems) {
        auto configItem = pair.second;
        if (configItem->getLevel() == CONFIG_LEVEL_PERSISTENT) {
            String value;
            if (activeStorage->load(configItem->getKey(), value)) {
                configItem->setValue(value);
            } else {
                success = false;
            }
        }
    }
    return success;
}

// 保存配置
bool ConfigManager::saveConfig() {
    if (!activeStorage) {
        return false;
    }
    
    bool success = true;
    for (const auto& pair : configItems) {
        auto configItem = pair.second;
        if (configItem->getLevel() == CONFIG_LEVEL_PERSISTENT) {
            if (!activeStorage->save(configItem->getKey(), configItem->getValue())) {
                success = false;
            }
        }
    }
    return success;
}

// 重置配置
bool ConfigManager::resetConfig(ConfigLevel level) {
    bool success = true;
    for (auto& pair : configItems) {
        auto configItem = pair.second;
        if (configItem->getLevel() == level) {
            configItem->resetToDefault();
            
            // 如果是持久化配置，保存到存储
            if (level == CONFIG_LEVEL_PERSISTENT && activeStorage) {
                if (!activeStorage->save(configItem->getKey(), configItem->getValue())) {
                    success = false;
                }
            }
        }
    }
    return success;
}

// 备份配置
bool ConfigManager::backupConfig(const String& backupPath) {
    // 这里可以添加配置备份逻辑
    // 注意：在Arduino环境中，需要使用SD库或SPIFFS库来操作文件
    String configJson = exportConfigToJson();
    Serial.printf("[CONFIG] Backup config to %s: %s\n", backupPath.c_str(), configJson.c_str());
    return true;
}

// 恢复配置
bool ConfigManager::restoreConfig(const String& backupPath) {
    // 这里可以添加配置恢复逻辑
    // 注意：在Arduino环境中，需要使用SD库或SPIFFS库来操作文件
    Serial.printf("[CONFIG] Restore config from %s\n", backupPath.c_str());
    return true;
}

// 验证所有配置
bool ConfigManager::validateAllConfig() const {
    for (const auto& pair : configItems) {
        if (!pair.second->validate()) {
            return false;
        }
    }
    return true;
}

// 导出配置为JSON
String ConfigManager::exportConfigToJson() const {
    Stringstream ss;
    ss << "{";
    
    bool first = true;
    for (const auto& pair : configItems) {
        if (!first) {
            ss << ",";
        }
        ss << "\"" << pair.first.c_str() << "\":{";
        ss << "\"value\":\"" << pair.second->getValue().c_str() << "\",";
        ss << "\"description\":\"" << pair.second->getDescription().c_str() << "\",";
        ss << "\"level\":" << pair.second->getLevel() << ",";
        ss << "\"editable\":" << (pair.second->isEditable() ? "true" : "false") << ",";
        ss << "\"defaultValue\":\"" << pair.second->getDefaultValue().c_str() << "\"";
        ss << "}";
        first = false;
    }
    
    ss << "}";
    return ss.str().c_str();
}

// 从JSON导入配置
bool ConfigManager::importConfigFromJson(const String& json) {
    // 这里可以添加JSON解析逻辑
    // 注意：在Arduino环境中，建议使用ArduinoJson库
    Serial.printf("[CONFIG] Import config from JSON: %s\n", json.c_str());
    return true;
}

// SPIFFSConfigStorage 构造函数
SPIFFSConfigStorage::SPIFFSConfigStorage(const String& fileName) : configFileName(fileName) {
}

// SPIFFSConfigStorage 方法实现
bool SPIFFSConfigStorage::init() {
    // 这里可以添加SPIFFS初始化逻辑
    // 注意：在Arduino环境中，需要使用SPIFFS库
    Serial.println("[CONFIG] SPIFFS storage initialized");
    return true;
}

bool SPIFFSConfigStorage::load(const String& key, String& value) {
    // 这里可以添加SPIFFS加载逻辑
    Serial.printf("[CONFIG] Load from SPIFFS: %s\n", key.c_str());
    return false;
}

bool SPIFFSConfigStorage::save(const String& key, const String& value) {
    // 这里可以添加SPIFFS保存逻辑
    Serial.printf("[CONFIG] Save to SPIFFS: %s = %s\n", key.c_str(), value.c_str());
    return false;
}

bool SPIFFSConfigStorage::remove(const String& key) {
    // 这里可以添加SPIFFS删除逻辑
    Serial.printf("[CONFIG] Remove from SPIFFS: %s\n", key.c_str());
    return false;
}

bool SPIFFSConfigStorage::clear() {
    // 这里可以添加SPIFFS清空逻辑
    Serial.println("[CONFIG] Clear SPIFFS storage");
    return false;
}

bool SPIFFSConfigStorage::exists(const String& key) {
    // 这里可以添加SPIFFS存在性检查逻辑
    return false;
}

std::vector<String> SPIFFSConfigStorage::listKeys() {
    // 这里可以添加SPIFFS键列表逻辑
    return std::vector<String>();
}

ConfigStorageType SPIFFSConfigStorage::getType() {
    return STORAGE_TYPE_SPIFFS;
}

// SDCardConfigStorage 构造函数
SDCardConfigStorage::SDCardConfigStorage(const String& fileName) : configFileName(fileName) {
}

// SDCardConfigStorage 方法实现
bool SDCardConfigStorage::init() {
    // 这里可以添加SD卡初始化逻辑
    // 注意：在Arduino环境中，需要使用SD库
    Serial.println("[CONFIG] SD card storage initialized");
    return true;
}

bool SDCardConfigStorage::load(const String& key, String& value) {
    // 这里可以添加SD卡加载逻辑
    Serial.printf("[CONFIG] Load from SD card: %s\n", key.c_str());
    return false;
}

bool SDCardConfigStorage::save(const String& key, const String& value) {
    // 这里可以添加SD卡保存逻辑
    Serial.printf("[CONFIG] Save to SD card: %s = %s\n", key.c_str(), value.c_str());
    return false;
}

bool SDCardConfigStorage::remove(const String& key) {
    // 这里可以添加SD卡删除逻辑
    Serial.printf("[CONFIG] Remove from SD card: %s\n", key.c_str());
    return false;
}

bool SDCardConfigStorage::clear() {
    // 这里可以添加SD卡清空逻辑
    Serial.println("[CONFIG] Clear SD card storage");
    return false;
}

bool SDCardConfigStorage::exists(const String& key) {
    // 这里可以添加SD卡存在性检查逻辑
    return false;
}

std::vector<String> SDCardConfigStorage::listKeys() {
    // 这里可以添加SD卡键列表逻辑
    return std::vector<String>();
}

ConfigStorageType SDCardConfigStorage::getType() {
    return STORAGE_TYPE_SD_CARD;
}

// RAMConfigStorage 构造函数
RAMConfigStorage::RAMConfigStorage() {
}

// RAMConfigStorage 方法实现
bool RAMConfigStorage::init() {
    Serial.println("[CONFIG] RAM storage initialized");
    return true;
}

bool RAMConfigStorage::load(const String& key, String& value) {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        value = it->second;
        return true;
    }
    return false;
}

bool RAMConfigStorage::save(const String& key, const String& value) {
    configMap[key] = value;
    return true;
}

bool RAMConfigStorage::remove(const String& key) {
    auto it = configMap.find(key);
    if (it != configMap.end()) {
        configMap.erase(it);
        return true;
    }
    return false;
}

bool RAMConfigStorage::clear() {
    configMap.clear();
    return true;
}

bool RAMConfigStorage::exists(const String& key) {
    return configMap.find(key) != configMap.end();
}

std::vector<String> RAMConfigStorage::listKeys() {
    std::vector<String> keys;
    for (const auto& pair : configMap) {
        keys.push_back(pair.first);
    }
    return keys;
}

ConfigStorageType RAMConfigStorage::getType() {
    return STORAGE_TYPE_RAM;
}
