#include "bme680_driver.h"

BME680Driver::BME680Driver() : bme680(nullptr), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

BME680Driver::~BME680Driver() {
  // 析构函数，清理资源
  if (bme680 != nullptr) {
    delete bme680;
    bme680 = nullptr;
  }
}

bool BME680Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 创建BME680对象
  bme680 = new Adafruit_BME680();
  
  // 使用配置中的地址，或默认地址
  uint8_t address = (config.address != 0x00) ? config.address : 0x76;
  
  // 初始化BME680传感器
  if (!bme680->begin(address)) {
    // 尝试另一个常见地址
    if (!bme680->begin(0x77)) {
      delete bme680;
      bme680 = nullptr;
      return false;
    }
  }
  
  // 配置BME680传感器
  // 设置过滤系数
  bme680->setTemperatureOversampling(BME680_OS_8X);
  bme680->setHumidityOversampling(BME680_OS_2X);
  bme680->setPressureOversampling(BME680_OS_4X);
  bme680->setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme680->setGasHeater(320, 150); // 320°C for 150 ms
  
  initialized = true;
  return true;
}

bool BME680Driver::readData(SensorData& data) {
  if (!initialized || bme680 == nullptr) {
    return false;
  }
  
  // 读取传感器数据
  if (!bme680->performReading()) {
    return false;
  }
  
  // 应用校准偏移量
  float t = bme680->temperature + tempOffset;
  float h = bme680->humidity + humOffset;
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = millis();
  data.temperature = t;
  data.humidity = h;
  data.motionDetected = false; // BME680不支持人体感应
  data.gasLevel = (int)bme680->gas_resistance / 1000; // 转换为更易读的值
  data.flameDetected = false; // BME680不支持火焰检测
  data.lightLevel = 0; // BME680不支持光照检测
  
  return true;
}

void BME680Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

String BME680Driver::getTypeName() const {
  return "BME680温湿度气压气体传感器";
}

SensorType BME680Driver::getType() const {
  return SENSOR_TYPE_BME680;
}

void BME680Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    delete bme680;
    bme680 = nullptr;
    init(config);
  }
}

SensorConfig BME680Driver::getConfig() const {
  return config;
}