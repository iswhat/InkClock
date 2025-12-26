#ifndef SHT40_DRIVER_H
#define SHT40_DRIVER_H

#include "sensor_driver.h"
#include <Adafruit_SHT4x.h>

class SHT40Driver : public ISensorDriver {
public:
  SHT40Driver();
  ~SHT40Driver() override;
  
  // 初始化传感器
  bool init(const SensorConfig& config) override;
  
  // 读取传感器数据
  bool readData(SensorData& data) override;
  
  // 校准传感器
  void calibrate(float tempOffset, float humOffset) override;
  
  // 获取传感器类型名称
  String getTypeName() const override;
  
  // 获取传感器类型
  SensorType getType() const override;
  
  // 设置传感器配置
  void setConfig(const SensorConfig& config) override;
  
  // 获取当前配置
  SensorConfig getConfig() const override;
  
private:
  Adafruit_SHT4x* sht40;
  SensorConfig config;
  float tempOffset;
  float humOffset;
  bool initialized;
};

#endif // SHT40_DRIVER_H