#ifndef DRIVER_REGISTRY_H
#define DRIVER_REGISTRY_H

#include <Arduino.h>
#include <vector>
#include "config.h"
#include "display_driver.h"
#include "sensor_driver.h"

// 驱动注册表类，用于管理所有可用的驱动
class DriverRegistry {
private:
  // 单例实例
  static DriverRegistry* instance;
  
  // 私有构造函数，防止外部实例化
  DriverRegistry() {}
  
  // 驱动列表
  std::vector<ISensorDriver*> sensorDrivers;
  std::vector<IDisplayDriver*> displayDrivers;
  
public:
  // 获取单例实例
  static DriverRegistry* getInstance() {
    if (instance == nullptr) {
      instance = new DriverRegistry();
    }
    return instance;
  }
  
  // 注册传感器驱动
  void registerSensorDriver(ISensorDriver* driver) {
    sensorDrivers.push_back(driver);
  }
  
  // 注册显示驱动
  void registerDisplayDriver(IDisplayDriver* driver) {
    displayDrivers.push_back(driver);
  }
  
  // 获取所有传感器驱动
  std::vector<ISensorDriver*> getSensorDrivers() {
    return sensorDrivers;
  }
  
  // 获取所有显示驱动
  std::vector<IDisplayDriver*> getDisplayDrivers() {
    return displayDrivers;
  }
  
  // 根据传感器类型获取驱动
  ISensorDriver* getSensorDriver(SensorType type) {
    for (auto driver : sensorDrivers) {
      if (driver->getType() == type) {
        return driver;
      }
    }
    return nullptr;
  }
  
  // 自动检测并返回合适的传感器驱动
  ISensorDriver* autoDetectSensorDriver() {
    // 定义常见传感器的默认引脚和地址配置
    struct SensorDefaultConfig {
      SensorType type;
      int defaultPin;
      uint8_t defaultAddress;
      uint8_t alternateAddresses[3];
    };
    
    // 常见传感器的默认配置
    SensorDefaultConfig commonConfigs[] = {
      // 温湿度传感器
      {SENSOR_TYPE_DHT11, DHT_PIN, 0x00, {0x00, 0x00, 0x00}},           // DHT11 温湿度传感器
      {SENSOR_TYPE_DHT22, DHT_PIN, 0x00, {0x00, 0x00, 0x00}},           // DHT22/AM2302 温湿度传感器
      {SENSOR_TYPE_SHT30, -1, 0x44, {0x45, 0x46, 0x00}},               // Sensirion SHT30 温湿度传感器
      {SENSOR_TYPE_SHT21, -1, 0x40, {0x00, 0x00, 0x00}},               // Sensirion SHT21 温湿度传感器
      {SENSOR_TYPE_SHT40, -1, 0x44, {0x00, 0x00, 0x00}},               // Sensirion SHT40 温湿度传感器
      {SENSOR_TYPE_AM2302, DHT_PIN, 0x00, {0x00, 0x00, 0x00}},         // AOSONG AM2302 温湿度传感器
      {SENSOR_TYPE_HDC1080, -1, 0x40, {0x00, 0x00, 0x00}},             // TI HDC1080 温湿度传感器
      {SENSOR_TYPE_HTU21D, -1, 0x40, {0x00, 0x00, 0x00}},              // Measurement Specialties HTU21D 温湿度传感器
      {SENSOR_TYPE_SI7021, -1, 0x40, {0x00, 0x00, 0x00}},              // Silicon Labs SI7021 温湿度传感器
      {SENSOR_TYPE_BME280, -1, 0x76, {0x77, 0x00, 0x00}},              // Bosch BME280 温湿度气压传感器
      {SENSOR_TYPE_BME680, -1, 0x76, {0x77, 0x00, 0x00}},              // Bosch BME680 温湿度气压气体传感器
      {SENSOR_TYPE_BME280, -1, 0x76, {0x77, 0x00, 0x00}},              // Bosch BME280 温湿度气压传感器
      
      // 光照传感器
      {SENSOR_TYPE_LIGHT_BH1750, -1, 0x23, {0x5C, 0x00, 0x00}},       // Rohm BH1750 光照传感器
      {SENSOR_TYPE_BH1750, -1, 0x23, {0x5C, 0x00, 0x00}},             // Rohm BH1750 光照传感器
      {SENSOR_TYPE_LIGHT_TSL2561, -1, 0x39, {0x29, 0x49, 0x00}},      // ams TSL2561 光照传感器
      {SENSOR_TYPE_LIGHT_GY30, -1, 0x23, {0x5C, 0x00, 0x00}},         // 杭州晶华 GY30 光照传感器
      {SENSOR_TYPE_LIGHT_SI1145, -1, 0x60, {0x00, 0x00, 0x00}},       // Silicon Labs SI1145 光照传感器
      
      // 人体感应传感器
      {SENSOR_TYPE_PIR, PIR_SENSOR_PIN, 0x00, {0x00, 0x00, 0x00}},    // HC-SR501/HC-SR505 人体感应传感器
      {SENSOR_TYPE_RE200B, PIR_SENSOR_PIN, 0x00, {0x00, 0x00, 0x00}},  // Excelitas RE200B 红外人体传感器
      
      // 气体传感器
      {SENSOR_TYPE_GAS_MQ2, GAS_SENSOR_PIN, 0x00, {0x00, 0x00, 0x00}}, // 武汉敏芯 MQ-2 气体传感器
      {SENSOR_TYPE_GAS_MQ5, GAS_SENSOR_PIN, 0x00, {0x00, 0x00, 0x00}}, // 深圳炜盛 MQ-5 气体传感器
      {SENSOR_TYPE_GAS_MQ7, GAS_SENSOR_PIN, 0x00, {0x00, 0x00, 0x00}}, // 广州汉威 MQ-7 气体传感器
      {SENSOR_TYPE_GAS_MQ135, GAS_SENSOR_PIN, 0x00, {0x00, 0x00, 0x00}}, // 郑州炜盛 MQ-135 气体传感器
      {SENSOR_TYPE_GAS_TGS2600, GAS_SENSOR_PIN, 0x00, {0x00, 0x00, 0x00}}, // Figaro TGS2600 气体传感器
      
      // 火焰传感器
      {SENSOR_TYPE_FLAME_IR, FLAME_SENSOR_PIN, 0x00, {0x00, 0x00, 0x00}}, // 深圳捷顺 IR 火焰传感器
    };
    
    // 尝试初始化每个传感器驱动
    for (auto driver : sensorDrivers) {
      SensorType driverType = driver->getType();
      
      // 查找该传感器类型的默认配置
      SensorConfig config;
      config.type = driverType;
      config.pin = -1;
      config.address = 0x00;
      config.tempOffset = 0.0;
      config.humOffset = 0.0;
      config.updateInterval = SENSOR_UPDATE_INTERVAL;
      
      // 为常见传感器设置默认配置
      for (auto& commonConfig : commonConfigs) {
        if (commonConfig.type == driverType) {
          config.pin = commonConfig.defaultPin;
          config.address = commonConfig.defaultAddress;
          break;
        }
      }
      
      // 尝试使用默认配置初始化
      if (driver->init(config)) {
        return driver;
      }
      
      // 如果默认配置失败，尝试其他可能的地址（针对I2C传感器）
      if (config.pin == -1) { // I2C传感器
        for (auto& commonConfig : commonConfigs) {
          if (commonConfig.type == driverType) {
            for (uint8_t addr : commonConfig.alternateAddresses) {
              if (addr != 0x00) {
                config.address = addr;
                if (driver->init(config)) {
                  return driver;
                }
              }
            }
            break;
          }
        }
      }
      
      // 对于单总线传感器，尝试默认的DHT引脚
      if (driverType == SENSOR_TYPE_DHT11 || driverType == SENSOR_TYPE_DHT22 || 
          driverType == SENSOR_TYPE_DHT12 || driverType == SENSOR_TYPE_AM2302) {
        config.pin = DHT_PIN;
        if (driver->init(config)) {
          return driver;
        }
      }
    }
    
    return nullptr;
  }
  
  // 根据显示类型获取驱动
  IDisplayDriver* getDisplayDriver(EinkDisplayType type) {
    for (auto driver : displayDrivers) {
      if (driver->getType() == type) {
        return driver;
      }
    }
    return nullptr;
  }
  
  // 自动检测并返回合适的显示驱动
  IDisplayDriver* autoDetectDisplayDriver() {
    // 尝试初始化每个显示驱动，返回第一个成功的
    for (auto driver : displayDrivers) {
      if (driver->init()) {
        return driver;
      }
    }
    return nullptr;
  }
  
  // 清除所有驱动
  void clear() {
    for (auto driver : sensorDrivers) {
      delete driver;
    }
    sensorDrivers.clear();
    
    for (auto driver : displayDrivers) {
      delete driver;
    }
    displayDrivers.clear();
  }
};

// 初始化单例实例
DriverRegistry* DriverRegistry::instance = nullptr;

// 驱动注册宏
template <typename T>
void registerSensorDriver() {
  DriverRegistry::getInstance()->registerSensorDriver(new T());
}

template <typename T>
void registerDisplayDriver() {
  DriverRegistry::getInstance()->registerDisplayDriver(new T());
}

#endif // DRIVER_REGISTRY_H
