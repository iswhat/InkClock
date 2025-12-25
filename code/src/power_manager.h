#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "config.h"

class PowerManager {
private:
  float batteryVoltage;
  int batteryPercentage;
  bool isCharging;
  unsigned long lastUpdateTime;

  // 读取ADC值并转换为电压
  float readBatteryVoltage();
  
  // 根据电压计算电量百分比
  int calculateBatteryPercentage(float voltage);
  
  // 读取充电状态
  bool readChargingStatus();
  
public:
  PowerManager();
  ~PowerManager();
  
  void init();
  void loop();
  void update();
  
  // 获取电池电压
  float getBatteryVoltage() { return batteryVoltage; }
  
  // 获取电池电量百分比
  int getBatteryPercentage() { return batteryPercentage; }
  
  // 获取充电状态
  bool getChargingStatus() { return isCharging; }
  
  // 检查是否需要充电
  bool isLowBattery() { return batteryVoltage <= LOW_BATTERY_THRESHOLD; }
};

#endif // POWER_MANAGER_H