#ifndef DRIVER_REGISTRY_H
#define DRIVER_REGISTRY_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include <memory>
#include <map>
#include <unordered_map>

// FreeRTOS headers
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>
#include "event_bus.h"
#include "../drivers/peripherals/sensor_driver.h"
#include "../drivers/peripherals/display_driver.h"
#include "../drivers/audio_driver.h"

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

// 基础驱动接口，所有驱动类型都应继承自该接口
class IDriver {
public:
  virtual ~IDriver() {}
  
  // 获取驱动名称
  virtual String getName() const = 0;
  
  // 获取驱动类型
  virtual DriverType getDriverType() const = 0;
  
  // 检测驱动与硬件是否匹配
  virtual bool matchHardware() = 0;
  
  // 获取驱动状态
  virtual DriverStatus getStatus() const = 0;
  
  // 设置驱动状态
  virtual void setStatus(DriverStatus status) = 0;
  
  // 判断驱动是否启用
  virtual bool isEnabled() const = 0;
  
  // 设置驱动启用状态
  virtual void setEnabled(bool enabled) = 0;
};

// 驱动注册中心类
class DriverRegistry {
private:
  static DriverRegistry* instance;
  
  // 驱动列表
  std::map<String, ISensorDriver*> sensorDrivers;     // 传感器驱动，键为驱动名称
  std::map<String, IDisplayDriver*> displayDrivers;   // 显示驱动，键为驱动名称
  std::map<String, AudioDriver*> audioDrivers;        // 音频驱动，键为驱动名称
  
  // 设备信息列表
  std::map<String, DeviceInfo> deviceInfos;           // 设备信息，键为设备ID
  
  // 驱动信息列表
  std::map<String, DriverInfo> driverInfos;           // 驱动信息，键为驱动名称
  
  // 事件总线指针
  EventBus* eventBus;
  
  // 扫描配置
  bool scanningEnabled;
  unsigned long scanInterval;
  unsigned long lastScanTime;
  
  // 互斥锁
  SemaphoreHandle_t registryMutex;
  
  /**
   * @brief 构造函数
   * 
   * 私有构造函数，用于初始化驱动注册中心。
   */
  DriverRegistry() {
    scanningEnabled = true;
    scanInterval = 30000; // 默认30秒扫描一次
    lastScanTime = 0;
    eventBus = EventBus::getInstance();
    registryMutex = xSemaphoreCreateMutex();
  }
  
  // 私有方法：更新驱动状态
  void updateDriverStatus(const String& driverName, DriverStatus status) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    auto it = driverInfos.find(driverName);
    if (it != driverInfos.end()) {
      it->second.status = status;
      it->second.lastActiveTime = millis();
      
      // 发布驱动更新事件
      auto driverData = std::make_shared<DriverEventData>(it->second.name, it->second.type);
      eventBus->publish(EVENT_DRIVER_UPDATED, driverData);
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
  }
  
  // 私有方法：更新设备状态
  void updateDeviceStatus(const String& deviceId, DeviceStatus status) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    auto it = deviceInfos.find(deviceId);
    if (it != deviceInfos.end()) {
      it->second.status = status;
      it->second.lastUpdateTime = millis();
      
      // 发布设备状态变化事件
      auto deviceData = std::make_shared<DeviceEventData>(it->second.deviceName, it->second.deviceType, deviceId);
      eventBus->publish(EVENT_DEVICE_STATUS_CHANGED, deviceData);
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
  }
  
  // 私有方法：创建设备信息
  DeviceInfo createDeviceInfo(const String& deviceId, const String& deviceName, 
                              const String& deviceType, const String& driverName, 
                              DeviceStatus status, const String& connectionInfo) {
    DeviceInfo deviceInfo;
    deviceInfo.deviceId = deviceId;
    deviceInfo.deviceName = deviceName;
    deviceInfo.deviceType = deviceType;
    deviceInfo.driverName = driverName;
    deviceInfo.status = status;
    deviceInfo.connectionInfo = connectionInfo;
    deviceInfo.discoveredTime = millis();
    deviceInfo.lastUpdateTime = millis();
    return deviceInfo;
  }
  
  // 私有方法：发布设备发现事件
  void publishDeviceDiscovered(const DeviceInfo& deviceInfo) {
    auto deviceData = std::make_shared<DeviceEventData>(deviceInfo.deviceName, deviceInfo.deviceType, deviceInfo.deviceId);
    eventBus->publish(EVENT_DEVICE_DISCOVERED, deviceData);
  }
  
  // 私有方法：发布驱动错误事件
  void publishDriverError(const String& driverName, const String& message, int errorCode) {
    auto errorData = std::make_shared<SystemErrorEventData>(message, errorCode, driverName);
    eventBus->publish(EVENT_DRIVER_ERROR, errorData);
  }
  
  // 私有方法：发布设备连接事件
  void publishDeviceConnected(const DeviceInfo& deviceInfo) {
    auto deviceData = std::make_shared<DeviceEventData>(deviceInfo.deviceName, deviceInfo.deviceType, deviceInfo.deviceId);
    eventBus->publish(EVENT_DEVICE_CONNECTED, deviceData);
  }
  
  // 私有方法：处理传感器设备扫描结果
  void handleSensorScanResult(ISensorDriver* driver, bool initResult, const String& deviceId, const String& deviceName);

public:
  // 获取单例实例
  static DriverRegistry* getInstance() {
    static DriverRegistry instance;
    return &instance;
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
    
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    String driverName = driver->getTypeName();
    if (sensorDrivers.find(driverName) != sensorDrivers.end()) {
      Serial.printf("Error: Sensor driver already registered: %s\n", driverName.c_str());
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return false;
    }
    
    sensorDrivers[driverName] = driver;
    
    // 创建驱动信息
    DriverInfo info;
    info.name = driverName;
    info.type = "sensor";
    info.version = "1.0.0";
    info.vendor = "Unknown";
    info.driverType = DRIVER_TYPE_SENSOR;
    info.status = DRIVER_STATUS_UNINITIALIZED;
    info.enabled = false;
    info.deviceId = String(driver->getType());
    info.deviceName = driverName;
    info.deviceType = "sensor";
    info.firmwareVersion = "1.0.0";
    info.lastActiveTime = millis();
    info.startTime = millis();
    info.errorCount = 0;
    
    driverInfos[driverName] = info;
    
    // 发布驱动注册事件
    auto driverData = std::make_shared<DriverEventData>(info.name, info.type);
    eventBus->publish(EVENT_DRIVER_REGISTERED, driverData);
    
    Serial.printf("Sensor driver registered: %s\n", info.name.c_str());
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    return true;
  }
  
  // 注册显示驱动
  bool registerDisplayDriver(IDisplayDriver* driver) {
    if (driver == nullptr) {
      Serial.println("Error: Attempt to register null display driver");
      return false;
    }
    
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    String driverName = "Eink_Driver";
    if (displayDrivers.find(driverName) != displayDrivers.end()) {
      Serial.printf("Error: Display driver already registered: %s\n", driverName.c_str());
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return false;
    }
    
    displayDrivers[driverName] = driver;
    
    // 创建驱动信息（只支持EINK）
    DriverInfo info;
    info.name = driverName;
    info.type = "display";
    info.version = "1.0.0";
    info.vendor = "Unknown";
    info.driverType = DRIVER_TYPE_DISPLAY;
    info.status = DRIVER_STATUS_UNINITIALIZED;
    info.enabled = false;
    info.deviceId = "eink_display";
    info.deviceName = "EinkDisplay";
    info.deviceType = "display";
    info.firmwareVersion = "1.0.0";
    info.lastActiveTime = millis();
    info.startTime = millis();
    info.errorCount = 0;
    
    driverInfos[driverName] = info;
    
    // 发布驱动注册事件
    auto driverData = std::make_shared<DriverEventData>(info.name, info.type);
    eventBus->publish(EVENT_DRIVER_REGISTERED, driverData);
    
    Serial.printf("Display driver registered: %s\n", info.name.c_str());
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    return true;
  }
  
  // 注册音频驱动
  bool registerAudioDriver(AudioDriver* driver) {
    if (driver == nullptr) {
      Serial.println("Error: Attempt to register null audio driver");
      return false;
    }
    
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    String driverName = String(driver->getType());
    if (audioDrivers.find(driverName) != audioDrivers.end()) {
      Serial.printf("Error: Audio driver already registered: %s\n", driverName.c_str());
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return false;
    }
    
    audioDrivers[driverName] = driver;
    
    // 创建驱动信息
    DriverInfo info;
    info.name = driverName;
    info.type = "audio";
    info.version = "1.0.0";
    info.vendor = "Unknown";
    info.driverType = DRIVER_TYPE_AUDIO;
    info.status = DRIVER_STATUS_UNINITIALIZED;
    info.enabled = false;
    info.deviceId = String(driver->getType());
    info.deviceName = "AudioDevice";
    info.deviceType = "audio";
    info.firmwareVersion = "1.0.0";
    info.lastActiveTime = millis();
    info.startTime = millis();
    info.errorCount = 0;
    
    driverInfos[driverName] = info;
    
    // 发布驱动注册事件
    auto driverData = std::make_shared<DriverEventData>(info.name, info.type);
    eventBus->publish(EVENT_DRIVER_REGISTERED, driverData);
    
    Serial.printf("Audio driver registered: %s\n", info.name.c_str());
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    return true;
  }
  
  // 卸载驱动
  bool unregisterDriver(const String& driverName) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    // 查找并移除传感器驱动
    auto sensorIt = sensorDrivers.find(driverName);
    if (sensorIt != sensorDrivers.end()) {
      // 发布驱动卸载事件
      auto driverData = std::make_shared<DriverEventData>(driverName, "sensor");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      // 更新驱动状态
      updateDriverStatus(driverName, DRIVER_STATUS_UNREGISTERED);
      
      delete sensorIt->second;
      sensorDrivers.erase(sensorIt);
      
      // 移除驱动信息
      driverInfos.erase(driverName);
      
      Serial.printf("Sensor driver unregistered: %s\n", driverName.c_str());
      
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return true;
    }
    
    // 查找并移除显示驱动
    auto displayIt = displayDrivers.find(driverName);
    if (displayIt != displayDrivers.end()) {
      // 发布驱动卸载事件
      auto driverData = std::make_shared<DriverEventData>(driverName, "display");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      // 更新驱动状态
      updateDriverStatus(driverName, DRIVER_STATUS_UNREGISTERED);
      
      delete displayIt->second;
      displayDrivers.erase(displayIt);
      
      // 移除驱动信息
      driverInfos.erase(driverName);
      
      Serial.printf("Display driver unregistered: %s\n", driverName.c_str());
      
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return true;
    }
    
    // 查找并移除音频驱动
    auto audioIt = audioDrivers.find(driverName);
    if (audioIt != audioDrivers.end()) {
      // 发布驱动卸载事件
      auto driverData = std::make_shared<DriverEventData>(driverName, "audio");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      // 更新驱动状态
      updateDriverStatus(driverName, DRIVER_STATUS_UNREGISTERED);
      
      delete audioIt->second;
      audioDrivers.erase(audioIt);
      
      // 移除驱动信息
      driverInfos.erase(driverName);
      
      Serial.printf("Audio driver unregistered: %s\n", driverName.c_str());
      
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return true;
    }
    
    Serial.printf("Error: Driver not found for unregistration: %s\n", driverName.c_str());
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return false;
  }
  
  // 获取所有传感器驱动
  std::vector<ISensorDriver*> getSensorDrivers() {
    std::vector<ISensorDriver*> drivers;
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    for (const auto& pair : sensorDrivers) {
      drivers.push_back(pair.second);
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return drivers;
  }
  
  // 获取所有显示驱动
  std::vector<IDisplayDriver*> getDisplayDrivers() {
    std::vector<IDisplayDriver*> drivers;
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    for (const auto& pair : displayDrivers) {
      drivers.push_back(pair.second);
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return drivers;
  }
  
  // 获取所有音频驱动
  std::vector<AudioDriver*> getAudioDrivers() {
    std::vector<AudioDriver*> drivers;
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    for (const auto& pair : audioDrivers) {
      drivers.push_back(pair.second);
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return drivers;
  }
  
  // 根据传感器类型获取驱动
  ISensorDriver* getSensorDriver(SensorType type) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    for (const auto& pair : sensorDrivers) {
      if (pair.second->getType() == type) {
        ISensorDriver* driver = pair.second;
        if (registryMutex) {
          xSemaphoreGive(registryMutex);
        }
        return driver;
      }
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return nullptr;
  }
  
  // 根据驱动名称获取传感器驱动
  ISensorDriver* getSensorDriverByName(const String& name) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    auto it = sensorDrivers.find(name);
    if (it != sensorDrivers.end()) {
      ISensorDriver* driver = it->second;
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return driver;
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return nullptr;
  }
  
  // 根据显示类型获取驱动
  IDisplayDriver* getDisplayDriver(DisplayCategory type) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    for (const auto& pair : displayDrivers) {
      if (pair.second->getDisplayType() == type) {
        IDisplayDriver* driver = pair.second;
        if (registryMutex) {
          xSemaphoreGive(registryMutex);
        }
        return driver;
      }
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return nullptr;
  }
  
  // 自动检测传感器驱动 - 优化版，减少初始化时间
  ISensorDriver* autoDetectSensorDriver() {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    std::vector<ISensorDriver*> drivers;
    for (const auto& pair : sensorDrivers) {
      drivers.push_back(pair.second);
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    for (auto driver : drivers) {
      String driverName = driver->getTypeName();
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      // 先使用matchHardware()快速检测硬件，减少不必要的完整初始化
      if (driver->matchHardware()) {
        // 硬件匹配成功，进行完整初始化
        SensorConfig config;
        config.type = driver->getType();
        config.pin = -1;
        config.address = 0;
        config.updateInterval = 60000;
        config.tempOffset = 0.0;
        config.humOffset = 0.0;
        
        if (driver->init(config)) {
          updateDriverStatus(driverName, DRIVER_STATUS_READY);
          
          // 创建设备信息
          String deviceId = String(driver->getType());
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, driverName, "sensor", 
                                                 driverName, DEVICE_STATUS_DISCOVERED, "Auto-detected");
          
          if (registryMutex) {
            xSemaphoreTake(registryMutex, portMAX_DELAY);
          }
          deviceInfos[deviceId] = deviceInfo;
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          
          // 发布传感器发现事件
          publishDeviceDiscovered(deviceInfo);
          
          return driver;
        }
      }
      
      updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
      
      // 发布驱动错误事件
      publishDriverError(driverName, "Driver initialization failed", 2001);
    }
    return nullptr;
  }
  
  // 自动检测显示驱动（只支持EINK）
  IDisplayDriver* autoDetectDisplayDriver() {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    if (displayDrivers.empty()) {
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      Serial.println("No display drivers registered");
      return nullptr;
    }
    
    std::vector<IDisplayDriver*> drivers;
    for (const auto& pair : displayDrivers) {
      drivers.push_back(pair.second);
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    for (auto driver : drivers) {
      String driverName = "Eink_Driver";
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      // 先使用matchHardware()快速检测硬件，减少不必要的完整初始化
      if (driver->matchHardware()) {
        // 硬件匹配成功，进行完整初始化
        if (driver->init()) {
          updateDriverStatus(driverName, DRIVER_STATUS_READY);
          
          // 创建设备信息
          String deviceId = "eink_display";
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, "EinkDisplay", "display", 
                                                 driverName, DEVICE_STATUS_DISCOVERED, "Auto-detected");
          
          if (registryMutex) {
            xSemaphoreTake(registryMutex, portMAX_DELAY);
          }
          deviceInfos[deviceId] = deviceInfo;
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          
          // 发布设备发现事件
          publishDeviceDiscovered(deviceInfo);
          
          return driver;
        }
      }
      
      updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
      
      // 发布驱动错误事件
      publishDriverError(driverName, "Eink driver initialization failed", 2002);
    }
    
    return nullptr;
  }
  
  // 自动检测音频驱动
  AudioDriver* autoDetectAudioDriver() {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    if (audioDrivers.empty()) {
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      Serial.println("No audio drivers registered");
      return nullptr;
    }
    
    std::vector<AudioDriver*> drivers;
    for (const auto& pair : audioDrivers) {
      drivers.push_back(pair.second);
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    for (auto driver : drivers) {
      String driverName = String(driver->getType());
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      // 先使用matchHardware()快速检测硬件，减少不必要的完整初始化
      if (driver->matchHardware()) {
        // 硬件匹配成功，进行完整初始化
        if (driver->init()) {
          updateDriverStatus(driverName, DRIVER_STATUS_READY);
          
          // 创建设备信息
          String deviceId = String(driver->getType());
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, "AudioDevice", "audio", 
                                                 driverName, DEVICE_STATUS_DISCOVERED, "Auto-detected");
          
          if (registryMutex) {
            xSemaphoreTake(registryMutex, portMAX_DELAY);
          }
          deviceInfos[deviceId] = deviceInfo;
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          
          // 发布设备发现事件
          publishDeviceDiscovered(deviceInfo);
          
          return driver;
        }
      }
      
      updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
      
      // 发布驱动错误事件
      publishDriverError(driverName, "Audio driver initialization failed", 2003);
    }
    
    return nullptr;
  }
  
  // 启用驱动
  bool enableDriver(const String& driverName) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    auto it = driverInfos.find(driverName);
    if (it != driverInfos.end() && !it->second.enabled) {
      it->second.enabled = true;
      it->second.status = DRIVER_STATUS_RUNNING;
      it->second.lastActiveTime = millis();
      
      // 发布驱动启用事件
      auto driverData = std::make_shared<DriverEventData>(driverName, it->second.type);
      eventBus->publish(EVENT_DRIVER_ENABLED, driverData);
      
      Serial.printf("Driver enabled: %s\n", driverName.c_str());
      
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return true;
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return false;
  }
  
  // 禁用驱动
  bool disableDriver(const String& driverName) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    auto it = driverInfos.find(driverName);
    if (it != driverInfos.end() && it->second.enabled) {
      it->second.enabled = false;
      it->second.status = DRIVER_STATUS_DISABLED;
      
      // 发布驱动禁用事件
      auto driverData = std::make_shared<DriverEventData>(driverName, it->second.type);
      eventBus->publish(EVENT_DRIVER_DISABLED, driverData);
      
      Serial.printf("Driver disabled: %s\n", driverName.c_str());
      
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return true;
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return false;
  }
  
  // 获取驱动信息
  std::vector<DriverInfo> getDriverInfos() {
    std::vector<DriverInfo> infos;
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    for (const auto& pair : driverInfos) {
      infos.push_back(pair.second);
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return infos;
  }
  
  // 获取设备信息
  std::vector<DeviceInfo> getDeviceInfos() {
    std::vector<DeviceInfo> infos;
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    for (const auto& pair : deviceInfos) {
      infos.push_back(pair.second);
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return infos;
  }
  
  // 根据设备ID获取设备信息
  DeviceInfo* getDeviceInfo(const String& deviceId) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    auto it = deviceInfos.find(deviceId);
    if (it != deviceInfos.end()) {
      DeviceInfo* info = &(it->second);
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return info;
    }
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return nullptr;
  }
  
  // 设置设备属性
  bool setDeviceProperty(const String& deviceId, const String& propertyName, const String& propertyValue) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    auto it = deviceInfos.find(deviceId);
    if (it != deviceInfos.end()) {
      it->second.properties[propertyName] = propertyValue;
      it->second.lastUpdateTime = millis();
      
      if (registryMutex) {
        xSemaphoreGive(registryMutex);
      }
      return true;
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    return false;
  }
  
  // 获取设备属性
  String getDeviceProperty(const String& deviceId, const String& propertyName) {
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    auto it = deviceInfos.find(deviceId);
    if (it != deviceInfos.end()) {
      auto propIt = it->second.properties.find(propertyName);
      if (propIt != it->second.properties.end()) {
        String value = propIt->second;
        if (registryMutex) {
          xSemaphoreGive(registryMutex);
        }
        return value;
      }
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
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
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    for (auto& pair : deviceInfos) {
      if (pair.second.status == DEVICE_STATUS_CONNECTED) {
        updateDeviceStatus(pair.first, DEVICE_STATUS_DISCONNECTED);
      }
    }
    
    // 复制驱动列表，避免在扫描过程中修改
    std::vector<ISensorDriver*> sensorDriversCopy;
    for (const auto& pair : sensorDrivers) {
      sensorDriversCopy.push_back(pair.second);
    }
    
    std::vector<IDisplayDriver*> displayDriversCopy;
    for (const auto& pair : displayDrivers) {
      displayDriversCopy.push_back(pair.second);
    }
    
    std::vector<AudioDriver*> audioDriversCopy;
    for (const auto& pair : audioDrivers) {
      audioDriversCopy.push_back(pair.second);
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    // 扫描传感器设备
    for (auto driver : sensorDriversCopy) {
      String driverName = driver->getTypeName();
      String deviceId = String(driver->getType());
      String deviceName = driverName;
      
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      // 尝试初始化驱动
      SensorConfig config;
      config.type = driver->getType();
      config.pin = -1;
      config.address = 0;
      config.updateInterval = 60000;
      config.tempOffset = 0.0;
      config.humOffset = 0.0;
      
      bool initResult = driver->init(config);
      
      if (initResult) {
        updateDriverStatus(driverName, DRIVER_STATUS_READY);
        
        if (registryMutex) {
          xSemaphoreTake(registryMutex, portMAX_DELAY);
        }
        auto it = deviceInfos.find(deviceId);
        if (it != deviceInfos.end()) {
          // 更新现有设备信息
          updateDeviceStatus(deviceId, DEVICE_STATUS_CONNECTED);
          it->second.lastUpdateTime = millis();
          
          // 发布设备发现和连接事件
          DeviceInfo tempDeviceInfo = it->second;
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          publishDeviceDiscovered(tempDeviceInfo);
          publishDeviceConnected(tempDeviceInfo);
        } else {
          // 创建新设备信息
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, deviceName, "sensor", 
                                                 driverName, DEVICE_STATUS_CONNECTED, "Connected");
          deviceInfos[deviceId] = deviceInfo;
          
          // 发布设备发现和连接事件
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          publishDeviceDiscovered(deviceInfo);
          publishDeviceConnected(deviceInfo);
        }
      } else {
        updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
        publishDriverError(driverName, "Device scan failed", 2003);
      }
    }
    
    // 扫描显示设备（只支持EINK）
    for (auto driver : displayDriversCopy) {
      String driverName = "Eink_Driver";
      // 使用DisplayCategory枚举值作为deviceId
      String deviceId = String(static_cast<int>(driver->getDisplayType()));
      String deviceName = "EinkDisplay";
      
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      bool initResult = driver->init();
      
      if (initResult) {
        updateDriverStatus(driverName, DRIVER_STATUS_READY);
        
        if (registryMutex) {
          xSemaphoreTake(registryMutex, portMAX_DELAY);
        }
        auto it = deviceInfos.find(deviceId);
        if (it != deviceInfos.end()) {
          // 更新现有设备信息
          updateDeviceStatus(deviceId, DEVICE_STATUS_CONNECTED);
          it->second.lastUpdateTime = millis();
          
          // 发布设备发现和连接事件
          DeviceInfo tempDeviceInfo = it->second;
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          publishDeviceDiscovered(tempDeviceInfo);
          publishDeviceConnected(tempDeviceInfo);
        } else {
          // 创建新设备信息
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, deviceName, "display", 
                                                 driverName, DEVICE_STATUS_CONNECTED, "Connected");
          deviceInfos[deviceId] = deviceInfo;
          
          // 发布设备发现和连接事件
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          publishDeviceDiscovered(deviceInfo);
          publishDeviceConnected(deviceInfo);
        }
      } else {
        updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
        publishDriverError(driverName, "Eink device scan failed", 2004);
      }
    }
    
    // 扫描音频设备
    for (auto driver : audioDriversCopy) {
      String driverName = String(driver->getType());
      String deviceId = String(driver->getType());
      String deviceName = "AudioDevice";
      
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      bool initResult = driver->init();
      
      if (initResult) {
        updateDriverStatus(driverName, DRIVER_STATUS_READY);
        
        if (registryMutex) {
          xSemaphoreTake(registryMutex, portMAX_DELAY);
        }
        auto it = deviceInfos.find(deviceId);
        if (it != deviceInfos.end()) {
          // 更新现有设备信息
          updateDeviceStatus(deviceId, DEVICE_STATUS_CONNECTED);
          it->second.lastUpdateTime = millis();
          
          // 发布设备发现和连接事件
          DeviceInfo tempDeviceInfo = it->second;
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          publishDeviceDiscovered(tempDeviceInfo);
          publishDeviceConnected(tempDeviceInfo);
        } else {
          // 创建新设备信息
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, deviceName, "audio", 
                                                 driverName, DEVICE_STATUS_CONNECTED, "Connected");
          deviceInfos[deviceId] = deviceInfo;
          
          // 发布设备发现和连接事件
          if (registryMutex) {
            xSemaphoreGive(registryMutex);
          }
          publishDeviceDiscovered(deviceInfo);
          publishDeviceConnected(deviceInfo);
        }
      } else {
        updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
        publishDriverError(driverName, "Audio device scan failed", 2005);
      }
    }
    
    lastScanTime = millis();
    
    size_t deviceCount = 0;
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
      deviceCount = deviceInfos.size();
      xSemaphoreGive(registryMutex);
    }
    
    Serial.printf("Device scan completed in %lu ms. Found %d devices.\n", millis() - scanStartTime, deviceCount);
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
  
  // 动态检测硬件变化
  bool detectHardwareChanges() {
    Serial.println("检测硬件变化...");
    
    bool hardwareChanged = false;
    
    // 记录当前设备状态
    std::map<String, bool> currentDevices;
    for (auto& device : deviceInfos) {
      currentDevices[device.second.deviceId] = true;
    }
    
    // 执行硬件匹配检测
    performHardwareMatch();
    
    // 检测新增设备
    for (auto& info : driverInfos) {
      if (info.second.status == DRIVER_STATUS_READY) {
        String deviceId = String(info.second.deviceId);
        if (currentDevices.find(deviceId) == currentDevices.end()) {
          // 新设备被检测到
          Serial.printf("检测到新设备: %s\n", info.second.name.c_str());
          hardwareChanged = true;
        }
      }
    }
    
    // 检测移除的设备
    for (auto& device : deviceInfos) {
      bool driverExists = false;
      for (auto& info : driverInfos) {
        if (info.second.name == device.second.driverName && info.second.status == DRIVER_STATUS_READY) {
          driverExists = true;
          break;
        }
      }
      
      if (!driverExists && device.second.status == DEVICE_STATUS_CONNECTED) {
        // 设备被移除
        Serial.printf("设备已移除: %s\n", device.second.deviceName.c_str());
        updateDeviceStatus(device.second.deviceId, DEVICE_STATUS_DISCONNECTED);
        hardwareChanged = true;
      }
    }
    
    if (hardwareChanged) {
      // 发布硬件变化事件
      eventBus->publish(EVENT_HARDWARE_CHANGED, nullptr);
      Serial.println("硬件变化检测完成，发现变化");
    } else {
      Serial.println("硬件变化检测完成，未发现变化");
    }
    
    return hardwareChanged;
  }
  
  // 重新适配硬件变化
  void reconfigureHardware() {
    Serial.println("重新适配硬件...");
    
    // 执行硬件匹配检测
    performHardwareMatch();
    
    // 启用兼容模块
    enableCompatibleModules();
    
    // 禁用不兼容模块
    disableIncompatibleModules();
    
    // 打印自检结果
    printSelfCheckResult();
    
    Serial.println("硬件重新适配完成");
  }
  
  // 清除所有驱动
  void clear() {
    // 发布系统关闭事件
    eventBus->publish(EVENT_SYSTEM_SHUTDOWN, nullptr);
    
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    Serial.println("Clearing all drivers...");
    
    // 清除传感器驱动
    for (const auto& pair : sensorDrivers) {
      // 发布驱动卸载事件
      auto driverData = std::make_shared<DriverEventData>(pair.first, "sensor");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      delete pair.second;
    }
    sensorDrivers.clear();
    
    // 清除显示驱动
    for (const auto& pair : displayDrivers) {
      // 发布驱动卸载事件
      String driverName = "Eink_Driver";
      auto driverData = std::make_shared<DriverEventData>(driverName, "display");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      delete pair.second;
    }
    displayDrivers.clear();
    
    // 清除音频驱动
    for (const auto& pair : audioDrivers) {
      // 发布驱动卸载事件
      String driverName = pair.first;
      auto driverData = std::make_shared<DriverEventData>(driverName, "audio");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      delete pair.second;
    }
    audioDrivers.clear();
    
    driverInfos.clear();
    deviceInfos.clear();
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    Serial.println("All drivers cleared");
  }
  
  // 执行驱动硬件匹配检测
  bool performHardwareMatch() {
    Serial.println("Performing hardware match detection...");
    
    if (registryMutex) {
      xSemaphoreTake(registryMutex, portMAX_DELAY);
    }
    
    // 复制驱动列表，避免在检测过程中修改
    std::vector<ISensorDriver*> sensorDriversCopy;
    for (const auto& pair : sensorDrivers) {
      sensorDriversCopy.push_back(pair.second);
    }
    
    std::vector<IDisplayDriver*> displayDriversCopy;
    for (const auto& pair : displayDrivers) {
      displayDriversCopy.push_back(pair.second);
    }
    
    std::vector<AudioDriver*> audioDriversCopy;
    for (const auto& pair : audioDrivers) {
      audioDriversCopy.push_back(pair.second);
    }
    
    if (registryMutex) {
      xSemaphoreGive(registryMutex);
    }
    
    bool allMatched = true;
    
    // 检测传感器驱动
    Serial.println("Checking sensor drivers...");
    for (auto driver : sensorDriversCopy) {
      String driverName = driver->getTypeName();
      bool matched = driver->matchHardware();
      
      if (matched) {
        Serial.printf("✓ %s driver matches hardware\n", driverName.c_str());
      } else {
        Serial.printf("✗ %s driver does not match hardware\n", driverName.c_str());
        allMatched = false;
      }
      
      // 更新驱动状态
      updateDriverStatus(driverName, matched ? DRIVER_STATUS_READY : DRIVER_STATUS_ERROR);
    }
    
    // 检测显示驱动（只支持EINK）
    Serial.println("Checking display drivers...");
    for (auto driver : displayDriversCopy) {
      String driverName = "Eink_Driver";
      bool matched = driver->matchHardware();
      
      if (matched) {
        Serial.printf("✓ %s driver matches hardware\n", driverName.c_str());
      } else {
        Serial.printf("✗ %s driver does not match hardware\n", driverName.c_str());
        allMatched = false;
      }
      
      // 更新驱动状态
      updateDriverStatus(driverName, matched ? DRIVER_STATUS_READY : DRIVER_STATUS_ERROR);
    }
    
    // 检测音频驱动
    Serial.println("Checking audio drivers...");
    for (auto driver : audioDriversCopy) {
      String driverName = String(driver->getType());
      bool matched = driver->matchHardware();
      
      if (matched) {
        Serial.printf("✓ %s driver matches hardware\n", driverName.c_str());
      } else {
        Serial.printf("✗ %s driver does not match hardware\n", driverName.c_str());
        allMatched = false;
      }
      
      // 更新驱动状态
      updateDriverStatus(driverName, matched ? DRIVER_STATUS_READY : DRIVER_STATUS_ERROR);
    }
    
    return allMatched;
  }
  
  // 根据检测结果启用兼容模块
  void enableCompatibleModules() {
    Serial.println("Enabling compatible modules...");
    
    // 启用所有状态为READY的驱动
    for (auto& info : driverInfos) {
      if (info.second.status == DRIVER_STATUS_READY && !info.second.enabled) {
        enableDriver(info.second.name);
      }
    }
  }
  
  // 禁用不支持的功能模块
  void disableIncompatibleModules() {
    Serial.println("Disabling incompatible modules...");
    
    // 禁用所有状态为ERROR的驱动
    for (auto& info : driverInfos) {
      if (info.second.status == DRIVER_STATUS_ERROR && info.second.enabled) {
        disableDriver(info.second.name);
      }
    }
    
    // 标记设备为不可用
    for (auto& device : deviceInfos) {
      bool driverFound = false;
      for (auto& info : driverInfos) {
        if (info.second.name == device.second.driverName && info.second.status == DRIVER_STATUS_READY) {
          driverFound = true;
          break;
        }
      }
      
      if (!driverFound) {
        updateDeviceStatus(device.second.deviceId, DEVICE_STATUS_ERROR);
      }
    }
  }
  
  // 打印自检结果
  void printSelfCheckResult() {
    Serial.println("====================================");
    Serial.println("Self-Check Results:");
    Serial.println("====================================");
    
    // 打印驱动状态
    Serial.println("Driver Status:");
    for (auto& info : driverInfos) {
      String statusStr;
      switch (info.second.status) {
        case DRIVER_STATUS_READY:
          statusStr = "READY   ";
          break;
        case DRIVER_STATUS_ERROR:
          statusStr = "ERROR   ";
          break;
        case DRIVER_STATUS_DISABLED:
          statusStr = "DISABLED";
          break;
        default:
          statusStr = "UNKNOWN ";
          break;
      }
      
      String enabledStr = info.second.enabled ? "✓" : "✗";
      Serial.printf("%s %s [%s] %s\n", enabledStr, statusStr.c_str(), info.second.type.c_str(), info.second.name.c_str());
    }
    
    Serial.println();
    
    // 打印设备状态
    Serial.println("Device Status:");
    for (auto& device : deviceInfos) {
      String statusStr;
      switch (device.second.status) {
        case DEVICE_STATUS_CONNECTED:
          statusStr = "CONNECTED";
          break;
        case DEVICE_STATUS_DISCONNECTED:
          statusStr = "DISCONNECTED";
          break;
        case DEVICE_STATUS_ERROR:
          statusStr = "ERROR";
          break;
        case DEVICE_STATUS_DISCOVERED:
          statusStr = "DISCOVERED";
          break;
        default:
          statusStr = "UNKNOWN";
          break;
      }
      
      Serial.printf("%s [%s] %s\n", statusStr.c_str(), device.second.deviceType.c_str(), device.second.deviceName.c_str());
    }
    
    // 发布自检完成事件
    eventBus->publish(EVENT_SYSTEM_ACTIVE, nullptr);
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

template <typename T>
void registerAudioDriver() {
  DriverRegistry::getInstance()->registerAudioDriver(new T());
}

#endif // DRIVER_REGISTRY_H
