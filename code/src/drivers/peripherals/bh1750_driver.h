#ifndef BH1750_DRIVER_H
#define BH1750_DRIVER_H

#include "base_sensor_driver.h"



// 尝试包含BH1750库
#ifdef HAVE_BH1750_LIB
#include <BH1750.h>
#endif

class BH1750Driver : public BaseSensorDriver {
public:
  BH1750Driver();
  ~BH1750Driver() override;
  
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
#ifdef HAVE_BH1750_LIB
  BH1750* bh1750;
#endif
};

#endif // BH1750_DRIVER_H