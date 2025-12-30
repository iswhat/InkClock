#include "sht30_driver.h"

SHT30Driver::SHT30Driver() : sht30(nullptr), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

SHT30Driver::~SHT30Driver() {
  // 析构函数，清理资源
  if (sht30 != nullptr) {
    delete sht30;
    sht30 = nullptr;
  }
}

bool SHT30Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 创建SHT30对象
  sht30 = new Adafruit_SHT31();
  
  // 使用配置中的地址，或默认地址
  uint8_t address = (config.address != 0x00) ? config.address : 0x44;
  
  // 初始化SHT30传感器
  if (!sht30->begin(address)) {
    delete sht30;
    sht30 = nullptr;
    return false;
  }
  
  initialized = true;
  return true;
}

bool SHT30Driver::readData(SensorData& data) {
  if (!initialized || sht30 == nullptr) {
    return false;
  }
  
  // 读取温湿度数据
  float h = sht30->readHumidity();
  float t = sht30->readTemperature();
  
  // 检查数据是否有效
  if (isnan(h) || isnan(t)) {
    return false;
  }
  
  // 应用校准偏移量
  t += tempOffset;
  h += humOffset;
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = millis();
  data.temperature = t;
  data.humidity = h;
  data.motionDetected = false; // SHT30不支持人体感应
  data.gasLevel = 0; // SHT30不支持气体检测
  data.flameDetected = false; // SHT30不支持火焰检测
  data.lightLevel = 0; // SHT30不支持光照检测
  
  return true;
}

void SHT30Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

String SHT30Driver::getTypeName() const {
  return "SHT30温湿度传感器";
}

SensorType SHT30Driver::getType() const {
  return SENSOR_TYPE_SHT30;
}

void SHT30Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    delete sht30;
    sht30 = nullptr;
    init(config);
  }
}

SensorConfig SHT30Driver::getConfig() const {
  return config;
}