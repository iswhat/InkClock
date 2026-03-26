#include "mq5_driver.h"

/**
 * @brief 获取清洁空气中的Rs/R0比值
 * 
 * MQ5在清洁空气中的Rs/R0比值约为6.5
 * 
 * @return 清洁空气中的比值
 */
float MQ5Driver::getCleanAirRatio() {
  return 6.5;
}

/**
 * @brief 计算特定气体的浓度
 * 
 * MQ5主要检测液化石油气、天然气等可燃气体
 * 使用经验公式计算气体浓度
 * 
 * @param ratio Rs/R0比值
 * @return 气体浓度（ppm）
 */
float MQ5Driver::calculateSpecificGasConcentration(float ratio) {
  // 对于MQ5，丙烷浓度的经验公式
  // 浓度 = 500 * ratio^-2.1
  float concentration = 500 * pow(ratio, -2.1);
  return concentration;
}

/**
 * @brief 根据传感器类型设置气体浓度值
 * 
 * MQ5主要检测可燃气体，所以设置gasLevel字段
 * 
 * @param data 传感器数据结构
 * @param concentration 气体浓度
 */
void MQ5Driver::setGasConcentration(SensorData& data, float concentration) {
  data.gasLevel = (int)concentration;
}