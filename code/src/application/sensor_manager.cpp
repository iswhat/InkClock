#include "sensor_manager.h"
#include "../coresystem/driver_registry.h"
#include "../coresystem/event_bus.h"
#include "../coresystem/data_types.h"

SensorManager::SensorManager() : sensorDriver(nullptr) {
  // 初始化传感器数据
  currentData.valid = false;
  currentData.temperature = 0.0;
  currentData.humidity = 0.0;
  currentData.pressure = 0.0;
  currentData.altitude = 0.0;
  currentData.light = 0.0;
  currentData.co2 = 0.0;
  currentData.voc = 0.0;
  currentData.pm25 = 0.0;
  currentData.pm10 = 0.0;
  currentData.no2 = 0.0;
  currentData.so2 = 0.0;
  currentData.co = 0.0;
  currentData.o3 = 0.0;
  currentData.ch2o = 0.0;
  currentData.noise = 0.0;
  currentData.soilMoisture = 0.0;
  currentData.soilTemperature = 0.0;
  currentData.motionDetected = false;
  currentData.gasLevel = 0;
  currentData.flameDetected = false;
  currentData.lightLevel = 0;
  
  // 初始化校准偏移量
  tempOffset = 0.0;
  humOffset = 0.0;
  
  // 初始化报警相关变量
  gasAlarmThreshold = 1000; // 默认气体报警阈值
  flameAlarmThreshold = true; // 默认火焰报警阈值
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
  // 暂时注释掉这个订阅，因为SensorConfigEventData未定义
  /*
  EVENT_SUBSCRIBE(EVENT_SENSOR_CONFIG_CHANGED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto sensorData = static_cast<SensorConfigEventData*>(data.get());
    if (sensorData) {
      setSensorConfig(sensorData->config);
    }
  }, "SensorManager");
  */
  
  EVENT_SUBSCRIBE(EVENT_DRIVER_REGISTERED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto driverData = static_cast<DriverEventData*>(data.get());
    if (driverData && driverData->driverType == "sensor") {
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
    auto driverData = static_cast<DriverEventData*>(data.get());
    if (driverData && driverData->driverType == "sensor") {
      // 传感器驱动注销，重置传感器驱动
      if (sensorDriver != nullptr) {
        delete sensorDriver;
        sensorDriver = nullptr;
        currentData.valid = false;
      }
    }
  }, "SensorManager");
  
  EVENT_SUBSCRIBE(EVENT_POWER_STATE_CHANGED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto powerData = static_cast<PowerStateEventData*>(data.get());
    if (powerData) {
      // 根据电源状态调整传感器采样频率
      if (powerData->isLowPower) {
        setUpdateInterval(60000); // 低功耗模式下每分钟更新一次
      } else {
        setUpdateInterval(5000); // 使用固定值，因为SENSOR_UPDATE_INTERVAL未定义
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
    setUpdateInterval(5000); // 使用固定值，因为SENSOR_UPDATE_INTERVAL未定义
  }, "SensorManager");

  EVENT_SUBSCRIBE(EVENT_LOW_POWER_SENSOR_ADJUST, [this](EventType type, std::shared_ptr<EventData> data) {
    // 低功耗模式下传感器采样频率调整事件
    DEBUG_PRINTLN("低功耗模式传感器采样频率调整");
    // 报警相关传感器保持正常采样频率，其他传感器降低采样频率
    // 具体实现根据传感器类型和优先级进行调整
  }, "SensorManager");
}

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
    // 如果仍然无法获取传感器驱动，使用默认配置
    DEBUG_PRINTLN("无法自动检测或初始化传感器驱动，使用默认配置");
    
    // 初始化默认传感器配置
    currentConfig.type = SENSOR_TYPE_AUTO_DETECT;
    currentConfig.pin = 4; // 默认DHT引脚
    currentConfig.address = 0x44; // 默认SHT30地址
    currentConfig.updateInterval = 5000; // 默认5秒更新一次
    currentConfig.tempOffset = 0.0;
    currentConfig.humOffset = 0.0;
    
    // 设置报警阈值
    currentConfig.tempMinThreshold = -10.0;
    currentConfig.tempMaxThreshold = 40.0;
    currentConfig.humidityMinThreshold = 20.0;
    currentConfig.humidityMaxThreshold = 80.0;
    currentConfig.gasThreshold = 1000;
    currentConfig.flameThreshold = true;
    currentConfig.lightThreshold = 500;
    
    currentData.valid = false;
  } else {
    // 初始化成功，获取传感器配置
    currentConfig = sensorDriver->getConfig();
    currentConfig.updateInterval = 5000; // 默认5秒更新一次
    currentData.valid = true;
    DEBUG_PRINTLN("传感器驱动初始化成功: " + sensorDriver->getTypeName());
  }
  
  DEBUG_PRINTLN("传感器管理器初始化完成");
}

void SensorManager::update() {
  bool success = false;
  static int consecutiveFailures = 0;
  const int MAX_CONSECUTIVE_FAILURES = 5;
  
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
        } else {
          DEBUG_PRINT("温湿度传感器读取失败，重试 (" + String(retryCount) + "/" + String(MAX_RETRIES) + ")...");
        }
        
        if (!success) {
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
  // 通过传感器驱动读取DHT22数据
  return false;
}

bool SensorManager::readSHT30() {
  // 通过传感器驱动读取SHT30数据
  return false;
}

bool SensorManager::readDHT11() {
  // 通过传感器驱动读取DHT11数据
  return false;
}

bool SensorManager::readSHT21() {
  // 通过传感器驱动读取SHT21数据
  return false;
}

bool SensorManager::readAM2302() {
  // 通过传感器驱动读取AM2302数据
  return false;
}

bool SensorManager::readHDC1080() {
  // 通过传感器驱动读取HDC1080数据
  return false;
}

bool SensorManager::readDHT12() {
  // 通过传感器驱动读取DHT12数据
  return false;
}

bool SensorManager::readSHT40() {
  // 通过传感器驱动读取SHT40数据
  return false;
}

bool SensorManager::readBME280() {
  // 通过传感器驱动读取BME280数据
  return false;
}

bool SensorManager::readBME680() {
  // 通过传感器驱动读取BME680数据
  return false;
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