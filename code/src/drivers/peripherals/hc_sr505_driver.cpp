#include "hc_sr505_driver.h"

HC_SR505Driver::HC_SR505Driver() : pin(-1), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

HC_SR505Driver::~HC_SR505Driver() {
  // 析构函数，清理资源
}

bool HC_SR505Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 使用配置中的引脚，或默认引脚
  pin = (config.pin != -1) ? config.pin : PIR_SENSOR_PIN;
  
  // 设置引脚模式
  pinMode(pin, INPUT);
  
  initialized = true;
  return true;
}

bool HC_SR505Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取人体感应数据
  bool motionDetected = digitalRead(pin);
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = platformGetMillis();
  data.temperature = 0.0; // HC-SR505不支持温度检测
  data.humidity = 0.0; // HC-SR505不支持湿度检测
  data.motionDetected = motionDetected;
  data.gasLevel = 0; // HC-SR505不支持气体检测
  data.flameDetected = false; // HC-SR505不支持火焰检测
  data.lightLevel = 0; // HC-SR505不支持光照检测
  
  return true;
}

void HC_SR505Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
  // HC-SR505不需要校准，这里只是为了满足接口要求
}

String HC_SR505Driver::getTypeName() const {
  return "HC-SR505人体感应传感器";
}

SensorType HC_SR505Driver::getType() const {
  return SENSOR_TYPE_HC_SR505;
}

void HC_SR505Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    pinMode(pin, INPUT);
  }
}

SensorConfig HC_SR505Driver::getConfig() const {
  return config;
}