#include "lps25hb_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
LPS25HBDriver::LPS25HBDriver() {
  typeName = "LPS25HB";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool LPS25HBDriver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 初始化LPS25HB传感器
  bool success = lps25hb.begin(config.address);
  initialized = success;
  
  if (success) {
    Serial.printf("LPS25HB传感器初始化成功，I2C地址: 0x%02X\n", config.address);
  } else {
    Serial.printf("LPS25HB传感器初始化失败，I2C地址: 0x%02X\n", config.address);
  }
  
  return success;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool LPS25HBDriver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取气压和温度数据
  float pressure = lps25hb.readPressure();
  float temperature = lps25hb.readTemperature();
  
  // 检查数据是否有效
  if (isnan(temperature) || isnan(pressure)) {
    Serial.println("LPS25HB传感器数据无效");
    return false;
  }
  
  // 应用校准偏移量
  temperature += config.tempOffset;
  
  // 设置传感器数据
  data.valid = true;
  data.timestamp = millis();
  data.temperature = temperature;
  // LPS25HB没有湿度数据，所以保留默认值
  
  return true;
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void LPS25HBDriver::calibrate(float tempOffset, float humOffset) {
  config.tempOffset = tempOffset;
  config.humOffset = humOffset;
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String LPS25HBDriver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType LPS25HBDriver::getType() const {
  // 使用BME280类型，因为它们都是气压传感器
  return SENSOR_TYPE_BME280;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void LPS25HBDriver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig LPS25HBDriver::getConfig() const {
  return config;
}
