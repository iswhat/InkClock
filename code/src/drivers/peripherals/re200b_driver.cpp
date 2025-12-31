#include "re200b_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
RE200BDriver::RE200BDriver() {
  typeName = "RE200B";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
  threshold = 512;      // 默认阈值，可根据实际情况调整
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool RE200BDriver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 设置传感器引脚为输入
  pinMode(config.pin, INPUT);
  initialized = true;
  
  // 如果配置了光照阈值，则使用配置的阈值
  if (config.lightThreshold > 0) {
    threshold = config.lightThreshold;
  }
  
  Serial.printf("RE200B人体感应传感器初始化成功，引脚: %d，阈值: %d\n", config.pin, threshold);
  
  return true;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool RE200BDriver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取模拟值
  int analogValue = analogRead(config.pin);
  
  // 判断是否检测到人体
  bool motionDetected = (analogValue > threshold);
  
  // 设置传感器数据
  data.valid = true;
  data.timestamp = platformGetMillis();
  data.motionDetected = motionDetected;
  data.lightLevel = analogValue;  // 将模拟值保存到lightLevel字段
  
  return true;
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void RE200BDriver::calibrate(float tempOffset, float humOffset) {
  // RE200B传感器不需要温湿度校准，所以这里不做任何操作
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String RE200BDriver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType RE200BDriver::getType() const {
  return SENSOR_TYPE_RE200B;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void RE200BDriver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig RE200BDriver::getConfig() const {
  return config;
}
