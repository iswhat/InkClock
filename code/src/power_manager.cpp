#include "power_manager.h"

PowerManager::PowerManager() {
  batteryVoltage = 0.0;
  batteryPercentage = 0;
  isCharging = false;
  lastUpdateTime = 0;
  
  // 低功耗模式初始化
  isLowPowerMode = false;
  lastMotionTime = millis();
  lastDisplayUpdateTime = millis();
}

PowerManager::~PowerManager() {
}

void PowerManager::init() {
  // 初始化电池检测引脚
  pinMode(BATTERY_ADC_PIN, INPUT);
  
  // 如果有充电状态引脚，初始化它
  if (CHARGE_STATUS_PIN != -1) {
    pinMode(CHARGE_STATUS_PIN, INPUT);
    DEBUG_PRINTLN("Charge status pin initialized on pin " + String(CHARGE_STATUS_PIN));
  }
  
  // 初始化人体感应传感器引脚
  if (LOW_POWER_MODE_ENABLED) {
    pinMode(PIR_SENSOR_PIN, INPUT);
    DEBUG_PRINTLN("PIR sensor initialized on pin " + String(PIR_SENSOR_PIN));
  }
  
  // 初始更新
  update();
  
  DEBUG_PRINTLN("PowerManager initialized");
}

void PowerManager::loop() {
  // 定期更新电池状态
  if (millis() - lastUpdateTime > BATTERY_UPDATE_INTERVAL) {
    update();
  }
  
  // 低功耗模式处理
  if (LOW_POWER_MODE_ENABLED) {
    bool motionDetected = readPIRSensor();
    
    if (motionDetected) {
      lastMotionTime = millis();
      if (isLowPowerMode) {
        exitLowPowerMode();
      }
    } else {
      if (!isLowPowerMode && (millis() - lastMotionTime > NO_MOTION_TIMEOUT)) {
        enterLowPowerMode();
      }
    }
  }
}

void PowerManager::update() {
  // 读取电池电压
  batteryVoltage = readBatteryVoltage();
  
  // 计算电量百分比
  batteryPercentage = calculateBatteryPercentage(batteryVoltage);
  
  // 读取充电状态
  isCharging = readChargingStatus();
  
  lastUpdateTime = millis();
  
  DEBUG_PRINT("Battery: ");
  DEBUG_PRINT(batteryVoltage);
  DEBUG_PRINT("V, ");
  DEBUG_PRINT(batteryPercentage);
  DEBUG_PRINT("%, Charging: ");
  DEBUG_PRINT(isCharging ? "Yes" : "No");
  DEBUG_PRINT(", Low Power: ");
  DEBUG_PRINTLN(isLowPowerMode ? "Yes" : "No");
}

bool PowerManager::readPIRSensor() {
  #if LOW_POWER_MODE_ENABLED
    return digitalRead(PIR_SENSOR_PIN) == HIGH;
  #else
    return true;
  #endif
}

void PowerManager::enterLowPowerMode() {
  if (!isLowPowerMode) {
    isLowPowerMode = true;
    DEBUG_PRINTLN("Entering low power mode...");
    // 这里可以添加更多低功耗操作，如降低CPU频率、关闭不必要的模块等
  }
}

void PowerManager::exitLowPowerMode() {
  if (isLowPowerMode) {
    isLowPowerMode = false;
    DEBUG_PRINTLN("Exiting low power mode...");
    // 这里可以添加恢复正常操作，如恢复CPU频率、开启必要的模块等
  }
}

bool PowerManager::shouldUpdateDisplay() {
  unsigned long currentTime = millis();
  unsigned long refreshInterval = isLowPowerMode ? LOW_POWER_REFRESH_INTERVAL : NORMAL_REFRESH_INTERVAL;
  
  if (currentTime - lastDisplayUpdateTime >= refreshInterval) {
    lastDisplayUpdateTime = currentTime;
    return true;
  }
  
  return false;
}

float PowerManager::readBatteryVoltage() {
  // 读取ADC值（0-4095）
  int adcValue = analogRead(BATTERY_ADC_PIN);
  
  // 转换为电压值
  // 注意：需要根据实际的分压电路进行调整
  // 假设分压电阻为100kΩ和100kΩ，总电阻200kΩ
  // ADC参考电压为3.3V
  float voltage = (adcValue / 4095.0) * 3.3 * 2.0;
  
  return voltage;
}

int PowerManager::calculateBatteryPercentage(float voltage) {
  // 确保电压在合理范围内
  if (voltage >= FULL_BATTERY_VOLTAGE) {
    return 100;
  } else if (voltage <= EMPTY_BATTERY_VOLTAGE) {
    return 0;
  }
  
  // 线性计算电量百分比
  int percentage = ((voltage - EMPTY_BATTERY_VOLTAGE) / (FULL_BATTERY_VOLTAGE - EMPTY_BATTERY_VOLTAGE)) * 100;
  
  return percentage;
}

bool PowerManager::readChargingStatus() {
  // 如果没有充电状态引脚，返回false
  if (CHARGE_STATUS_PIN == -1) {
    return false;
  }
  
  // 读取充电状态引脚
  // 注意：根据实际硬件设计，充电状态引脚的电平可能不同
  // 这里假设高电平表示正在充电
  return digitalRead(CHARGE_STATUS_PIN) == HIGH;
}