#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

#include <Arduino.h>
#include "config.h"

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
  
  // 报警阈值配置
  float tempMinThreshold; // 温度下限报警阈值
  float tempMaxThreshold; // 温度上限报警阈值
  float humidityMinThreshold; // 湿度下限报警阈值
  float humidityMaxThreshold; // 湿度上限报警阈值
  int gasThreshold;    // 气体浓度报警阈值
  bool flameThreshold; // 火焰检测报警阈值
  int lightThreshold;  // 光照强度报警阈值
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
  
  // 设置传感器配置
  virtual void setConfig(const SensorConfig& config) = 0;
  
  // 获取当前配置
  virtual SensorConfig getConfig() const = 0;
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
