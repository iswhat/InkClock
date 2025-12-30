#include "sensor_manager.h"
#include "../coresystem/driver_registry.h"
#include "../coresystem/event_bus.h"

SensorManager::SensorManager() : sensorDriver(nullptr) {
  // 初始化传感器数据
  currentData.valid = false;
  currentData.timestamp = 0;
  currentData.temperature = 0.0;
  currentData.humidity = 0.0;
  currentData.motionDetected = false;
  currentData.gasLevel = 0;
  currentData.flameDetected = false;
  currentData.lightLevel = 0;
  
  // 初始化校准偏移量
  tempOffset = 0.0;
  humOffset = 0.0;
  
  // 初始化报警相关变量
  gasAlarmThreshold = GAS_ALARM_THRESHOLD;
  flameAlarmThreshold = FLAME_ALARM_THRESHOLD;
  tempMinAlarmThreshold = -10.0; // 默认温度下限报警阈值
  tempMaxAlarmThreshold = 40.0; // 默认温度上限报警阈值
  humidityMinAlarmThreshold = 20.0; // 默认湿度下限报警阈值
  humidityMaxAlarmThreshold = 80.0; // 默认湿度上限报警阈值
  lightAlarmThreshold = 500; // 默认光照强度报警阈值
  gasAlarmTriggered = false;
  flameAlarmTriggered = false;
  tempAlarmTriggered = false;
  humidityAlarmTriggered = false;
  lightAlarmTriggered = false;
  
  lastUpdate = 0;
  dataUpdated = false;
  
  // 初始化传感器配置标志
  pirSensorEnabled = true;
  gasSensorEnabled = true;
  flameSensorEnabled = true;
  lightSensorEnabled = true;
  
  // 订阅事件
  EVENT_SUBSCRIBE(EVENT_SENSOR_CONFIG_CHANGED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto sensorData = std::dynamic_pointer_cast<SensorConfigEventData>(data);
    if (sensorData) {
      setSensorConfig(sensorData->config);
    }
  }, "SensorManager");
  
  EVENT_SUBSCRIBE(EVENT_DRIVER_REGISTERED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto driverData = std::dynamic_pointer_cast<DriverEventData>(data);
    if (driverData && driverData->driverType == DRIVER_TYPE_SENSOR) {
      // 传感器驱动注册，尝试重新检测传感器
      DriverRegistry* registry = DriverRegistry::getInstance();
      ISensorDriver* newDriver = registry->autoDetectSensorDriver();
      if (newDriver != nullptr) {
        if (sensorDriver != nullptr) {
          delete sensorDriver;
        }
        sensorDriver = newDriver;
        currentConfig = sensorDriver->getConfig();
        currentData.valid = true;
      }
    }
  }, "SensorManager");
  
  EVENT_SUBSCRIBE(EVENT_DRIVER_UNREGISTERED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto driverData = std::dynamic_pointer_cast<DriverEventData>(data);
    if (driverData && driverData->driverType == DRIVER_TYPE_SENSOR) {
      // 传感器驱动注销，重置传感器驱动
      if (sensorDriver != nullptr) {
        delete sensorDriver;
        sensorDriver = nullptr;
        currentData.valid = false;
      }
    }
  }, "SensorManager");
  
  EVENT_SUBSCRIBE(EVENT_POWER_STATE_CHANGED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto powerData = std::dynamic_pointer_cast<PowerStateEventData>(data);
    if (powerData) {
      // 根据电源状态调整传感器采样频率
      if (powerData->isLowPower) {
        setUpdateInterval(60000); // 低功耗模式下每分钟更新一次
      } else {
        setUpdateInterval(SENSOR_UPDATE_INTERVAL); // 正常模式下使用默认间隔
      }
    }
  }, "SensorManager");

  EVENT_SUBSCRIBE(EVENT_LOW_POWER_ENTER, [this](EventType type, std::shared_ptr<EventData> data) {
    // 进入低功耗模式事件
    DEBUG_PRINTLN("进入低功耗模式，调整传感器采样频率");
    // 非报警传感器降低采样频率
    setUpdateInterval(60000);
    // 报警相关传感器保持正常采样频率（通过单独处理）
  }, "SensorManager");

  EVENT_SUBSCRIBE(EVENT_LOW_POWER_EXIT, [this](EventType type, std::shared_ptr<EventData> data) {
    // 退出低功耗模式事件
    DEBUG_PRINTLN("退出低功耗模式，恢复正常采样频率");
    // 恢复正常采样频率
    setUpdateInterval(SENSOR_UPDATE_INTERVAL);
  }, "SensorManager");

  EVENT_SUBSCRIBE(EVENT_LOW_POWER_SENSOR_ADJUST, [this](EventType type, std::shared_ptr<EventData> data) {
    // 低功耗模式下传感器采样频率调整事件
    DEBUG_PRINTLN("低功耗模式传感器采样频率调整");
    // 报警相关传感器保持正常采样频率，其他传感器降低采样频率
    // 具体实现根据传感器类型和优先级进行调整
  }, "SensorManager");


SensorManager::~SensorManager() {
  // 清理传感器驱动资源
  if (sensorDriver != nullptr) {
    delete sensorDriver;
    sensorDriver = nullptr;
    DEBUG_PRINTLN("传感器驱动资源已清理");
  }
}

void SensorManager::init() {
  DEBUG_PRINTLN("初始化传感器管理器...");
  
  try {
    // 初始化人体感应传感器引脚
    pinMode(PIR_SENSOR_PIN, INPUT);
    
    // 初始化气体、火焰和光照传感器的引脚
    pinMode(GAS_SENSOR_PIN, INPUT);
    pinMode(FLAME_SENSOR_PIN, INPUT);
    pinMode(LIGHT_SENSOR_PIN, INPUT);
    
    DEBUG_PRINTLN("人体感应、气体、火焰和光照传感器引脚初始化完成");
    
    // 获取驱动注册表实例
    DriverRegistry* registry = DriverRegistry::getInstance();
    
    // 尝试自动检测传感器驱动
    sensorDriver = registry->autoDetectSensorDriver();
    
    if (sensorDriver == nullptr) {
      // 如果无法自动检测，尝试使用配置的传感器类型
      #if SENSOR_TYPE != SENSOR_TYPE_AUTO_DETECT
        sensorDriver = registry->getSensorDriver(SENSOR_TYPE);
        if (sensorDriver != nullptr) {
          // 初始化传感器驱动
          SensorConfig config;
          config.type = SENSOR_TYPE;
          config.pin = DHT_PIN;
          config.address = SHT30_ADDRESS;
          config.updateInterval = SENSOR_UPDATE_INTERVAL;
          config.tempOffset = 0.0;
          config.humOffset = 0.0;
          
          if (sensorDriver->init(config)) {
            currentConfig = config;
            currentData.valid = true;
            DEBUG_PRINTLN("使用配置的传感器驱动: " + sensorDriver->getTypeName());
          } else {
            delete sensorDriver;
            sensorDriver = nullptr;
            DEBUG_PRINTLN("无法初始化配置的传感器驱动");
          }
        }
      #endif
      
      if (sensorDriver == nullptr) {
        // 如果仍然无法获取传感器驱动，使用默认配置
        DEBUG_PRINTLN("无法自动检测或初始化传感器驱动，使用默认配置");
        
        // 初始化默认传感器配置
        currentConfig.type = SENSOR_TYPE_AUTO_DETECT;
        currentConfig.pin = DHT_PIN;
        currentConfig.address = SHT30_ADDRESS;
        currentConfig.updateInterval = SENSOR_UPDATE_INTERVAL;
        currentConfig.tempOffset = 0.0;
        currentConfig.humOffset = 0.0;
        
        // 设置报警阈值
        currentConfig.tempMinAlarmThreshold = -10.0;
        currentConfig.tempMaxAlarmThreshold = 40.0;
        currentConfig.humidityMinAlarmThreshold = 20.0;
        currentConfig.humidityMaxAlarmThreshold = 80.0;
        currentConfig.gasThreshold = GAS_ALARM_THRESHOLD;
        currentConfig.flameThreshold = FLAME_ALARM_THRESHOLD;
        currentConfig.lightThreshold = 500;
        
        currentData.valid = false;
      }
    } else {
      // 初始化成功，获取传感器配置
      currentConfig = sensorDriver->getConfig();
      currentConfig.updateInterval = SENSOR_UPDATE_INTERVAL;
      currentData.valid = true;
      DEBUG_PRINTLN("传感器驱动初始化成功: " + sensorDriver->getTypeName());
    }
    
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
    // 读取气体、火焰、光照和人体感应传感器数据
    readGasSensor();
    readFlameSensor();
    readLightSensor();
    readPIRSensor();
    
      // 读取温湿度传感器数据（使用传感器驱动）
    if (sensorDriver != nullptr) {
      int retryCount = 0;
      const int MAX_RETRIES = 3;
      
      while (!success && retryCount < MAX_RETRIES) {
        retryCount++;
        
        try {
          // 使用传感器驱动读取数据
          SensorData tempData;
          if (sensorDriver->readData(tempData)) {
            // 复制温度和湿度数据
            currentData.temperature = tempData.temperature;
            currentData.humidity = tempData.humidity;
            
            // 合并其他传感器数据（如果传感器驱动支持）
            if (tempData.motionDetected) {
              currentData.motionDetected = tempData.motionDetected;
            }
            if (tempData.gasLevel > 0) {
              currentData.gasLevel = tempData.gasLevel;
            }
            if (tempData.flameDetected) {
              currentData.flameDetected = tempData.flameDetected;
            }
            if (tempData.lightLevel > 0) {
              currentData.lightLevel = tempData.lightLevel;
            }
            
            success = true;
          }
        } catch (const std::exception& e) {
          DEBUG_PRINT("温湿度传感器读取异常，重试 (" + String(retryCount) + "/" + String(MAX_RETRIES) + "): ");
          DEBUG_PRINTLN(e.what());
        }
        
        if (!success) {
          DEBUG_PRINT("温湿度传感器数据读取失败，重试 (" + String(retryCount) + "/" + String(MAX_RETRIES) + ")...");
          delay(200); // 延迟后重试
        }
      }
      
      // 如果连续多次失败，尝试重新检测传感器驱动
      if (!success) {
        consecutiveFailures++;
        
        if (consecutiveFailures >= MAX_CONSECUTIVE_FAILURES) {
          DEBUG_PRINTLN("温湿度传感器连续多次读取失败，尝试重新检测传感器驱动...");
          consecutiveFailures = 0;
          
          // 获取驱动注册表实例
          DriverRegistry* registry = DriverRegistry::getInstance();
          
          // 重新检测传感器驱动
          ISensorDriver* newDriver = registry->autoDetectSensorDriver();
          if (newDriver != nullptr) {
            // 清理旧驱动
            delete sensorDriver;
            // 使用新驱动
            sensorDriver = newDriver;
            DEBUG_PRINTLN("传感器驱动重新检测成功: " + sensorDriver->getTypeName());
          } else {
            DEBUG_PRINTLN("传感器驱动重新检测失败");
          }
        }
      } else {
        consecutiveFailures = 0; // 重置连续失败计数
      }
    } else {
      // 尝试获取传感器驱动
      DriverRegistry* registry = DriverRegistry::getInstance();
      sensorDriver = registry->autoDetectSensorDriver();
      if (sensorDriver != nullptr) {
        DEBUG_PRINTLN("成功获取传感器驱动: " + sensorDriver->getTypeName());
      }
    }
    
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
      DEBUG_PRINT("%, 气体浓度 = ");
      DEBUG_PRINT(currentData.gasLevel);
      DEBUG_PRINT(", 火焰检测 = ");
      DEBUG_PRINT(currentData.flameDetected ? "有" : "无");
      DEBUG_PRINT(", 光照强度 = ");
      DEBUG_PRINT(currentData.lightLevel);
      DEBUG_PRINTLN();
      
      // 发布传感器数据更新事件
      auto sensorData = std::make_shared<SensorDataEventData>(currentData);
      EVENT_PUBLISH(EVENT_SENSOR_DATA_UPDATED, sensorData);
      
      // 检查报警条件
      checkAlarmConditions();
    }
  } catch (const std::exception& e) {
    DEBUG_PRINT("传感器更新异常: ");
    DEBUG_PRINTLN(e.what());
    DEBUG_PRINTLN("保持上一次有效的传感器数据");
  }
  
  lastUpdate = millis();
}

void SensorManager::loop() {
  // 定期更新传感器数据
  if (millis() - lastUpdate > currentConfig.updateInterval) {
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

// 读取气体传感器数据
bool SensorManager::readGasSensor() {
  // 读取气体传感器的模拟值
  int gasValue = analogRead(GAS_SENSOR_PIN);
  
  // 更新数据
  currentData.gasLevel = gasValue;
  
  return true;
}

// 读取火焰传感器数据
bool SensorManager::readFlameSensor() {
  // 读取火焰传感器的数字值
  bool flameDetected = digitalRead(FLAME_SENSOR_PIN);
  
  // 更新数据
  currentData.flameDetected = flameDetected;
  
  return true;
}

// 读取光照传感器数据
bool SensorManager::readLightSensor() {
  // 读取光照传感器的模拟值
  int lightValue = analogRead(LIGHT_SENSOR_PIN);
  
  // 更新数据
  currentData.lightLevel = lightValue;
  
  return true;
}

// 读取人体感应传感器数据
bool SensorManager::readPIRSensor() {
  // 读取人体感应传感器的数字值
  bool motionDetected = digitalRead(PIR_SENSOR_PIN);
  
  // 更新数据
  currentData.motionDetected = motionDetected;
  
  return true;
}

// 检查报警条件
void SensorManager::checkAlarmConditions() {
  bool newGasAlarm = false;
  bool newFlameAlarm = false;
  bool newTempAlarm = false;
  bool newHumidityAlarm = false;
  bool newLightAlarm = false;
  
  // 检查气体浓度是否超过阈值
  if (gasSensorEnabled && currentData.gasLevel > gasAlarmThreshold) {
    newGasAlarm = true;
    if (!gasAlarmTriggered) {
      triggerAlarm("gas");
    }
  }
  
  // 检查是否检测到火焰
  if (flameSensorEnabled && currentData.flameDetected == flameAlarmThreshold) {
    newFlameAlarm = true;
    if (!flameAlarmTriggered) {
      triggerAlarm("flame");
    }
  }
  
  // 检查温度是否超出阈值范围
  if (currentData.temperature < tempMinAlarmThreshold || currentData.temperature > tempMaxAlarmThreshold) {
    newTempAlarm = true;
    if (!tempAlarmTriggered) {
      triggerAlarm("temperature");
    }
  }
  
  // 检查湿度是否超出阈值范围
  if (currentData.humidity < humidityMinAlarmThreshold || currentData.humidity > humidityMaxAlarmThreshold) {
    newHumidityAlarm = true;
    if (!humidityAlarmTriggered) {
      triggerAlarm("humidity");
    }
  }
  
  // 检查光照强度是否超过阈值
  if (lightSensorEnabled && currentData.lightLevel > lightAlarmThreshold) {
    newLightAlarm = true;
    if (!lightAlarmTriggered) {
      triggerAlarm("light");
    }
  }
  
  // 更新报警状态
  gasAlarmTriggered = newGasAlarm;
  flameAlarmTriggered = newFlameAlarm;
  tempAlarmTriggered = newTempAlarm;
  humidityAlarmTriggered = newHumidityAlarm;
  lightAlarmTriggered = newLightAlarm;
}

// 触发报警
void SensorManager::triggerAlarm(String alarmType) {
  DEBUG_PRINT("触发报警: ");
  DEBUG_PRINTLN(alarmType);
  
  String alarmTitle = "";
  String alarmMessage = "";
  
  // 根据报警类型设置报警标题和信息
  if (alarmType == "gas") {
    alarmTitle = "燃气报警";
    alarmMessage = "检测到危险气体！";
  } else if (alarmType == "flame") {
    alarmTitle = "火焰报警";
    alarmMessage = "检测到火焰！";
  } else if (alarmType == "temperature") {
    alarmTitle = "温度报警";
    alarmMessage = "温度异常！";
  } else if (alarmType == "humidity") {
    alarmTitle = "湿度报警";
    alarmMessage = "湿度异常！";
  } else if (alarmType == "light") {
    alarmTitle = "光照报警";
    alarmMessage = "光照异常！";
  } else {
    alarmTitle = "系统报警";
    alarmMessage = "检测到异常！";
  }
  
  // 发布报警事件，让其他模块处理
  auto alarmData = std::make_shared<AlarmEventData>(alarmTitle, alarmMessage);
  EVENT_PUBLISH(EVENT_ALARM_TRIGGERED, alarmData);
  
  // 这里可以添加更多报警处理逻辑，如：
  // 3. 发送通知到手机
  // 4. 发送短信或邮件报警
}

// 使用传感器驱动读取数据
bool SensorManager::readSensor() {
  if (sensorDriver == nullptr) {
    return false;
  }
  
  SensorData tempData;
  if (sensorDriver->readData(tempData)) {
    // 复制温度和湿度数据
    currentData.temperature = tempData.temperature;
    currentData.humidity = tempData.humidity;
    return true;
  }
  
  return false;
}

void SensorManager::filterData() {
  // 简单的移动平均滤波
  // 这里可以根据需要实现更复杂的滤波算法
  static float tempHistory[10] = {0};
  static float humHistory[10] = {0};
  static int gasHistory[10] = {0};
  static int lightHistory[10] = {0};
  static int historyIndex = 0;
  
  // 添加新数据到历史记录
  tempHistory[historyIndex] = currentData.temperature;
  humHistory[historyIndex] = currentData.humidity;
  gasHistory[historyIndex] = currentData.gasLevel;
  lightHistory[historyIndex] = currentData.lightLevel;
  
  // 计算移动平均值
  float tempSum = 0;
  float humSum = 0;
  int gasSum = 0;
  int lightSum = 0;
  
  for (int i = 0; i < 10; i++) {
    tempSum += tempHistory[i];
    humSum += humHistory[i];
    gasSum += gasHistory[i];
    lightSum += lightHistory[i];
  }
  
  // 更新滤波后的数据
  currentData.temperature = tempSum / 10;
  currentData.humidity = humSum / 10;
  currentData.gasLevel = gasSum / 10;
  currentData.lightLevel = lightSum / 10;
  
  // 更新历史记录索引
  historyIndex = (historyIndex + 1) % 10;
}

void SensorManager::setSensorConfig(SensorConfig config) {
  currentConfig = config;
  
  // 尝试获取新的传感器驱动
  DriverRegistry* registry = DriverRegistry::getInstance();
  ISensorDriver* newDriver = registry->getSensorDriver(config.type);
  if (newDriver != nullptr) {
    if (sensorDriver != nullptr) {
      delete sensorDriver;
    }
    sensorDriver = newDriver;
    
    // 初始化传感器驱动
    if (sensorDriver->init(config)) {
      currentConfig = sensorDriver->getConfig();
      currentData.valid = true;
      
      // 发布传感器配置更新事件
      auto sensorConfigData = std::make_shared<SensorConfigEventData>(currentConfig);
      EVENT_PUBLISH(EVENT_SENSOR_CONFIG_UPDATED, sensorConfigData);
    } else {
      delete sensorDriver;
      sensorDriver = nullptr;
      currentData.valid = false;
    }
  }
}

void SensorManager::setSensorType(SensorType type) {
  currentConfig.type = type;
  setSensorConfig(currentConfig);
}

void SensorManager::setI2CAddress(uint8_t address) {
  currentConfig.address = address;
  setSensorConfig(currentConfig);
}

void SensorManager::setPin(int pin) {
  currentConfig.pin = pin;
  setSensorConfig(currentConfig);
}

void SensorManager::setUpdateInterval(unsigned long interval) {
  currentConfig.updateInterval = interval;
  setSensorConfig(currentConfig);
}