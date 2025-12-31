#include "sgp30_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称。
 */
SGP30Driver::SGP30Driver() : BaseSensorDriver() {
  typeName = "SGP30";  // 设置传感器类型名称
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool SGP30Driver::init(const SensorConfig& config) {
  // 调用基类初始化
  if (!BaseSensorDriver::init(config)) {
    return false;
  }
  
  // 初始化SGP30传感器
  bool success = sgp30.begin();
  if (success) {
    Serial.println("SGP30传感器初始化成功");
    // 初始化SGP30的基线校准
    sgp30.iaqInit();
  } else {
    Serial.println("SGP30传感器初始化失败");
    return false;
  }
  
  return true;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool SGP30Driver::readData(SensorData& data) {
  if (!isInitialized()) {
    recordError();
    return false;
  }
  
  // 读取SGP30数据
  if (sgp30.iaqMeasure()) {
    // 获取CO2和VOC数据
    uint16_t eco2 = sgp30.eCO2;
    uint16_t tvoc = sgp30.TVOC;
    
    // 使用基类的fillSensorData方法填充数据
    fillSensorData(data, 0.0, 0.0, false, eco2, false, 0);
    
    recordSuccess();
    return true;
  } else {
    Serial.println("SGP30传感器数据读取失败");
    recordError();
    return false;
  }
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
 * @brief 检测驱动与硬件是否匹配
 * 
 * @return 硬件是否匹配
 */
bool SGP30Driver::matchHardware() {
  DEBUG_PRINTLN("检测SGP30硬件匹配...");
  
  try {
    // 尝试初始化SGP30传感器
    bool success = sgp30.begin();
    
    if (success) {
      DEBUG_PRINTLN("SGP30硬件匹配成功");
      // 调用end()方法释放资源（如果有的话）
      // 注意：Adafruit_SGP30库可能没有end()方法，根据实际情况调整
      return true;
    } else {
      DEBUG_PRINTLN("SGP30硬件匹配失败");
      return false;
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("SGP30硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("SGP30硬件匹配未知错误");
    return false;
  }
}
