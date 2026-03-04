#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <map>
#include <set>
#include <Arduino.h>

// FreeRTOS 条件编译
#if defined(ESP32)
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#elif defined(ESP8266)
// ESP8266 没有完整的 FreeRTOS 支持，定义简化的互斥锁
typedef void* SemaphoreHandle_t;
#define xSemaphoreCreateMutex() (SemaphoreHandle_t)1
#define xSemaphoreTake(mutex, delay) (void)mutex
#define xSemaphoreGive(mutex) (void)mutex
#define vSemaphoreDelete(mutex) (void)mutex
#define portMAX_DELAY 0
#endif

class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    
    virtual bool initialize() = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;
    
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    
    // 插件优先级，数值越大优先级越高
    virtual int getPriority() const {
        return 5; // 默认中等优先级
    }
    
    // 获取插件依赖
    virtual std::vector<std::string> getDependencies() const {
        return {}; // 默认无依赖
    }
    
    // 检查插件是否就绪
    virtual bool isReady() const {
        return isEnabled();
    }
};

// 插件信息结构
struct PluginInfo {
    std::string name;
    std::string version;
    std::string description;
    bool enabled;
    bool initialized;
    int priority;
    std::vector<std::string> dependencies;
    std::vector<std::string> dependents;
};

class PluginManager {
public:
    static PluginManager& getInstance();
    
    bool registerPlugin(std::unique_ptr<IPlugin> plugin);
    bool unregisterPlugin(const std::string& name);
    
    void initializeAll();
    void updateAll();
    void shutdownAll();
    
    IPlugin* getPlugin(const std::string& name);
    std::vector<IPlugin*> getAllPlugins();
    std::vector<IPlugin*> getEnabledPlugins();
    std::vector<PluginInfo> getPluginInfos();
    
    bool enablePlugin(const std::string& name);
    bool disablePlugin(const std::string& name);
    
    // 检查插件依赖
    bool checkDependencies(const std::string& pluginName);
    
    // 按优先级排序插件
    std::vector<IPlugin*> getPluginsByPriority();
    
private:
    PluginManager();
    ~PluginManager();
    
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    std::map<std::string, std::unique_ptr<IPlugin>> plugins;
    std::map<std::string, PluginInfo> pluginInfos;
    SemaphoreHandle_t pluginMutex;
    
    // 更新插件信息
    void updatePluginInfo(const std::string& name, IPlugin* plugin);
    
    // 解析插件依赖
    void resolveDependencies();
    
    // 检查插件是否可以启用
    bool canEnablePlugin(const std::string& pluginName);
    
    // 启用插件及其依赖
    bool enablePluginWithDependencies(const std::string& pluginName);
    
    // 禁用插件及其依赖
    bool disablePluginWithDependents(const std::string& pluginName);
};

#define REGISTER_PLUGIN(PluginClass) \
    class PluginClass##Registrar { \
    public: \
        PluginClass##Registrar() { \
            PluginManager::getInstance().registerPlugin(std::make_unique<PluginClass>()); \
        } \
    }; \
    static PluginClass##Registrar pluginRegistrar_##PluginClass;

#endif // PLUGIN_MANAGER_H
