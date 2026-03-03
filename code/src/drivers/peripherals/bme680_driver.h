#ifndef BME680_DRIVER_H
#define BME680_DRIVER_H

#include "base_sensor_driver.h"



// 尝试包含Adafruit_BME680库
#ifdef HAVE_BME680_LIB
#include <Adafruit_BME680.h>
#endif

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
#ifdef HAVE_BME680_LIB
  Adafruit_BME680* bme680;
#endif
};

#endif // BME680_DRIVER_H