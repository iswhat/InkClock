#ifndef DRIVER_REGISTRY_H
#define DRIVER_REGISTRY_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include <memory>
#include "event_bus.h"
#include "../drivers/sensors/sensor_driver.h"
#include "../drivers/displays/display_driver.h"

// 驱动类型枚举
enum DriverType {
  DRIVER_TYPE_SENSOR,
  DRIVER_TYPE_DISPLAY,
  DRIVER_TYPE_AUDIO,
  DRIVER_TYPE_INPUT,
  DRIVER_TYPE_NETWORK,
  DRIVER_TYPE_STORAGE,
  DRIVER_TYPE_OTHER
};

// 驱动状态枚举
enum DriverStatus {
  DRIVER_STATUS_UNINITIALIZED,
  DRIVER_STATUS_INITIALIZING,
  DRIVER_STATUS_READY,
  DRIVER_STATUS_RUNNING,
  DRIVER_STATUS_ERROR,
  DRIVER_STATUS_DISABLED,
  DRIVER_STATUS_UNREGISTERED
};

// 设备状态枚举
enum DeviceStatus {
  DEVICE_STATUS_DISCONNECTED,
  DEVICE_STATUS_CONNECTING,
  DEVICE_STATUS_CONNECTED,
  DEVICE_STATUS_DISCOVERED,
  DEVICE_STATUS_ERROR
};

// 驱动信息结构
typedef struct {
  String name;
  String type;
  String version;
  String vendor;
  DriverType driverType;
  DriverStatus status;
  bool enabled;
  String deviceId;
  String deviceName;
  String deviceType;
  String firmwareVersion;
  unsigned long lastActiveTime;
  unsigned long startTime;
  int errorCount;
} DriverInfo;

// 设备信息结构
typedef struct {
  String deviceId;
  String deviceName;
  String deviceType;
  String driverName;
  DeviceStatus status;
  String connectionInfo;
  unsigned long discoveredTime;
  unsigned long lastUpdateTime;
  std::map<String, String> properties;
} DeviceInfo;

// 驱动注册中心类
class DriverRegistry {
private:
  static DriverRegistry* instance;
  
  // 私有构造函数
  DriverRegistry() {
    scanningEnabled = true;
    scanInterval = 30000; // 默认30秒扫描一次
    lastScanTime = 0;
  }
  
  // 驱动列表
  std::vector<ISensorDriver*> sensorDrivers;
  std::vector<IDisplayDriver*> displayDrivers;
  
  // 设备信息列表
  std::vector<DeviceInfo> deviceInfos;
  
  // 驱动信息列表
  std::vector<DriverInfo> driverInfos;
  
  // 事件总线指针
  EventBus* eventBus;
  
  // 扫描配置
  bool scanningEnabled;
  unsigned long scanInterval;
  unsigned long lastScanTime;
  
  // 私有方法：更新驱动状态
  void updateDriverStatus(const String& driverName, DriverStatus status) {
    for (auto& info : driverInfos) {
      if (info.name == driverName) {
        info.status = status;
        info.lastActiveTime = millis();
        
        // 发布驱动更新事件
        auto driverData = std::make_shared<DriverEventData>(info.name, info.type);
        eventBus->publish(EVENT_DRIVER_UPDATED, driverData);
        break;
      }
    }
  }
  
  // 私有方法：更新设备状态
  void updateDeviceStatus(const String& deviceId, DeviceStatus status) {
    for (auto& info : deviceInfos) {
      if (info.deviceId == deviceId) {
        info.status = status;
        info.lastUpdateTime = millis();
        
        // 发布设备状态变化事件
        auto deviceData = std::make_shared<DeviceEventData>(info.deviceName, info.deviceType, deviceId);
        eventBus->publish(EVENT_DEVICE_STATUS_CHANGED, deviceData);
        break;
      }
    }
  }
  
public:
  // 获取单例实例
  static DriverRegistry* getInstance() {
    if (instance == nullptr) {
      instance = new DriverRegistry();
      instance->eventBus = EventBus::getInstance();
    }
    return instance;
  }
  
  // 初始化驱动注册中心
  void init() {
    eventBus->publish(EVENT_SYSTEM_STARTUP, nullptr);
    Serial.println("DriverRegistry initialized");
    updateDriverStatus("DriverRegistry", DRIVER_STATUS_READY);
  }
  
  // 注册传感器驱动
  bool registerSensorDriver(ISensorDriver* driver) {
    if (driver == nullptr) {
      Serial.println("Error: Attempt to register null sensor driver");
      return false;
    }
    
    sensorDrivers.push_back(driver);
    
    // 创建驱动信息
    DriverInfo info;
    info.name = driver->getTypeName();
    info.type = "sensor";
    info.version = "1.0.0";
    info.vendor = "Unknown";
    info.driverType = DRIVER_TYPE_SENSOR;
    info.status = DRIVER_STATUS_UNINITIALIZED;
    info.enabled = false;
    info.deviceId = String(driver->getType());
    info.deviceName = driver->getTypeName();
    info.deviceType = "sensor";
    info.firmwareVersion = "1.0.0";
    info.lastActiveTime = millis();
    info.startTime = millis();
    info.errorCount = 0;
    
    driverInfos.push_back(info);
    
    // 发布驱动注册事件
    auto driverData = std::make_shared<DriverEventData>(info.name, info.type);
    eventBus->publish(EVENT_DRIVER_REGISTERED, driverData);
    
    Serial.printf("Sensor driver registered: %s\n", info.name.c_str());
    return true;
  }
  
  // 注册显示驱动
  bool registerDisplayDriver(IDisplayDriver* driver) {
    if (driver == nullptr) {
      Serial.println("Error: Attempt to register null display driver");
      return false;
    }
    
    displayDrivers.push_back(driver);
    
    // 创建驱动信息
    DriverInfo info;
    info.name = "EinkDriver";
    info.type = "display";
    info.version = "1.0.0";
    info.vendor = "Unknown";
    info.driverType = DRIVER_TYPE_DISPLAY;
    info.status = DRIVER_STATUS_UNINITIALIZED;
    info.enabled = false;
    info.deviceId = String(driver->getType());
    info.deviceName = "EinkDisplay";
    info.deviceType = "display";
    info.firmwareVersion = "1.0.0";
    info.lastActiveTime = millis();
    info.startTime = millis();
    info.errorCount = 0;
    
    driverInfos.push_back(info);
    
    // 发布驱动注册事件
    auto driverData = std::make_shared<DriverEventData>(info.name, info.type);
    eventBus->publish(EVENT_DRIVER_REGISTERED, driverData);
    
    Serial.printf("Display driver registered: %s\n", info.name.c_str());
    return true;
  }
  
  // 卸载驱动
  bool unregisterDriver(const String& driverName) {
    // 查找并移除传感器驱动
    for (auto it = sensorDrivers.begin(); it != sensorDrivers.end(); ++it) {
      if ((*it)->getTypeName() == driverName) {
        // 发布驱动卸载事件
        auto driverData = std::make_shared<DriverEventData>(driverName, "sensor");
        eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
        
        // 更新驱动状态
        updateDriverStatus(driverName, DRIVER_STATUS_UNREGISTERED);
        
        delete *it;
        sensorDrivers.erase(it);
        
        // 移除驱动信息
        for (auto infoIt = driverInfos.begin(); infoIt != driverInfos.end(); ++infoIt) {
          if (infoIt->name == driverName) {
            driverInfos.erase(infoIt);
            break;
          }
        }
        
        Serial.printf("Sensor driver unregistered: %s\n", driverName.c_str());
        return true;
      }
    }
    
    // 查找并移除显示驱动
    for (auto it = displayDrivers.begin(); it != displayDrivers.end(); ++it) {
      if (String("EinkDriver") == driverName) {
        // 发布驱动卸载事件
        auto driverData = std::make_shared<DriverEventData>(driverName, "display");
        eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
        
        // 更新驱动状态
        updateDriverStatus(driverName, DRIVER_STATUS_UNREGISTERED);
        
        delete *it;
        displayDrivers.erase(it);
        
        // 移除驱动信息
        for (auto infoIt = driverInfos.begin(); infoIt != driverInfos.end(); ++infoIt) {
          if (infoIt->name == driverName) {
            driverInfos.erase(infoIt);
            break;
          }
        }
        
        Serial.printf("Display driver unregistered: %s\n", driverName.c_str());
        return true;
      }
    }
    
    Serial.printf("Error: Driver not found for unregistration: %s\n", driverName.c_str());
    return false;
  }
  
  // 获取所有传感器驱动
  std::vector<ISensorDriver*> getSensorDrivers() {
    return sensorDrivers;
  }
  
  // 获取所有显示驱动
  std::vector<IDisplayDriver*> getDisplayDrivers() {
    return displayDrivers;
  }
  
  // 根据传感器类型获取驱动
  ISensorDriver* getSensorDriver(SensorType type) {
    for (auto driver : sensorDrivers) {
      if (driver->getType() == type) {
        return driver;
      }
    }
    return nullptr;
  }
  
  // 根据驱动名称获取传感器驱动
  ISensorDriver* getSensorDriverByName(const String& name) {
    for (auto driver : sensorDrivers) {
      if (driver->getTypeName() == name) {
        return driver;
      }
    }
    return nullptr;
  }
  
  // 根据显示类型获取驱动
  IDisplayDriver* getDisplayDriver(EinkDisplayType type) {
    for (auto driver : displayDrivers) {
      if (driver->getType() == type) {
        return driver;
      }
    }
    return nullptr;
  }
  
  // 自动检测传感器驱动
  ISensorDriver* autoDetectSensorDriver() {
    for (auto driver : sensorDrivers) {
      updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_INITIALIZING);
      
      SensorConfig config;
      config.type = driver->getType();
      config.pin = -1;
      config.address = 0;
      config.updateInterval = 60000;
      config.tempOffset = 0.0;
      config.humOffset = 0.0;
      
      if (driver->init(config)) {
        updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_READY);
        
        // 创建设备信息
        DeviceInfo deviceInfo;
        deviceInfo.deviceId = String(driver->getType());
        deviceInfo.deviceName = driver->getTypeName();
        deviceInfo.deviceType = "sensor";
        deviceInfo.driverName = driver->getTypeName();
        deviceInfo.status = DEVICE_STATUS_DISCOVERED;
        deviceInfo.connectionInfo = "Auto-detected";
        deviceInfo.discoveredTime = millis();
        deviceInfo.lastUpdateTime = millis();
        
        deviceInfos.push_back(deviceInfo);
        
        // 发布传感器发现事件
        auto deviceData = std::make_shared<DeviceEventData>(deviceInfo.deviceName, deviceInfo.deviceType, deviceInfo.deviceId);
        eventBus->publish(EVENT_DEVICE_DISCOVERED, deviceData);
        
        return driver;
      } else {
        updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_ERROR);
        
        // 发布驱动错误事件
        auto errorData = std::make_shared<SystemErrorEventData>("Driver initialization failed", 2001, driver->getTypeName());
        eventBus->publish(EVENT_DRIVER_ERROR, errorData);
      }
    }
    return nullptr;
  }
  
  // 自动检测显示驱动
  IDisplayDriver* autoDetectDisplayDriver() {
    for (auto driver : displayDrivers) {
      updateDriverStatus("EinkDriver", DRIVER_STATUS_INITIALIZING);
      
      if (driver->init()) {
        updateDriverStatus("EinkDriver", DRIVER_STATUS_READY);
        
        // 创建设备信息
        DeviceInfo deviceInfo;
        deviceInfo.deviceId = String(driver->getType());
        deviceInfo.deviceName = "EinkDisplay";
        deviceInfo.deviceType = "display";
        deviceInfo.driverName = "EinkDriver";
        deviceInfo.status = DEVICE_STATUS_DISCOVERED;
        deviceInfo.connectionInfo = "Auto-detected";
        deviceInfo.discoveredTime = millis();
        deviceInfo.lastUpdateTime = millis();
        
        deviceInfos.push_back(deviceInfo);
        
        // 发布设备发现事件
        auto deviceData = std::make_shared<DeviceEventData>(deviceInfo.deviceName, deviceInfo.deviceType, deviceInfo.deviceId);
        eventBus->publish(EVENT_DEVICE_DISCOVERED, deviceData);
        
        return driver;
      } else {
        updateDriverStatus("EinkDriver", DRIVER_STATUS_ERROR);
        
        // 发布驱动错误事件
        auto errorData = std::make_shared<SystemErrorEventData>("Display driver initialization failed", 2002, "EinkDriver");
        eventBus->publish(EVENT_DRIVER_ERROR, errorData);
      }
    }
    return nullptr;
  }
  
  // 启用驱动
  bool enableDriver(const String& driverName) {
    for (auto& info : driverInfos) {
      if (info.name == driverName && !info.enabled) {
        info.enabled = true;
        info.status = DRIVER_STATUS_RUNNING;
        info.lastActiveTime = millis();
        
        // 发布驱动启用事件
        auto driverData = std::make_shared<DriverEventData>(driverName, info.type);
        eventBus->publish(EVENT_DRIVER_ENABLED, driverData);
        
        Serial.printf("Driver enabled: %s\n", driverName.c_str());
        return true;
      }
    }
    return false;
  }
  
  // 禁用驱动
  bool disableDriver(const String& driverName) {
    for (auto& info : driverInfos) {
      if (info.name == driverName && info.enabled) {
        info.enabled = false;
        info.status = DRIVER_STATUS_DISABLED;
        
        // 发布驱动禁用事件
        auto driverData = std::make_shared<DriverEventData>(driverName, info.type);
        eventBus->publish(EVENT_DRIVER_DISABLED, driverData);
        
        Serial.printf("Driver disabled: %s\n", driverName.c_str());
        return true;
      }
    }
    return false;
  }
  
  // 获取驱动信息
  std::vector<DriverInfo> getDriverInfos() {
    return driverInfos;
  }
  
  // 获取设备信息
  std::vector<DeviceInfo> getDeviceInfos() {
    return deviceInfos;
  }
  
  // 根据设备ID获取设备信息
  DeviceInfo* getDeviceInfo(const String& deviceId) {
    for (auto& info : deviceInfos) {
      if (info.deviceId == deviceId) {
        return &info;
      }
    }
    return nullptr;
  }
  
  // 设置设备属性
  bool setDeviceProperty(const String& deviceId, const String& propertyName, const String& propertyValue) {
    DeviceInfo* deviceInfo = getDeviceInfo(deviceId);
    if (deviceInfo != nullptr) {
      deviceInfo->properties[propertyName] = propertyValue;
      deviceInfo->lastUpdateTime = millis();
      return true;
    }
    return false;
  }
  
  // 获取设备属性
  String getDeviceProperty(const String& deviceId, const String& propertyName) {
    DeviceInfo* deviceInfo = getDeviceInfo(deviceId);
    if (deviceInfo != nullptr) {
      auto it = deviceInfo->properties.find(propertyName);
      if (it != deviceInfo->properties.end()) {
        return it->second;
      }
    }
    return "";
  }
  
  // 扫描设备
  void scanDevices() {
    if (!scanningEnabled) {
      return;
    }
    
    Serial.println("Scanning for devices...");
    unsigned long scanStartTime = millis();
    
    // 标记所有设备为断开连接
    for (auto& info : deviceInfos) {
      if (info.status == DEVICE_STATUS_CONNECTED) {
        updateDeviceStatus(info.deviceId, DEVICE_STATUS_DISCONNECTED);
      }
    }
    
    // 扫描传感器设备
    for (auto driver : sensorDrivers) {
      updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_INITIALIZING);
      
      // 尝试初始化驱动
      SensorConfig config;
      config.type = driver->getType();
      config.pin = -1;
      config.address = 0;
      config.updateInterval = 60000;
      config.tempOffset = 0.0;
      config.humOffset = 0.0;
      
      if (driver->init(config)) {
        updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_READY);
        
        String deviceId = String(driver->getType());
        DeviceInfo* existingDevice = getDeviceInfo(deviceId);
        
        if (existingDevice != nullptr) {
          // 更新现有设备信息
          updateDeviceStatus(deviceId, DEVICE_STATUS_CONNECTED);
          existingDevice->lastUpdateTime = millis();
        } else {
          // 创建新设备信息
          DeviceInfo deviceInfo;
          deviceInfo.deviceId = deviceId;
          deviceInfo.deviceName = driver->getTypeName();
          deviceInfo.deviceType = "sensor";
          deviceInfo.driverName = driver->getTypeName();
          deviceInfo.status = DEVICE_STATUS_CONNECTED;
          deviceInfo.connectionInfo = "Connected";
          deviceInfo.discoveredTime = millis();
          deviceInfo.lastUpdateTime = millis();
          
          deviceInfos.push_back(deviceInfo);
        }
        
        // 发布设备发现事件
        auto deviceData = std::make_shared<DeviceEventData>(driver->getTypeName(), "sensor", deviceId);
        eventBus->publish(EVENT_DEVICE_DISCOVERED, deviceData);
        
        // 发布设备连接事件
        eventBus->publish(EVENT_DEVICE_CONNECTED, deviceData);
      } else {
        updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_ERROR);
        
        // 发布驱动错误事件
        auto errorData = std::make_shared<SystemErrorEventData>("Device scan failed", 2003, driver->getTypeName());
        eventBus->publish(EVENT_DRIVER_ERROR, errorData);
      }
    }
    
    lastScanTime = millis();
    Serial.printf("Device scan completed in %lu ms. Found %d devices.\n", millis() - scanStartTime, deviceInfos.size());
  }
  
  // 设置扫描间隔
  void setScanInterval(unsigned long interval) {
    scanInterval = interval;
    Serial.printf("Device scan interval set to %lu ms\n", interval);
  }
  
  // 启用/禁用设备扫描
  void setScanningEnabled(bool enabled) {
    scanningEnabled = enabled;
    Serial.printf("Device scanning %s\n", enabled ? "enabled" : "disabled");
  }
  
  // 获取扫描状态
  bool isScanningEnabled() {
    return scanningEnabled;
  }
  
  // 循环处理，用于定期扫描设备
  void loop() {
    // 定期扫描设备
    if (scanningEnabled && (millis() - lastScanTime > scanInterval)) {
      scanDevices();
    }
  }
  
  // 清除所有驱动
  void clear() {
    // 发布系统关闭事件
    eventBus->publish(EVENT_SYSTEM_SHUTDOWN, nullptr);
    
    Serial.println("Clearing all drivers...");
    
    for (auto driver : sensorDrivers) {
      // 发布驱动卸载事件
      auto driverData = std::make_shared<DriverEventData>(driver->getTypeName(), "sensor");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      delete driver;
    }
    sensorDrivers.clear();
    
    for (auto driver : displayDrivers) {
      // 发布驱动卸载事件
      auto driverData = std::make_shared<DriverEventData>("EinkDriver", "display");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      delete driver;
    }
    displayDrivers.clear();
    
    driverInfos.clear();
    deviceInfos.clear();
    
    Serial.println("All drivers cleared");
  }
};

// 初始化单例实例
DriverRegistry* DriverRegistry::instance = nullptr;

// 驱动注册宏
template <typename T>
void registerSensorDriver() {
  DriverRegistry::getInstance()->registerSensorDriver(new T());
}

template <typename T>
void registerDisplayDriver() {
  DriverRegistry::getInstance()->registerDisplayDriver(new T());
}

#endif // DRIVER_REGISTRY_H