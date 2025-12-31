#ifndef BME680_DRIVER_H
#define BME680_DRIVER_H

#include "base_sensor_driver.h"
#include <Adafruit_BME680.h>

class BME680Driver : public BaseSensorDriver {
public:
  BME680Driver();
  ~BME680Driver() override;
  
  // 初始化传感器
  bool init(const SensorConfig& config) override;
  
  // 读取传感器数据
  bool readData(SensorData& data) override;
  
  // 获取传感器类型名称
  String getTypeName() const override;
  
  // 获取传感器类型
  SensorType getType() const override;
  
  // 检测驱动与硬件是否匹配
  bool matchHardware() override;
  
private:
  Adafruit_BME680* bme680;
};

#endif // BME680_DRIVER_H