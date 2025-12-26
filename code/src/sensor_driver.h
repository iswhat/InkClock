#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <Arduino.h>
#include "config.h"

// 传感器数据结构
typedef struct {
  float temperature; // 温度（摄氏度）
  float humidity;    // 湿度（%）
  bool valid;        // 数据是否有效
  unsigned long timestamp; // 数据采集时间戳
} SensorData;

// 传感器配置结构
typedef struct {
  int pin;             // 传感器引脚（单总线传感器）
  uint8_t address;     // I2C传感器地址
  float tempOffset;    // 温度校准偏移量
  float humOffset;     // 湿度校准偏移量
  unsigned long updateInterval; // 更新间隔（毫秒）
} SensorConfig;

// 传感器抽象接口
class ISensorDriver {
public:
  virtual ~ISensorDriver() {}
  
  // 初始化传感器
  virtual bool init(const SensorConfig& config) = 0;
  
  // 读取传感器数据
  virtual bool readData(SensorData& data) = 0;
  
  // 校准传感器
  virtual void calibrate(float tempOffset, float humOffset) = 0;
  
  // 获取传感器类型名称
  virtual String getTypeName() const = 0;
};

// 传感器驱动工厂类
template <typename T>
class SensorDriverFactory {
public:
  static ISensorDriver* create() {
    return new T();
  }
};

#endif // SENSOR_DRIVER_H
