#include "mq2_driver.h"

MQ2Driver::MQ2Driver() : pin(-1), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

MQ2Driver::~MQ2Driver() {
  // 析构函数，清理资源
}

bool MQ2Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 使用配置中的引脚，或默认引脚
  pin = (config.pin != -1) ? config.pin : GAS_SENSOR_PIN;
  
  // 设置引脚模式
  pinMode(pin, INPUT);
  
  initialized = true;
  return true;
}

bool MQ2Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取气体传感器数据
  int gasValue = analogRead(pin);
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = millis();
  data.temperature = 0.0; // MQ-2不支持温度检测
  data.humidity = 0.0; // MQ-2不支持湿度检测
  data.motionDetected = false; // MQ-2不支持人体感应
  data.gasLevel = gasValue; // 0-1023
  data.flameDetected = false; // MQ-2不支持火焰检测
  data.lightLevel = 0; // MQ-2不支持光照检测
  
  return true;
}

void MQ2Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
  // MQ-2不需要校准，这里只是为了满足接口要求
}

String MQ2Driver::getTypeName() const {
  return "MQ-2气体传感器";
}

SensorType MQ2Driver::getType() const {
  return SENSOR_TYPE_GAS_MQ2;
}

void MQ2Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    pinMode(pin, INPUT);
  }
}

SensorConfig MQ2Driver::getConfig() const {
  return config;
}