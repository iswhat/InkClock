#pragma once

#include <Arduino.h>
#include "sensor_driver.h"

// 基础传感器驱动类，提供通用功能
class BaseSensorDriver : public ISensorDriver {
public:
  BaseSensorDriver();
  virtual ~BaseSensorDriver();
  
  // 初始化传感器
  virtual bool init(const SensorConfig& config) override;
  
  // 读取传感器数据（基类不实现，由子类实现）
  virtual bool readData(SensorData& data) override = 0;
  
  // 校准传感器
  virtual void calibrate(float tempOffset, float humOffset) override;
  
  // 获取传感器类型名称
  virtual String getTypeName() const override = 0;
  
  // 获取传感器类型
  virtual SensorType getType() const override = 0;
  
  // 设置传感器配置
  virtual void setConfig(const SensorConfig& config) override;
  
  // 获取当前配置
  virtual SensorConfig getConfig() const override;
  
  // 获取传感器状态
  bool isInitialized() const;
  
  // 重置传感器
  bool reset();
  
  // 检查传感器是否正常工作
  bool isWorking() const;
  
  // 检测驱动与硬件是否匹配
  virtual bool matchHardware() override;
  
protected:
  // 传感器配置
  SensorConfig config;
  
  // 传感器状态
  bool initialized;
  bool working;
  
  // 校准偏移量
  float tempOffset;
  float humOffset;
  
  // 错误计数
  int errorCount;
  
  // 上次成功读取时间
  unsigned long lastSuccessReadTime;
  
  // 检查配置是否有效
  bool isValidConfig(const SensorConfig& config) const;
  
  // 记录传感器错误
  void recordError();
  
  // 记录传感器成功
  void recordSuccess();
  
  // 填充传感器数据的通用方法
  void fillSensorData(SensorData& data, float temperature = 0.0, float humidity = 0.0, 
                     bool motionDetected = false, float gasLevel = 0.0,
                     bool flameDetected = false, float lightLevel = 0.0);
};
