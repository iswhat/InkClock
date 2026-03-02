#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "platform_abstraction.h"
#include "event_bus.h"

// 电源状态事件数据
struct PowerStateEventData {
  int batteryPercentage;
  bool isCharging;
  bool isLowPower;
  
  PowerStateEventData(int percentage, bool charging, bool lowPower) 
    : batteryPercentage(percentage), isCharging(charging), isLowPower(lowPower) {}
};

// 电源管理类
class PowerManager {
private:
  static PowerManager* instance;
  
  // 电源状态
  float batteryVoltage;
  int batteryPercentage;
  bool isCharging;
  bool isLowPowerMode;
  unsigned long lastPowerUpdate;
  
  // 构造函数
  PowerManager() {
    batteryVoltage = 0.0;
    batteryPercentage = 0;
    isCharging = false;
    isLowPowerMode = false;
    lastPowerUpdate = 0;
  }
  
  // 读取电池电压
  float readBatteryVoltage();
  
  // 计算电池百分比
  int calculateBatteryPercentage(float voltage);
  
  // 读取充电状态
  bool readChargingStatus();
  
  // 进入低功耗模式
  void enterLowPowerMode();
  
  // 退出低功耗模式
  void exitLowPowerMode();
  
public:
  // 获取单例实例
  static PowerManager* getInstance();
  
  // 初始化
  bool init();
  
  // 更新电源状态
  void updatePowerState();
  
  // 获取电池电压
  float getBatteryVoltage() const;
  
  // 获取电池百分比
  int getBatteryPercentage() const;
  
  // 获取充电状态
  bool isChargingState() const;
  
  // 获取低功耗模式状态
  bool isInLowPowerMode() const;
  
  // 设置低功耗模式
  void setLowPowerMode(bool enable);
  
  // 进入深度睡眠模式
  void enterDeepSleep(uint64_t sleepTimeMs);
  
  // 进入轻度睡眠模式
  void enterLightSleep(uint64_t sleepTimeMs);
  
  // 优化功耗
  void optimizePowerConsumption();
};

#endif // POWER_MANAGER_H