#ifndef SGP30_DRIVER_H
#define SGP30_DRIVER_H

#include "base_sensor_driver.h"
#include <Adafruit_SGP30.h>

/**
 * @brief SGP30气体传感器驱动类
 * 
 * 该类实现了SGP30气体传感器的驱动，用于检测CO2和VOC气体。
 * SGP30是一种数字输出的空气质量传感器，采用I2C接口通信。
 */
class SGP30Driver : public BaseSensorDriver {
private:
  Adafruit_SGP30 sgp30;           ///< SGP30传感器实例
  String typeName;               ///< 传感器类型名称

public:
  /**
   * @brief 构造函数
   * 
   * 初始化传感器类型名称。
   */
  SGP30Driver();

  /**
   * @brief 初始化传感器
   * 
   * @param config 传感器配置
   * @return 初始化是否成功
   */
  bool init(const SensorConfig& config) override;

  /**
   * @brief 读取传感器数据
   * 
   * @param data 传感器数据结构，用于存储读取到的数据
   * @return 读取是否成功
   */
  bool readData(SensorData& data) override;

  /**
   * @brief 获取传感器类型名称
   * 
   * @return 传感器类型名称
   */
  String getTypeName() const override;

  /**
   * @brief 获取传感器类型
   * 
   * @return 传感器类型枚举值
   */
  SensorType getType() const override;

  /**
   * @brief 检测驱动与硬件是否匹配
   * 
   * @return 硬件是否匹配
   */
  bool matchHardware() override;
};

#endif // SGP30_DRIVER_H
