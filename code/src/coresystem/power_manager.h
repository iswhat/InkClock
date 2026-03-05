#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include "platform_abstraction.h"
#include "event_bus.h"

// 使用event_bus.h中定义的PowerStateEventData

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
  unsigned long updateInterval;
  
  // 功耗等级
  enum PowerConsumptionLevel {
    POWER_LEVEL_NORMAL = 0,
    POWER_LEVEL_LIGHT_LOW = 1,
    POWER_LEVEL_MEDIUM_LOW = 2,
    POWER_LEVEL_DEEP_LOW = 3
  };
  
  PowerConsumptionLevel currentPowerLevel;
  
  // 构造函数
  PowerManager() {
    batteryVoltage = 0.0;
    batteryPercentage = 0;
    isCharging = false;
    isLowPowerMode = false;
    lastPowerUpdate = 0;
    updateInterval = 1000; // 默认1秒更新一次
    currentPowerLevel = POWER_LEVEL_NORMAL;
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
  
  // 调整功耗等级
  void adjustPowerLevel();
  
  // 执行功耗优化措施
  void applyPowerOptimizations();
  
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
  
  // 获取当前功耗等级
  PowerConsumptionLevel getCurrentPowerLevel() const;
  
  // 设置低功耗模式
  void setLowPowerMode(bool enable);
  
  // 进入深度睡眠模式
  void enterDeepSleep(uint64_t sleepTimeMs);
  
  // 进入轻度睡眠模式
  void enterLightSleep(uint64_t sleepTimeMs);
  
  // 优化功耗
  void optimizePowerConsumption();
  
  // 获取最佳睡眠模式和时间
  void getOptimalSleepParameters(uint64_t& sleepTimeMs, bool& useDeepSleep);
  
  // 检查是否应该更新显示
  bool shouldUpdateDisplay() {
    // 简单实现：如果不是低功耗模式，允许更新显示
    return !isLowPowerMode;
  }
}; 

#endif // POWER_MANAGER_H