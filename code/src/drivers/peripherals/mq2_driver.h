#ifndef MQ2_DRIVER_H
#define MQ2_DRIVER_H

#include "base_mq_sensor_driver.h"

class MQ2Driver : public BaseMQSensorDriver {
public:
  MQ2Driver() : BaseMQSensorDriver("MQ-2气体传感器") {
  }
  
  ~MQ2Driver() override {
  }
  
  // 获取传感器类型
  SensorType getType() const override {
    return SENSOR_TYPE_GAS_MQ2;
  }
};

#endif // MQ2_DRIVER_H