#include "mq2_driver.h"

MQ2Driver::MQ2Driver() : BaseMQSensorDriver("MQ-2气体传感器") {
  // 构造函数
}

MQ2Driver::~MQ2Driver() {
  // 析构函数，清理资源
}

SensorType MQ2Driver::getType() const {
  return SENSOR_TYPE_GAS_MQ2;
}