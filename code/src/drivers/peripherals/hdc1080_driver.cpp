#include "hdc1080_driver.h"
#include "coresystem/platform_abstraction.h"

#ifdef HAVE_HDC1080_LIB

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
  hdc1080 = new ClosedCube_HDC1080();
  
  // 初始化HDC1080传感器
  hdc1080->begin(config.address);
  
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

bool HDC1080Driver::matchHardware() {
  DEBUG_PRINTLN("检测HDC1080硬件匹配...");
  
#ifdef HAVE_HDC1080_LIB
  // 创建临时HDC1080对象
  ClosedCube_HDC1080 tempHDC1080;
  
  // 尝试初始化HDC1080传感器
  tempHDC1080.begin(0x40); // HDC1080的默认I2C地址
  uint16_t deviceId = tempHDC1080.readDeviceId();
  
  bool found = (deviceId == 0x1050); // HDC1080的设备ID
  
  if (found) {
    DEBUG_PRINTLN("HDC1080硬件匹配成功");
  } else {
    DEBUG_PRINTLN("HDC1080硬件匹配失败：未在I2C总线上检测到设备");
  }
  
  return found;
#else
  DEBUG_PRINTLN("HDC1080驱动: 硬件检测功能不可用");
  return false;
#endif
}

#endif // HAVE_HDC1080_LIB