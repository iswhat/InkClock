#ifndef HC_SR501_DRIVER_H
#define HC_SR501_DRIVER_H

#include "sensor_driver.h"

class HC_SR501Driver : public ISensorDriver {
public:
  HC_SR501Driver();
  ~HC_SR501Driver() override;
  
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
  SensorConfig config;
  int pin;
  float tempOffset;
  float humOffset;
  bool initialized;
};

#endif // HC_SR501_DRIVER_H