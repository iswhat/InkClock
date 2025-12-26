#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"

// 传感器类型枚举
enum SensorType {
  SENSOR_TYPE_AUTO_DETECT,  // 自动检测传感器类型
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
  SENSOR_TYPE_SI7021
};

// 传感器数据结构
typedef struct {
  float temperature; // 温度（摄氏度）
  float humidity;    // 湿度（%）
  bool valid;        // 数据是否有效
  unsigned long timestamp; // 数据采集时间戳
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
  
private:
  // 传感器配置
  SensorConfig currentConfig;
  
  // 传感器数据
  SensorData currentData;
  
  // 更新标志
  unsigned long lastUpdate;
  bool dataUpdated;
  
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
  bool readHTU21D();
  bool readSI7021();
  void filterData();
};

#endif // SENSOR_MANAGER_H