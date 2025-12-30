#include "mq5_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
MQ5Driver::MQ5Driver() : BaseMQSensorDriver("MQ5") {
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType MQ5Driver::getType() const {
  return SENSOR_TYPE_GAS_MQ5;
}
