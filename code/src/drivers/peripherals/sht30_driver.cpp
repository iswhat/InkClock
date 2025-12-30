#include "sht30_driver.h"

SHT30Driver::SHT30Driver() : sht30(nullptr) {
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
  // 调用基类初始化
  if (!BaseSensorDriver::init(config)) {
    return false;
  }
  
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
  
  return true;
}

bool SHT30Driver::readData(SensorData& data) {
  if (!isInitialized() || sht30 == nullptr) {
    recordError();
    return false;
  }
  
  // 读取温湿度数据
  float h = sht30->readHumidity();
  float t = sht30->readTemperature();
  
  // 检查数据是否有效
  if (isnan(h) || isnan(t)) {
    recordError();
    return false;
  }
  
  // 应用校准偏移量
  t += tempOffset;
  h += humOffset;
  
  // 使用通用方法填充传感器数据
  fillSensorData(data, t, h);
  
  recordSuccess();
  return true;
}

String SHT30Driver::getTypeName() const {
  return "SHT30温湿度传感器";
}

SensorType SHT30Driver::getType() const {
  return SENSOR_TYPE_SHT30;
}

bool SHT30Driver::matchHardware() {
  DEBUG_PRINTLN("检测SHT30硬件匹配...");
  
  try {
    // SHT30使用I2C接口，通常有两个地址可选：0x44和0x45
    uint8_t addresses[] = {0x44, 0x45};
    bool matched = false;
    
    // 尝试创建SHT30对象并检测两个可能的地址
    Adafruit_SHT31* tempSht30 = new Adafruit_SHT31();
    
    for (uint8_t address : addresses) {
      if (tempSht30->begin(address)) {
        // 初始化成功，尝试读取一次数据验证
        float h = tempSht30->readHumidity();
        float t = tempSht30->readTemperature();
        if (!isnan(h) && !isnan(t)) {
          matched = true;
          break;
        }
      }
    }
    
    delete tempSht30;
    return matched;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("SHT30硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("SHT30硬件匹配失败: 未知异常");
    return false;
  }
}