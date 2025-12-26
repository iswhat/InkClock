#ifndef HDC1080_DRIVER_H
#define HDC1080_DRIVER_H

#include "sensor_driver.h"
#include <Adafruit_HDC1080.h>

class HDC1080Driver : public ISensorDriver {
public:
  HDC1080Driver();
  ~HDC1080Driver() override;
  
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
  Adafruit_HDC1080* hdc1080;
  SensorConfig config;
  float tempOffset;
  float humOffset;
  bool initialized;
};

#endif // HDC1080_DRIVER_H