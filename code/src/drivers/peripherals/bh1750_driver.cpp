#include "bh1750_driver.h"

BH1750Driver::BH1750Driver() : bh1750(nullptr) {
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
  // 调用基类初始化
  if (!BaseSensorDriver::init(config)) {
    return false;
  }
  
  // 创建BH1750对象
  bh1750 = new BH1750();
  
  // 使用配置中的地址，或默认地址
  uint8_t address = (this->config.address != 0x00) ? this->config.address : BH1750_ADDRESS;
  
  // 初始化BH1750传感器
  if (!bh1750->begin(BH1750::CONTINUOUS_HIGH_RES_MODE, address)) {
    delete bh1750;
    bh1750 = nullptr;
    return false;
  }
  
  return true;
}

bool BH1750Driver::readData(SensorData& data) {
  if (!isInitialized() || bh1750 == nullptr) {
    recordError();
    return false;
  }
  
  // 读取光照数据
  float lux = bh1750->readLightLevel();
  
  // 检查数据是否有效
  if (isnan(lux)) {
    recordError();
    return false;
  }
  
  // 使用基类的fillSensorData方法填充数据
  fillSensorData(data, 0.0, 0.0, false, 0, false, (int)lux);
  
  recordSuccess();
  return true;
}

String BH1750Driver::getTypeName() const {
  return "BH1750光照传感器";
}

SensorType BH1750Driver::getType() const {
  return SENSOR_TYPE_LIGHT_BH1750;
}

bool BH1750Driver::matchHardware() {
  DEBUG_PRINTLN("检测BH1750硬件匹配...");
  
  try {
    // BH1750使用I2C接口，通常有两个地址可选：0x23和0x5C
    uint8_t addresses[] = {BH1750_ADDRESS, BH1750_ADDRESS_LOW};
    bool matched = false;
    
    // 尝试创建BH1750对象并检测两个可能的地址
    BH1750* tempBh1750 = new BH1750();
    
    for (uint8_t address : addresses) {
      if (tempBh1750->begin(BH1750::CONTINUOUS_HIGH_RES_MODE, address)) {
        // 初始化成功，尝试读取一次数据验证
        float lux = tempBh1750->readLightLevel();
        if (!isnan(lux)) {
          matched = true;
          break;
        }
      }
    }
    
    delete tempBh1750;
    return matched;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("BH1750硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("BH1750硬件匹配失败: 未知异常");
    return false;
  }
}