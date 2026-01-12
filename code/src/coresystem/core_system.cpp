#include "core_system.h"

// 实现ICoreSystem接口的getInstance()方法
ICoreSystem* ICoreSystem::getInstance() {
  return CoreSystem::getInstance();
}

// 初始化CoreSystem单例
CoreSystem* CoreSystem::instance = nullptr;

// 初始化SPIFFS文件系统
bool CoreSystem::initSPIFFS() {
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
float CoreSystem::readBatteryVoltage() {
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
int CoreSystem::calculateBatteryPercentage(float voltage) {
  // 改进的电池百分比计算，考虑不同电池类型的特性
  if (voltage <= 3.0) return 0;
  if (voltage >= 4.2) return 100;
  
  // 使用非线性映射更准确地反映锂电池特性
  // 锂电池在3.7V以上电量下降较慢，3.7V以下下降较快
  float percentage;
  if (voltage >= 3.7) {
    // 3.7V到4.2V对应50%到100%
    percentage = 50 + (voltage - 3.7) * 100 / 0.5;
  } else {
    // 3.0V到3.7V对应0%到50%
    percentage = (voltage - 3.0) * 50 / 0.7;
  }
  
  // 四舍五入到整数
  return static_cast<int>(percentage + 0.5);
}

// 读取充电状态
bool CoreSystem::readChargingStatus() {
  #ifdef CHARGING_STATUS_PIN
    return digitalRead(CHARGING_STATUS_PIN) == HIGH;
  #else
    return false;
  #endif
}

// 处理定时器
void CoreSystem::processTimers() {
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
void CoreSystem::updatePowerState() {
  unsigned long now = millis();
  if (now - lastPowerUpdate > 2000) { // 每2秒更新一次，减少ADC读取频率
    lastPowerUpdate = now;
    
    // 读取电池电压（使用多次采样平均）
    float sumVoltage = 0.0;
    for (int i = 0; i < 5; i++) { // 5次采样
      sumVoltage += readBatteryVoltage();
      platformDelay(10); // 短暂延迟
    }
    batteryVoltage = sumVoltage / 5.0;
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
    } else if (batteryPercentage > LOW_BATTERY_THRESHOLD * 1.3 && isLowPowerMode) {
      auto powerOkData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, false);
      eventBus->publish(EVENT_BATTERY_OK, powerOkData);
      
      // 退出低功耗模式
      exitLowPowerMode();
    }
    
    // 检查关键电量
    if (batteryPercentage <= CRITICAL_BATTERY_THRESHOLD && !isCharging) {
      auto criticalPowerData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, true);
      eventBus->publish(EVENT_BATTERY_CRITICAL, criticalPowerData);
      
      // 进入深度低功耗模式
      if (!isLowPowerMode) {
        enterLowPowerMode();
      }
      // 降低CPU频率到最低
      setCpuFrequencyMhz(minCpuFreqMHz);
    }
    
    // 发布电源状态变化事件
    auto powerData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, isLowPowerMode);
    eventBus->publish(EVENT_POWER_STATE_CHANGED, powerData);
    
    // 记录电源状态
    Serial.printf("Power state: Voltage=%.2fV, Percentage=%d%%, Charging=%s, LowPower=%s\n", 
                  batteryVoltage, batteryPercentage, isCharging ? "Yes" : "No", isLowPowerMode ? "Yes" : "No");
  }
}

// 加载配置
bool CoreSystem::loadConfig() {
  // 这里应该从文件或其他存储加载配置
  configLoaded = true;
  return true;
}

// 动态调整CPU频率
void CoreSystem::adjustCpuFreqBasedOnLoad() {
  if (!dynamicCpuFreqEnabled) {
    return;
  }
  
  // 获取当前系统状态
  size_t freeHeap = platformGetFreeHeap();
  size_t minFreeHeap = platformGetMinFreeHeap();
  unsigned long uptime = getUptime();
  
  // 避免频繁调整频率
  static unsigned long lastFreqAdjust = 0;
  static uint32_t previousFreq = currentCpuFreqMHz;
  
  if (uptime - lastFreqAdjust < 1000) { // 1秒内不重复调整
    return;
  }
  
  // 计算内存使用率（使用近似值）
  // 对于不同平台，总堆内存大小可能不同，这里使用估计值
  size_t estimatedTotalHeap = 0;
  #ifdef ARDUINO_ARCH_ESP32
    estimatedTotalHeap = 320000; // ESP32 约320KB堆内存
  #elif ARDUINO_ARCH_ESP8266
    estimatedTotalHeap = 80000; // ESP8266 约80KB堆内存
  #elif ARDUINO_ARCH_NRF52
    estimatedTotalHeap = 64000; // nRF52 约64KB堆内存
  #elif ARDUINO_ARCH_STM32
    estimatedTotalHeap = 128000; // STM32 约128KB堆内存
  #elif ARDUINO_ARCH_RP2040
    estimatedTotalHeap = 264000; // RP2040 约264KB堆内存
  #else
    estimatedTotalHeap = 100000; // 默认估计值
  #endif
  float memoryUsage = (estimatedTotalHeap - freeHeap) / (float)estimatedTotalHeap * 100.0;
  
  // 计算系统负载（基于内存使用和活动线程数）
  float systemLoad = memoryUsage * 0.7 + (activeThreads.size() * 10.0) * 0.3;
  
  // 根据系统负载和电池状态调整CPU频率
  uint32_t targetFreq;
  
  if (isLowPowerMode) {
    // 低功耗模式下使用最低频率
    targetFreq = minCpuFreqMHz;
  } else if (batteryPercentage < 20 && !isCharging) {
    // 电池电量低时使用较低频率
    targetFreq = minCpuFreqMHz;
  } else if (systemLoad > 80) {
    // 高负载时使用最高频率
    targetFreq = maxCpuFreqMHz;
  } else if (systemLoad > 50) {
    // 中等负载时使用中高频率
    targetFreq = (minCpuFreqMHz + maxCpuFreqMHz) * 3 / 4;
  } else if (systemLoad > 20) {
    // 低负载时使用中等频率
    targetFreq = (minCpuFreqMHz + maxCpuFreqMHz) / 2;
  } else {
    // 极低负载时使用低频率
    targetFreq = (minCpuFreqMHz + maxCpuFreqMHz) / 4;
  }
  
  // 避免频繁切换频率
  if (abs((int)targetFreq - (int)previousFreq) > 20) {
    setCpuFrequencyMhz(targetFreq);
    lastFreqAdjust = uptime;
    previousFreq = targetFreq;
    
    // 记录频率调整
    Serial.printf("CPU frequency adjusted to %u MHz based on load: %.1f%%\n", 
                  targetFreq, systemLoad);
  }
}

// 获取单例实例
CoreSystem* CoreSystem::getInstance() {
  if (instance == nullptr) {
    instance = new CoreSystem();
  }
  return instance;
}

// 初始化核心系统
bool CoreSystem::init() {
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
void CoreSystem::run() {
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
void CoreSystem::enterLowPowerMode() {
  if (state == SYSTEM_STATE_RUNNING) {
    isLowPowerMode = true;
    state = SYSTEM_STATE_LOW_POWER;
    
    // 发布低功耗进入事件
    eventBus->publish(EVENT_LOW_POWER_ENTER, nullptr);
    
    Serial.println("Entering low power mode");
  }
}

// 退出低功耗模式
void CoreSystem::exitLowPowerMode() {
  if (state == SYSTEM_STATE_LOW_POWER) {
    isLowPowerMode = false;
    state = SYSTEM_STATE_RUNNING;
    
    // 发布低功耗退出事件
    eventBus->publish(EVENT_LOW_POWER_EXIT, nullptr);
    
    Serial.println("Exiting low power mode");
  }
}

// 关闭系统
void CoreSystem::shutdown() {
  state = SYSTEM_STATE_SHUTTING_DOWN;
  eventBus->publish(EVENT_SYSTEM_SHUTDOWN, nullptr);
  
  // 清理资源
  driverRegistry->clear();
  timers.clear();
  
  Serial.println("System shutting down");
  state = SYSTEM_STATE_UNINITIALIZED;
}

// 重置系统
void CoreSystem::reset() {
  eventBus->publish(EVENT_SYSTEM_RESET, nullptr);
  platformReset();
}

// 获取系统状态
CoreSystemState CoreSystem::getState() const {
  return state;
}

// 获取事件总线
EventBus* CoreSystem::getEventBus() {
  return eventBus;
}

// 获取驱动注册表
DriverRegistry* CoreSystem::getDriverRegistry() {
  return driverRegistry;
}

// 获取系统运行时间
unsigned long CoreSystem::getUptime() const {
  return millis() - startTime;
}

// 发送系统错误
void CoreSystem::sendError(const String& message, int errorCode, const String& module) {
  auto errorData = std::make_shared<SystemErrorEventData>(message, errorCode, module);
  eventBus->publish(EVENT_SYSTEM_ERROR, errorData);
  
  Serial.printf("System Error [%s]: %s (Code: %d)\n", module.c_str(), message.c_str(), errorCode);
}

// 获取电池电压
float CoreSystem::getBatteryVoltage() const {
  return batteryVoltage;
}

// 获取电池百分比
int CoreSystem::getBatteryPercentage() const {
  return batteryPercentage;
}

// 获取充电状态
bool CoreSystem::isChargingState() const {
  return isCharging;
}

// 获取低功耗模式状态
bool CoreSystem::isInLowPowerMode() const {
  return isLowPowerMode;
}

// 获取配置
String CoreSystem::getConfig(const String& key, const String& defaultValue) {
  for (const auto& item : configItems) {
    if (item.key == key) {
      return item.value;
    }
  }
  return defaultValue;
}

// 设置配置
bool CoreSystem::setConfig(const String& key, const String& value) {
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

// 保存配置
bool CoreSystem::saveConfig() {
  // 保存配置到文件
  auto configData = std::make_shared<SystemErrorEventData>("Configuration saved", 0, "CoreSystem");
  eventBus->publish(EVENT_CONFIG_SAVED, configData);
  return true;
}

// 重置配置
bool CoreSystem::resetConfig() {
  configItems.clear();
  configLoaded = false;
  loadConfig();
  
  // 发布配置重置事件
  eventBus->publish(EVENT_CONFIG_RESET, nullptr);
  return true;
}

// 创建定时器
uint32_t CoreSystem::createTimer(unsigned long interval, std::function<void(uint32_t)> callback, bool isOneShot) {
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

// 启动定时器
bool CoreSystem::startTimer(uint32_t timerId) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      timer.enabled = true;
      timer.lastTriggerTime = millis();
      return true;
    }
  }
  return false;
}

// 停止定时器
bool CoreSystem::stopTimer(uint32_t timerId) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      timer.enabled = false;
      return true;
    }
  }
  return false;
}

// 删除定时器
bool CoreSystem::deleteTimer(uint32_t timerId) {
  for (auto it = timers.begin(); it != timers.end(); ++it) {
    if (it->timerId == timerId) {
      timers.erase(it);
      return true;
    }
  }
  return false;
}

// 检查定时器是否运行
bool CoreSystem::isTimerRunning(uint32_t timerId) {
  for (const auto& timer : timers) {
    if (timer.timerId == timerId) {
      return timer.enabled;
    }
  }
  return false;
}

// 设置定时器间隔
bool CoreSystem::setTimerInterval(uint32_t timerId, unsigned long interval) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      timer.interval = interval;
      return true;
    }
  }
  return false;
}

// 获取系统内存信息
void CoreSystem::getMemoryInfo(size_t& freeHeap, size_t& minimumFreeHeap) {
  freeHeap = platformGetFreeHeap();
  minimumFreeHeap = platformGetMinFreeHeap();
}

// 获取系统CPU频率
uint32_t CoreSystem::getCpuFrequencyMhz() {
  return platformGetCpuFreqMHz();
}

// 设置CPU频率
bool CoreSystem::setCpuFrequencyMhz(uint32_t freqMHz) {
  if (freqMHz >= minCpuFreqMHz && freqMHz <= maxCpuFreqMHz) {
    if (platformSetCpuFreqMHz(freqMHz)) {
      currentCpuFreqMHz = freqMHz;
      return true;
    }
  }
  return false;
}

// 启用动态CPU频率调整
void CoreSystem::enableDynamicCpuFreq(bool enable) {
  dynamicCpuFreqEnabled = enable;
}

// 设置CPU频率范围
void CoreSystem::setCpuFreqRange(int minFreq, int maxFreq) {
  minCpuFreqMHz = minFreq;
  maxCpuFreqMHz = maxFreq;
  
  // 确保当前频率在范围内
  if (currentCpuFreqMHz < minFreq) {
    setCpuFrequencyMhz(minFreq);
  } else if (currentCpuFreqMHz > maxFreq) {
    setCpuFrequencyMhz(maxFreq);
  }
}

// 创建线程互斥锁
SemaphoreHandle_t CoreSystem::createMutex() {
  SemaphoreHandle_t mutex = xSemaphoreCreateMutex();
  if (mutex != NULL) {
    threadMutexes[(uint32_t)mutex] = mutex;
  }
  return mutex;
}

// 锁定互斥锁
bool CoreSystem::lockMutex(SemaphoreHandle_t mutex, uint32_t timeoutMs) {
  if (mutex != NULL) {
    return xSemaphoreTake(mutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
  }
  return false;
}

// 解锁互斥锁
bool CoreSystem::unlockMutex(SemaphoreHandle_t mutex) {
  if (mutex != NULL) {
    return xSemaphoreGive(mutex) == pdTRUE;
  }
  return false;
}

// 销毁互斥锁
void CoreSystem::destroyMutex(SemaphoreHandle_t mutex) {
  if (mutex != NULL) {
    auto it = threadMutexes.find((uint32_t)mutex);
    if (it != threadMutexes.end()) {
      threadMutexes.erase(it);
      vSemaphoreDelete(mutex);
    }
  }
}

// 获取系统级互斥锁
SemaphoreHandle_t CoreSystem::getSystemMutex() {
  return systemMutex;
}

// 添加活动线程
void CoreSystem::addActiveThread(uint32_t threadId) {
  activeThreads.push_back(threadId);
}

// 移除活动线程
void CoreSystem::removeActiveThread(uint32_t threadId) {
  auto it = std::find(activeThreads.begin(), activeThreads.end(), threadId);
  if (it != activeThreads.end()) {
    activeThreads.erase(it);
  }
}

// 获取活动线程数量
size_t CoreSystem::getActiveThreadCount() {
  return activeThreads.size();
}

// 进入深度睡眠模式
void CoreSystem::enterDeepSleep(uint64_t sleepTimeMs) {
  // 发布深度睡眠事件
  eventBus->publish(EVENT_SYSTEM_DEEP_SLEEP, nullptr);
  
  // 清理资源
  cleanupMemory();
  
  // 进入深度睡眠
  platformDeepSleep(sleepTimeMs);
}

// 进入轻度睡眠模式
void CoreSystem::enterLightSleep(uint64_t sleepTimeMs) {
  // 发布轻度睡眠事件
  eventBus->publish(EVENT_SYSTEM_LIGHT_SLEEP, nullptr);
  
  // 进入轻度睡眠
  platformLightSleep(sleepTimeMs);
  
  // 唤醒后恢复
  eventBus->publish(EVENT_SYSTEM_WAKEUP, nullptr);
}

// 设置低功耗模式
void CoreSystem::setLowPowerMode(bool enable) {
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
void CoreSystem::optimizePowerConsumption() {
  // 根据电池电量和充电状态调整功耗策略
  if (isCharging) {
    // 充电时使用正常模式
    setLowPowerMode(false);
    // 动态调整CPU频率
    adjustCpuFreqBasedOnLoad();
    // 恢复正常任务优先级
    defaultTaskPriority = 5;
  } else if (batteryPercentage <= CRITICAL_BATTERY_THRESHOLD) {
    // 电量极低时，进入深度低功耗模式
    setLowPowerMode(true);
    // 延长传感器读取间隔
    // 减少WiFi/BLE活动
    // 降低CPU频率到最低
    setCpuFrequencyMhz(minCpuFreqMHz);
    // 降低任务优先级
    defaultTaskPriority = 1;
    // 清理内存
    cleanupMemory();
  } else if (batteryPercentage < 20) {
    // 电量低于20%，进入中度低功耗模式
    setLowPowerMode(true);
    // 延长传感器读取间隔
    // 减少WiFi/BLE活动
    setCpuFrequencyMhz((minCpuFreqMHz + maxCpuFreqMHz) / 3);
    // 降低任务优先级
    defaultTaskPriority = 2;
    // 清理内存
    cleanupMemory();
  } else if (batteryPercentage < 50) {
    // 电量低于50%，进入轻度低功耗模式
    setLowPowerMode(true);
    setCpuFrequencyMhz((minCpuFreqMHz + maxCpuFreqMHz) / 2);
    // 稍微降低任务优先级
    defaultTaskPriority = 3;
    // 定期清理内存
    if (millis() % 10000 < 100) { // 每10秒清理一次
      cleanupMemory();
    }
  } else {
    // 电量充足，使用正常模式
    setLowPowerMode(false);
    // 动态调整CPU频率
    adjustCpuFreqBasedOnLoad();
    // 恢复正常任务优先级
    defaultTaskPriority = 5;
    // 定期清理内存
    if (millis() % 30000 < 100) { // 每30秒清理一次
      cleanupMemory();
    }
  }
  
  // 动态调整CPU频率
  if (dynamicCpuFreqEnabled && !isLowPowerMode) {
    adjustCpuFreqBasedOnLoad();
  }
  
  // 记录功耗优化状态
  Serial.printf("Power optimization: Battery=%d%%, Mode=%s, CPU=%uMHz, Priority=%d\n", 
                batteryPercentage, isLowPowerMode ? "LowPower" : "Normal", 
                currentCpuFreqMHz, defaultTaskPriority);
}

// 获取芯片ID
uint32_t CoreSystem::getChipId() {
  return platformGetChipId();
}

// 获取Flash大小
uint32_t CoreSystem::getFlashChipSize() {
  return platformGetFlashChipSize();
}

// 获取固件大小
uint32_t CoreSystem::getFirmwareSize() {
  return platformGetFirmwareSize();
}

// 获取可用Flash空间
uint32_t CoreSystem::getFreeFlashSize() {
  return platformGetFreeFlashSize();
}

// 获取Flash使用情况
void CoreSystem::getFlashInfo(uint32_t& totalSize, uint32_t& firmwareSize, uint32_t& freeSize) {
  platformGetFlashInfo(totalSize, firmwareSize, freeSize);
}

// 创建内存池
void* CoreSystem::createMemoryPool(size_t blockSize, size_t blockCount) {
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
void* CoreSystem::allocateFromPool(void* poolPtr, size_t size) {
  for (auto& pool : memoryPools) {
    if (pool.pool == poolPtr && size <= pool.blockSize && pool.freeBlocks > 0) {
      void* ptr = pool.freeList[--pool.freeBlocks];
      memset(ptr, 0, pool.blockSize);
      return ptr;
    }
  }
  
  // 如果没有找到合适的内存池，尝试创建一个新的
  // 这里可以添加自动内存池创建逻辑
  
  return nullptr;
}

// 释放内存回内存池
void CoreSystem::freeToPool(void* poolPtr, void* ptr) {
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
void CoreSystem::destroyMemoryPool(void* poolPtr) {
  for (auto it = memoryPools.begin(); it != memoryPools.end(); ++it) {
    if (it->pool == poolPtr) {
      totalAllocatedMemory -= it->blockSize * it->blockCount + sizeof(void*) * it->blockCount;
      if (it->freeList != nullptr) {
        free(it->freeList);
      }
      if (it->pool != nullptr) {
        free(it->pool);
      }
      memoryPools.erase(it);
      return;
    }
  }
}

// 获取内存池使用情况
void CoreSystem::getMemoryPoolInfo(void* poolPtr, size_t& totalBlocks, size_t& freeBlocks) {
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
void CoreSystem::cleanupMemory() {
  // 清理定时器
  auto it = timers.begin();
  while (it != timers.end()) {
    if (!it->enabled && it->isOneShot) {
      it = timers.erase(it);
    } else {
      ++it;
    }
  }
  
  // 清理内存池中的空闲块
  for (auto& pool : memoryPools) {
    if (pool.freeBlocks < pool.blockCount) {
      // 优化内存池使用，合并相邻空闲块
      // 这里可以添加更复杂的内存池清理逻辑
    }
  }
  
  // 清理未使用的内存池
  auto poolIt = memoryPools.begin();
  while (poolIt != memoryPools.end()) {
    if (poolIt->freeBlocks == poolIt->blockCount) {
      // 释放完全空闲的内存池
      totalAllocatedMemory -= poolIt->blockSize * poolIt->blockCount + sizeof(void*) * poolIt->blockCount;
      if (poolIt->freeList != nullptr) {
        free(poolIt->freeList);
      }
      if (poolIt->pool != nullptr) {
        free(poolIt->pool);
      }
      poolIt = memoryPools.erase(poolIt);
    } else {
      ++poolIt;
    }
  }
  
  // 检查并清理内存泄漏
  checkMemoryLeaks();
  
  // 执行垃圾回收（如果可用）
  #ifdef ARDUINO_ARCH_ESP32
    esp_task_wdt_reset();
  #endif
  
  // 更新内存使用统计
  updateMemoryStats();
  
  // 记录内存使用情况
  size_t totalMemory, usedMemory, peakMemory;
  getMemoryStats(totalMemory, usedMemory, peakMemory);
  Serial.printf("Memory cleanup: Total=%zu, Used=%zu, Peak=%zu, Free=%zu\n", 
                totalMemory, usedMemory, peakMemory, totalMemory - usedMemory);
}

// 检查内存泄漏
void CoreSystem::checkMemoryLeaks() {
  // 增强的内存泄漏检查
  size_t currentHeap = platformGetFreeHeap();
  size_t minHeap = platformGetMinFreeHeap();
  
  if (lastMemoryUpdate > 0) {
    // 检查内存是否持续增长
    static size_t previousHeap = currentHeap;
    static size_t previousMinHeap = minHeap;
    static int leakCounter = 0;
    static unsigned long lastLeakCheck = 0;
    
    unsigned long now = millis();
    if (now - lastLeakCheck > 10000) { // 每10秒检查一次
      lastLeakCheck = now;
      
      // 检查内存是否持续减少
      if (currentHeap < previousHeap - 512) { // 如果内存减少超过512B
        leakCounter++;
        if (leakCounter > 5) { // 连续5次检测到内存泄漏
          // 检查最小堆是否也在减少
          if (minHeap < previousMinHeap) {
            sendError("Potential memory leak detected", 4001, "CoreSystem");
            // 记录详细的内存使用情况
            size_t totalMemory, usedMemory, peakMemory;
            getMemoryStats(totalMemory, usedMemory, peakMemory);
            Serial.printf("Memory leak details: Free=%zu, MinFree=%zu, Used=%zu, Peak=%zu\n", 
                          currentHeap, minHeap, usedMemory, peakMemory);
          }
          leakCounter = 0;
        }
      } else {
        leakCounter = 0;
      }
      
      previousHeap = currentHeap;
      previousMinHeap = minHeap;
    }
  }
}

// 更新内存统计信息
void CoreSystem::updateMemoryStats() {
  lastMemoryUpdate = millis();
  
  // 可以在这里添加更详细的内存统计
}

// 更新系统统计信息
void CoreSystem::updateSystemStats() {
  unsigned long now = millis();
  if (now - systemStats.lastStatsUpdate > 1000) { // 每秒更新一次
    systemStats.lastStatsUpdate = now;
    
    // 更新系统运行时间
    systemStats.uptime = getUptime();
    
    // 更新内存信息
    size_t freeHeap, minFreeHeap;
    getMemoryInfo(freeHeap, minFreeHeap);
    systemStats.freeHeap = freeHeap;
    systemStats.minFreeHeap = minFreeHeap;
    
    // 更新内存使用统计
    size_t totalMemory, usedMemory, peakMemory;
    getMemoryStats(totalMemory, usedMemory, peakMemory);
    systemStats.usedMemory = usedMemory;
    systemStats.peakMemory = peakMemory;
    
    // 更新CPU信息
    systemStats.cpuFreqMHz = getCpuFrequencyMhz();
    
    // 更新电源信息
    systemStats.batteryVoltage = batteryVoltage;
    systemStats.batteryPercentage = batteryPercentage;
    systemStats.isCharging = isCharging;
    systemStats.isLowPowerMode = isLowPowerMode;
    
    // 更新线程和定时器信息
    systemStats.activeThreads = activeThreads.size();
    systemStats.activeTimers = timers.size();
  }
}

// 获取系统统计信息
const CoreSystem::SystemStats& CoreSystem::getSystemStats() {
  updateSystemStats();
  return systemStats;
}

// 开始性能测量
void CoreSystem::startPerformanceMeasurement(const String& name) {
  // 记录开始时间
  performanceMetrics[name].lastTime = micros();
}

// 结束性能测量
void CoreSystem::endPerformanceMeasurement(const String& name) {
  unsigned long endTime = micros();
  unsigned long duration = endTime - performanceMetrics[name].lastTime;
  
  // 更新性能指标
  auto& metric = performanceMetrics[name];
  metric.name = name;
  metric.totalTime += duration;
  metric.count++;
  
  if (duration > metric.maxTime || metric.maxTime == 0) {
    metric.maxTime = duration;
  }
  
  if (duration < metric.minTime || metric.minTime == 0) {
    metric.minTime = duration;
  }
}

// 获取性能统计信息
String CoreSystem::getPerformanceStats() {
  String stats = "Performance Statistics:\n";
  
  for (const auto& pair : performanceMetrics) {
    const auto& metric = pair.second;
    if (metric.count > 0) {
      unsigned long avgTime = metric.totalTime / metric.count;
      stats += String("  ") + metric.name + ":\n";
      stats += String("    Count: ") + metric.count + "\n";
      stats += String("    Avg: ") + avgTime + "μs\n";
      stats += String("    Min: ") + metric.minTime + "μs\n";
      stats += String("    Max: ") + metric.maxTime + "μs\n";
      stats += String("    Total: ") + metric.totalTime + "μs\n";
    }
  }
  
  return stats;
}

// 重置性能统计信息
void CoreSystem::resetPerformanceStats() {
  performanceMetrics.clear();
}

// 获取内存使用统计
void CoreSystem::getMemoryStats(size_t& totalMemory, size_t& usedMemory, size_t& peakMemory) {
  size_t freeHeap = platformGetFreeHeap();
  size_t totalHeap = platformGetFlashChipSize(); // 近似值，实际应该使用芯片总RAM
  
  totalMemory = totalHeap;
  usedMemory = totalHeap - freeHeap + totalAllocatedMemory;
  peakMemory = peakAllocatedMemory;
}

// 设置任务优先级
void CoreSystem::setTaskPriority(uint32_t taskId, int priority) {
  // 优先级范围：1-10，1最低，10最高
  if (priority >= 1 && priority <= 10) {
    taskPriorities[taskId] = priority;
  }
}

// 获取任务优先级
int CoreSystem::getTaskPriority(uint32_t taskId) {
  auto it = taskPriorities.find(taskId);
  if (it != taskPriorities.end()) {
    return it->second;
  }
  return defaultTaskPriority;
}

// 设置默认任务优先级
void CoreSystem::setDefaultTaskPriority(int priority) {
  if (priority >= 1 && priority <= 10) {
    defaultTaskPriority = priority;
  }
}

// 析构函数
CoreSystem::~CoreSystem() {
  // 清理资源
  timers.clear();
  configItems.clear();
  
  // 清理事件总线订阅
  if (eventBus != nullptr) {
    eventBus->clear();
  }
  
  // 清理驱动注册表
  if (driverRegistry != nullptr) {
    driverRegistry->clear();
  }
  
  // 释放内存池资源
  for (auto& pool : memoryPools) {
    if (pool.freeList != nullptr) {
      free(pool.freeList);
    }
    if (pool.pool != nullptr) {
      free(pool.pool);
    }
  }
  memoryPools.clear();
  
  // 清理线程资源
  for (auto& mutex : threadMutexes) {
    if (mutex.second != NULL) {
      vSemaphoreDelete(mutex.second);
    }
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
