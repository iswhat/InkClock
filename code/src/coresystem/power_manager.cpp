#include "power_manager.h"

// 初始化单例实例
PowerManager* PowerManager::instance = nullptr;

// 获取单例实例
PowerManager* PowerManager::getInstance() {
  if (instance == nullptr) {
    instance = new PowerManager();
  }
  return instance;
}

// 初始化
bool PowerManager::init() {
  // 初始化电源管理
  updatePowerState();
  return true;
}

// 读取电池电压
float PowerManager::readBatteryVoltage() {
  // 默认实现，返回模拟读取值
  #ifdef BATTERY_ADC_PIN
    int adcValue = analogRead(BATTERY_ADC_PIN);
    float voltage = adcValue * (FULL_BATTERY_VOLTAGE / 4096.0);
    return voltage;
  #else
    return 0.0;
  #endif
}

// 计算电池百分比
int PowerManager::calculateBatteryPercentage(float voltage) {
  // 简单的线性映射，实际应该根据电池特性调整
  if (voltage <= 3.0) return 0;
  if (voltage >= 4.2) return 100;
  return (voltage - 3.0) * 100 / 1.2;
}

// 读取充电状态
bool PowerManager::readChargingStatus() {
  #ifdef CHARGING_STATUS_PIN
    return digitalRead(CHARGING_STATUS_PIN) == HIGH;
  #else
    return false;
  #endif
}

// 进入低功耗模式
void PowerManager::enterLowPowerMode() {
  isLowPowerMode = true;
  
  // 发布低功耗进入事件
  EventBus::getInstance()->publish(EVENT_LOW_POWER_ENTER, nullptr);
  
  Serial.println("Entering low power mode");
}

// 退出低功耗模式
void PowerManager::exitLowPowerMode() {
  isLowPowerMode = false;
  
  // 发布低功耗退出事件
  EventBus::getInstance()->publish(EVENT_LOW_POWER_EXIT, nullptr);
  
  Serial.println("Exiting low power mode");
}

// 更新电源状态
void PowerManager::updatePowerState() {
  unsigned long now = platformGetMillis();
  if (now - lastPowerUpdate > 1000) { // 每秒更新一次
    lastPowerUpdate = now;
    
    // 读取电池电压
    batteryVoltage = readBatteryVoltage();
    batteryPercentage = calculateBatteryPercentage(batteryVoltage);
    isCharging = readChargingStatus();
    
    // 检查低电量
    if (batteryPercentage <= LOW_BATTERY_THRESHOLD && !isCharging) {
      auto lowPowerData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, true);
      EventBus::getInstance()->publish(EVENT_BATTERY_LOW, lowPowerData);
      
      // 进入低功耗模式
      if (!isLowPowerMode) {
        enterLowPowerMode();
      }
    } else if (batteryPercentage > LOW_BATTERY_THRESHOLD * 1.2 && isLowPowerMode) {
      auto powerOkData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, false);
      EventBus::getInstance()->publish(EVENT_BATTERY_OK, powerOkData);
      
      // 退出低功耗模式
      exitLowPowerMode();
    }
    
    // 发布电源状态变化事件
    auto powerData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, isLowPowerMode);
    EventBus::getInstance()->publish(EVENT_POWER_STATE_CHANGED, powerData);
  }
}

// 获取电池电压
float PowerManager::getBatteryVoltage() const {
  return batteryVoltage;
}

// 获取电池百分比
int PowerManager::getBatteryPercentage() const {
  return batteryPercentage;
}

// 获取充电状态
bool PowerManager::isChargingState() const {
  return isCharging;
}

// 获取低功耗模式状态
bool PowerManager::isInLowPowerMode() const {
  return isLowPowerMode;
}

// 设置低功耗模式
void PowerManager::setLowPowerMode(bool enable) {
  if (isLowPowerMode != enable) {
    isLowPowerMode = enable;
    
    if (enable) {
      // 进入低功耗模式
      EventBus::getInstance()->publish(EVENT_SYSTEM_LOW_POWER, nullptr);
    } else {
      // 退出低功耗模式
      EventBus::getInstance()->publish(EVENT_SYSTEM_NORMAL_POWER, nullptr);
    }
  }
}

// 进入深度睡眠模式
void PowerManager::enterDeepSleep(uint64_t sleepTimeMs) {
  // 发布深度睡眠事件
  EventBus::getInstance()->publish(EVENT_SYSTEM_DEEP_SLEEP, nullptr);
  
  // 进入深度睡眠
  platformDeepSleep(sleepTimeMs);
}

// 进入轻度睡眠模式
void PowerManager::enterLightSleep(uint64_t sleepTimeMs) {
  // 发布轻度睡眠事件
  EventBus::getInstance()->publish(EVENT_SYSTEM_LIGHT_SLEEP, nullptr);
  
  // 进入轻度睡眠
  platformLightSleep(sleepTimeMs);
  
  // 唤醒后恢复
  EventBus::getInstance()->publish(EVENT_SYSTEM_WAKEUP, nullptr);
}

// 调整功耗等级
void PowerManager::adjustPowerLevel() {
  if (isCharging) {
    currentPowerLevel = POWER_LEVEL_NORMAL;
    updateInterval = 5000; // 充电时更新频率降低
  } else if (batteryPercentage <= 10) {
    currentPowerLevel = POWER_LEVEL_DEEP_LOW;
    updateInterval = 30000; // 深度低功耗时更新频率更低
  } else if (batteryPercentage <= 20) {
    currentPowerLevel = POWER_LEVEL_MEDIUM_LOW;
    updateInterval = 15000; // 中度低功耗时更新频率降低
  } else if (batteryPercentage <= 50) {
    currentPowerLevel = POWER_LEVEL_LIGHT_LOW;
    updateInterval = 5000; // 轻度低功耗时更新频率降低
  } else {
    currentPowerLevel = POWER_LEVEL_NORMAL;
    updateInterval = 1000; // 正常模式时标准更新频率
  }
}

// 执行功耗优化措施
void PowerManager::applyPowerOptimizations() {
  switch (currentPowerLevel) {
    case POWER_LEVEL_DEEP_LOW:
      // 深度低功耗模式：关闭所有非必要功能
      EventBus::getInstance()->publish(EVENT_SYSTEM_LOW_POWER, nullptr);
      break;
    case POWER_LEVEL_MEDIUM_LOW:
      // 中度低功耗模式：减少网络连接和传感器采样
      EventBus::getInstance()->publish(EVENT_SYSTEM_LOW_POWER, nullptr);
      break;
    case POWER_LEVEL_LIGHT_LOW:
      // 轻度低功耗模式：减少显示更新频率
      EventBus::getInstance()->publish(EVENT_SYSTEM_LOW_POWER, nullptr);
      break;
    case POWER_LEVEL_NORMAL:
      // 正常模式：所有功能正常运行
      EventBus::getInstance()->publish(EVENT_SYSTEM_NORMAL_POWER, nullptr);
      break;
  }
}

// 获取当前功耗等级
PowerManager::PowerConsumptionLevel PowerManager::getCurrentPowerLevel() const {
  return currentPowerLevel;
}

// 优化功耗
void PowerManager::optimizePowerConsumption() {
  adjustPowerLevel();
  applyPowerOptimizations();
  
  // 根据功耗等级设置低功耗模式
  isLowPowerMode = (currentPowerLevel != POWER_LEVEL_NORMAL);
}

// 获取最佳睡眠模式和时间
void PowerManager::getOptimalSleepParameters(uint64_t& sleepTimeMs, bool& useDeepSleep) {
  switch (currentPowerLevel) {
    case POWER_LEVEL_DEEP_LOW:
      sleepTimeMs = 300000; // 5分钟
      useDeepSleep = true;
      break;
    case POWER_LEVEL_MEDIUM_LOW:
      sleepTimeMs = 180000; // 3分钟
      useDeepSleep = true;
      break;
    case POWER_LEVEL_LIGHT_LOW:
      sleepTimeMs = 60000; // 1分钟
      useDeepSleep = false;
      break;
    case POWER_LEVEL_NORMAL:
      sleepTimeMs = 30000; // 30秒
      useDeepSleep = false;
      break;
  }
}

