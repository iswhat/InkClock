#include "am2302_driver.h"

AM2302Driver::AM2302Driver() : dht(nullptr), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

AM2302Driver::~AM2302Driver() {
  // 析构函数，清理资源
  if (dht != nullptr) {
    delete dht;
    dht = nullptr;
  }
}

bool AM2302Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 使用配置中的引脚，或默认引脚
  int pin = (config.pin != -1) ? config.pin : DHT_PIN;
  
  // 创建DHT对象，AM2302实际上是DHT22的防水版本
  dht = new DHT(pin, DHT22);
  
  // 初始化DHT传感器
  dht->begin();
  
  initialized = true;
  return true;
}

bool AM2302Driver::readData(SensorData& data) {
  if (!initialized || dht == nullptr) {
    return false;
  }
  
  // 读取温湿度数据
  float h = dht->readHumidity();
  float t = dht->readTemperature();
  
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
  data.motionDetected = false; // AM2302不支持人体感应
  data.gasLevel = 0; // AM2302不支持气体检测
  data.flameDetected = false; // AM2302不支持火焰检测
  data.lightLevel = 0; // AM2302不支持光照检测
  
  return true;
}

void AM2302Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

String AM2302Driver::getTypeName() const {
  return "AM2302温湿度传感器";
}

SensorType AM2302Driver::getType() const {
  return SENSOR_TYPE_AM2302;
}

void AM2302Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    delete dht;
    dht = nullptr;
    init(config);
  }
}

SensorConfig AM2302Driver::getConfig() const {
  return config;
}