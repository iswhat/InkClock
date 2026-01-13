#ifndef DEPENDENCY_INJECTION_H
#define DEPENDENCY_INJECTION_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "arduino_compat.h"
#endif

#include <map>
#include <string>
#include <functional>

// 前向声明所有管理器类
class WiFiManager;
class TimeManager;
class LunarManager;
class WeatherManager;
class SensorManager;
class StockManager;
class MessageManager;
class PowerManager;
class DisplayManager;
class APIManager;
class GeoManager;
class PluginManager;
class BluetoothManager;
class ErrorHandlingManager;
class ConfigManager;
class NetworkManager;

// 依赖注入容器类
class DependencyInjectionContainer {
public:
  static DependencyInjectionContainer* getInstance();
  
  // 注册实例
  template<typename T>
  void registerInstance(T* instance, const String& key) {
    instances[key] = reinterpret_cast<void*>(instance);
  }
  
  // 注册工厂函数
  template<typename T>
  void registerFactory(std::function<T*()> factory, const String& key) {
    factories[key] = [factory]() -> void* {
      return factory();
    };
  }
  
  // 获取实例
  template<typename T>
  T* getInstance(const String& key) {
    // 首先检查是否已有实例
    if (instances.find(key) != instances.end()) {
      return reinterpret_cast<T*>(instances[key]);
    }
    
    // 如果没有实例，尝试使用工厂函数创建
    if (factories.find(key) != factories.end()) {
      void* instance = factories[key]();
      instances[key] = instance;
      return reinterpret_cast<T*>(instance);
    }
    
    return nullptr;
  }
  
  // 便捷方法：获取常用管理器实例
  WiFiManager* getWiFiManager();
  TimeManager* getTimeManager();
  LunarManager* getLunarManager();
  WeatherManager* getWeatherManager();
  SensorManager* getSensorManager();
  StockManager* getStockManager();
  MessageManager* getMessageManager();
  PowerManager* getPowerManager();
  DisplayManager* getDisplayManager();
  APIManager* getAPIManager();
  GeoManager* getGeoManager();
  PluginManager* getPluginManager();
  ErrorHandlingManager* getErrorHandlingManager();
  ConfigManager* getConfigManager();
  NetworkManager* getNetworkManager();
  
  // 初始化所有管理器
  void initializeAll();
  
  // 清理资源
  void cleanup();
  
private:
  DependencyInjectionContainer();
  ~DependencyInjectionContainer();
  
  static DependencyInjectionContainer* instance;
  
  std::map<String, void*> instances;
  std::map<String, std::function<void*()>> factories;
  
  bool initialized;
};

// 全局依赖注入容器实例
extern DependencyInjectionContainer* diContainer;

#endif // DEPENDENCY_INJECTION_H
