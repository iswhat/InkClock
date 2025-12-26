#ifndef LPS25HB_DRIVER_H
#define LPS25HB_DRIVER_H

#include "sensor_driver.h"
#include <Adafruit_LPS25HB.h>

/**
 * @brief LPS25HB气压传感器驱动类
 * 
 * 该类实现了LPS25HB气压传感器的驱动，用于获取环境气压、温度数据。
 * LPS25HB是一种高精度、低功耗的数字气压传感器，采用I2C/SPI接口通信。
 */
class LPS25HBDriver : public ISensorDriver {
private:
  Adafruit_LPS25HB lps25hb;       ///< LPS25HB传感器实例
  SensorConfig config;           ///< 传感器配置
  bool initialized;              ///< 初始化状态标志
  String typeName;               ///< 传感器类型名称

public:
  /**
   * @brief 构造函数
   * 
   * 初始化传感器类型名称和初始化状态。
   */
  LPS25HBDriver();

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

#endif // LPS25HB_DRIVER_H
