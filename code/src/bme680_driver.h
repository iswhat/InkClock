#ifndef BME680_DRIVER_H
#define BME680_DRIVER_H

#include "sensor_driver.h"
#include <Adafruit_BME680.h>

class BME680Driver : public ISensorDriver {
public:
  BME680Driver();
  ~BME680Driver() override;
  
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
  Adafruit_BME680* bme680;
  SensorConfig config;
  float tempOffset;
  float humOffset;
  bool initialized;
};

#endif // BME680_DRIVER_H