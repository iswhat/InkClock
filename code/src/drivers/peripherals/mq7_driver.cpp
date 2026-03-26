#include "mq7_driver.h"

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType MQ7Driver::getType() const {
  return SENSOR_TYPE_MQ7;
}

/**
 * @brief 获取清洁空气中的Rs/R0比值
 * 
 * MQ7在清洁空气中的Rs/R0比值约为27.5
 * 
 * @return 清洁空气中的比值
 */
float MQ7Driver::getCleanAirRatio() {
  return 27.5;
}

/**
 * @brief 计算特定气体的浓度
 * 
 * MQ7主要检测一氧化碳气体
 * 使用经验公式计算气体浓度
 * 
 * @param ratio Rs/R0比值
 * @return 气体浓度（ppm）
 */
float MQ7Driver::calculateSpecificGasConcentration(float ratio) {
  // 对于MQ7，一氧化碳浓度的经验公式
  // 浓度 = 4.474 * ratio^-1.453
  float concentration = 4.474 * pow(ratio, -1.453);
  return concentration;
}

/**
 * @brief 根据传感器类型设置气体浓度值
 * 
 * MQ7主要检测一氧化碳，所以设置co字段
 * 
 * @param data 传感器数据结构
 * @param concentration 气体浓度
 */
void MQ7Driver::setGasConcentration(SensorData& data, float concentration) {
  data.co = concentration;
  data.gasLevel = (int)concentration;
}