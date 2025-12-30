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

bool AM2302Driver::matchHardware() {
  DEBUG_PRINTLN("检测AM2302硬件匹配...");
  
  try {
    // AM2302使用单总线接口，需要检测特定引脚
    // 尝试常见的引脚
    int testPins[] = {4, 5, 12, 13, 14, 15, 25, 26, 27, 32, 33}; // 常见的GPIO引脚
    
    for (int pin : testPins) {
      // 创建临时DHT对象进行检测
      DHT* tempDht = new DHT(pin, DHT22);
      tempDht->begin();
      
      // 等待传感器稳定
      delay(2000);
      
      // 读取温湿度数据
      float h = tempDht->readHumidity();
      float t = tempDht->readTemperature();
      
      delete tempDht;
      
      // 检查数据是否有效
      if (!isnan(h) && !isnan(t)) {
        DEBUG_PRINTF("AM2302硬件匹配成功，引脚: %d\n", pin);
        return true;
      }
    }
    
    DEBUG_PRINTLN("未检测到AM2302硬件");
    return false;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("AM2302硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("AM2302硬件匹配未知错误");
    return false;
  }
}