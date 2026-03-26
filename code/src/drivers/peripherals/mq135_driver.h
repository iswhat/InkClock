#ifndef MQ135_DRIVER_H
#define MQ135_DRIVER_H

#include "base_mq_sensor_driver.h"

/**
 * @brief MQ135气体传感器驱动类
 * 
 * 该类实现了MQ135气体传感器的驱动，用于检测甲醛、苯、甲苯、NH3、NOx等多种有害气体。
 * MQ135是一种模拟输出的气体传感器。
 */
class MQ135Driver : public BaseMQSensorDriver {
public:
  /**
   * @brief 构造函数
   * 
   * 初始化传感器类型名称和初始化状态。
   */
  MQ135Driver() : BaseMQSensorDriver("MQ135") {
  }

  /**
   * @brief 获取传感器类型
   * 
   * @return 传感器类型枚举值
   */
  SensorType getType() const override;
  
protected:
  /**
   * @brief 获取清洁空气中的Rs/R0比值
   * 
   * @return 清洁空气中的比值
   */
  float getCleanAirRatio() override;
  
  /**
   * @brief 计算特定气体的浓度
   * 
   * @param ratio Rs/R0比值
   * @return 气体浓度
   */
  float calculateSpecificGasConcentration(float ratio) override;
  
  /**
   * @brief 根据传感器类型设置气体浓度值
   * 
   * @param data 传感器数据结构
   * @param concentration 气体浓度
   */
  void setGasConcentration(SensorData& data, float concentration) override;
};

#endif // MQ135_DRIVER_H
