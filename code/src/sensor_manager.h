#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"

// 传感器类型枚举
enum SensorType {
  SENSOR_TYPE_AUTO_DETECT,  // 自动检测传感器类型
  
  // 温湿度传感器
  SENSOR_TYPE_DHT22,
  SENSOR_TYPE_DHT11,
  SENSOR_TYPE_DHT12,
  SENSOR_TYPE_SHT30,
  SENSOR_TYPE_SHT21,
  SENSOR_TYPE_SHT40,
  SENSOR_TYPE_AM2302,
  SENSOR_TYPE_HDC1080,
  SENSOR_TYPE_BME280,
  SENSOR_TYPE_BME680,
  SENSOR_TYPE_HTU21D,
  SENSOR_TYPE_SI7021,
  
  // 人体感应传感器
  SENSOR_TYPE_PIR,
  SENSOR_TYPE_HC_SR501,
  SENSOR_TYPE_HC_SR505,
  SENSOR_TYPE_RE200B,
  SENSOR_TYPE_LD2410,
  SENSOR_TYPE_BH1750,
  
  // 气体传感器
  SENSOR_TYPE_GAS_MQ2,
  SENSOR_TYPE_GAS_MQ5,
  SENSOR_TYPE_GAS_MQ7,
  SENSOR_TYPE_GAS_MQ8,
  SENSOR_TYPE_GAS_MQ135,
  SENSOR_TYPE_GAS_TGS2600,
  
  // 火焰传感器
  SENSOR_TYPE_FLAME_IR,
  SENSOR_TYPE_FLAME_UV,
  SENSOR_TYPE_FLAME_YG1006,
  SENSOR_TYPE_FLAME_MQ2,
  SENSOR_TYPE_FLAME_TGS2600,
  
  // 光照传感器
  SENSOR_TYPE_LIGHT_BH1750,
  SENSOR_TYPE_LIGHT_VEML6075,
  SENSOR_TYPE_LIGHT_TSL2561,
  SENSOR_TYPE_LIGHT_GY30,
  SENSOR_TYPE_LIGHT_SI1145
};

// 传感器数据结构
typedef struct {
  // 基本状态
  bool valid;        // 数据是否有效
  unsigned long timestamp; // 数据采集时间戳
  
  // 温湿度数据
  float temperature; // 温度（摄氏度）
  float humidity;    // 湿度（%）
  
  // 人体感应数据
  bool motionDetected; // 是否检测到人体移动
  
  // 气体传感器数据
  int gasLevel;      // 气体浓度（0-1023）
  
  // 火焰传感器数据
  bool flameDetected; // 是否检测到火焰
  
  // 光照传感器数据
  int lightLevel;    // 光照强度（0-1023）
} SensorData;

// 传感器配置结构
typedef struct {
  SensorType type;      // 传感器类型
  int pin;             // 传感器引脚（单总线传感器）
  uint8_t address;     // I2C传感器地址
  float tempOffset;    // 温度校准偏移量
  float humOffset;     // 湿度校准偏移量
  unsigned long updateInterval; // 更新间隔（毫秒）
} SensorConfig;

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
  bool readDHT22();
  bool readDHT11();
  bool readDHT12();
  bool readSHT30();
  bool readSHT21();
  bool readSHT40();
  bool readAM2302();
  bool readHDC1080();
  bool readBME280();
  bool readBME680();
  bool readGasSensor();
  bool readFlameSensor();
  bool readLightSensor();
  bool readPIRSensor();
  void filterData();
  void checkAlarmConditions();
  void triggerAlarm(String alarmType);
};

#endif // SENSOR_MANAGER_H