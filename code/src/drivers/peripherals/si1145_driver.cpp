#include "si1145_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
SI1145Driver::SI1145Driver() {
  typeName = "SI1145";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool SI1145Driver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 初始化SI1145传感器
  bool success = si1145.begin();
  initialized = success;
  
  if (success) {
    Serial.printf("SI1145传感器初始化成功\n");
  } else {
    Serial.println("SI1145传感器初始化失败");
  }
  
  return success;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool SI1145Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取光照数据
  uint16_t vis = si1145.readVisible();
  uint16_t ir = si1145.readIR();
  float uvIndex = si1145.readUV() / 100.0;
  
  // 设置传感器数据
  data.valid = true;
  data.timestamp = platformGetMillis();
  data.lightLevel = vis;  // 将可见光强度保存到lightLevel字段
  
  return true;
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void SI1145Driver::calibrate(float tempOffset, float humOffset) {
  // SI1145传感器不需要温湿度校准，所以这里不做任何操作
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String SI1145Driver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType SI1145Driver::getType() const {
  return SENSOR_TYPE_LIGHT_SI1145;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void SI1145Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig SI1145Driver::getConfig() const {
  return config;
}
