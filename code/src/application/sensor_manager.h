#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include <memory>  // For smart pointers
#include "../coresystem/config.h"
#include "../drivers/peripherals/sensor_driver.h"
#include "../coresystem/event_bus.h"

// ========================================
// 传感器阈值常量
// ========================================
#ifndef GAS_ALARM_THRESHOLD_DEFAULT
  #define GAS_ALARM_THRESHOLD_DEFAULT      1000   // 气体报警阈值
#endif

#ifndef FLAME_ALARM_THRESHOLD_DEFAULT
  #define FLAME_ALARM_THRESHOLD_DEFAULT    true   // 火焰报警阈值（默认启用）
#endif

#ifndef TEMP_MIN_ALARM_THRESHOLD_DEFAULT
  #define TEMP_MIN_ALARM_THRESHOLD_DEFAULT -10.0  // 温度最低报警阈值（°C）
#endif

#ifndef TEMP_MAX_ALARM_THRESHOLD_DEFAULT
  #define TEMP_MAX_ALARM_THRESHOLD_DEFAULT 40.0   // 温度最高报警阈值（°C）
#endif

#ifndef HUMIDITY_MIN_ALARM_THRESHOLD_DEFAULT
  #define HUMIDITY_MIN_ALARM_THRESHOLD_DEFAULT 20.0  // 湿度最低报警阈值（%）
#endif

#ifndef HUMIDITY_MAX_ALARM_THRESHOLD_DEFAULT
  #define HUMIDITY_MAX_ALARM_THRESHOLD_DEFAULT 80.0  // 湿度最高报警阈值（%）
#endif

#ifndef LIGHT_ALARM_THRESHOLD_DEFAULT
  #define LIGHT_ALARM_THRESHOLD_DEFAULT    500    // 光照报警阈值
#endif

// ========================================
// 传感器时间间隔常量（毫秒）
// ========================================
#ifndef SENSOR_LOW_POWER_INTERVAL
  #define SENSOR_LOW_POWER_INTERVAL        60000UL  // 低功耗模式更新间隔：60 秒
#endif

#ifndef SENSOR_NORMAL_INTERVAL
  #define SENSOR_NORMAL_INTERVAL           5000UL   // 正常模式更新间隔：5 秒
#endif

// ========================================
// 传感器数据平滑常量
// ========================================
#ifndef TEMP_HISTORY_SIZE
  #define TEMP_HISTORY_SIZE                10       // 温度历史记录大小
#endif

#ifndef TEMP_HISTORY_WINDOW
  #define TEMP_HISTORY_WINDOW              5        // 温度历史计算窗口
#endif

#ifndef TEMP_SMOOTHING_FACTOR
  #define TEMP_SMOOTHING_FACTOR            0.2f     // 温度平滑系数
#endif



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
  
  // 获取传感器数据
  int getGasLevel() { return currentData.gasLevel; }
  int getLightLevel() { return currentData.lightLevel; }
  bool getMotionDetected() { return currentData.motionDetected; }
  bool getFlameDetected() { return currentData.flameDetected; }
  
private:
  // 传感器配置
  SensorConfig currentConfig;
  
  // 传感器数据
  SensorData currentData;
  
  // 传感器驱动
  // Security: Use smart pointer to prevent memory leaks and double-free
  std::unique_ptr<ISensorDriver> sensorDriver;
  
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
  
  // 传感器读取方法
  bool readDHT22();
  bool readSHT30();
  bool readDHT11();
  bool readSHT21();
  bool readAM2302();
  bool readHDC1080();
  bool readDHT12();
  bool readSHT40();
  bool readBME280();
  bool readBME680();
  bool readGasSensor();
  bool readFlameSensor();
  bool readLightSensor();
  bool readPIRSensor();
};

#endif // SENSOR_MANAGER_H