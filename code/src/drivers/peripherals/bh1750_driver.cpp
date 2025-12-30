#include "bh1750_driver.h"

BH1750Driver::BH1750Driver() : bh1750(nullptr), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

BH1750Driver::~BH1750Driver() {
  // 析构函数，清理资源
  if (bh1750 != nullptr) {
    delete bh1750;
    bh1750 = nullptr;
  }
}

bool BH1750Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 创建BH1750对象
  bh1750 = new BH1750();
  
  // 使用配置中的地址，或默认地址
  uint8_t address = (config.address != 0x00) ? config.address : BH1750_ADDRESS;
  
  // 初始化BH1750传感器
  if (!bh1750->begin(BH1750::CONTINUOUS_HIGH_RES_MODE, address)) {
    delete bh1750;
    bh1750 = nullptr;
    return false;
  }
  
  initialized = true;
  return true;
}

bool BH1750Driver::readData(SensorData& data) {
  if (!initialized || bh1750 == nullptr) {
    return false;
  }
  
  // 读取光照数据
  float lux = bh1750->readLightLevel();
  
  // 检查数据是否有效
  if (isnan(lux)) {
    return false;
  }
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = millis();
  data.temperature = 0.0; // BH1750不支持温度检测
  data.humidity = 0.0; // BH1750不支持湿度检测
  data.motionDetected = false; // BH1750不支持人体感应
  data.gasLevel = 0; // BH1750不支持气体检测
  data.flameDetected = false; // BH1750不支持火焰检测
  data.lightLevel = (int)lux; // 转换为整数
  
  return true;
}

void BH1750Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
  // BH1750不需要校准，这里只是为了满足接口要求
}

String BH1750Driver::getTypeName() const {
  return "BH1750光照传感器";
}

SensorType BH1750Driver::getType() const {
  return SENSOR_TYPE_LIGHT_BH1750;
}

void BH1750Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    delete bh1750;
    bh1750 = nullptr;
    init(config);
  }
}

SensorConfig BH1750Driver::getConfig() const {
  return config;
}