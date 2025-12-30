#include "ld2410_driver.h"

/**
 * @brief 构造函数
 * 
 * 初始化传感器类型名称和初始化状态。
 */
LD2410Driver::LD2410Driver() {
  typeName = "LD2410";  // 设置传感器类型名称
  initialized = false;  // 初始化为未初始化状态
  detectionDistance = 0;  // 初始检测距离为0
  motionDetected = false;  // 初始未检测到人体
  serial = nullptr;
}

/**
 * @brief 初始化传感器
 * 
 * @param config 传感器配置
 * @return 初始化是否成功
 */
bool LD2410Driver::init(const SensorConfig& config) {
  // 保存配置
  this->config = config;
  
  // 初始化串口
  serial = &Serial1;  // 使用Serial1作为默认串口
  serial->begin(256000, SERIAL_8N1, 16, 17);  // 默认使用GPIO16和GPIO17作为TX和RX
  
  initialized = true;
  
  Serial.printf("LD2410毫米波雷达传感器初始化成功\n");
  
  return true;
}

/**
 * @brief 读取传感器数据
 * 
 * @param data 传感器数据结构，用于存储读取到的数据
 * @return 读取是否成功
 */
bool LD2410Driver::readData(SensorData& data) {
  if (!initialized || serial == nullptr) {
    return false;
  }
  
  // 检查串口是否有数据
  if (serial->available() > 0) {
    // 读取串口数据
    uint8_t buffer[32];
    size_t len = serial->readBytes(buffer, sizeof(buffer));
    
    // 解析LD2410数据
    parseLD2410Data(buffer, len);
  }
  
  // 设置传感器数据
  data.valid = true;
  data.timestamp = millis();
  data.motionDetected = motionDetected;
  data.lightLevel = detectionDistance;  // 将检测距离保存到lightLevel字段
  
  return true;
}

/**
 * @brief 校准传感器
 * 
 * @param tempOffset 温度偏移量
 * @param humOffset 湿度偏移量
 */
void LD2410Driver::calibrate(float tempOffset, float humOffset) {
  // LD2410传感器不需要温湿度校准，所以这里不做任何操作
}

/**
 * @brief 获取传感器类型名称
 * 
 * @return 传感器类型名称
 */
String LD2410Driver::getTypeName() const {
  return typeName;
}

/**
 * @brief 获取传感器类型
 * 
 * @return 传感器类型枚举值
 */
SensorType LD2410Driver::getType() const {
  return SENSOR_TYPE_LD2410;
}

/**
 * @brief 设置传感器配置
 * 
 * @param config 传感器配置
 */
void LD2410Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  // 重新初始化传感器
  init(config);
}

/**
 * @brief 获取传感器配置
 * 
 * @return 传感器配置
 */
SensorConfig LD2410Driver::getConfig() const {
  return config;
}

/**
 * @brief 解析LD2410传感器的串口数据
 * 
 * @param data 串口数据
 * @param len 数据长度
 */
void LD2410Driver::parseLD2410Data(uint8_t* data, size_t len) {
  // 简单的LD2410数据解析，实际解析逻辑需要根据LD2410的通信协议进行调整
  // 这里只是一个示例，实际使用时需要根据LD2410的协议进行完整的解析
  
  // 检查数据头
  if (len >= 4 && data[0] == 0xF4 && data[1] == 0xF3 && data[2] == 0xF2 && data[3] == 0xF1) {
    // 解析检测结果
    if (len >= 8) {
      motionDetected = (data[4] & 0x01) == 0x01;
      detectionDistance = (data[5] << 8) | data[6];
    }
  }
}
