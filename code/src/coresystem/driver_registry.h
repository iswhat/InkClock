#ifndef DRIVER_REGISTRY_H
#define DRIVER_REGISTRY_H

#include <Arduino.h>
#include <vector>
#include <functional>
#include <memory>
#include "event_bus.h"
#include "../drivers/sensors/sensor_driver.h"
#include "../drivers/displays/display_driver.h"
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
  
  // 私有构造函数
  DriverRegistry() {
    scanningEnabled = true;
    scanInterval = 30000; // 默认30秒扫描一次
    lastScanTime = 0;
  }
  
  // 驱动列表
  std::vector<ISensorDriver*> sensorDrivers;
  std::vector<IDisplayDriver*> displayDrivers;
  std::vector<AudioDriver*> audioDrivers;
  
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
    
    // 创建驱动信息（只支持EINK）
    DriverInfo info;
    info.name = "Eink_Driver";
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
  
  // 注册音频驱动
  bool registerAudioDriver(AudioDriver* driver) {
    if (driver == nullptr) {
      Serial.println("Error: Attempt to register null audio driver");
      return false;
    }
    
    audioDrivers.push_back(driver);
    
    // 创建驱动信息
    DriverInfo info;
    info.name = String(driver->getType());
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
    
    driverInfos.push_back(info);
    
    // 发布驱动注册事件
    auto driverData = std::make_shared<DriverEventData>(info.name, info.type);
    eventBus->publish(EVENT_DRIVER_REGISTERED, driverData);
    
    Serial.printf("Audio driver registered: %s\n", info.name.c_str());
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
      if (driverName == "Eink_Driver") {
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
    
    // 查找并移除音频驱动
    for (auto it = audioDrivers.begin(); it != audioDrivers.end(); ++it) {
      String audioDriverName = String((*it)->getType());
      if (audioDriverName == driverName) {
        // 发布驱动卸载事件
        auto driverData = std::make_shared<DriverEventData>(driverName, "audio");
        eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
        
        // 更新驱动状态
        updateDriverStatus(driverName, DRIVER_STATUS_UNREGISTERED);
        
        delete *it;
        audioDrivers.erase(it);
        
        // 移除驱动信息
        for (auto infoIt = driverInfos.begin(); infoIt != driverInfos.end(); ++infoIt) {
          if (infoIt->name == driverName) {
            driverInfos.erase(infoIt);
            break;
          }
        }
        
        Serial.printf("Audio driver unregistered: %s\n", driverName.c_str());
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
  
  // 获取所有音频驱动
  std::vector<AudioDriver*> getAudioDrivers() {
    return audioDrivers;
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
  IDisplayDriver* getDisplayDriver(DisplayType type) {
    for (auto driver : displayDrivers) {
      if (driver->getType() == type) {
        return driver;
      }
    }
    return nullptr;
  }
  
  // 自动检测传感器驱动 - 优化版，减少初始化时间
  ISensorDriver* autoDetectSensorDriver() {
    for (auto driver : sensorDrivers) {
      updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_INITIALIZING);
      
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
          updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_READY);
          
          // 创建设备信息
          String deviceId = String(driver->getType());
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, driver->getTypeName(), "sensor", 
                                                 driver->getTypeName(), DEVICE_STATUS_DISCOVERED, "Auto-detected");
          deviceInfos.push_back(deviceInfo);
          
          // 发布传感器发现事件
          publishDeviceDiscovered(deviceInfo);
          
          return driver;
        }
      }
      
      updateDriverStatus(driver->getTypeName(), DRIVER_STATUS_ERROR);
      
      // 发布驱动错误事件
      publishDriverError(driver->getTypeName(), "Driver initialization failed", 2001);
    }
    return nullptr;
  }
  
  // 自动检测显示驱动（只支持EINK）
  IDisplayDriver* autoDetectDisplayDriver() {
    if (displayDrivers.empty()) {
      Serial.println("No display drivers registered");
      return nullptr;
    }
    
    for (auto driver : displayDrivers) {
      String driverName = "Eink_Driver";
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      try {
        // 先使用matchHardware()快速检测硬件，减少不必要的完整初始化
        if (driver->matchHardware()) {
          // 硬件匹配成功，进行完整初始化
          if (driver->init()) {
            updateDriverStatus(driverName, DRIVER_STATUS_READY);
            
            // 创建设备信息
            String deviceId = String(driver->getType());
            DeviceInfo deviceInfo = createDeviceInfo(deviceId, "EinkDisplay", "display", 
                                                   driverName, DEVICE_STATUS_DISCOVERED, "Auto-detected");
            deviceInfos.push_back(deviceInfo);
            
            // 发布设备发现事件
            publishDeviceDiscovered(deviceInfo);
            
            return driver;
          }
        }
      } catch (const std::exception& e) {
        Serial.printf("Display driver detection failed: %s\n", e.what());
        publishDriverError(driverName, String("Eink driver initialization failed: ") + e.what(), 2002);
      } catch (...) {
        Serial.println("Display driver detection failed with unknown error");
        publishDriverError(driverName, "Eink driver initialization failed with unknown error", 2002);
      }
      
      updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
    }
    
    return nullptr;
  }
  
  // 自动检测音频驱动
  AudioDriver* autoDetectAudioDriver() {
    if (audioDrivers.empty()) {
      Serial.println("No audio drivers registered");
      return nullptr;
    }
    
    for (auto driver : audioDrivers) {
      String driverName = String(driver->getType());
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      try {
        // 先使用matchHardware()快速检测硬件，减少不必要的完整初始化
        if (driver->matchHardware()) {
          // 硬件匹配成功，进行完整初始化
          if (driver->init()) {
            updateDriverStatus(driverName, DRIVER_STATUS_READY);
            
            // 创建设备信息
            String deviceId = String(driver->getType());
            DeviceInfo deviceInfo = createDeviceInfo(deviceId, "AudioDevice", "audio", 
                                                   driverName, DEVICE_STATUS_DISCOVERED, "Auto-detected");
            deviceInfos.push_back(deviceInfo);
            
            // 发布设备发现事件
            publishDeviceDiscovered(deviceInfo);
            
            return driver;
          }
        }
      } catch (const std::exception& e) {
        Serial.printf("Audio driver detection failed: %s\n", e.what());
        publishDriverError(driverName, String("Audio driver initialization failed: ") + e.what(), 2003);
      } catch (...) {
        Serial.println("Audio driver detection failed with unknown error");
        publishDriverError(driverName, "Audio driver initialization failed with unknown error", 2003);
      }
      
      updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
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
        
        DeviceInfo* existingDevice = getDeviceInfo(deviceId);
        
        if (existingDevice != nullptr) {
          // 更新现有设备信息
          updateDeviceStatus(deviceId, DEVICE_STATUS_CONNECTED);
          existingDevice->lastUpdateTime = millis();
          
          // 发布设备发现和连接事件
          DeviceInfo tempDeviceInfo = *existingDevice;
          publishDeviceDiscovered(tempDeviceInfo);
          publishDeviceConnected(tempDeviceInfo);
        } else {
          // 创建新设备信息
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, deviceName, "sensor", 
                                                 driverName, DEVICE_STATUS_CONNECTED, "Connected");
          deviceInfos.push_back(deviceInfo);
          
          // 发布设备发现和连接事件
          publishDeviceDiscovered(deviceInfo);
          publishDeviceConnected(deviceInfo);
        }
      } else {
        updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
        publishDriverError(driverName, "Device scan failed", 2003);
      }
    }
    
    // 扫描显示设备（只支持EINK）
    for (auto driver : displayDrivers) {
      String driverName = "Eink_Driver";
      String deviceId = String(driver->getType());
      String deviceName = "EinkDisplay";
      
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      bool initResult = driver->init();
      
      if (initResult) {
        updateDriverStatus(driverName, DRIVER_STATUS_READY);
        
        DeviceInfo* existingDevice = getDeviceInfo(deviceId);
        
        if (existingDevice != nullptr) {
          // 更新现有设备信息
          updateDeviceStatus(deviceId, DEVICE_STATUS_CONNECTED);
          existingDevice->lastUpdateTime = millis();
          
          // 发布设备发现和连接事件
          DeviceInfo tempDeviceInfo = *existingDevice;
          publishDeviceDiscovered(tempDeviceInfo);
          publishDeviceConnected(tempDeviceInfo);
        } else {
          // 创建新设备信息
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, deviceName, "display", 
                                                 driverName, DEVICE_STATUS_CONNECTED, "Connected");
          deviceInfos.push_back(deviceInfo);
          
          // 发布设备发现和连接事件
          publishDeviceDiscovered(deviceInfo);
          publishDeviceConnected(deviceInfo);
        }
      } else {
        updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
        publishDriverError(driverName, "Eink device scan failed", 2004);
      }
    }
    
    // 扫描音频设备
    for (auto driver : audioDrivers) {
      String driverName = String(driver->getType());
      String deviceId = String(driver->getType());
      String deviceName = "AudioDevice";
      
      updateDriverStatus(driverName, DRIVER_STATUS_INITIALIZING);
      
      bool initResult = driver->init();
      
      if (initResult) {
        updateDriverStatus(driverName, DRIVER_STATUS_READY);
        
        DeviceInfo* existingDevice = getDeviceInfo(deviceId);
        
        if (existingDevice != nullptr) {
          // 更新现有设备信息
          updateDeviceStatus(deviceId, DEVICE_STATUS_CONNECTED);
          existingDevice->lastUpdateTime = millis();
          
          // 发布设备发现和连接事件
          DeviceInfo tempDeviceInfo = *existingDevice;
          publishDeviceDiscovered(tempDeviceInfo);
          publishDeviceConnected(tempDeviceInfo);
        } else {
          // 创建新设备信息
          DeviceInfo deviceInfo = createDeviceInfo(deviceId, deviceName, "audio", 
                                                 driverName, DEVICE_STATUS_CONNECTED, "Connected");
          deviceInfos.push_back(deviceInfo);
          
          // 发布设备发现和连接事件
          publishDeviceDiscovered(deviceInfo);
          publishDeviceConnected(deviceInfo);
        }
      } else {
        updateDriverStatus(driverName, DRIVER_STATUS_ERROR);
        publishDriverError(driverName, "Audio device scan failed", 2005);
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
  
  // 动态检测硬件变化
  bool detectHardwareChanges() {
    Serial.println("检测硬件变化...");
    
    bool hardwareChanged = false;
    
    // 记录当前设备状态
    std::map<String, bool> currentDevices;
    for (auto& device : deviceInfos) {
      currentDevices[device.deviceId] = true;
    }
    
    // 执行硬件匹配检测
    performHardwareMatch();
    
    // 检测新增设备
    for (auto& info : driverInfos) {
      if (info.status == DRIVER_STATUS_READY) {
        String deviceId = String(info.deviceId);
        if (currentDevices.find(deviceId) == currentDevices.end()) {
          // 新设备被检测到
          Serial.printf("检测到新设备: %s\n", info.name.c_str());
          hardwareChanged = true;
        }
      }
    }
    
    // 检测移除的设备
    for (auto& device : deviceInfos) {
      bool driverExists = false;
      for (auto& info : driverInfos) {
        if (info.name == device.driverName && info.status == DRIVER_STATUS_READY) {
          driverExists = true;
          break;
        }
      }
      
      if (!driverExists && device.status == DEVICE_STATUS_CONNECTED) {
        // 设备被移除
        Serial.printf("设备已移除: %s\n", device.deviceName.c_str());
        updateDeviceStatus(device.deviceId, DEVICE_STATUS_DISCONNECTED);
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
      String driverName = "Eink_Driver";
      auto driverData = std::make_shared<DriverEventData>(driverName, "display");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      delete driver;
    }
    displayDrivers.clear();
    
    for (auto driver : audioDrivers) {
      // 发布驱动卸载事件
      String driverName = String(driver->getType());
      auto driverData = std::make_shared<DriverEventData>(driverName, "audio");
      eventBus->publish(EVENT_DRIVER_UNREGISTERED, driverData);
      
      delete driver;
    }
    audioDrivers.clear();
    
    driverInfos.clear();
    deviceInfos.clear();
    
    Serial.println("All drivers cleared");
  }
  
  // 执行驱动硬件匹配检测
  bool performHardwareMatch() {
    Serial.println("Performing hardware match detection...");
    
    bool allMatched = true;
    
    // 检测传感器驱动
    Serial.println("Checking sensor drivers...");
    for (auto driver : sensorDrivers) {
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
    for (auto driver : displayDrivers) {
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
    for (auto driver : audioDrivers) {
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
      if (info.status == DRIVER_STATUS_READY && !info.enabled) {
        enableDriver(info.name);
      }
    }
  }
  
  // 禁用不支持的功能模块
  void disableIncompatibleModules() {
    Serial.println("Disabling incompatible modules...");
    
    // 禁用所有状态为ERROR的驱动
    for (auto& info : driverInfos) {
      if (info.status == DRIVER_STATUS_ERROR && info.enabled) {
        disableDriver(info.name);
      }
    }
    
    // 标记设备为不可用
    for (auto& device : deviceInfos) {
      bool driverFound = false;
      for (auto& info : driverInfos) {
        if (info.name == device.driverName && info.status == DRIVER_STATUS_READY) {
          driverFound = true;
          break;
        }
      }
      
      if (!driverFound) {
        updateDeviceStatus(device.deviceId, DEVICE_STATUS_ERROR);
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
      switch (info.status) {
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
      
      String enabledStr = info.enabled ? "✓" : "✗";
      Serial.printf("%s %s [%s] %s\n", enabledStr, statusStr.c_str(), info.type.c_str(), info.name.c_str());
    }
    
    Serial.println();
    
    // 打印设备状态
    Serial.println("Device Status:");
    for (auto& device : deviceInfos) {
      String statusStr;
      switch (device.status) {
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
      
      Serial.printf("%s [%s] %s\n", statusStr.c_str(), device.deviceType.c_str(), device.deviceName.c_str());
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
