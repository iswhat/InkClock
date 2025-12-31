#include "htu21d_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
HTU21DDriver::HTU21DDriver() {
  typeName = "HTU21D";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool HTU21DDriver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 初始化HTU21D传感器
  bool success = htu21d.begin(config.address);
  initialized = success;
  
  if (success) {
    Serial.printf("HTU21D传感器初始化成功，I2C地址: 0x%02X\n", config.address);
  } else {
    Serial.printf("HTU21D传感器初始化失败，I2C地址: 0x%02X\n", config.address);
  }
  
  return success;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool HTU21DDriver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取温度和湿度数据
  float temperature = htu21d.readTemperature();
  float humidity = htu21d.readHumidity();
  
  // 检查数据是否有效
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("HTU21D传感器数据无效");
    return false;
  }
  
  // 应用校准偏移量
  temperature += config.tempOffset;
  humidity += config.humOffset;
  
  // 设置传感器数据
  data.valid = true;
  data.timestamp = platformGetMillis();
  data.temperature = temperature;
  data.humidity = humidity;
  
  return true;
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void HTU21DDriver::calibrate(float tempOffset, float humOffset) {
  config.tempOffset = tempOffset;
  config.humOffset = humOffset;
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String HTU21DDriver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType HTU21DDriver::getType() const {
  return SENSOR_TYPE_HTU21D;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void HTU21DDriver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig HTU21DDriver::getConfig() const {
  return config;
}
