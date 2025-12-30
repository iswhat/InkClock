#include "sgp30_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
SGP30Driver::SGP30Driver() {
  typeName = "SGP30";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool SGP30Driver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 初始化SGP30传感器
  bool success = sgp30.begin();
  initialized = success;
  
  if (success) {
    Serial.println("SGP30传感器初始化成功");
    // 初始化SGP30的基线校准
    sgp30.iaqInit();
  } else {
    Serial.println("SGP30传感器初始化失败");
  }
  
  return success;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool SGP30Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取SGP30数据
  if (sgp30.iaqMeasure()) {
    // 获取CO2和VOC数据
    uint16_t eco2 = sgp30.eCO2;
    uint16_t tvoc = sgp30.TVOC;
    
    // 设置传感器数据
    data.valid = true;
    data.timestamp = millis();
    data.gasLevel = eco2;  // 将CO2值保存到gasLevel字段
    
    return true;
  } else {
    Serial.println("SGP30传感器数据读取失败");
    return false;
  }
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void SGP30Driver::calibrate(float tempOffset, float humOffset) {
  // SGP30传感器不需要温湿度校准，所以这里不做任何操作
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String SGP30Driver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType SGP30Driver::getType() const {
  // 使用MQ135类型，因为它们都是气体传感器
  return SENSOR_TYPE_GAS_MQ135;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void SGP30Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig SGP30Driver::getConfig() const {
  return config;
}
