#include "sht40_driver.h"

SHT40Driver::SHT40Driver() : sht40(nullptr), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

SHT40Driver::~SHT40Driver() {
  // 析构函数，清理资源
  if (sht40 != nullptr) {
    delete sht40;
    sht40 = nullptr;
  }
}

bool SHT40Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 创建SHT40对象
  sht40 = new Adafruit_SHT4x();
  
  // 初始化SHT40传感器
  if (!sht40->begin()) {
    delete sht40;
    sht40 = nullptr;
    return false;
  }
  
  initialized = true;
  return true;
}

bool SHT40Driver::readData(SensorData& data) {
  if (!initialized || sht40 == nullptr) {
    return false;
  }
  
  sensors_event_t humidity, temp;
  
  // 读取温湿度数据
  if (!sht40->getEvent(&humidity, &temp)) {
    return false;
  }
  
  // 应用校准偏移量
  float t = temp.temperature + tempOffset;
  float h = humidity.relative_humidity + humOffset;
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = millis();
  data.temperature = t;
  data.humidity = h;
  data.motionDetected = false; // SHT40不支持人体感应
  data.gasLevel = 0; // SHT40不支持气体检测
  data.flameDetected = false; // SHT40不支持火焰检测
  data.lightLevel = 0; // SHT40不支持光照检测
  
  return true;
}

void SHT40Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

String SHT40Driver::getTypeName() const {
  return "SHT40温湿度传感器";
}

SensorType SHT40Driver::getType() const {
  return SENSOR_TYPE_SHT40;
}

void SHT40Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    delete sht40;
    sht40 = nullptr;
    init(config);
  }
}

SensorConfig SHT40Driver::getConfig() const {
  return config;
}