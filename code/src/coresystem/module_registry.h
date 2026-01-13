#ifndef MODULE_REGISTRY_H
#define MODULE_REGISTRY_H

#include <Arduino.h>
#include <vector>
#include <map>
#include <memory>
#include <functional>
#include "event_bus.h"

// 模块类型枚举
enum ModuleType {
  MODULE_TYPE_DISPLAY,
  MODULE_TYPE_WIFI,
  MODULE_TYPE_TIME,
  MODULE_TYPE_LUNAR,
  MODULE_TYPE_WEATHER,
  MODULE_TYPE_SENSOR,
  MODULE_TYPE_BUTTON,
  MODULE_TYPE_FEEDBACK,
  MODULE_TYPE_POWER,
  MODULE_TYPE_WEB_SERVER,
  MODULE_TYPE_API,
  MODULE_TYPE_GEO,
  MODULE_TYPE_AUDIO,
  MODULE_TYPE_BLUETOOTH,
  MODULE_TYPE_CAMERA,
  MODULE_TYPE_STOCK,
  MODULE_TYPE_MESSAGE,
  MODULE_TYPE_PLUGIN,
  MODULE_TYPE_WEBCLIENT,
  MODULE_TYPE_FONT,
  MODULE_TYPE_FIRMWARE,
  MODULE_TYPE_TOUCH
};

// 模块状态枚举
enum ModuleStatus {
  MODULE_STATUS_UNINITIALIZED,
  MODULE_STATUS_INITIALIZING,
  MODULE_STATUS_READY,
  MODULE_STATUS_RUNNING,
  MODULE_STATUS_ERROR,
  MODULE_STATUS_DISABLED,
  MODULE_STATUS_UNLOADED
};

// 模块信息结构
typedef struct {
  String name;
  ModuleType type;
  ModuleStatus status;
  bool enabled;
  bool loaded;
  unsigned long lastActiveTime;
  unsigned long startTime;
  int errorCount;
  void* modulePtr;
} ModuleInfo;

// 基础模块接口，所有模块都应继承自该接口
class IModule {
public:
  virtual ~IModule() {}
  
  // 初始化模块
  virtual void init() = 0;
  
  // 运行模块
  virtual void loop() = 0;
  
  // 获取模块名称
  virtual String getName() const = 0;
  
  // 获取模块类型
  virtual ModuleType getModuleType() const = 0;
  
  // 检查模块是否需要运行
  virtual bool shouldRun() const {
    return true;
  }
};

// 模块注册中心类
class ModuleRegistry {
private:
  static ModuleRegistry* instance;
  
  // 模块信息列表
  std::vector<ModuleInfo> modules;
  
  // 事件总线指针
  EventBus* eventBus;
  
  // 私有构造函数
  ModuleRegistry() {
    eventBus = EventBus::getInstance();
  }
  
  // 私有方法：更新模块状态
  void updateModuleStatus(const String& moduleName, ModuleStatus status);
  
public:
  // 获取单例实例
  static ModuleRegistry* getInstance() {
    if (instance == nullptr) {
      instance = new ModuleRegistry();
    }
    return instance;
  }
  
  // 注册模块
  template <typename T>
  bool registerModule() {
    T* module = new T();
    return registerModule(module);
  }
  
  // 注册模块（使用指针）
  bool registerModule(IModule* module);
  
  // 卸载模块
  bool unregisterModule(const String& moduleName);
  
  // 加载模块
  bool loadModule(const String& moduleName);
  
  // 卸载模块
  bool unloadModule(const String& moduleName);
  
  // 启用模块
  bool enableModule(const String& moduleName);
  
  // 禁用模块
  bool disableModule(const String& moduleName);
  
  // 获取模块
  template <typename T>
  T* getModule() {
    for (auto& module : modules) {
      if (module.loaded && module.enabled) {
        T* castedModule = dynamic_cast<T*>(module.modulePtr);
        if (castedModule) {
          return castedModule;
        }
      }
    }
    return nullptr;
  }
  
  // 根据名称获取模块
  void* getModuleByName(const String& moduleName);
  
  // 根据类型获取模块
  void* getModuleByType(ModuleType type);
  
  // 获取所有模块信息
  std::vector<ModuleInfo> getModulesInfo();
  
  // 运行所有启用的模块
  void runModules();
  
  // 清理未使用的模块
  void cleanupUnusedModules();
  
  // 初始化所有模块
  void initAllModules();
  
  // 关闭所有模块
  void shutdownAllModules();
};

// 模块注册宏
template <typename T>
void registerModule() {
  ModuleRegistry::getInstance()->registerModule<T>();
}

#endif // MODULE_REGISTRY_H
