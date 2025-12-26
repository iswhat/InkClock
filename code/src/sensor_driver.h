#ifndef SENSOR_DRIVER_H
#define SENSOR_DRIVER_H

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
  
  // 获取传感器类型
  virtual SensorType getType() const = 0;
  
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
