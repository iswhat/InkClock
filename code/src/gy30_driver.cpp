#include "gy30_driver.h"

// GY30传感器命令定义
#define GY30_POWER_DOWN 0x00  // 关闭传感器
#define GY30_POWER_ON 0x01     // 开启传感器
#define GY30_RESET 0x07        // 重置传感器
#define GY30_SINGLE_HRES 0x20  // 单次高分辨率测量
#define GY30_SINGLE_HRES2 0x21 // 单次高分辨率测量（2）
#define GY30_SINGLE_LRES 0x23  // 单次低分辨率测量
#define GY30_CONT_HRES 0x10     // 连续高分辨率测量
#define GY30_CONT_HRES2 0x11    // 连续高分辨率测量（2）
#define GY30_CONT_LRES 0x13     // 连续低分辨率测量

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
GY30Driver::GY30Driver() {
  typeName = "GY30";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
  address = 0x23;  // 默认I2C地址
}

/**
 * @brief 发送命令到GY30传感器
 * 
 * @param cmd 命令字节
 */
void GY30Driver::sendCommand(uint8_t cmd) {
  Wire.beginTransmission(address);
  Wire.write(cmd);
  Wire.endTransmission();
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool GY30Driver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 如果配置中指定了地址，则使用配置的地址
  if (config.address != 0) {
    address = config.address;
  }
  
  // 初始化Wire库
  Wire.begin();
  
  // 开启传感器
  sendCommand(GY30_POWER_ON);
  delay(100);
  
  initialized = true;
  
  Serial.printf("GY30传感器初始化成功，I2C地址: 0x%02X\n", address);
  
  return true;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool GY30Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 发送单次高分辨率测量命令
  sendCommand(GY30_SINGLE_HRES);
  delay(180);  // 等待测量完成（高分辨率模式需要180ms）
  
  // 读取2字节数据
  Wire.requestFrom(address, 2);
  if (Wire.available() == 2) {
    uint8_t highByte = Wire.read();
    uint8_t lowByte = Wire.read();
    
    // 计算光照强度（lux）
    uint16_t rawData = (highByte << 8) | lowByte;
    float lux = rawData / 1.2;
    
    // 设置传感器数据
    data.valid = true;
    data.timestamp = millis();
    data.lightLevel = static_cast<int>(lux);  // 将光照强度保存到lightLevel字段
    
    return true;
  } else {
    Serial.println("GY30传感器数据读取失败");
    return false;
  }
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void GY30Driver::calibrate(float tempOffset, float humOffset) {
  // GY30传感器不需要温湿度校准，所以这里不做任何操作
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String GY30Driver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType GY30Driver::getType() const {
  return SENSOR_TYPE_LIGHT_GY30;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void GY30Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig GY30Driver::getConfig() const {
  return config;
}
