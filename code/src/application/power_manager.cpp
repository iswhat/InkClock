#include "power_manager.h"
#include "../coresystem/core_system.h"
#include "../coresystem/event_bus.h"
#include "../coresystem/config_manager.h"

// 电池更新间隔（毫秒）
#define BATTERY_UPDATE_INTERVAL 2000

#if defined(ESP32) || defined(ESP8266)
#include <WiFi.h>
#endif

#if defined(ESP32)
#include "driver/gpio.h"
#endif

PowerManager::PowerManager() {
  batteryVoltage = 0.0;
  batteryPercentage = 0;
  isCharging = false;
  lastUpdateTime = 0;
  
  // 低功耗模式初始化
  isLowPowerMode = false;
  lastMotionTime = millis();
  lastDisplayUpdateTime = millis();
  
  // 充电相关初始化
  chargingInterface = USB_TYPE_C; // 仅支持USB-Type-C
  hasChargingProtection = CONFIG_GET_BOOL("charging.protection_enabled", true);
  
  // 获取CoreSystem实例
  coreSystem = CoreSystem::getInstance();
}

PowerManager::~PowerManager() {
}

void PowerManager::init() {
  // 初始化电池检测引脚
  int batteryAdcPin = CONFIG_GET_INT("pins.battery_adc", 34);
  pinMode(batteryAdcPin, INPUT);
  
  // 如果有充电状态引脚，初始化它
  int chargeStatusPin = CONFIG_GET_INT("pins.charge_status", -1);
  if (chargeStatusPin != -1) {
    pinMode(chargeStatusPin, INPUT);
    DEBUG_PRINTLN("Charge status pin initialized on pin " + String(chargeStatusPin));
  }
  
  // 初始化人体感应传感器引脚
  bool lowPowerModeEnabled = CONFIG_GET_BOOL("power.low_power_mode", false);
  if (lowPowerModeEnabled) {
    int pirSensorPin = CONFIG_GET_INT("pins.pir_sensor", -1);
    if (pirSensorPin != -1) {
      pinMode(pirSensorPin, INPUT);
      DEBUG_PRINTLN("PIR sensor initialized on pin " + String(pirSensorPin));
    }
  }
  
  // 检查充电接口类型
  checkChargingInterface();
  
  // 订阅电源相关事件
  EVENT_SUBSCRIBE(EVENT_POWER_STATE_CHANGED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto powerData = static_cast<PowerStateEventData*>(data.get());
    if (powerData) {
      this->batteryPercentage = powerData->batteryPercentage;
      this->isCharging = powerData->isCharging;
      this->isLowPowerMode = powerData->isLowPower;
      this->lastUpdateTime = millis();
    }
  }, "PowerManager");
  
  EVENT_SUBSCRIBE(EVENT_BATTERY_LOW, [this](EventType type, std::shared_ptr<EventData> data) {
    DEBUG_PRINTLN("低电量警告");
  }, "PowerManager");
  
  EVENT_SUBSCRIBE(EVENT_BATTERY_OK, [this](EventType type, std::shared_ptr<EventData> data) {
    DEBUG_PRINTLN("电量恢复正常");
  }, "PowerManager");
  
  EVENT_SUBSCRIBE(EVENT_CHARGING_STARTED, [this](EventType type, std::shared_ptr<EventData> data) {
    this->isCharging = true;
    this->lastUpdateTime = millis();
  }, "PowerManager");
  
  EVENT_SUBSCRIBE(EVENT_CHARGING_STOPPED, [this](EventType type, std::shared_ptr<EventData> data) {
    this->isCharging = false;
    this->lastUpdateTime = millis();
  }, "PowerManager");
  
  // 初始更新
  update();
  
  DEBUG_PRINTLN("PowerManager initialized with USB-Type-C charging interface");
  DEBUG_PRINTLN("Charging protection: " + String(hasChargingProtection ? "Enabled" : "Disabled"));
  DEBUG_PRINTLN("DC power support: " + String(isDCPowerSupported() ? "Enabled" : "Disabled"));
}

void PowerManager::loop() {
  // 定期更新电池状态
  if (millis() - lastUpdateTime > BATTERY_UPDATE_INTERVAL) {
    update();
  }
  
  // 低功耗模式处理
  bool lowPowerModeEnabled = CONFIG_GET_BOOL("power.low_power_mode", false);
  if (lowPowerModeEnabled) {
    // 1. 读取人体感应传感器
    bool motionDetected = readPIRSensor();
    
    // 2. 读取光照传感器（如果启用了光照节能功能）
    bool nightMode = false;
    bool enableLightSaving = CONFIG_GET_BOOL("feature.enable_light_saving", false);
    if (enableLightSaving) {
      int lightSensorPin = CONFIG_GET_INT("pins.light_sensor", -1);
      if (lightSensorPin != -1) {
        int lightLevel = analogRead(lightSensorPin);
        int nightLightThreshold = CONFIG_GET_INT("power.night_light_threshold", 100);
        nightMode = (lightLevel < nightLightThreshold);
      }
    }
    
    // 3. 人体感应处理
    if (motionDetected) {
      lastMotionTime = millis();
      if (isLowPowerMode) {
        // 通过CoreSystem退出低功耗模式
        coreSystem->exitLowPowerMode();
      }
    } else {
      // 4. 光感应处理
      if (nightMode) {
        // 夜间模式，立即进入低功耗模式
        if (!isLowPowerMode) {
          // 通过CoreSystem进入低功耗模式
          coreSystem->enterLowPowerMode();
        }
      } else {
        // 白天模式，根据无运动时间判断
        int noMotionTimeout = CONFIG_GET_INT("power.no_motion_timeout", 30000);
        if (!isLowPowerMode && (millis() - lastMotionTime > noMotionTimeout)) {
          // 通过CoreSystem进入低功耗模式
          coreSystem->enterLowPowerMode();
        }
      }
    }
  }
  
  // 同步CoreSystem的电源状态
  isLowPowerMode = coreSystem->isInLowPowerMode();
}

void PowerManager::update() {
  // 读取电池电压
  batteryVoltage = readBatteryVoltage();
  
  // 计算电量百分比
  batteryPercentage = calculateBatteryPercentage(batteryVoltage);
  
  // 读取充电状态
  isCharging = readChargingStatus();
  
  lastUpdateTime = millis();
  
  // 发布电源状态更新事件
  auto powerData = std::make_shared<PowerStateEventData>(batteryPercentage, isCharging, isLowPowerMode);
  EVENT_PUBLISH(EVENT_POWER_STATE_CHANGED, powerData);
  
  DEBUG_PRINT("Battery: ");
  DEBUG_PRINT(batteryVoltage);
  DEBUG_PRINT("V, ");
  DEBUG_PRINT(batteryPercentage);
  DEBUG_PRINT("%, Charging: ");
  DEBUG_PRINT(isCharging ? "Yes" : "No");
  DEBUG_PRINT(", Low Power: ");
  DEBUG_PRINTLN(isLowPowerMode ? "Yes" : "No");
}

bool PowerManager::readPIRSensor() {
  bool lowPowerModeEnabled = CONFIG_GET_BOOL("power.low_power_mode", false);
  if (lowPowerModeEnabled) {
    int pirSensorPin = CONFIG_GET_INT("pins.pir_sensor", -1);
    if (pirSensorPin != -1) {
      return digitalRead(pirSensorPin) == HIGH;
    }
  }
  return true;
}

void PowerManager::checkChargingInterface() {
  // 由于硬件设计只支持USB-Type-C，这里直接设置为USB_TYPE_C
  chargingInterface = USB_TYPE_C;
  hasChargingProtection = CONFIG_GET_BOOL("charging.protection_enabled", true);
  
  float chargingPowerMin = CONFIG_GET_FLOAT("charging.power_min", 5.0);
  float chargingPowerMax = CONFIG_GET_FLOAT("charging.power_max", 18.0);
  
  DEBUG_PRINTLN("Charging interface confirmed as USB-Type-C");
  DEBUG_PRINTLN("Charging power range: " + String(chargingPowerMin) + "W - " + String(chargingPowerMax) + "W");
}

void PowerManager::enterLowPowerMode() {
  if (!isLowPowerMode) {
    DEBUG_PRINTLN("Entering low power mode...");
    
    // 通过CoreSystem进入低功耗模式
    coreSystem->enterLowPowerMode();
    isLowPowerMode = coreSystem->isInLowPowerMode();
    
    // 发布低功耗进入事件
    EVENT_PUBLISH(EVENT_LOW_POWER_ENTER, nullptr);
    
    // 实际的低功耗操作
    // 1. 降低CPU频率（如果支持）
    #if defined(ESP32)
      // 降低CPU频率到80MHz
      coreSystem->setCpuFrequencyMhz(80);
      DEBUG_PRINTLN("CPU frequency reduced to 80MHz");
    #endif
    
    // 2. 关闭不必要的模块电源
    // 关闭蓝牙（如果支持）
    #if defined(CONFIG_BT_ENABLED)
      // 关闭蓝牙（如果支持）
      #if defined(CONFIG_BT_ENABLED)
        btStop();
        DEBUG_PRINTLN("Bluetooth disabled");
      #endif
    #endif
    
    // 关闭WiFi扫描
      // 关闭WiFi（如果支持）
      #if defined(ESP32) || defined(ESP8266)
        #if defined(ESP32)
          WiFi.mode(WIFI_OFF);
        #elif defined(ESP8266)
          WiFi.mode(WIFI_OFF);
        #endif
        DEBUG_PRINTLN("WiFi disabled");
      #endif
    
    // 3. 关闭不必要的外设时钟，但保留报警相关传感器
    #if defined(ESP32)
      // 关闭不需要的外设时钟
      
      gpio_hold_en(GPIO_NUM_0);
      gpio_hold_en(GPIO_NUM_1);
      gpio_hold_en(GPIO_NUM_2);
      gpio_hold_en(GPIO_NUM_3);
      DEBUG_PRINTLN("GPIO hold enabled for unused pins");
      
      // 确保报警相关传感器引脚不被关闭
      int gasSensorPin = CONFIG_GET_INT("pins.gas_sensor", -1);
      int flameSensorPin = CONFIG_GET_INT("pins.flame_sensor", -1);
      int pirSensorPin = CONFIG_GET_INT("pins.pir_sensor", -1);
      int lightSensorPin = CONFIG_GET_INT("pins.light_sensor", -1);
      
      if (gasSensorPin != -1) gpio_hold_dis((gpio_num_t)gasSensorPin);
      if (flameSensorPin != -1) gpio_hold_dis((gpio_num_t)flameSensorPin);
      if (pirSensorPin != -1) gpio_hold_dis((gpio_num_t)pirSensorPin);
      if (lightSensorPin != -1) gpio_hold_dis((gpio_num_t)lightSensorPin);
      
      DEBUG_PRINTLN("保留报警相关传感器引脚功能");
    #endif
    
    // 4. 优化显示刷新策略
    int lowPowerRefreshInterval = CONFIG_GET_INT("power.low_power_refresh_interval", 300000);
    DEBUG_PRINTLN("Display refresh interval set to " + String(lowPowerRefreshInterval) + "ms");
    
    // 5. 降低传感器采样频率，但保留报警相关传感器的正常采样频率
    DEBUG_PRINTLN("Low power mode enabled, reducing non-alarm sensor sampling rate");
    DEBUG_PRINTLN("报警相关传感器保持正常采样频率");
    
    // 发布传感器采样频率调整事件
    EVENT_PUBLISH(EVENT_LOW_POWER_SENSOR_ADJUST, nullptr);
  }
}

void PowerManager::exitLowPowerMode() {
  if (isLowPowerMode) {
    DEBUG_PRINTLN("Exiting low power mode...");
    
    // 通过CoreSystem退出低功耗模式
    coreSystem->exitLowPowerMode();
    isLowPowerMode = coreSystem->isInLowPowerMode();
    
    // 发布低功耗退出事件
    EVENT_PUBLISH(EVENT_LOW_POWER_EXIT, nullptr);
    
    // 恢复正常操作
    // 1. 恢复CPU频率
    #if defined(ESP32)
      // 恢复CPU频率到240MHz
      coreSystem->setCpuFrequencyMhz(240);
      DEBUG_PRINTLN("CPU frequency restored to 240MHz");
    #endif
    
    // 2. 恢复WiFi连接
      // 恢复WiFi连接
      #if defined(ESP32) || defined(ESP8266)
        #if defined(ESP32)
          WiFi.mode(WIFI_MODE_STA);
        #elif defined(ESP8266)
          WiFi.mode(WIFI_STA);
        #endif
        DEBUG_PRINTLN("WiFi mode set to STA");
      #endif
    
    // 恢复蓝牙（如果支持）
    #if defined(CONFIG_BT_ENABLED)
      btStart();
      DEBUG_PRINTLN("Bluetooth enabled");
    #endif
    
    // 3. 恢复GPIO状态
    #if defined(ESP32)
      // 恢复GPIO状态
      
      gpio_hold_dis(GPIO_NUM_0);
      gpio_hold_dis(GPIO_NUM_1);
      gpio_hold_dis(GPIO_NUM_2);
      gpio_hold_dis(GPIO_NUM_3);
      DEBUG_PRINTLN("GPIO hold disabled");
    #endif
    
    // 4. 恢复显示刷新频率
    int normalRefreshInterval = CONFIG_GET_INT("power.normal_refresh_interval", 60000);
    DEBUG_PRINTLN("Display refresh interval set to " + String(normalRefreshInterval) + "ms");
    
    // 5. 恢复正常的传感器采样频率
    DEBUG_PRINTLN("Normal mode enabled, restoring sensor sampling rate");
  }
}

bool PowerManager::shouldUpdateDisplay() {
  unsigned long currentTime = millis();
  unsigned long refreshInterval;
  
  // 根据电池电量和充电状态动态调整刷新间隔
  if (isCharging) {
    // 充电时，使用正常刷新间隔
    refreshInterval = CONFIG_GET_INT("power.normal_refresh_interval", 60000);
  } else if (batteryPercentage <= CONFIG_GET_INT("power.critical_battery_threshold", 10)) {
    // 电量极低时，大幅延长刷新间隔
    refreshInterval = CONFIG_GET_INT("power.critical_low_power_refresh_interval", 600000);
  } else if (isLowPowerMode) {
    // 低功耗模式下，延长刷新间隔
    refreshInterval = CONFIG_GET_INT("power.low_power_refresh_interval", 300000);
  } else {
    // 正常模式下，使用正常刷新间隔
    refreshInterval = CONFIG_GET_INT("power.normal_refresh_interval", 60000);
  }
  
  // 确保刷新间隔在合理范围内
  int minRefreshInterval = CONFIG_GET_INT("power.min_refresh_interval", 10000);
  int maxRefreshInterval = CONFIG_GET_INT("power.max_refresh_interval", 3600000);
  refreshInterval = constrain(refreshInterval, minRefreshInterval, maxRefreshInterval);
  
  if (currentTime - lastDisplayUpdateTime >= refreshInterval) {
    lastDisplayUpdateTime = currentTime;
    return true;
  }
  
  return false;
}

float PowerManager::readBatteryVoltage() {
  // 读取ADC值（0-4095）
  int batteryAdcPin = CONFIG_GET_INT("pins.battery_adc", 34);
  int adcValue = analogRead(batteryAdcPin);
  
  // 转换为电压值
  // 注意：需要根据实际的分压电路进行调整
  // 假设分压电阻为100kΩ和100kΩ，总电阻200kΩ
  // ADC参考电压为3.3V
  float voltage = (adcValue / 4095.0) * 3.3 * 2.0;
  
  return voltage;
}

int PowerManager::calculateBatteryPercentage(float voltage) {
  // 确保电压在合理范围内
  float fullBatteryVoltage = CONFIG_GET_FLOAT("battery.full_voltage", 4.2);
  float emptyBatteryVoltage = CONFIG_GET_FLOAT("battery.empty_voltage", 3.0);
  
  if (voltage >= fullBatteryVoltage) {
    return 100;
  } else if (voltage <= emptyBatteryVoltage) {
    return 0;
  }
  
  // 线性计算电量百分比
  int percentage = ((voltage - emptyBatteryVoltage) / (fullBatteryVoltage - emptyBatteryVoltage)) * 100;
  
  return percentage;
}

bool PowerManager::readChargingStatus() {
  // 如果没有充电状态引脚，返回false
  int chargeStatusPin = CONFIG_GET_INT("pins.charge_status", -1);
  if (chargeStatusPin == -1) {
    return false;
  }
  
  // 读取充电状态引脚
  // 注意：根据实际硬件设计，充电状态引脚的电平可能不同
  // 这里假设高电平表示正在充电
  return digitalRead(chargeStatusPin) == HIGH;
}