#include "dependency_injection.h"

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

// 静态实例初始化
DependencyInjectionContainer* DependencyInjectionContainer::instance = nullptr;

// 全局依赖注入容器实例
DependencyInjectionContainer* diContainer = nullptr;

// 构造函数
DependencyInjectionContainer::DependencyInjectionContainer() {
  initialized = false;
}

// 析构函数
DependencyInjectionContainer::~DependencyInjectionContainer() {
  cleanup();
}

// 获取单例实例
DependencyInjectionContainer* DependencyInjectionContainer::getInstance() {
  if (instance == nullptr) {
    instance = new DependencyInjectionContainer();
  }
  return instance;
}

// 便捷方法：获取WiFi管理器
WiFiManager* DependencyInjectionContainer::getWiFiManager() {
  return getInstance<WiFiManager>("WiFiManager");
}

// 便捷方法：获取时间管理器
TimeManager* DependencyInjectionContainer::getTimeManager() {
  return getInstance<TimeManager>("TimeManager");
}

// 便捷方法：获取农历管理器
LunarManager* DependencyInjectionContainer::getLunarManager() {
  return getInstance<LunarManager>("LunarManager");
}

// 便捷方法：获取天气管理器
WeatherManager* DependencyInjectionContainer::getWeatherManager() {
  return getInstance<WeatherManager>("WeatherManager");
}

// 便捷方法：获取传感器管理器
SensorManager* DependencyInjectionContainer::getSensorManager() {
  return getInstance<SensorManager>("SensorManager");
}

// 便捷方法：获取股票管理器
StockManager* DependencyInjectionContainer::getStockManager() {
  return getInstance<StockManager>("StockManager");
}

// 便捷方法：获取消息管理器
MessageManager* DependencyInjectionContainer::getMessageManager() {
  return getInstance<MessageManager>("MessageManager");
}

// 便捷方法：获取电源管理器
PowerManager* DependencyInjectionContainer::getPowerManager() {
  return getInstance<PowerManager>("PowerManager");
}

// 便捷方法：获取显示管理器
DisplayManager* DependencyInjectionContainer::getDisplayManager() {
  return getInstance<DisplayManager>("DisplayManager");
}

// 便捷方法：获取API管理器
APIManager* DependencyInjectionContainer::getAPIManager() {
  return getInstance<APIManager>("APIManager");
}

// 便捷方法：获取地理位置管理器
GeoManager* DependencyInjectionContainer::getGeoManager() {
  return getInstance<GeoManager>("GeoManager");
}

// 便捷方法：获取插件管理器
PluginManager* DependencyInjectionContainer::getPluginManager() {
  return getInstance<PluginManager>("PluginManager");
}

// 便捷方法：获取错误处理管理器
ErrorHandlingManager* DependencyInjectionContainer::getErrorHandlingManager() {
  return getInstance<ErrorHandlingManager>("ErrorHandlingManager");
}

// 便捷方法：获取配置管理器
ConfigManager* DependencyInjectionContainer::getConfigManager() {
  return getInstance<ConfigManager>("ConfigManager");
}

// 便捷方法：获取网络管理器
NetworkManager* DependencyInjectionContainer::getNetworkManager() {
  return getInstance<NetworkManager>("NetworkManager");
}

// 初始化所有管理器
void DependencyInjectionContainer::initializeAll() {
  if (initialized) {
    return;
  }
  
  // 按照依赖顺序初始化管理器
  // 1. 基础服务
  ErrorHandlingManager* errorManager = getErrorHandlingManager();
  if (errorManager) {
    errorManager->init();
  }
  
  ConfigManager* configManager = getConfigManager();
  if (configManager) {
    configManager->init();
  }
  
  WiFiManager* wifiManager = getWiFiManager();
  if (wifiManager) {
    wifiManager->init();
  }
  
  NetworkManager* networkManager = getNetworkManager();
  if (networkManager) {
    networkManager->initialize();
  }
  
  APIManager* apiManager = getAPIManager();
  if (apiManager) {
    apiManager->init();
  }
  
  // 2. 核心管理器
  TimeManager* timeManager = getTimeManager();
  if (timeManager) {
    timeManager->init();
  }
  
  GeoManager* geoManager = getGeoManager();
  if (geoManager) {
    geoManager->init();
  }
  
  // 3. 功能管理器
  LunarManager* lunarManager = getLunarManager();
  if (lunarManager) {
    lunarManager->init();
  }
  
  WeatherManager* weatherManager = getWeatherManager();
  if (weatherManager) {
    weatherManager->init();
  }
  
  SensorManager* sensorManager = getSensorManager();
  if (sensorManager) {
    sensorManager->init();
  }
  
  StockManager* stockManager = getStockManager();
  if (stockManager) {
    stockManager->init();
  }
  
  MessageManager* messageManager = getMessageManager();
  if (messageManager) {
    messageManager->init();
  }
  
  PowerManager* powerManager = getPowerManager();
  if (powerManager) {
    powerManager->init();
  }
  
  PluginManager* pluginManager = getPluginManager();
  if (pluginManager) {
    pluginManager->initializeAll();
  }
  
  // 4. 显示管理器（最后初始化，因为它依赖其他所有管理器）
  DisplayManager* displayManager = getDisplayManager();
  if (displayManager) {
    displayManager->init();
  }
  
  initialized = true;
}

// 清理资源
void DependencyInjectionContainer::cleanup() {
  // 清理所有实例
  for (auto& pair : instances) {
    void* instance = pair.second;
    if (instance) {
      delete instance;
    }
  }
  instances.clear();
  factories.clear();
  
  initialized = false;
}
