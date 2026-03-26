#ifndef MQ2_DRIVER_H
#define MQ2_DRIVER_H

#include "base_mq_sensor_driver.h"

class MQ2Driver : public BaseMQSensorDriver {
public:
  MQ2Driver() : BaseMQSensorDriver("MQ-2气体传感器") {
  }
  
  ~MQ2Driver() override {
  }
  
  // 获取传感器类型
  SensorType getType() const override {
    return SENSOR_TYPE_MQ2;
  }
  
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

#endif // MQ2_DRIVER_H