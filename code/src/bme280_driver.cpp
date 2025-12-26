#include "bme280_driver.h"

BME280Driver::BME280Driver() : bme280(nullptr), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

BME280Driver::~BME280Driver() {
  // 析构函数，清理资源
  if (bme280 != nullptr) {
    delete bme280;
    bme280 = nullptr;
  }
}

bool BME280Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 创建BME280对象
  bme280 = new Adafruit_BME280();
  
  // 使用配置中的地址，或默认地址
  uint8_t address = (config.address != 0x00) ? config.address : 0x76;
  
  // 初始化BME280传感器
  if (!bme280->begin(address)) {
    // 尝试另一个常见地址
    if (!bme280->begin(0x77)) {
      delete bme280;
      bme280 = nullptr;
      return false;
    }
  }
  
  initialized = true;
  return true;
}

bool BME280Driver::readData(SensorData& data) {
  if (!initialized || bme280 == nullptr) {
    return false;
  }
  
  // 读取温湿度数据
  float t = bme280->readTemperature();
  float h = bme280->readHumidity();
  
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
  data.motionDetected = false; // BME280不支持人体感应
  data.gasLevel = 0; // BME280不支持气体检测
  data.flameDetected = false; // BME280不支持火焰检测
  data.lightLevel = 0; // BME280不支持光照检测
  
  return true;
}

void BME280Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

String BME280Driver::getTypeName() const {
  return "BME280温湿度气压传感器";
}

SensorType BME280Driver::getType() const {
  return SENSOR_TYPE_BME280;
}

void BME280Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    delete bme280;
    bme280 = nullptr;
    init(config);
  }
}

SensorConfig BME280Driver::getConfig() const {
  return config;
}