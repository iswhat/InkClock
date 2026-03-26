#ifndef MQ7_DRIVER_H
#define MQ7_DRIVER_H

#include "base_mq_sensor_driver.h"

/**
 * @brief MQ7气体传感器驱动类
 * 
 * 该类实现了MQ7气体传感器的驱动，用于检测一氧化碳气体。
 * MQ7是一种模拟输出的气体传感器。
 */
class MQ7Driver : public BaseMQSensorDriver {
public:
  /**
   * @brief 构造函数
   * 
   * 初始化传感器类型名称和初始化状态。
   */
  MQ7Driver() : BaseMQSensorDriver("MQ7") {
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

#endif // MQ7_DRIVER_H
