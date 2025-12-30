#ifndef SHT30_DRIVER_H
#define SHT30_DRIVER_H

#include "base_sensor_driver.h"
#include <Adafruit_SHT31.h>

class SHT30Driver : public BaseSensorDriver {
public:
  SHT30Driver();
  ~SHT30Driver() override;
  
  // 初始化传感器
  bool init(const SensorConfig& config) override;
  
  // 读取传感器数据
  bool readData(SensorData& data) override;
  
  // 获取传感器类型名称
  String getTypeName() const override;
  
  // 获取传感器类型
  SensorType getType() const override;
  
private:
  Adafruit_SHT31* sht30;
};

#endif // SHT30_DRIVER_H