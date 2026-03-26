#include "mq135_driver.h"

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType MQ135Driver::getType() const {
  return SENSOR_TYPE_MQ135;
}

/**
 * @brief 获取清洁空气中的Rs/R0比值
 * 
 * MQ135在清洁空气中的Rs/R0比值约为3.6
 * 
 * @return 清洁空气中的比值
 */
float MQ135Driver::getCleanAirRatio() {
  return 3.6;
}

/**
 * @brief 计算特定气体的浓度
 * 
 * MQ135主要检测CO2、甲醛、苯等有害气体
 * 使用经验公式计算气体浓度
 * 
 * @param ratio Rs/R0比值
 * @return 气体浓度（ppm）
 */
float MQ135Driver::calculateSpecificGasConcentration(float ratio) {
  // 对于MQ135，CO2浓度的经验公式
  // 浓度 = 116.6020682 * ratio^-2.769034857
  float concentration = 116.6020682 * pow(ratio, -2.769034857);
  return concentration;
}

/**
 * @brief 根据传感器类型设置气体浓度值
 * 
 * MQ135主要检测CO2，所以设置co2字段
 * 
 * @param data 传感器数据结构
 * @param concentration 气体浓度
 */
void MQ135Driver::setGasConcentration(SensorData& data, float concentration) {
  data.co2 = concentration;
  data.gasLevel = (int)concentration;
}
