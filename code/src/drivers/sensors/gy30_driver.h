#ifndef GY30_DRIVER_H
#define GY30_DRIVER_H

#include "sensor_driver.h"
#include <Wire.h>

/**
 * @brief GY30光照传感器驱动类
 * 
 * 该类实现了GY30光照传感器的驱动，用于测量环境光照强度。
 * GY30是一种高精度的光照传感器，采用I2C接口通信。
 */
class GY30Driver : public ISensorDriver {
private:
  SensorConfig config;           ///< 传感器配置
  bool initialized;              ///< 初始化状态标志
  String typeName;               ///< 传感器类型名称
  uint8_t address;               ///< I2C地址

public:
  /**
   * @brief 构造函数
   * 
   * 初始化传感器类型名称和初始化状态。
   */
  GY30Driver();

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

private:
  /**
   * @brief 发送命令到GY30传感器
   * 
   * @param cmd 命令字节
   */
  void sendCommand(uint8_t cmd);
};

#endif // GY30_DRIVER_H
