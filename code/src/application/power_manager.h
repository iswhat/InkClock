#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include "../coresystem/config.h"

// 电池历史数据大小
#define BATTERY_HISTORY_SIZE 10

class CoreSystem;

// 电池状态事件数据结构
class BatteryStatusEventData {
public:
  float voltage;
  int percentage;
  int health;
  float temperature;
  bool isCharging;
  
  BatteryStatusEventData(float v, int p, int h, float t, bool c) 
    : voltage(v), percentage(p), health(h), temperature(t), isCharging(c) {}
};

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
  
  // 电池监控相关
  int batteryHealth; // 电池健康度（0-100%）
  float batteryTemperature; // 电池温度（°C）
  bool lowBatteryAlarmTriggered;
  bool criticalBatteryAlarmTriggered;
  bool overheatAlarmTriggered;
  
  // 报警阈值
  int lowBatteryThreshold;
  int criticalBatteryThreshold;
  float overheatThreshold;
  
  // 电池历史数据
  int batteryHistory[BATTERY_HISTORY_SIZE];
  int batteryHistoryIndex;
  bool batteryHistoryInitialized;
  
  // CoreSystem指针
  CoreSystem* coreSystem;
  
  // 读取ADC值并转换为电压
  float readBatteryVoltage();
  
  // 根据电压计算电量百分比
  int calculateBatteryPercentage(float voltage);
  
  // 读取充电状态
  bool readChargingStatus();
  
  // 读取电池温度
  float readBatteryTemperature();
  
  // 计算电池健康度
  void calculateBatteryHealth();
  
  // 更新电池历史数据
  void updateBatteryHistory();
  
  // 检查电池状态并触发报警
  void checkBatteryStatus();
  
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
  
  // 获取电池健康度
  int getBatteryHealth() const;
  
  // 获取电池温度
  float getBatteryTemperature() const;
  
  // 检查是否低电量
  bool isBatteryLow() const;
  
  // 检查是否临界低电量
  bool isBatteryCritical() const;
  
  // 检查是否电池过热
  bool isBatteryOverheated() const;
  
  // 获取平均电池电量
  int getAverageBatteryLevel() const;
  
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