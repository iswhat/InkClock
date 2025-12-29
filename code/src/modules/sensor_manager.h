#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "../core/config.h"
#include "../drivers/sensors/sensor_driver.h"



class SensorManager {
public:
  SensorManager();
  ~SensorManager();
  
  void init();
  void update();
  void loop();
  
  // 配置传感器
  void setSensorConfig(SensorConfig config);
  SensorConfig getSensorConfig() { return currentConfig; }
  
  // 获取传感器数据
  SensorData getSensorData() { return currentData; }
  
  // 校准传感器
  void calibrate(float tempOffset = 0.0, float humOffset = 0.0);
  
  // 选择传感器类型
  void setSensorType(SensorType type);
  SensorType getSensorType() { return currentConfig.type; }
  
  // 选择I2C传感器地址
  void setI2CAddress(uint8_t address);
  
  // 选择单总线传感器引脚
  void setPin(int pin);
  
  // 设置更新间隔
  void setUpdateInterval(unsigned long interval);
  
  // 报警相关方法
  void setGasAlarmThreshold(int threshold) { gasAlarmThreshold = threshold; }
  void setFlameAlarmThreshold(bool threshold) { flameAlarmThreshold = threshold; }
  void setTempAlarmThreshold(float minThreshold, float maxThreshold) { tempMinAlarmThreshold = minThreshold; tempMaxAlarmThreshold = maxThreshold; }
  void setHumidityAlarmThreshold(float minThreshold, float maxThreshold) { humidityMinAlarmThreshold = minThreshold; humidityMaxAlarmThreshold = maxThreshold; }
  void setLightAlarmThreshold(int threshold) { lightAlarmThreshold = threshold; }
  bool isGasAlarmTriggered() { return gasAlarmTriggered; }
  bool isFlameAlarmTriggered() { return flameAlarmTriggered; }
  bool isTempAlarmTriggered() { return tempAlarmTriggered; }
  bool isHumidityAlarmTriggered() { return humidityAlarmTriggered; }
  bool isLightAlarmTriggered() { return lightAlarmTriggered; }
  
  // 传感器开关控制
  void enablePIRSensor(bool enable) { pirSensorEnabled = enable; }
  void enableGasSensor(bool enable) { gasSensorEnabled = enable; }
  void enableFlameSensor(bool enable) { flameSensorEnabled = enable; }
  void enableLightSensor(bool enable) { lightSensorEnabled = enable; }
  
  // 获取传感器开关状态
  bool isPIRSensorEnabled() { return pirSensorEnabled; }
  bool isGasSensorEnabled() { return gasSensorEnabled; }
  bool isFlameSensorEnabled() { return flameSensorEnabled; }
  bool isLightSensorEnabled() { return lightSensorEnabled; }
  
private:
  // 传感器配置
  SensorConfig currentConfig;
  
  // 传感器数据
  SensorData currentData;
  
  // 传感器驱动
  ISensorDriver* sensorDriver;
  
  // 更新标志
  unsigned long lastUpdate;
  bool dataUpdated;
  
  // 校准偏移量
  float tempOffset;
  float humOffset;
  
  // 报警相关变量
  int gasAlarmThreshold;
  bool flameAlarmThreshold;
  float tempMinAlarmThreshold;
  float tempMaxAlarmThreshold;
  float humidityMinAlarmThreshold;
  float humidityMaxAlarmThreshold;
  int lightAlarmThreshold;
  bool gasAlarmTriggered;
  bool flameAlarmTriggered;
  bool tempAlarmTriggered;
  bool humidityAlarmTriggered;
  bool lightAlarmTriggered;
  
  // 传感器配置标志
  bool pirSensorEnabled;
  bool gasSensorEnabled;
  bool flameSensorEnabled;
  bool lightSensorEnabled;
  
  // 私有方法
  bool readSensor();
  void filterData();
  void checkAlarmConditions();
  void triggerAlarm(String alarmType);
};

#endif // SENSOR_MANAGER_H