#include "dht22_driver.h"

DHT22Driver::DHT22Driver() : tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
#ifdef HAVE_DHT_LIB
  dht = nullptr;
#endif
}

DHT22Driver::~DHT22Driver() {
  // 析构函数，清理资源
#ifdef HAVE_DHT_LIB
  if (dht != nullptr) {
    delete dht;
    dht = nullptr;
  }
#endif
}

bool DHT22Driver::init(const SensorConfig& config) {
  this->config = config;
  
#ifdef HAVE_DHT_LIB
  // 使用配置中的引脚，或默认引脚
  int pin = (config.pin != -1) ? config.pin : DHT_PIN;
  
  // 创建DHT对象
  dht = new DHT(pin, DHT22);
  
  // 初始化DHT传感器
  dht->begin();
  
  initialized = true;
  return true;
#else
  DEBUG_PRINTLN("DHT22驱动: DHT库不可用");
  initialized = false;
  return false;
#endif
}

bool DHT22Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
#ifdef HAVE_DHT_LIB
  if (dht == nullptr) {
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
  data.timestamp = platformGetMillis();
  data.temperature = t;
  data.humidity = h;
  data.motionDetected = false; // DHT22不支持人体感应
  data.gasLevel = 0; // DHT22不支持气体检测
  data.flameDetected = false; // DHT22不支持火焰检测
  data.lightLevel = 0; // DHT22不支持光照检测
  
  return true;
#else
  DEBUG_PRINTLN("DHT22驱动: 读取功能不可用");
  return false;
#endif
}

void DHT22Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

String DHT22Driver::getTypeName() const {
  return "DHT22温湿度传感器";
}

SensorType DHT22Driver::getType() const {
  return SENSOR_TYPE_DHT22;
}

void DHT22Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
#ifdef HAVE_DHT_LIB
    delete dht;
    dht = nullptr;
#endif
    init(config);
  }
}

SensorConfig DHT22Driver::getConfig() const {
  return config;
}

bool DHT22Driver::matchHardware() {
  DEBUG_PRINTLN("检测DHT22硬件匹配...");
  
#ifdef HAVE_DHT_LIB
  try {
    // 尝试在默认引脚上初始化DHT22传感器
    DHT tempDHT(DHT_PIN, DHT22);
    tempDHT.begin();
    
    // 等待传感器稳定
    platformDelay(2000);
    
    // 尝试读取数据
    float h = tempDHT.readHumidity();
    float t = tempDHT.readTemperature();
    
    // 检查数据是否有效
    if (!isnan(h) && !isnan(t)) {
      DEBUG_PRINTLN("DHT22硬件匹配成功");
      return true;
    } else {
      DEBUG_PRINTLN("DHT22硬件匹配失败：读取数据无效");
      return false;
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("DHT22硬件匹配失败：" + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("DHT22硬件匹配失败：未知异常");
    return false;
  }
#else
  DEBUG_PRINTLN("DHT22驱动: 硬件检测功能不可用");
  return false;
#endif
}