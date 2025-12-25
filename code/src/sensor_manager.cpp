#include "sensor_manager.h"

// 根据不同的板子类型选择不同的传感器库
#if defined(ESP32_C3)
  // ESP32-C3使用DHT22传感器
  #include <DHT.h>
  DHT dht(DHT_PIN, DHT22);
#elif defined(ESP32_S3)
  // ESP32-S3使用SHT30传感器
  #include <Adafruit_SHT31.h>
  Adafruit_SHT31 sht30 = Adafruit_SHT31();
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
  #if defined(ESP32_S3)
    // 关闭SHT30传感器
    sht30.end();
  #endif
}

void SensorManager::init() {
  DEBUG_PRINTLN("初始化传感器管理器...");
  
  // 初始化传感器
  #if defined(ESP32_C3)
    // 初始化DHT22传感器
    dht.begin();
    DEBUG_PRINTLN("DHT22传感器初始化完成");
  #elif defined(ESP32_S3)
    // 初始化SHT30传感器
    if (!sht30.begin(SHT30_ADDRESS)) {
      DEBUG_PRINTLN("SHT30传感器初始化失败");
      currentData.valid = false;
    } else {
      DEBUG_PRINTLN("SHT30传感器初始化完成");
    }
  #endif
  
  DEBUG_PRINTLN("传感器管理器初始化完成");
}

void SensorManager::update() {
  bool success = false;
  
  // 读取传感器数据
  #if defined(ESP32_C3)
    success = readDHT22();
  #elif defined(ESP32_S3)
    success = readSHT30();
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