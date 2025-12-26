#ifndef LD2410_DRIVER_H
#define LD2410_DRIVER_H

#include "sensor_driver.h"
#include <HardwareSerial.h>

/**
 * @brief LD2410毫米波雷达传感器驱动类
 * 
 * 该类实现了LD2410毫米波雷达传感器的驱动，用于检测人体移动和距离。
 * LD2410是一种高精度的毫米波雷达传感器，采用UART接口通信。
 */
class LD2410Driver : public ISensorDriver {
private:
  HardwareSerial* serial;        ///< 串口实例
  SensorConfig config;           ///< 传感器配置
  bool initialized;              ///< 初始化状态标志
  String typeName;               ///< 传感器类型名称
  int detectionDistance;         ///< 检测距离（厘米）
  bool motionDetected;           ///< 是否检测到人体移动

public:
  /**
   * @brief 构造函数
   * 
   * 初始化传感器类型名称和初始化状态。
   */
  LD2410Driver();

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
   * @brief 解析LD2410传感器的串口数据
   * 
   * @param data 串口数据
   * @param len 数据长度
   */
  void parseLD2410Data(uint8_t* data, size_t len);
};

#endif // LD2410_DRIVER_H