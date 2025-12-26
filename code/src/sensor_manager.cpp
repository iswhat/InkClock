#include "sensor_manager.h"

// 传感器类型选择（可根据需要修改）
// 支持的传感器类型：DHT11, DHT22, AM2302, SHT30, SHT21, HDC1080
#define SENSOR_TYPE SENSOR_SHT30

// 传感器类型定义
enum SensorType {
  SENSOR_DHT11,
  SENSOR_DHT22,
  SENSOR_AM2302,
  SENSOR_SHT30,
  SENSOR_SHT21,
  SENSOR_HDC1080,
  SENSOR_DHT12,        // DHT12温湿度传感器
  SENSOR_SHT40,        // SHT40温湿度传感器
  SENSOR_BME280,       // BME280温湿度气压传感器
  SENSOR_BME680        // BME680温湿度气压气体传感器
};

// 根据传感器类型包含相应的库
#if SENSOR_TYPE == SENSOR_DHT11 || SENSOR_TYPE == SENSOR_DHT22 || SENSOR_TYPE == SENSOR_AM2302 || SENSOR_TYPE == SENSOR_DHT12
  #include <DHT.h>
  #if SENSOR_TYPE == SENSOR_DHT11
    DHT dht(DHT_PIN, DHT11);
  #elif SENSOR_TYPE == SENSOR_DHT12
    DHT dht(DHT_PIN, DHT12); // DHT12使用专用驱动
  #else
    DHT dht(DHT_PIN, DHT22); // DHT22和AM2302使用相同的驱动
  #endif
  #define USE_DHT_SENSOR
#elif SENSOR_TYPE == SENSOR_SHT30
  #include <Adafruit_SHT31.h>
  Adafruit_SHT31 sht30 = Adafruit_SHT31();
  #define USE_SHT30_SENSOR
#elif SENSOR_TYPE == SENSOR_SHT21
  #include <Adafruit_SHT21.h>
  Adafruit_SHT21 sht21 = Adafruit_SHT21();
  #define USE_SHT21_SENSOR
#elif SENSOR_TYPE == SENSOR_HDC1080
  #include <Adafruit_HDC1080.h>
  Adafruit_HDC1080 hdc1080 = Adafruit_HDC1080();
  #define USE_HDC1080_SENSOR
#elif SENSOR_TYPE == SENSOR_SHT40
  #include <Adafruit_SHT4x.h>
  Adafruit_SHT4x sht40 = Adafruit_SHT4x();
  #define USE_SHT40_SENSOR
#elif SENSOR_TYPE == SENSOR_BME280
  #include <Adafruit_BME280.h>
  Adafruit_BME280 bme280;
  #define USE_BME280_SENSOR
#elif SENSOR_TYPE == SENSOR_BME680
  #include <Adafruit_BME680.h>
  Adafruit_BME680 bme680;
  #define USE_BME680_SENSOR
#endif

SensorManager::SensorManager() {
  // 初始化传感器数据
  currentData.temperature = 0.0;
  currentData.humidity = 0.0;
  currentData.valid = false;
  currentData.timestamp = 0;
  
  // 初始化校准偏移量
  tempOffset = 0.0;
  humOffset = 0.0;
  
  lastUpdate = 0;
  dataUpdated = false;
}

SensorManager::~SensorManager() {
  // 清理资源
  #if defined(USE_SHT30_SENSOR)
    // 关闭SHT30传感器
    sht30.end();
    DEBUG_PRINTLN("SHT30 sensor cleaned up");
  #elif defined(USE_SHT21_SENSOR)
    // 关闭SHT21传感器
    sht21.end();
    DEBUG_PRINTLN("SHT21 sensor cleaned up");
  #elif defined(USE_HDC1080_SENSOR)
    // HDC1080传感器无需特殊清理
    DEBUG_PRINTLN("HDC1080 sensor cleaned up");
  #elif defined(USE_SHT40_SENSOR)
    // SHT40传感器无需特殊清理
    DEBUG_PRINTLN("SHT40 sensor cleaned up");
  #elif defined(USE_BME280_SENSOR)
    // BME280传感器无需特殊清理
    DEBUG_PRINTLN("BME280 sensor cleaned up");
  #elif defined(USE_BME680_SENSOR)
    // BME680传感器无需特殊清理
    DEBUG_PRINTLN("BME680 sensor cleaned up");
  #endif
}

void SensorManager::init() {
  DEBUG_PRINTLN("初始化传感器管理器...");
  
  // 初始化传感器
  #if defined(USE_DHT_SENSOR)
    // 初始化DHT系列传感器
    dht.begin();
    #if SENSOR_TYPE == SENSOR_DHT11
      DEBUG_PRINTLN("DHT11传感器初始化完成");
    #elif SENSOR_TYPE == SENSOR_DHT22
      DEBUG_PRINTLN("DHT22传感器初始化完成");
    #elif SENSOR_TYPE == SENSOR_AM2302
      DEBUG_PRINTLN("AM2302传感器初始化完成");
    #elif SENSOR_TYPE == SENSOR_DHT12
      DEBUG_PRINTLN("DHT12传感器初始化完成");
    #endif
  #elif defined(USE_SHT30_SENSOR)
    // 初始化SHT30传感器
    if (!sht30.begin(SHT30_ADDRESS)) {
      DEBUG_PRINTLN("SHT30传感器初始化失败");
      currentData.valid = false;
    } else {
      DEBUG_PRINTLN("SHT30传感器初始化完成");
    }
  #elif defined(USE_SHT21_SENSOR)
    // 初始化SHT21传感器
    if (!sht21.begin()) {
      DEBUG_PRINTLN("SHT21传感器初始化失败");
      currentData.valid = false;
    } else {
      DEBUG_PRINTLN("SHT21传感器初始化完成");
    }
  #elif defined(USE_HDC1080_SENSOR)
    // 初始化HDC1080传感器
    hdc1080.begin();
    DEBUG_PRINTLN("HDC1080传感器初始化完成");
  #elif defined(USE_SHT40_SENSOR)
    // 初始化SHT40传感器
    if (!sht40.begin()) {
      DEBUG_PRINTLN("SHT40传感器初始化失败");
      currentData.valid = false;
    } else {
      DEBUG_PRINTLN("SHT40传感器初始化完成");
    }
  #elif defined(USE_BME280_SENSOR)
    // 初始化BME280传感器
    if (!bme280.begin(0x76)) {
      DEBUG_PRINTLN("BME280传感器初始化失败");
      currentData.valid = false;
    } else {
      DEBUG_PRINTLN("BME280传感器初始化完成");
    }
  #elif defined(USE_BME680_SENSOR)
    // 初始化BME680传感器
    if (!bme680.begin(0x77)) {
      DEBUG_PRINTLN("BME680传感器初始化失败");
      currentData.valid = false;
    } else {
      DEBUG_PRINTLN("BME680传感器初始化完成");
    }
  #endif
  
  DEBUG_PRINTLN("传感器管理器初始化完成");
}

void SensorManager::update() {
  bool success = false;
  
  // 根据传感器类型选择读取方法
  #if defined(USE_DHT_SENSOR)
    #if SENSOR_TYPE == SENSOR_DHT11
      success = readDHT11();
    #elif SENSOR_TYPE == SENSOR_DHT22
      success = readDHT22();
    #elif SENSOR_TYPE == SENSOR_AM2302
      success = readAM2302();
    #elif SENSOR_TYPE == SENSOR_DHT12
      success = readDHT12();
    #endif
  #elif defined(USE_SHT30_SENSOR)
    success = readSHT30();
  #elif defined(USE_SHT21_SENSOR)
    success = readSHT21();
  #elif defined(USE_HDC1080_SENSOR)
    success = readHDC1080();
  #elif defined(USE_SHT40_SENSOR)
    success = readSHT40();
  #elif defined(USE_BME280_SENSOR)
    success = readBME280();
  #elif defined(USE_BME680_SENSOR)
    success = readBME680();
  #endif
  
  if (success) {
    // 应用校准偏移量
    currentData.temperature += tempOffset;
    currentData.humidity += humOffset;
    
    // 数据有效性检查
    if (currentData.temperature < -40 || currentData.temperature > 80) {
      // 温度超出合理范围
      currentData.valid = false;
    } else if (currentData.humidity < 0 || currentData.humidity > 100) {
      // 湿度超出合理范围
      currentData.valid = false;
    } else {
      // 数据有效
      currentData.valid = true;
      currentData.timestamp = millis();
      dataUpdated = true;
      
      DEBUG_PRINT("传感器数据更新成功: 温度 = ");
      DEBUG_PRINT(currentData.temperature);
      DEBUG_PRINT("°C, 湿度 = ");
      DEBUG_PRINT(currentData.humidity);
      DEBUG_PRINTLN("%");
    }
  } else {
    currentData.valid = false;
    DEBUG_PRINTLN("传感器数据读取失败");
  }
  
  lastUpdate = millis();
}

void SensorManager::loop() {
  // 定期更新传感器数据
  if (millis() - lastUpdate > SENSOR_UPDATE_INTERVAL) {
    update();
  }
}

void SensorManager::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
  
  DEBUG_PRINT("传感器校准完成: 温度偏移 = ");
  DEBUG_PRINT(tempOffset);
  DEBUG_PRINT("°C, 湿度偏移 = ");
  DEBUG_PRINT(humOffset);
  DEBUG_PRINTLN("%");
}

bool SensorManager::readDHT22() {
  // 读取DHT22传感器数据
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // 检查读数是否有效
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("DHT22传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = t;
  currentData.humidity = h;
  
  return true;
}

bool SensorManager::readSHT30() {
  // 读取SHT30传感器数据
  float t = sht30.readTemperature();
  float h = sht30.readHumidity();
  
  // 检查读数是否有效
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("SHT30传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = t;
  currentData.humidity = h;
  
  return true;
}

bool SensorManager::readDHT11() {
  // 读取DHT11传感器数据
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // 检查读数是否有效
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("DHT11传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = t;
  currentData.humidity = h;
  
  return true;
}

bool SensorManager::readSHT21() {
  // 读取SHT21传感器数据
  float t = sht21.readTemperature();
  float h = sht21.readHumidity();
  
  // 检查读数是否有效
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("SHT21传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = t;
  currentData.humidity = h;
  
  return true;
}

bool SensorManager::readAM2302() {
  // AM2302和DHT22使用相同的读取方式
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // 检查读数是否有效
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("AM2302传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = t;
  currentData.humidity = h;
  
  return true;
}

bool SensorManager::readHDC1080() {
  // 读取HDC1080传感器数据
  float t = hdc1080.readTemperature();
  float h = hdc1080.readHumidity();
  
  // 检查读数是否有效
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("HDC1080传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = t;
  currentData.humidity = h;
  
  return true;
}

bool SensorManager::readDHT12() {
  // 读取DHT12传感器数据
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  
  // 检查读数是否有效
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("DHT12传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = t;
  currentData.humidity = h;
  
  return true;
}

bool SensorManager::readSHT40() {
  // 读取SHT40传感器数据
  sensors_event_t humidity, temp;
  
  if (!sht40.getEvent(&humidity, &temp)) {
    DEBUG_PRINTLN("SHT40传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = temp.temperature;
  currentData.humidity = humidity.relative_humidity;
  
  return true;
}

bool SensorManager::readBME280() {
  // 读取BME280传感器数据
  float t = bme280.readTemperature();
  float h = bme280.readHumidity();
  
  // 检查读数是否有效
  if (isnan(h) || isnan(t)) {
    DEBUG_PRINTLN("BME280传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = t;
  currentData.humidity = h;
  
  return true;
}

bool SensorManager::readBME680() {
  // 读取BME680传感器数据
  if (!bme680.performReading()) {
    DEBUG_PRINTLN("BME680传感器读数无效");
    return false;
  }
  
  // 更新数据
  currentData.temperature = bme680.temperature;
  currentData.humidity = bme680.humidity;
  
  return true;
}

void SensorManager::filterData() {
  // 简单的移动平均滤波
  // 这里可以根据需要实现更复杂的滤波算法
  static float tempHistory[10] = {0};
  static float humHistory[10] = {0};
  static int historyIndex = 0;
  
  // 添加新数据到历史记录
  tempHistory[historyIndex] = currentData.temperature;
  humHistory[historyIndex] = currentData.humidity;
  
  // 计算移动平均值
  float tempSum = 0;
  float humSum = 0;
  
  for (int i = 0; i < 10; i++) {
    tempSum += tempHistory[i];
    humSum += humHistory[i];
  }
  
  // 更新滤波后的数据
  currentData.temperature = tempSum / 10;
  currentData.humidity = humSum / 10;
  
  // 更新历史记录索引
  historyIndex = (historyIndex + 1) % 10;
}