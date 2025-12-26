#ifndef RE200B_DRIVER_H
#define RE200B_DRIVER_H

#include "sensor_driver.h"

/**
 * @brief RE200B人体感应传感器驱动类
 * 
 * 该类实现了RE200B人体感应传感器的驱动，用于检测人体移动。
 * RE200B是一种高灵敏度的红外人体感应传感器，采用模拟输出。
 */
class RE200BDriver : public ISensorDriver {
private:
  SensorConfig config;           ///< 传感器配置
  bool initialized;              ///< 初始化状态标志
  String typeName;               ///< 传感器类型名称
  int threshold;                 ///< 检测阈值

public:
  /**
   * @brief 构造函数
   * 
   * 初始化传感器类型名称和初始化状态。
   */
  RE200BDriver();

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
   * @brief 校准传感器
   * 
   * @param tempOffset 温度偏移量
   * @param humOffset 湿度偏移量
   */
  void calibrate(float tempOffset, float humOffset) override;

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
   * @brief 设置传感器配置
   * 
   * @param config 传感器配置
   */
  void setConfig(const SensorConfig& config) override;

  /**
   * @brief 获取传感器配置
   * 
   * @return 传感器配置
   */
  SensorConfig getConfig() const override;
};

#endif // RE200B_DRIVER_H
