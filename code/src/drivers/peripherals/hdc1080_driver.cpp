#include "hdc1080_driver.h"

HDC1080Driver::HDC1080Driver() : hdc1080(nullptr), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

HDC1080Driver::~HDC1080Driver() {
  // 析构函数，清理资源
  if (hdc1080 != nullptr) {
    delete hdc1080;
    hdc1080 = nullptr;
  }
}

bool HDC1080Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 创建HDC1080对象
  hdc1080 = new Adafruit_HDC1080();
  
  // 初始化HDC1080传感器
  if (!hdc1080->begin()) {
    delete hdc1080;
    hdc1080 = nullptr;
    return false;
  }
  
  initialized = true;
  return true;
}

bool HDC1080Driver::readData(SensorData& data) {
  if (!initialized || hdc1080 == nullptr) {
    return false;
  }
  
  // 读取温湿度数据
  float t = hdc1080->readTemperature();
  float h = hdc1080->readHumidity();
  
  // 检查数据是否有效
  if (isnan(h) || isnan(t)) {
    return false;
  }
  
  // 应用校准偏移量
  t += tempOffset;
  h += humOffset;
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = platformGetMillis();
  data.temperature = t;
  data.humidity = h;
  data.motionDetected = false; // HDC1080不支持人体感应
  data.gasLevel = 0; // HDC1080不支持气体检测
  data.flameDetected = false; // HDC1080不支持火焰检测
  data.lightLevel = 0; // HDC1080不支持光照检测
  
  return true;
}

void HDC1080Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

String HDC1080Driver::getTypeName() const {
  return "HDC1080温湿度传感器";
}

SensorType HDC1080Driver::getType() const {
  return SENSOR_TYPE_HDC1080;
}

void HDC1080Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    delete hdc1080;
    hdc1080 = nullptr;
    init(config);
  }
}

SensorConfig HDC1080Driver::getConfig() const {
  return config;
}