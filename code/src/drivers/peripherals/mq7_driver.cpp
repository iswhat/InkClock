#include "mq7_driver.h"

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType MQ7Driver::getType() const {
  return SENSOR_TYPE_MQ7;
}
