#include "sht21_driver.h"
#include "coresystem/platform_abstraction.h"

#ifdef HAVE_SHT21_LIB

SHT21Driver::SHT21Driver() : sht21(nullptr), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

SHT21Driver::~SHT21Driver() {
  // 析构函数，清理资源
  if (sht21 != nullptr) {
    delete sht21;
    sht21 = nullptr;
  }
}

bool SHT21Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 创建SHT21对象
  sht21 = new SHT21();
  
  // 初始化SHT21传感器
  if (!sht21->begin()) {
    delete sht21;
    sht21 = nullptr;
    return false;
  }
  
  initialized = true;
  return true;
}

bool SHT21Driver::readData(SensorData& data) {
  if (!initialized || sht21 == nullptr) {
    return false;
  }
  
  // 读取温湿度数据
  if (!sht21->read()) {
    return false;
  }
  float h = sht21->getHumidity();
  float t = sht21->getTemperature();
  
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
  data.motionDetected = false; // SHT21不支持人体感应
  data.gasLevel = 0; // SHT21不支持气体检测
  data.flameDetected = false; // SHT21不支持火焰检测
  data.lightLevel = 0; // SHT21不支持光照检测
  
  return true;
}

void SHT21Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

String SHT21Driver::getTypeName() const {
  return "SHT21温湿度传感器";
}

SensorType SHT21Driver::getType() const {
  return SENSOR_TYPE_SHT21;
}

void SHT21Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    delete sht21;
    sht21 = nullptr;
    init(config);
  }
}

SensorConfig SHT21Driver::getConfig() const {
  return config;
}

bool SHT21Driver::matchHardware() {
  DEBUG_PRINTLN("检测SHT21硬件匹配...");
  
#ifdef HAVE_SHT21_LIB
  // 创建临时SHT21对象
  SHT21 tempSHT21;
  
  // 尝试初始化SHT21传感器
  bool found = tempSHT21.begin();
  
  if (found) {
    DEBUG_PRINTLN("SHT21硬件匹配成功");
  } else {
    DEBUG_PRINTLN("SHT21硬件匹配失败：未在I2C总线上检测到设备");
  }
  
  return found;
#else
  DEBUG_PRINTLN("SHT21驱动: 硬件检测功能不可用");
  return false;
#endif
}

#endif // HAVE_SHT21_LIB