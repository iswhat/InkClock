#include "mq7_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
MQ7Driver::MQ7Driver() : BaseMQSensorDriver("MQ7") {
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType MQ7Driver::getType() const {
  return SENSOR_TYPE_GAS_MQ7;
}
