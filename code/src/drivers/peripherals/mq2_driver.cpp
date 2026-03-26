#include "mq2_driver.h"

/**
 * @brief 获取清洁空气中的Rs/R0比值
 * 
 * MQ2在清洁空气中的Rs/R0比值约为9.83
 * 
 * @return 清洁空气中的比值
 */
float MQ2Driver::getCleanAirRatio() {
  return 9.83;
}

/**
 * @brief 计算特定气体的浓度
 * 
 * MQ2主要检测可燃气体，如甲烷、丙烷等
 * 使用经验公式计算气体浓度
 * 
 * @param ratio Rs/R0比值
 * @return 气体浓度（ppm）
 */
float MQ2Driver::calculateSpecificGasConcentration(float ratio) {
  // 对于MQ2，甲烷浓度的经验公式
  // 浓度 = 613.9 * ratio^-2.074
  float concentration = 613.9 * pow(ratio, -2.074);
  return concentration;
}

/**
 * @brief 根据传感器类型设置气体浓度值
 * 
 * MQ2主要检测可燃气体，所以设置gasLevel字段
 * 
 * @param data 传感器数据结构
 * @param concentration 气体浓度
 */
void MQ2Driver::setGasConcentration(SensorData& data, float concentration) {
  data.gasLevel = (int)concentration;
}