#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H

#include <Arduino.h>
#include "event_bus.h"
#include "driver_registry.h"
#include "config.h"
#include "spiffs_manager.h"

// 核心系统状态枚举
enum CoreSystemState {
  SYSTEM_STATE_UNINITIALIZED,
  SYSTEM_STATE_INITIALIZING,
  SYSTEM_STATE_RUNNING,
  SYSTEM_STATE_LOW_POWER,
  SYSTEM_STATE_ERROR,
  SYSTEM_STATE_SHUTTING_DOWN
};

// 配置项结构体
typedef struct {
  String key;
  String value;
  String description;
  bool isReadOnly;
  unsigned long lastModified;
} ConfigItem;

// 定时器结构体
typedef struct {
  uint32_t timerId;
  unsigned long interval;
  unsigned long lastTriggerTime;
  bool enabled;
  bool isOneShot;
  std::function<void(uint32_t)> callback;
} TimerItem;

// 核心系统类，作为底层操作系统的核心组件
class CoreSystem {
private:
  static CoreSystem* instance;
  
  // 私有构造函数
  CoreSystem() {
    state = SYSTEM_STATE_UNINITIALIZED;
    eventBus = EventBus::getInstance();
    driverRegistry = DriverRegistry::getInstance();
    nextTimerId = 0;
    
    // 初始化电源管理
    batteryVoltage = 0.0;
    batteryPercentage = 0;
    isCharging = false;
    isLowPowerMode = false;
    lastPowerUpdate = 0;
    
    // 初始化配置管理
    configLoaded = false;
  }
  
  // 系统状态
  CoreSystemState state;
  
  // 核心组件指针
  EventBus* eventBus;
  DriverRegistry* driverRegistry;
  
  // 初始化时间
  unsigned long startTime;
  
  // 电源管理
  float batteryVoltage;
  int batteryPercentage;
  bool isCharging;
  bool isLowPowerMode;
  unsigned long lastPowerUpdate;
  
  // 配置管理
  bool configLoaded;
  std::vector<ConfigItem> configItems;
  
  // 定时器管理
  std::vector<TimerItem> timers;
  uint32_t nextTimerId;
  
  // 初始化SPIFFS文件系统
  bool initSPIFFS() {
    if (!isSPIFFSMounted()) {
      Serial.println("Initializing SPIFFS...");
      if (!::initSPIFFS()) {
        Serial.println("SPIFFS initialization failed");
        return false;
      }
    }
    Serial.println("SPIFFS initialized successfully");
    return true;
  }
  
  // 读取电池电压
  float readBatteryVoltage() {
    // 默认实现，返回模拟读取值
    #ifdef BATTERY_ADC_PIN
      int adcValue = analogRead(BATTERY_ADC_PIN);
      float voltage = adcValue * (BATTERY_MAX_VOLTAGE / 4096.0);
      return voltage;
    #else
      return 0.0;
    #endif
  }
  
  // 计算电池百分比
  int calculateBatteryPercentage(float voltage) {
    // 简单的线性映射，实际应该根据电池特性调整
    if (voltage <= 3.0) return 0;
    if (voltage >= 4.2) return 100;
    return (voltage - 3.0) * 100 / 1.2;
  }
  
  // 读取充电状态
  bool readChargingStatus() {
    #ifdef CHARGING_STATUS_PIN
      return digitalRead(CHARGING_STATUS_PIN) == HIGH;
    #else
      return false;
    #endif
  }
  
  // 处理定时器
  void processTimers() {
    unsigned long now = millis();
    
    for (auto it = timers.begin(); it != timers.end(); ) {
      if (it->enabled && (now - it->lastTriggerTime >= it->interval)) {
        // 触发定时器回调
        if (it->callback) {
          try {
            it->callback(it->timerId);
          } catch (const std::exception& e) {
            sendError("Timer callback exception", 3001, "CoreSystem");
          }
        }
        
        // 更新最后触发时间
        it->lastTriggerTime = now;
        
        // 如果是一次性定时器，禁用它
        if (it->isOneShot) {
          it->enabled = false;
          
          // 发布定时器到期事件
          auto timerData = std::make_shared<SystemErrorEventData>("Timer expired", 0, "Timer" + String(it->timerId));
          eventBus->publish(EVENT_TIMER_EXPIRED, timerData);
        }
      }
      
      // 移除已禁用的一次性定时器
      if (it->isOneShot && !it->enabled) {
        it = timers.erase(it);
      } else {
        ++it;
      }
    }
  }
  
  // 更新电源状态
  void updatePowerState() {
    unsigned long now = millis();
    if (now - lastPowerUpdate > 1000) { // 每秒更新一次
      lastPowerUpdate = now;
      
      // 读取电池电压
      batteryVoltage = readBatteryVoltage();
      batteryPercentage = calculateBatteryPercentage(batteryVoltage);
      isCharging = readChargingStatus();
      
      // 检查低电量
      if (batteryPercentage <= LOW_BATTERY_THRESHOLD && !isCharging) {
        auto lowPowerData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, true);
        eventBus->publish(EVENT_BATTERY_LOW, lowPowerData);
        
        // 进入低功耗模式
        if (!isLowPowerMode) {
          enterLowPowerMode();
        }
      } else if (batteryPercentage > LOW_BATTERY_THRESHOLD * 1.2 && isLowPowerMode) {
        auto powerOkData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, false);
        eventBus->publish(EVENT_BATTERY_OK, powerOkData);
        
        // 退出低功耗模式
        exitLowPowerMode();
      }
      
      // 发布电源状态变化事件
      auto powerData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, isLowPowerMode);
      eventBus->publish(EVENT_POWER_STATE_CHANGED, powerData);
    }
  }
  
  // 加载配置
  bool loadConfig() {
    // 这里应该从文件或其他存储加载配置
    configLoaded = true;
    return true;
  }
  
public:
  // 获取单例实例
  static CoreSystem* getInstance() {
    if (instance == nullptr) {
      instance = new CoreSystem();
    }
    return instance;
  }
  
  // 初始化核心系统
  bool init() {
    state = SYSTEM_STATE_INITIALIZING;
    startTime = millis();
    
    Serial.println("====================================");
    Serial.println("Initializing Core System...");
    Serial.println("====================================");
    
    // 1. 初始化SPIFFS文件系统
    if (!initSPIFFS()) {
      state = SYSTEM_STATE_ERROR;
      return false;
    }
    
    // 2. 加载配置
    Serial.println("Loading system configuration...");
    if (!loadConfig()) {
      Serial.println("Warning: Failed to load configuration, using defaults");
    }
    
    // 3. 初始化事件总线 - 事件总线通过单例模式自动初始化
    Serial.println("Initializing Event Bus...");
    
    // 4. 初始化驱动注册表
    Serial.println("Initializing Driver Registry...");
    driverRegistry->init();
    
    // 5. 扫描设备
    Serial.println("Scanning for devices...");
    driverRegistry->scanDevices();
    
    // 6. 初始化电源管理
    Serial.println("Initializing Power Management...");
    updatePowerState();
    
    // 7. 发布系统启动事件
    eventBus->publish(EVENT_SYSTEM_STARTUP, nullptr);
    
    state = SYSTEM_STATE_RUNNING;
    
    Serial.println("====================================");
    Serial.println("Core System initialized successfully");
    Serial.printf("Boot time: %lu ms\n", millis() - startTime);
    Serial.println("====================================");
    
    return true;
  }
  
  // 运行核心系统
  void run() {
    if (state != SYSTEM_STATE_RUNNING && state != SYSTEM_STATE_LOW_POWER) {
      return;
    }
    
    // 处理事件总线消息（如果需要）
    // 事件总线采用发布-订阅模式，不需要主动轮询
    
    // 运行驱动注册中心的循环
    driverRegistry->loop();
    
    // 处理定时器
    processTimers();
    
    // 更新电源状态
    updatePowerState();
  }
  
  // 进入低功耗模式
  void enterLowPowerMode() {
    if (state == SYSTEM_STATE_RUNNING) {
      isLowPowerMode = true;
      state = SYSTEM_STATE_LOW_POWER;
      
      // 发布低功耗进入事件
      eventBus->publish(EVENT_LOW_POWER_ENTER, nullptr);
      
      Serial.println("Entering low power mode");
    }
  }
  
  // 退出低功耗模式
  void exitLowPowerMode() {
    if (state == SYSTEM_STATE_LOW_POWER) {
      isLowPowerMode = false;
      state = SYSTEM_STATE_RUNNING;
      
      // 发布低功耗退出事件
      eventBus->publish(EVENT_LOW_POWER_EXIT, nullptr);
      
      Serial.println("Exiting low power mode");
    }
  }
  
  // 关闭系统
  void shutdown() {
    state = SYSTEM_STATE_SHUTTING_DOWN;
    eventBus->publish(EVENT_SYSTEM_SHUTDOWN, nullptr);
    
    // 清理资源
    driverRegistry->clear();
    timers.clear();
    
    Serial.println("System shutting down");
    state = SYSTEM_STATE_UNINITIALIZED;
  }
  
  // 重置系统
  void reset() {
    eventBus->publish(EVENT_SYSTEM_RESET, nullptr);
    #ifdef ESP32
      ESP.restart();
    #else
      // 其他平台的重置实现
      Serial.println("System reset not implemented for this platform");
    #endif
  }
  
  // 获取系统状态
  CoreSystemState getState() const {
    return state;
  }
  
  // 获取事件总线
  EventBus* getEventBus() {
    return eventBus;
  }
  
  // 获取驱动注册表
  DriverRegistry* getDriverRegistry() {
    return driverRegistry;
  }
  
  // 获取系统运行时间
  unsigned long getUptime() const {
    return millis() - startTime;
  }
  
  // 发送系统错误
  void sendError(const String& message, int errorCode, const String& module) {
    auto errorData = std::make_shared<SystemErrorEventData>(message, errorCode, module);
    eventBus->publish(EVENT_SYSTEM_ERROR, errorData);
    
    Serial.printf("System Error [%s]: %s (Code: %d)\n", module.c_str(), message.c_str(), errorCode);
  }
  
  // 电源管理API
  float getBatteryVoltage() const {
    return batteryVoltage;
  }
  
  int getBatteryPercentage() const {
    return batteryPercentage;
  }
  
  bool isChargingState() const {
    return isCharging;
  }
  
  bool isInLowPowerMode() const {
    return isLowPowerMode;
  }
  
  // 配置管理API
  String getConfig(const String& key, const String& defaultValue = "") {
    for (const auto& item : configItems) {
      if (item.key == key) {
        return item.value;
      }
    }
    return defaultValue;
  }
  
  bool setConfig(const String& key, const String& value) {
    for (auto& item : configItems) {
      if (item.key == key) {
        if (item.isReadOnly) {
          return false;
        }
        item.value = value;
        item.lastModified = millis();
        
        // 发布配置更新事件
        auto configData = std::make_shared<ConfigEventData>(key, value);
        eventBus->publish(EVENT_CONFIG_UPDATED, configData);
        
        return true;
      }
    }
    
    // 添加新配置项
    ConfigItem newItem;
    newItem.key = key;
    newItem.value = value;
    newItem.description = "";
    newItem.isReadOnly = false;
    newItem.lastModified = millis();
    configItems.push_back(newItem);
    
    // 发布配置更新事件
    auto configData = std::make_shared<ConfigEventData>(key, value);
    eventBus->publish(EVENT_CONFIG_UPDATED, configData);
    
    return true;
  }
  
  bool saveConfig() {
    // 保存配置到文件
    auto configData = std::make_shared<SystemErrorEventData>("Configuration saved", 0, "CoreSystem");
    eventBus->publish(EVENT_CONFIG_SAVED, configData);
    return true;
  }
  
  bool resetConfig() {
    configItems.clear();
    configLoaded = false;
    loadConfig();
    
    // 发布配置重置事件
    eventBus->publish(EVENT_CONFIG_RESET, nullptr);
    return true;
  }
  
  // 定时器管理API
  uint32_t createTimer(unsigned long interval, std::function<void(uint32_t)> callback, bool isOneShot = false) {
    TimerItem timer;
    timer.timerId = nextTimerId++;
    timer.interval = interval;
    timer.lastTriggerTime = millis();
    timer.enabled = true;
    timer.isOneShot = isOneShot;
    timer.callback = callback;
    
    timers.push_back(timer);
    return timer.timerId;
  }
  
  bool startTimer(uint32_t timerId) {
    for (auto& timer : timers) {
      if (timer.timerId == timerId) {
        timer.enabled = true;
        timer.lastTriggerTime = millis();
        return true;
      }
    }
    return false;
  }
  
  bool stopTimer(uint32_t timerId) {
    for (auto& timer : timers) {
      if (timer.timerId == timerId) {
        timer.enabled = false;
        return true;
      }
    }
    return false;
  }
  
  bool deleteTimer(uint32_t timerId) {
    for (auto it = timers.begin(); it != timers.end(); ++it) {
      if (it->timerId == timerId) {
        timers.erase(it);
        return true;
      }
    }
    return false;
  }
  
  bool isTimerRunning(uint32_t timerId) {
    for (const auto& timer : timers) {
      if (timer.timerId == timerId) {
        return timer.enabled;
      }
    }
    return false;
  }
  
  // 设置定时器间隔
  bool setTimerInterval(uint32_t timerId, unsigned long interval) {
    for (auto& timer : timers) {
      if (timer.timerId == timerId) {
        timer.interval = interval;
        return true;
      }
    }
    return false;
  }
  
  // 获取系统内存信息
  void getMemoryInfo(size_t& freeHeap, size_t& minimumFreeHeap) {
    #ifdef ESP32
      freeHeap = ESP.getFreeHeap();
      minimumFreeHeap = ESP.getMinFreeHeap();
    #else
      // 其他平台的内存信息实现
      freeHeap = 0;
      minimumFreeHeap = 0;
      Serial.println("Memory info not implemented for this platform");
    #endif
  }
  
  // 获取系统CPU频率
  uint32_t getCpuFrequencyMhz() {
    #ifdef ESP32
      return ESP.getCpuFreqMHz();
    #else
      // 其他平台的CPU频率实现
      Serial.println("CPU frequency not implemented for this platform");
      return 0;
    #endif
  }
  
  // 获取芯片ID
  uint32_t getChipId() {
    #ifdef ESP32
      return ESP.getChipId();
    #else
      // 其他平台的芯片ID实现
      Serial.println("Chip ID not implemented for this platform");
      return 0;
    #endif
  }
  
  // 获取Flash大小
  uint32_t getFlashChipSize() {
    #ifdef ESP32
      return ESP.getFlashChipSize();
    #else
      // 其他平台的Flash大小实现
      Serial.println("Flash size not implemented for this platform");
      return 0;
    #endif
  }
  
  // 析构函数
  ~CoreSystem() {
    // 清理资源
    timers.clear();
    configItems.clear();
  }
};

// 初始化单例实例
CoreSystem* CoreSystem::instance = nullptr;

#endif // CORE_SYSTEM_H