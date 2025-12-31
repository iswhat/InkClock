#include "tsl2561_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
TSL2561Driver::TSL2561Driver() {
  typeName = "TSL2561";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool TSL2561Driver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 初始化TSL2561传感器
  bool success = tsl2561.begin(config.address);
  initialized = success;
  
  if (success) {
    Serial.printf("TSL2561传感器初始化成功，I2C地址: 0x%02X\n", config.address);
    
    // 配置TSL2561的测量参数
    tsl2561.enableAutoRange(true);  // 自动范围
    tsl2561.setIntegrationTime(TSL2561_INTEGRATIONTIME_13MS);  // 13ms积分时间
  } else {
    Serial.printf("TSL2561传感器初始化失败，I2C地址: 0x%02X\n", config.address);
  }
  
  return success;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool TSL2561Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取光照数据
  sensors_event_t event;
  tsl2561.getEvent(&event);
  
  // 检查数据是否有效
  if (event.light > 0) {
    // 设置传感器数据
    data.valid = true;
    data.timestamp = platformGetMillis();
    data.lightLevel = static_cast<int>(event.light);  // 将光照强度保存到lightLevel字段
    
    return true;
  } else {
    Serial.println("TSL2561传感器数据无效");
    return false;
  }
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void TSL2561Driver::calibrate(float tempOffset, float humOffset) {
  // TSL2561传感器不需要温湿度校准，所以这里不做任何操作
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String TSL2561Driver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType TSL2561Driver::getType() const {
  return SENSOR_TYPE_LIGHT_TSL2561;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void TSL2561Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig TSL2561Driver::getConfig() const {
  return config;
}

bool TSL2561Driver::matchHardware() {
  DEBUG_PRINTLN("检测TSL2561硬件匹配...");
  
  try {
    // TSL2561使用I2C接口，通常有两个地址可选：0x29（默认）和0x39（AD0接地）
    uint8_t addresses[] = {0x29, 0x39};
    bool matched = false;
    
    for (uint8_t address : addresses) {
      // 尝试初始化TSL2561传感器
      if (tsl2561.begin(address)) {
        // 初始化成功，尝试读取一次数据验证
        sensors_event_t event;
        tsl2561.getEvent(&event);
        
        if (event.light >= 0) {
          // 数据有效，硬件匹配成功
          DEBUG_PRINTF("TSL2561硬件匹配成功，I2C地址: 0x%02X\n", address);
          matched = true;
          break;
        }
      }
    }
    
    if (!matched) {
      DEBUG_PRINTLN("未检测到TSL2561硬件");
    }
    
    return matched;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("TSL2561硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("TSL2561硬件匹配未知错误");
    return false;
  }
}
