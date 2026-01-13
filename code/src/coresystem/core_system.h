#ifndef CORE_SYSTEM_H
#define CORE_SYSTEM_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "arduino_compat.h"
#endif
#include "icore_system.h" // 包含ICoreSystem接口和CoreSystemState枚举
#include "event_bus.h"
#include "driver_registry.h"
#include "config.h"
#include "spiffs_manager.h"
#include "platform_abstraction.h"

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
class CoreSystem : public ICoreSystem {
private:
  static CoreSystem* instance;
  
  // 私有构造函数
  CoreSystem() {    
    // 初始化系统状态
    state = SYSTEM_STATE_UNINITIALIZED;
    
    // 初始化核心组件指针为nullptr
    eventBus = nullptr;
    driverRegistry = nullptr;
    systemMutex = nullptr;
    
    // 初始化电源管理
    batteryVoltage = 0.0;
    batteryPercentage = 0;
    isCharging = false;
    isLowPowerMode = false;
    lastPowerUpdate = 0;
    
    // 初始化配置管理
    configLoaded = false;
    
    // 初始化内存管理
    totalAllocatedMemory = 0;
    peakAllocatedMemory = 0;
    lastMemoryUpdate = 0;
    
    // 初始化运算资源管理
    currentCpuFreqMHz = platformGetCpuFreqMHz();
    minCpuFreqMHz = 80;    // 默认最小CPU频率80MHz
    maxCpuFreqMHz = 240;   // 默认最大CPU频率240MHz
    dynamicCpuFreqEnabled = true;
    
    // 初始化任务优先级管理
    defaultTaskPriority = 5; // 默认任务优先级5
    
    // 初始化线程管理
    systemMutex = xSemaphoreCreateMutex();
    if (systemMutex == NULL) {
      Serial.println("Failed to create system mutex");
      // 在构造函数中无法返回错误，所以我们设置一个标志
      state = SYSTEM_STATE_ERROR;
    }
    
    // 初始化核心组件
    try {
      eventBus = EventBus::getInstance();
      driverRegistry = DriverRegistry::getInstance();
    } catch (const std::exception& e) {
      Serial.printf("Failed to initialize core components: %s\n", e.what());
      state = SYSTEM_STATE_ERROR;
    }
    
    nextTimerId = 0;
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
  
  // 运算资源管理
  int currentCpuFreqMHz;
  int minCpuFreqMHz;
  int maxCpuFreqMHz;
  bool dynamicCpuFreqEnabled;
  
  // 任务优先级管理
  std::map<uint32_t, int> taskPriorities;
  int defaultTaskPriority;
  
  // 线程管理
  std::vector<uint32_t> activeThreads;
  std::map<uint32_t, SemaphoreHandle_t> threadMutexes;
  SemaphoreHandle_t systemMutex;  // 系统级互斥锁
  
  // 配置管理
  bool configLoaded;
  std::vector<ConfigItem> configItems;
  
  // 定时器管理
  std::vector<TimerItem> timers;
  uint32_t nextTimerId;
  
  // 内存管理
  struct MemoryPool {
    void* pool;          // 内存池指针
    size_t blockSize;    // 块大小
    size_t blockCount;   // 块数量
    size_t freeBlocks;   // 空闲块数量
    void** freeList;     // 空闲块链表
  };
  
  std::vector<MemoryPool> memoryPools;  // 内存池列表
  size_t totalAllocatedMemory;           // 总分配内存大小
  size_t peakAllocatedMemory;            // 峰值分配内存大小
  unsigned long lastMemoryUpdate;        // 上次内存更新时间
  
  // 资源监控和性能分析
  struct SystemStats {
    unsigned long uptime;                // 系统运行时间
    size_t freeHeap;                     // 可用堆内存
    size_t minFreeHeap;                  // 最小堆内存
    size_t usedMemory;                   // 已用内存
    size_t peakMemory;                   // 峰值内存使用
    uint32_t cpuFreqMHz;                 // CPU频率
    float batteryVoltage;                // 电池电压
    int batteryPercentage;               // 电池百分比
    bool isCharging;                     // 充电状态
    bool isLowPowerMode;                 // 低功耗模式
    size_t activeThreads;                // 活动线程数
    size_t activeTimers;                 // 活动定时器数
    unsigned long lastStatsUpdate;       // 上次统计更新时间
  };
  
  SystemStats systemStats;               // 系统统计信息
  
  // 性能分析
  struct PerformanceMetric {
    String name;                         // 指标名称
    unsigned long totalTime;             // 总执行时间
    unsigned long count;                 // 执行次数
    unsigned long maxTime;               // 最大执行时间
    unsigned long minTime;               // 最小执行时间
    unsigned long lastTime;              // 上次执行时间
  };
  
  std::map<String, PerformanceMetric> performanceMetrics;  // 性能指标
  
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
    // 检查构造函数是否成功
    if (state == SYSTEM_STATE_ERROR) {
      Serial.println("CoreSystem constructor failed, cannot initialize");
      return false;
    }
    
    // 检查核心组件是否初始化成功
    if (eventBus == nullptr || driverRegistry == nullptr) {
      Serial.println("Core components not initialized, cannot initialize");
      state = SYSTEM_STATE_ERROR;
      return false;
    }
    
    state = SYSTEM_STATE_INITIALIZING;
    startTime = millis();
    
    Serial.println("====================================");
    Serial.println("Initializing Core System...");
    Serial.println("====================================");
    
    // 1. 初始化SPIFFS文件系统
    if (!initSPIFFS()) {
      Serial.println("SPIFFS initialization failed, continuing with limited functionality");
      // 不返回错误，因为SPIFFS可能不是所有设备都需要
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
    try {
      driverRegistry->init();
    } catch (const std::exception& e) {
      Serial.printf("Driver Registry initialization failed: %s\n", e.what());
      state = SYSTEM_STATE_ERROR;
      return false;
    }
    
    // 5. 扫描设备
    Serial.println("Scanning for devices...");
    try {
      driverRegistry->scanDevices();
    } catch (const std::exception& e) {
      Serial.printf("Device scanning failed: %s\n", e.what());
      // 不返回错误，继续运行
    }
    
    // 6. 系统自检：驱动与硬件匹配检测
    Serial.println("====================================");
    Serial.println("Performing System Self-Check...");
    Serial.println("====================================");
    
    // 6.1 执行驱动硬件匹配检测
    bool hardwareMatch = false;
    try {
      hardwareMatch = driverRegistry->performHardwareMatch();
      if (!hardwareMatch) {
        Serial.println("Warning: Some drivers do not match hardware");
      }
    } catch (const std::exception& e) {
      Serial.printf("Hardware match detection failed: %s\n", e.what());
    }
    
    // 6.2 根据检测结果启用/禁用功能模块
    try {
      driverRegistry->enableCompatibleModules();
      
      // 6.3 禁用不支持的功能模块
      driverRegistry->disableIncompatibleModules();
      
      // 6.4 打印自检结果
      driverRegistry->printSelfCheckResult();
    } catch (const std::exception& e) {
      Serial.printf("Module configuration failed: %s\n", e.what());
      // 不返回错误，继续运行
    }
    
    // 7. 初始化电源管理
    Serial.println("Initializing Power Management...");
    try {
      updatePowerState();
    } catch (const std::exception& e) {
      Serial.printf("Power management initialization failed: %s\n", e.what());
      // 不返回错误，继续运行
    }
    
    // 8. 发布系统启动事件
    try {
      eventBus->publish(EVENT_SYSTEM_STARTUP, nullptr);
    } catch (const std::exception& e) {
      Serial.printf("Failed to publish system startup event: %s\n", e.what());
      // 不返回错误，继续运行
    }
    
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
    
    // 检查核心组件是否可用
    if (eventBus == nullptr || driverRegistry == nullptr) {
      Serial.println("Core components not available, cannot run");
      state = SYSTEM_STATE_ERROR;
      return;
    }
    
    // 运行驱动注册中心的循环
    try {
      driverRegistry->loop();
    } catch (const std::exception& e) {
      sendError("Driver Registry loop failed", 2001, "CoreSystem");
      // 继续运行，因为驱动注册中心可能只是部分功能失效
    }
    
    // 处理定时器
    try {
      processTimers();
    } catch (const std::exception& e) {
      sendError("Timer processing failed", 2002, "CoreSystem");
      // 继续运行，因为定时器可能只是部分功能失效
    }
    
    // 更新电源状态
    try {
      updatePowerState();
    } catch (const std::exception& e) {
      sendError("Power state update failed", 2003, "CoreSystem");
      // 继续运行，因为电源状态更新可能只是部分功能失效
    }
    
    // 动态调整CPU频率
    try {
      if (dynamicCpuFreqEnabled) {
        adjustCpuFreqBasedOnLoad();
      }
    } catch (const std::exception& e) {
      sendError("CPU frequency adjustment failed", 2004, "CoreSystem");
      // 继续运行，因为CPU频率调整可能只是部分功能失效
    }
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
    platformReset();
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
    freeHeap = platformGetFreeHeap();
    minimumFreeHeap = platformGetMinFreeHeap();
  }
  
  // 获取系统CPU频率
  uint32_t getCpuFrequencyMhz() {
    return platformGetCpuFreqMHz();
  }
  
  // 设置CPU频率
  bool setCpuFrequencyMhz(uint32_t freqMHz) {
    if (freqMHz >= minCpuFreqMHz && freqMHz <= maxCpuFreqMHz) {
      if (platformSetCpuFreqMHz(freqMHz)) {
        currentCpuFreqMHz = freqMHz;
        return true;
      }
    }
    return false;
  }
  
  // 启用动态CPU频率调整
  void enableDynamicCpuFreq(bool enable) {
    dynamicCpuFreqEnabled = enable;
  }
  
  // 设置CPU频率范围
  void setCpuFreqRange(int minFreq, int maxFreq) {
    minCpuFreqMHz = minFreq;
    maxCpuFreqMHz = maxFreq;
    
    // 确保当前频率在范围内
    if (currentCpuFreqMHz < minFreq) {
      setCpuFrequencyMhz(minFreq);
    } else if (currentCpuFreqMHz > maxFreq) {
      setCpuFrequencyMhz(maxFreq);
    }
  }
  
  // 动态调整CPU频率
  void adjustCpuFreqBasedOnLoad() {
    if (!dynamicCpuFreqEnabled) {
      return;
    }
    
    // 根据系统负载调整CPU频率
    // 这里使用简单的算法，实际应用中可以更复杂
    size_t freeHeap = platformGetFreeHeap();
    size_t minFreeHeap = platformGetMinFreeHeap();
    
    // 如果内存使用率高，提高CPU频率以加快处理速度
    if (freeHeap < minFreeHeap * 2) {
      setCpuFrequencyMhz(maxCpuFreqMHz);
    } 
    // 如果内存充足且系统处于低功耗模式，降低CPU频率
    else if (isLowPowerMode) {
      setCpuFrequencyMhz(minCpuFreqMHz);
    }
    // 否则使用中等频率
    else {
      setCpuFrequencyMhz((minCpuFreqMHz + maxCpuFreqMHz) / 2);
    }
  }
  
  // 设置任务优先级
  void setTaskPriority(uint32_t taskId, int priority) {
    // 优先级范围：1-10，1最低，10最高
    if (priority >= 1 && priority <= 10) {
      taskPriorities[taskId] = priority;
    }
  }
  
  // 获取任务优先级
  int getTaskPriority(uint32_t taskId) {
    auto it = taskPriorities.find(taskId);
    if (it != taskPriorities.end()) {
      return it->second;
    }
    return defaultTaskPriority;
  }
  
  // 设置默认任务优先级
  void setDefaultTaskPriority(int priority) {
    if (priority >= 1 && priority <= 10) {
      defaultTaskPriority = priority;
    }
  }
  
  // 线程管理方法
  
  // 创建线程互斥锁
  SemaphoreHandle_t createMutex() {
    SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
    if (mutex != NULL) {
      threadMutexes[(uint32_t)mutex] = mutex;
    }
    return mutex;
  }
  
  // 锁定互斥锁
  bool lockMutex(SemaphoreHandle_t mutex, uint32_t timeoutMs = portMAX_DELAY) {
    if (mutex != NULL) {
      return xSemaphoreTake(mutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
    }
    return false;
  }
  
  // 解锁互斥锁
  bool unlockMutex(SemaphoreHandle_t mutex) {
    if (mutex != NULL) {
      return xSemaphoreGive(mutex) == pdTRUE;
    }
    return false;
  }
  
  // 销毁互斥锁
  void destroyMutex(SemaphoreHandle_t mutex) {
    if (mutex != NULL) {
      auto it = threadMutexes.find((uint32_t)mutex);
      if (it != threadMutexes.end()) {
        threadMutexes.erase(it);
        vSemaphoreDelete(mutex);
      }
    }
  }
  
  // 获取系统级互斥锁
  SemaphoreHandle_t getSystemMutex() {
    return systemMutex;
  }
  
  // 添加活动线程
  void addActiveThread(uint32_t threadId) {
    activeThreads.push_back(threadId);
  }
  
  // 移除活动线程
  void removeActiveThread(uint32_t threadId) {
    auto it = std::find(activeThreads.begin(), activeThreads.end(), threadId);
    if (it != activeThreads.end()) {
      activeThreads.erase(it);
    }
  }
  
  // 获取活动线程数量
  size_t getActiveThreadCount() {
    return activeThreads.size();
  }
  
  // 功耗控制增强
  
  // 进入深度睡眠模式
  void enterDeepSleep(uint64_t sleepTimeMs) {
    // 发布深度睡眠事件
    eventBus->publish(EVENT_SYSTEM_DEEP_SLEEP, nullptr);
    
    // 清理资源
    cleanupMemory();
    
    // 进入深度睡眠
    platformDeepSleep(sleepTimeMs);
  }
  
  // 进入轻度睡眠模式
  void enterLightSleep(uint64_t sleepTimeMs) {
    // 发布轻度睡眠事件
    eventBus->publish(EVENT_SYSTEM_LIGHT_SLEEP, nullptr);
    
    // 进入轻度睡眠
    platformLightSleep(sleepTimeMs);
    
    // 唤醒后恢复
    eventBus->publish(EVENT_SYSTEM_WAKEUP, nullptr);
  }
  
  // 设置低功耗模式
  void setLowPowerMode(bool enable) {
    if (isLowPowerMode != enable) {
      isLowPowerMode = enable;
      
      if (enable) {
        // 进入低功耗模式
        eventBus->publish(EVENT_SYSTEM_LOW_POWER, nullptr);
        
        // 降低CPU频率
        setCpuFrequencyMhz(minCpuFreqMHz);
        
        // 关闭不必要的外设
        // 这里可以添加具体的外设关闭逻辑
      } else {
        // 退出低功耗模式
        eventBus->publish(EVENT_SYSTEM_NORMAL_POWER, nullptr);
        
        // 恢复CPU频率
        adjustCpuFreqBasedOnLoad();
        
        // 重新初始化必要的外设
        // 这里可以添加具体的外设初始化逻辑
      }
    }
  }
  
  // 优化功耗的周期性任务
  void optimizePowerConsumption() {
    // 根据电池电量调整功耗策略
    if (batteryPercentage <= CRITICAL_BATTERY_THRESHOLD) {
      // 电量极低时，进入深度低功耗模式
      setLowPowerMode(true);
      // 延长传感器读取间隔
      // 减少WiFi/BLE活动
      // 降低CPU频率
      setCpuFrequencyMhz(minCpuFreqMHz);
    } else if (batteryPercentage < 20) {
      // 电量低于20%，进入中度低功耗模式
      setLowPowerMode(true);
      // 延长传感器读取间隔
      // 减少WiFi/BLE活动
      setCpuFrequencyMhz((minCpuFreqMHz + maxCpuFreqMHz) / 3);
    } else if (batteryPercentage < 50) {
      // 电量低于50%，进入轻度低功耗模式
      setLowPowerMode(true);
      setCpuFrequencyMhz((minCpuFreqMHz + maxCpuFreqMHz) / 2);
    } else {
      // 电量充足，使用正常模式
      setLowPowerMode(false);
      // 动态调整CPU频率
      adjustCpuFreqBasedOnLoad();
    }
    
    // 动态调整CPU频率
    if (dynamicCpuFreqEnabled && !isLowPowerMode) {
      adjustCpuFreqBasedOnLoad();
    }
    
    // 清理内存，减少内存占用
    cleanupMemory();
    
    // 根据当前状态调整系统任务优先级
    if (isLowPowerMode) {
      // 低功耗模式下，降低非关键任务的优先级
      defaultTaskPriority = 3;
    } else {
      // 正常模式下，恢复正常优先级
      defaultTaskPriority = 5;
    }
  }
  
  // 获取芯片ID
  uint32_t getChipId() {
    return platformGetChipId();
  }
  
  // 获取Flash大小
  uint32_t getFlashChipSize() {
    return platformGetFlashChipSize();
  }
  
  // 获取固件大小（已使用的Flash空间）
  uint32_t getFirmwareSize() {
    return platformGetFirmwareSize();
  }
  
  // 获取可用Flash空间
  uint32_t getFreeFlashSize() {
    return platformGetFreeFlashSize();
  }
  
  // 获取Flash使用情况
  void getFlashInfo(uint32_t& totalSize, uint32_t& firmwareSize, uint32_t& freeSize) {
    platformGetFlashInfo(totalSize, firmwareSize, freeSize);
  }
  
  // 系统统计信息
  void updateSystemStats();
  const SystemStats& getSystemStats();
  
  // 性能分析
  void startPerformanceMeasurement(const String& name);
  void endPerformanceMeasurement(const String& name);
  String getPerformanceStats();
  void resetPerformanceStats();
  
  // 内存管理方法
  
  // 创建内存池
  void* createMemoryPool(size_t blockSize, size_t blockCount) {
    MemoryPool pool;
    pool.blockSize = blockSize;
    pool.blockCount = blockCount;
    pool.freeBlocks = blockCount;
    
    // 分配内存池
    pool.pool = malloc(blockSize * blockCount);
    if (pool.pool == nullptr) {
      return nullptr;
    }
    
    // 初始化空闲块链表
    pool.freeList = (void**)malloc(sizeof(void*) * blockCount);
    if (pool.freeList == nullptr) {
      free(pool.pool);
      return nullptr;
    }
    
    // 填充空闲块链表
    for (size_t i = 0; i < blockCount; i++) {
      pool.freeList[i] = (uint8_t*)pool.pool + (i * blockSize);
    }
    
    memoryPools.push_back(pool);
    totalAllocatedMemory += blockSize * blockCount + sizeof(void*) * blockCount;
    if (totalAllocatedMemory > peakAllocatedMemory) {
      peakAllocatedMemory = totalAllocatedMemory;
    }
    
    return pool.pool;
  }
  
  // 从内存池分配内存
  void* allocateFromPool(void* poolPtr, size_t size) {
    for (auto& pool : memoryPools) {
      if (pool.pool == poolPtr && size <= pool.blockSize && pool.freeBlocks > 0) {
        void* ptr = pool.freeList[--pool.freeBlocks];
        memset(ptr, 0, pool.blockSize);
        return ptr;
      }
    }
    return nullptr;
  }
  
  // 释放内存回内存池
  void freeToPool(void* poolPtr, void* ptr) {
    for (auto& pool : memoryPools) {
      if (pool.pool == poolPtr) {
        // 检查ptr是否属于该内存池
        if (ptr >= pool.pool && ptr < (uint8_t*)pool.pool + (pool.blockSize * pool.blockCount)) {
          pool.freeList[pool.freeBlocks++] = ptr;
          return;
        }
        break;
      }
    }
  }
  
  // 销毁内存池
  void destroyMemoryPool(void* poolPtr) {
    for (auto it = memoryPools.begin(); it != memoryPools.end(); ++it) {
      if (it->pool == poolPtr) {
        totalAllocatedMemory -= it->blockSize * it->blockCount + sizeof(void*) * it->blockCount;
        free(it->freeList);
        free(it->pool);
        memoryPools.erase(it);
        return;
      }
    }
  }
  
  // 获取内存池使用情况
  void getMemoryPoolInfo(void* poolPtr, size_t& totalBlocks, size_t& freeBlocks) {
    for (auto& pool : memoryPools) {
      if (pool.pool == poolPtr) {
        totalBlocks = pool.blockCount;
        freeBlocks = pool.freeBlocks;
        return;
      }
    }
    totalBlocks = 0;
    freeBlocks = 0;
  }
  
  // 执行内存清理
  void cleanupMemory() {
    // 清理定时器
    auto it = timers.begin();
    while (it != timers.end()) {
      if (!it->enabled && it->isOneShot) {
        it = timers.erase(it);
      } else {
        ++it;
      }
    }
    
    // 检查并清理内存泄漏
    checkMemoryLeaks();
    
    // 更新内存使用统计
    updateMemoryStats();
  }
  
  // 检查内存泄漏
  void checkMemoryLeaks() {
    // 简单的内存泄漏检查，实际应用中可以更复杂
    size_t currentHeap = platformGetFreeHeap();
    if (lastMemoryUpdate > 0) {
      // 检查内存是否持续增长
      static size_t previousHeap = currentHeap;
      static int leakCounter = 0;
      
      if (currentHeap < previousHeap - 1024) { // 如果内存减少超过1KB
        leakCounter++;
        if (leakCounter > 10) { // 连续10次检测到内存泄漏
          sendError("Potential memory leak detected", 4001, "CoreSystem");
          leakCounter = 0;
        }
      } else {
        leakCounter = 0;
      }
      previousHeap = currentHeap;
    }
  }
  
  // 更新内存统计信息
  void updateMemoryStats() {
    lastMemoryUpdate = millis();
    
    // 可以在这里添加更详细的内存统计
  }
  
  // 获取内存使用统计
  void getMemoryStats(size_t& totalMemory, size_t& usedMemory, size_t& peakMemory) {
    size_t freeHeap = platformGetFreeHeap();
    size_t totalHeap = platformGetFlashChipSize(); // 近似值，实际应该使用芯片总RAM
    
    totalMemory = totalHeap;
    usedMemory = totalHeap - freeHeap + totalAllocatedMemory;
    peakMemory = peakAllocatedMemory;
  }
  
  // 析构函数
  ~CoreSystem() {
    // 清理资源
    timers.clear();
    configItems.clear();
    
    // 清理事件总线订阅
    if (eventBus != nullptr) {
      eventBus->unsubscribeAll(this);
    }
    
    // 清理驱动注册表
    if (driverRegistry != nullptr) {
      driverRegistry->clear();
    }
    
    // 释放内存池资源
    for (auto& pool : memoryPools) {
      free(pool.freeList);
      free(pool.pool);
    }
    memoryPools.clear();
    
    // 清理线程资源
    for (auto& mutex : threadMutexes) {
      vSemaphoreDelete(mutex.second);
    }
    threadMutexes.clear();
    
    if (systemMutex != NULL) {
      vSemaphoreDelete(systemMutex);
      systemMutex = NULL;
    }
    
    activeThreads.clear();
    taskPriorities.clear();
    
    state = SYSTEM_STATE_UNINITIALIZED;
  }
};

// 初始化单例实例
CoreSystem* CoreSystem::instance = nullptr;

#endif // CORE_SYSTEM_H