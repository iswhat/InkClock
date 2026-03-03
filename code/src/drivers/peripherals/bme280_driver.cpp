#include "bme280_driver.h"

BME280Driver::BME280Driver() {
  // 构造函数
#ifdef HAVE_BME280_LIB
  bme280 = nullptr;
#endif
}

BME280Driver::~BME280Driver() {
  // 析构函数，清理资源
#ifdef HAVE_BME280_LIB
  if (bme280 != nullptr) {
    delete bme280;
    bme280 = nullptr;
  }
#endif
}

bool BME280Driver::init(const SensorConfig& config) {
  // 调用基类初始化
  if (!BaseSensorDriver::init(config)) {
    return false;
  }
  
#ifdef HAVE_BME280_LIB
  // 创建BME280对象
  bme280 = new Adafruit_BME280();
  
  // 使用配置中的地址，或默认地址
  uint8_t address = (config.address != 0x00) ? config.address : 0x76;
  
  // 初始化BME280传感器
  if (!bme280->begin(address)) {
    // 尝试另一个常见地址
    if (!bme280->begin(0x77)) {
      delete bme280;
      bme280 = nullptr;
      return false;
    }
  }
  
  return true;
#else
  DEBUG_PRINTLN("BME280驱动: Adafruit_BME280库不可用");
  return false;
#endif
}

bool BME280Driver::readData(SensorData& data) {
  if (!isInitialized()) {
    recordError();
    return false;
  }
  
#ifdef HAVE_BME280_LIB
  if (bme280 == nullptr) {
    recordError();
    return false;
  }
  
  // 读取温湿度数据
  float t = bme280->readTemperature();
  float h = bme280->readHumidity();
  
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
#else
  DEBUG_PRINTLN("BME280驱动: 读取功能不可用");
  recordError();
  return false;
#endif
}

String BME280Driver::getTypeName() const {
  return "BME280温湿度气压传感器";
}

SensorType BME280Driver::getType() const {
  return SENSOR_TYPE_BME280;
}

bool BME280Driver::matchHardware() {
  DEBUG_PRINTLN("检测BME280硬件匹配...");
  
#ifdef HAVE_BME280_LIB
  try {
    // 创建临时BME280对象
    Adafruit_BME280 tempBME280;
    
    // 尝试在常见地址初始化BME280传感器
    bool found = false;
    
    // 尝试地址0x76
    if (tempBME280.begin(0x76)) {
      found = true;
      DEBUG_PRINTLN("BME280硬件匹配成功（地址：0x76）");
    } 
    // 尝试地址0x77
    else if (tempBME280.begin(0x77)) {
      found = true;
      DEBUG_PRINTLN("BME280硬件匹配成功（地址：0x77）");
    } 
    else {
      DEBUG_PRINTLN("BME280硬件匹配失败：未在I2C总线上检测到设备");
      found = false;
    }
    
    return found;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("BME280硬件匹配失败：" + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("BME280硬件匹配失败：未知异常");
    return false;
  }
#else
  DEBUG_PRINTLN("BME280驱动: 硬件检测功能不可用");
  return false;
#endif
}