#include "bme680_driver.h"

BME680Driver::BME680Driver() : bme680(nullptr) {
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
  // 调用基类初始化
  if (!BaseSensorDriver::init(config)) {
    return false;
  }
  
  // 创建BME680对象
  bme680 = new Adafruit_BME680();
  
  // 使用配置中的地址，或默认地址
  uint8_t address = (this->config.address != 0x00) ? this->config.address : 0x76;
  
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
  
  return true;
}

bool BME680Driver::readData(SensorData& data) {
  if (!isInitialized() || bme680 == nullptr) {
    recordError();
    return false;
  }
  
  // 读取传感器数据
  if (!bme680->performReading()) {
    recordError();
    return false;
  }
  
  // 应用校准偏移量
  float t = bme680->temperature + tempOffset;
  float h = bme680->humidity + humOffset;
  int gasLevel = (int)bme680->gas_resistance / 1000; // 转换为更易读的值
  
  // 使用基类的fillSensorData方法填充数据
  fillSensorData(data, t, h, false, gasLevel, false, 0);
  
  recordSuccess();
  return true;
}

String BME680Driver::getTypeName() const {
  return "BME680温湿度气压气体传感器";
}

SensorType BME680Driver::getType() const {
  return SENSOR_TYPE_BME680;
}

bool BME680Driver::matchHardware() {
  DEBUG_PRINTLN("检测BME680硬件匹配...");
  
  try {
    // BME680使用I2C接口，通常有两个地址可选：0x76和0x77
    uint8_t addresses[] = {0x76, 0x77};
    bool matched = false;
    
    // 尝试创建BME680对象并检测两个可能的地址
    Adafruit_BME680* tempBme680 = new Adafruit_BME680();
    
    for (uint8_t address : addresses) {
      if (tempBme680->begin(address)) {
        // 初始化成功，尝试执行一次读数验证
        if (tempBme680->performReading()) {
          matched = true;
          break;
        }
      }
    }
    
    delete tempBme680;
    return matched;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("BME680硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("BME680硬件匹配失败: 未知异常");
    return false;
  }
}