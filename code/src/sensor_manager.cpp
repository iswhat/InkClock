#include "sensor_manager.h"

// 传感器类型定义已移至sensor_manager.h

// 传感器类型选择（默认使用自动检测模式）
#define SENSOR_TYPE SENSOR_AUTO_DETECT

// 根据传感器类型包含相应的库
#if defined(ARDUINO)
  #if SENSOR_TYPE != SENSOR_AUTO_DETECT
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
  #else
    // 自动检测模式，运行时检测传感器类型
    #include <DHT.h>
    #include <Adafruit_SHT31.h>
    #include <Adafruit_SHT21.h>
    #include <Adafruit_HDC1080.h>
    #include <Adafruit_SHT4x.h>
    #include <Adafruit_BME280.h>
    #include <Adafruit_BME680.h>
    
    // 声明传感器对象，运行时初始化
    DHT* dht = nullptr;
    Adafruit_SHT31* sht30 = nullptr;
    Adafruit_SHT21* sht21 = nullptr;
    Adafruit_HDC1080* hdc1080 = nullptr;
    Adafruit_SHT4x* sht40 = nullptr;
    Adafruit_BME280* bme280 = nullptr;
    Adafruit_BME680* bme680 = nullptr;
    
    // 检测到的传感器类型
    SensorType detectedSensorType = SENSOR_DHT22; // 默认值
  #endif
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

// 自动检测传感器类型的辅助函数
SensorType detectSensorType() {
  #if SENSOR_TYPE == SENSOR_TYPE_AUTO_DETECT
    // 首先尝试检测I2C传感器
    Wire.begin();
    
    // 检测SHT30
    Wire.beginTransmission(SHT30_ADDRESS);
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_SHT30;
    }
    
    // 检测SHT21
    Wire.beginTransmission(0x40); // SHT21地址
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_SHT21;
    }
    
    // 检测HDC1080
    Wire.beginTransmission(0x40); // HDC1080地址
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_HDC1080;
    }
    
    // 检测SHT40
    Wire.beginTransmission(0x44); // SHT40地址
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_SHT40;
    }
    
    // 检测BME280
    Wire.beginTransmission(0x76); // BME280地址1
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_BME280;
    }
    Wire.beginTransmission(0x77); // BME280地址2
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_BME280;
    }
    
    // 检测BME680
    Wire.beginTransmission(0x76); // BME680地址1
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_BME680;
    }
    Wire.beginTransmission(0x77); // BME680地址2
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_BME680;
    }
    
    // 检测HTU21D
    Wire.beginTransmission(0x40); // HTU21D地址
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_HTU21D;
    }
    
    // 检测SI7021
    Wire.beginTransmission(0x40); // SI7021地址
    if (Wire.endTransmission() == 0) {
      return SENSOR_TYPE_SI7021;
    }
    
    // 默认返回DHT22，因为它是常用的传感器
    return SENSOR_TYPE_DHT22;
  #else
    return SENSOR_TYPE;
  #endif
}

void SensorManager::init() {
  DEBUG_PRINTLN("初始化传感器管理器...");
  
  try {
    // 初始化传感器
    #if SENSOR_TYPE != SENSOR_TYPE_AUTO_DETECT
      // 固定传感器类型模式
      #if defined(USE_DHT_SENSOR)
        // 初始化DHT系列传感器
        dht.begin();
        #if SENSOR_TYPE == SENSOR_TYPE_DHT11
          DEBUG_PRINTLN("DHT11传感器初始化完成");
        #elif SENSOR_TYPE == SENSOR_TYPE_DHT22
          DEBUG_PRINTLN("DHT22传感器初始化完成");
        #elif SENSOR_TYPE == SENSOR_TYPE_AM2302
          DEBUG_PRINTLN("AM2302传感器初始化完成");
        #elif SENSOR_TYPE == SENSOR_TYPE_DHT12
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
          // 尝试另一个地址
          if (!bme280.begin(0x77)) {
            DEBUG_PRINTLN("BME280传感器初始化失败");
            currentData.valid = false;
          } else {
            DEBUG_PRINTLN("BME280传感器初始化完成");
          }
        } else {
          DEBUG_PRINTLN("BME280传感器初始化完成");
        }
      #elif defined(USE_BME680_SENSOR)
        // 初始化BME680传感器
        if (!bme680.begin(0x76)) {
          // 尝试另一个地址
          if (!bme680.begin(0x77)) {
            DEBUG_PRINTLN("BME680传感器初始化失败");
            currentData.valid = false;
          } else {
            DEBUG_PRINTLN("BME680传感器初始化完成");
          }
        } else {
          DEBUG_PRINTLN("BME680传感器初始化完成");
        }
      #endif
    #else
      // 自动检测模式
      Wire.begin();
      detectedSensorType = detectSensorType();
      
      DEBUG_PRINT("检测到传感器类型: ");
      switch (detectedSensorType) {
        case SENSOR_TYPE_DHT11: DEBUG_PRINTLN("DHT11"); break;
        case SENSOR_TYPE_DHT22: DEBUG_PRINTLN("DHT22"); break;
        case SENSOR_TYPE_AM2302: DEBUG_PRINTLN("AM2302"); break;
        case SENSOR_TYPE_DHT12: DEBUG_PRINTLN("DHT12"); break;
        case SENSOR_TYPE_SHT30: DEBUG_PRINTLN("SHT30"); break;
        case SENSOR_TYPE_SHT21: DEBUG_PRINTLN("SHT21"); break;
        case SENSOR_TYPE_SHT40: DEBUG_PRINTLN("SHT40"); break;
        case SENSOR_TYPE_HDC1080: DEBUG_PRINTLN("HDC1080"); break;
        case SENSOR_TYPE_BME280: DEBUG_PRINTLN("BME280"); break;
        case SENSOR_TYPE_BME680: DEBUG_PRINTLN("BME680"); break;
        case SENSOR_TYPE_HTU21D: DEBUG_PRINTLN("HTU21D"); break;
        case SENSOR_TYPE_SI7021: DEBUG_PRINTLN("SI7021"); break;
        default: DEBUG_PRINTLN("未知类型"); break;
      }
      
      // 根据检测到的传感器类型初始化对应传感器
      bool initSuccess = false;
      int retryCount = 0;
      const int MAX_RETRIES = 3;
      
      while (!initSuccess && retryCount < MAX_RETRIES) {
        try {
          retryCount++;
          
          switch (detectedSensorType) {
            case SENSOR_TYPE_DHT11:
              dht = new DHT(DHT_PIN, DHT11);
              dht->begin();
              initSuccess = true;
              break;
            case SENSOR_TYPE_DHT22:
            case SENSOR_TYPE_AM2302:
              dht = new DHT(DHT_PIN, DHT22);
              dht->begin();
              initSuccess = true;
              break;
            case SENSOR_TYPE_DHT12:
              dht = new DHT(DHT_PIN, DHT12);
              dht->begin();
              initSuccess = true;
              break;
            case SENSOR_TYPE_SHT30:
              sht30 = new Adafruit_SHT31();
              if (sht30->begin(SHT30_ADDRESS)) {
                initSuccess = true;
              }
              break;
            case SENSOR_TYPE_SHT21:
              sht21 = new Adafruit_SHT21();
              if (sht21->begin()) {
                initSuccess = true;
              }
              break;
            case SENSOR_TYPE_SHT40:
              sht40 = new Adafruit_SHT4x();
              if (sht40->begin()) {
                initSuccess = true;
              }
              break;
            case SENSOR_TYPE_HDC1080:
              hdc1080 = new Adafruit_HDC1080();
              hdc1080->begin();
              initSuccess = true;
              break;
            case SENSOR_TYPE_BME280:
              bme280 = new Adafruit_BME280();
              if (bme280->begin(0x76) || bme280->begin(0x77)) {
                initSuccess = true;
              }
              break;
            case SENSOR_TYPE_BME680:
              bme680 = new Adafruit_BME680();
              if (bme680->begin(0x76) || bme680->begin(0x77)) {
                initSuccess = true;
              }
              break;
            case SENSOR_TYPE_HTU21D:
              // HTU21D可以使用SHT21的驱动
              sht21 = new Adafruit_SHT21();
              if (sht21->begin()) {
                initSuccess = true;
              }
              break;
            case SENSOR_TYPE_SI7021:
              // SI7021可以使用SHT21的驱动
              sht21 = new Adafruit_SHT21();
              if (sht21->begin()) {
                initSuccess = true;
              }
              break;
          }
          
          if (initSuccess) {
            DEBUG_PRINTLN("传感器初始化成功");
          } else {
            DEBUG_PRINT("传感器初始化失败，重试 (" + String(retryCount) + "/" + String(MAX_RETRIES) + ")...");
            delay(500); // 延迟后重试
          }
        } catch (const std::exception& e) {
          DEBUG_PRINT("传感器初始化异常，重试 (" + String(retryCount) + "/" + String(MAX_RETRIES) + "): ");
          DEBUG_PRINTLN(e.what());
          delay(500); // 延迟后重试
        }
      }
      
      if (!initSuccess) {
        DEBUG_PRINTLN("传感器初始化最终失败，将使用模拟数据");
        currentData.valid = false;
      }
    #endif
    
    DEBUG_PRINTLN("传感器管理器初始化完成");
  } catch (const std::exception& e) {
    DEBUG_PRINT("传感器初始化异常: ");
    DEBUG_PRINTLN(e.what());
    currentData.valid = false;
  }
}

void SensorManager::update() {
  bool success = false;
  static int consecutiveFailures = 0;
  const int MAX_CONSECUTIVE_FAILURES = 5;
  
  try {
    // 根据传感器类型选择读取方法
    #if SENSOR_TYPE != SENSOR_TYPE_AUTO_DETECT
      // 固定传感器类型模式
      int retryCount = 0;
      const int MAX_RETRIES = 3;
      
      while (!success && retryCount < MAX_RETRIES) {
        retryCount++;
        
        #if defined(USE_DHT_SENSOR)
          #if SENSOR_TYPE == SENSOR_TYPE_DHT11
            success = readDHT11();
          #elif SENSOR_TYPE == SENSOR_TYPE_DHT22
            success = readDHT22();
          #elif SENSOR_TYPE == SENSOR_TYPE_AM2302
            success = readAM2302();
          #elif SENSOR_TYPE == SENSOR_TYPE_DHT12
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
        
        if (!success) {
          DEBUG_PRINT("传感器读取失败，重试 (" + String(retryCount) + "/" + String(MAX_RETRIES) + ")...");
          delay(200); // 延迟后重试
        }
      }
    #else
      // 自动检测模式，根据检测到的传感器类型读取数据
      int retryCount = 0;
      const int MAX_RETRIES = 3;
      
      while (!success && retryCount < MAX_RETRIES) {
        retryCount++;
        
        try {
          switch (detectedSensorType) {
            case SENSOR_TYPE_DHT11:
            case SENSOR_TYPE_DHT22:
            case SENSOR_TYPE_AM2302:
            case SENSOR_TYPE_DHT12:
              if (dht != nullptr) {
                // 读取DHT系列传感器数据
                float h = dht->readHumidity();
                float t = dht->readTemperature();
                
                if (!isnan(h) && !isnan(t)) {
                  currentData.temperature = t;
                  currentData.humidity = h;
                  success = true;
                }
              }
              break;
            case SENSOR_TYPE_SHT30:
              if (sht30 != nullptr) {
                float t = sht30->readTemperature();
                float h = sht30->readHumidity();
                
                if (!isnan(h) && !isnan(t)) {
                  currentData.temperature = t;
                  currentData.humidity = h;
                  success = true;
                }
              }
              break;
            case SENSOR_TYPE_SHT21:
            case SENSOR_TYPE_HTU21D:
            case SENSOR_TYPE_SI7021:
              if (sht21 != nullptr) {
                float t = sht21->readTemperature();
                float h = sht21->readHumidity();
                
                if (!isnan(h) && !isnan(t)) {
                  currentData.temperature = t;
                  currentData.humidity = h;
                  success = true;
                }
              }
              break;
            case SENSOR_TYPE_SHT40:
              if (sht40 != nullptr) {
                sensors_event_t humidity, temp;
                if (sht40->getEvent(&humidity, &temp)) {
                  currentData.temperature = temp.temperature;
                  currentData.humidity = humidity.relative_humidity;
                  success = true;
                }
              }
              break;
            case SENSOR_TYPE_HDC1080:
              if (hdc1080 != nullptr) {
                float t = hdc1080->readTemperature();
                float h = hdc1080->readHumidity();
                
                if (!isnan(h) && !isnan(t)) {
                  currentData.temperature = t;
                  currentData.humidity = h;
                  success = true;
                }
              }
              break;
            case SENSOR_TYPE_BME280:
              if (bme280 != nullptr) {
                float t = bme280->readTemperature();
                float h = bme280->readHumidity();
                
                if (!isnan(h) && !isnan(t)) {
                  currentData.temperature = t;
                  currentData.humidity = h;
                  success = true;
                }
              }
              break;
            case SENSOR_TYPE_BME680:
              if (bme680 != nullptr) {
                if (bme680->performReading()) {
                  currentData.temperature = bme680->temperature;
                  currentData.humidity = bme680->humidity;
                  success = true;
                }
              }
              break;
          }
        } catch (const std::exception& e) {
          DEBUG_PRINT("传感器读取异常，重试 (" + String(retryCount) + "/" + String(MAX_RETRIES) + "): ");
          DEBUG_PRINTLN(e.what());
        }
        
        if (!success) {
          DEBUG_PRINT("传感器数据读取失败，重试 (" + String(retryCount) + "/" + String(MAX_RETRIES) + ")...");
          delay(200); // 延迟后重试
        }
      }
      
      // 如果连续多次失败，尝试重新检测传感器类型
      if (!success) {
        consecutiveFailures++;
        
        if (consecutiveFailures >= MAX_CONSECUTIVE_FAILURES) {
          DEBUG_PRINTLN("传感器连续多次读取失败，尝试重新检测传感器类型...");
          consecutiveFailures = 0;
          
          // 重新检测传感器类型
          detectedSensorType = detectSensorType();
          DEBUG_PRINT("重新检测到传感器类型: ");
          switch (detectedSensorType) {
            case SENSOR_TYPE_DHT11: DEBUG_PRINTLN("DHT11"); break;
            case SENSOR_TYPE_DHT22: DEBUG_PRINTLN("DHT22"); break;
            case SENSOR_TYPE_AM2302: DEBUG_PRINTLN("AM2302"); break;
            case SENSOR_TYPE_DHT12: DEBUG_PRINTLN("DHT12"); break;
            case SENSOR_TYPE_SHT30: DEBUG_PRINTLN("SHT30"); break;
            case SENSOR_TYPE_SHT21: DEBUG_PRINTLN("SHT21"); break;
            case SENSOR_TYPE_SHT40: DEBUG_PRINTLN("SHT40"); break;
            case SENSOR_TYPE_HDC1080: DEBUG_PRINTLN("HDC1080"); break;
            case SENSOR_TYPE_BME280: DEBUG_PRINTLN("BME280"); break;
            case SENSOR_TYPE_BME680: DEBUG_PRINTLN("BME680"); break;
            case SENSOR_TYPE_HTU21D: DEBUG_PRINTLN("HTU21D"); break;
            case SENSOR_TYPE_SI7021: DEBUG_PRINTLN("SI7021"); break;
            default: DEBUG_PRINTLN("未知类型"); break;
          }
        }
      } else {
        consecutiveFailures = 0; // 重置连续失败计数
      }
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
        
        // 应用滤波
        filterData();
        
        DEBUG_PRINT("传感器数据更新成功: 温度 = ");
        DEBUG_PRINT(currentData.temperature);
        DEBUG_PRINT("°C, 湿度 = ");
        DEBUG_PRINT(currentData.humidity);
        DEBUG_PRINTLN("%");
      }
    } else {
      currentData.valid = false;
      DEBUG_PRINTLN("传感器数据读取失败");
      
      // 使用模拟数据，确保系统不会崩溃
      currentData.temperature = 25.0 + random(-5, 5);
      currentData.humidity = 50.0 + random(-10, 10);
      currentData.timestamp = millis();
      DEBUG_PRINTLN("使用模拟传感器数据");
    }
  } catch (const std::exception& e) {
    currentData.valid = false;
    DEBUG_PRINT("传感器更新异常: ");
    DEBUG_PRINTLN(e.what());
    
    // 使用模拟数据，确保系统不会崩溃
    currentData.temperature = 25.0 + random(-5, 5);
    currentData.humidity = 50.0 + random(-10, 10);
    currentData.timestamp = millis();
    DEBUG_PRINTLN("使用模拟传感器数据");
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