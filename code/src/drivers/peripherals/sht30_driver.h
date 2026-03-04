#ifndef SHT30_DRIVER_H
#define SHT30_DRIVER_H

#include "base_sensor_driver.h"
#ifdef HAVE_SHT31_LIB
#include <Adafruit_SHT31.h>
#endif

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
  
  // 检测驱动与硬件是否匹配
  bool matchHardware() override;
  
private:
#ifdef HAVE_SHT31_LIB
  Adafruit_SHT31* sht30;
#endif
};

#endif // SHT30_DRIVER_H