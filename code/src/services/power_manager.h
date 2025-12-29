#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "../core/config.h"

class PowerManager {
private:
  float batteryVoltage;
  int batteryPercentage;
  bool isCharging;
  unsigned long lastUpdateTime;
  
  // 低功耗模式相关
  bool isLowPowerMode;
  unsigned long lastMotionTime;
  unsigned long lastDisplayUpdateTime;
  
  // 充电相关
  ChargingInterfaceType chargingInterface;
  bool hasChargingProtection;
  
  // 读取ADC值并转换为电压
  float readBatteryVoltage();
  
  // 根据电压计算电量百分比
  int calculateBatteryPercentage(float voltage);
  
  // 读取充电状态
  bool readChargingStatus();
  
  // 检查充电接口类型
  void checkChargingInterface();
  
  // 读取人体感应传感器状态
  bool readPIRSensor();
  
  // 进入低功耗模式
  void enterLowPowerMode();
  
  // 退出低功耗模式
  void exitLowPowerMode();
  
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
  
  // 获取低功耗模式状态
  bool getLowPowerMode() { return isLowPowerMode; }
  
  // 检查是否需要更新显示
  bool shouldUpdateDisplay();
  
  // 获取充电接口类型
  ChargingInterfaceType getChargingInterface() { return chargingInterface; }
  
  // 检查是否有充电保护
  bool hasChargingProtectionEnabled() { return hasChargingProtection; }
  
  // 检查是否支持DC供电（始终返回false）
  bool isDCPowerSupported() { return DC_POWER_SUPPORTED; }
  
  // 检查是否仅支持USB供电
  bool isOnlyUSBPowerSupported() { return ONLY_USB_POWER_SUPPORTED; }
};

#endif // POWER_MANAGER_H