#ifndef AM2302_DRIVER_H
#define AM2302_DRIVER_H

#include "sensor_driver.h"



// 尝试包含DHT库
#ifdef HAVE_DHT_LIB
#include <DHT.h>
#endif

class AM2302Driver : public ISensorDriver {
public:
  AM2302Driver();
  ~AM2302Driver() override;
  
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
  
  // 检测驱动与硬件是否匹配
  bool matchHardware() override;
  
private:
#ifdef HAVE_DHT_LIB
  DHT* dht;
#endif
  SensorConfig config;
  float tempOffset;
  float humOffset;
  bool initialized;
};

#endif // AM2302_DRIVER_H