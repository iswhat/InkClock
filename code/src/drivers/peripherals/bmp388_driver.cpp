#include "bmp388_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
BMP388Driver::BMP388Driver() {
  typeName = "BMP388";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool BMP388Driver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 初始化BMP388传感器
  bool success = bmp388.begin_I2C(config.address);
  initialized = success;
  
  if (success) {
    Serial.printf("BMP388传感器初始化成功，I2C地址: 0x%02X\n", config.address);
    // 配置BMP388的测量参数
    bmp388.setTemperatureOversampling(BMP3_OVERSAMPLING_8X);
    bmp388.setPressureOversampling(BMP3_OVERSAMPLING_4X);
    bmp388.setIIRFilterCoeff(BMP3_IIR_FILTER_COEFF_3);
    bmp388.setOutputDataRate(BMP3_ODR_50_HZ);
  } else {
    Serial.printf("BMP388传感器初始化失败，I2C地址: 0x%02X\n", config.address);
  }
  
  return success;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool BMP388Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取气压和温度数据
  if (bmp388.performReading()) {
    float pressure = bmp388.readPressure();
    float temperature = bmp388.readTemperature();
    
    // 应用校准偏移量
    temperature += config.tempOffset;
    
    // 设置传感器数据
    data.valid = true;
    data.timestamp = millis();
    data.temperature = temperature;
    // BMP388没有湿度数据，所以保留默认值
    
    return true;
  } else {
    Serial.println("BMP388传感器数据读取失败");
    return false;
  }
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void BMP388Driver::calibrate(float tempOffset, float humOffset) {
  config.tempOffset = tempOffset;
  config.humOffset = humOffset;
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String BMP388Driver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType BMP388Driver::getType() const {
  return SENSOR_TYPE_BMP388;  // 返回正确的BMP388传感器类型
}

/**
 * @brief 检测硬件是否匹配
 * 
 * @return 硬件是否匹配
 */
bool BMP388Driver::matchHardware() {
  DEBUG_PRINTLN("检测BMP388硬件匹配...");
  
  try {
    // BMP388使用I2C接口，通常地址为0x76或0x77
    uint8_t addresses[] = {0x76, 0x77};
    bool matched = false;
    
    for (uint8_t address : addresses) {
      // 尝试使用当前地址初始化传感器
      if (bmp388.begin_I2C(address)) {
        // 初始化成功，尝试读取一次数据验证
        if (bmp388.performReading()) {
          matched = true;
          break;
        }
      }
    }
    
    return matched;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("BMP388硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("BMP388硬件匹配失败: 未知异常");
    return false;
  }
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void BMP388Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig BMP388Driver::getConfig() const {
  return config;
}
