#include "base_sensor_driver.h"

BaseSensorDriver::BaseSensorDriver() {
  initialized = false;
  working = false;
  tempOffset = 0.0;
  humOffset = 0.0;
  errorCount = 0;
  lastSuccessReadTime = 0;
}

BaseSensorDriver::~BaseSensorDriver() {
  // 析构函数，清理资源
  // 子类应该重写此方法以清理特定资源
}

bool BaseSensorDriver::init(const SensorConfig& config) {
  if (!isValidConfig(config)) {
    return false;
  }
  
  this->config = config;
  initialized = true;
  working = true;
  errorCount = 0;
  lastSuccessReadTime = millis();
  
  return true;
}

void BaseSensorDriver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
}

void BaseSensorDriver::setConfig(const SensorConfig& config) {
  if (isValidConfig(config)) {
    this->config = config;
  }
}

SensorConfig BaseSensorDriver::getConfig() const {
  return config;
}

bool BaseSensorDriver::isInitialized() const {
  return initialized;
}

bool BaseSensorDriver::reset() {
  errorCount = 0;
  working = true;
  lastSuccessReadTime = millis();
  return true;
}

bool BaseSensorDriver::isWorking() const {
  // 如果连续10次读取失败，认为传感器不工作
  return working && errorCount < 10;
}

bool BaseSensorDriver::isValidConfig(const SensorConfig& config) const {
  // 基本配置有效性检查
  if (config.type == SENSOR_TYPE_AUTO_DETECT) {
    return false; // 自动检测类型不能用于初始化
  }
  
  // 检查更新间隔是否合理
  if (config.updateInterval < 100) { // 最小100ms
    return false;
  }
  
  return true;
}

void BaseSensorDriver::recordError() {
  errorCount++;
  
  // 如果连续10次读取失败，标记传感器为不工作
  if (errorCount >= 10) {
    working = false;
  }
}

void BaseSensorDriver::recordSuccess() {
  errorCount = 0;
  working = true;
  lastSuccessReadTime = millis();
}

bool BaseSensorDriver::matchHardware() {
  DEBUG_PRINTLN("检测传感器硬件匹配...");
  
  try {
    // 创建一个默认配置用于硬件检测
    SensorConfig defaultConfig;
    defaultConfig.type = getType();
    defaultConfig.pin = -1; // 默认值，子类可以根据需要修改
    defaultConfig.address = 0; // 默认值，子类可以根据需要修改
    defaultConfig.updateInterval = 60000;
    defaultConfig.tempOffset = 0.0;
    defaultConfig.humOffset = 0.0;
    
    // 尝试初始化传感器来检测硬件
    bool initResult = init(defaultConfig);
    
    if (initResult) {
      // 初始化成功，尝试读取一次数据
      SensorData testData;
      bool readResult = readData(testData);
      
      // 重置传感器状态，因为这只是检测，不是真正的初始化
      initialized = false;
      working = false;
      
      // 如果初始化成功且能读取到有效数据，说明硬件匹配
      return readResult && testData.valid;
    }
    
    return false;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("传感器硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("传感器硬件匹配失败: 未知异常");
    return false;
  }
}
