#include "si7021_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
SI7021Driver::SI7021Driver() {
  typeName = "SI7021";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool SI7021Driver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 初始化SI7021传感器
  bool success = si7021.begin(config.address);
  initialized = success;
  
  if (success) {
    Serial.printf("SI7021传感器初始化成功，I2C地址: 0x%02X\n", config.address);
  } else {
    Serial.printf("SI7021传感器初始化失败，I2C地址: 0x%02X\n", config.address);
  }
  
  return success;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool SI7021Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取温度和湿度数据
  float temperature = si7021.readTemperature();
  float humidity = si7021.readHumidity();
  
  // 检查数据是否有效
  if (isnan(temperature) || isnan(humidity)) {
    Serial.println("SI7021传感器数据无效");
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
void SI7021Driver::calibrate(float tempOffset, float humOffset) {
  config.tempOffset = tempOffset;
  config.humOffset = humOffset;
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String SI7021Driver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType SI7021Driver::getType() const {
  return SENSOR_TYPE_SI7021;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void SI7021Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig SI7021Driver::getConfig() const {
  return config;
}
