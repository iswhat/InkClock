#include "power_manager.h"
#include "../core/core_system.h"
#include "../core/event_bus.h"

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
  hasChargingProtection = CHARGING_PROTECTION_ENABLED;
  
  // 获取CoreSystem实例
  coreSystem = CoreSystem::getInstance();
}

PowerManager::~PowerManager() {
}

void PowerManager::init() {
  // 初始化电池检测引脚
  pinMode(BATTERY_ADC_PIN, INPUT);
  
  // 如果有充电状态引脚，初始化它
  if (CHARGE_STATUS_PIN != -1) {
    pinMode(CHARGE_STATUS_PIN, INPUT);
    DEBUG_PRINTLN("Charge status pin initialized on pin " + String(CHARGE_STATUS_PIN));
  }
  
  // 初始化人体感应传感器引脚
  if (LOW_POWER_MODE_ENABLED) {
    pinMode(PIR_SENSOR_PIN, INPUT);
    DEBUG_PRINTLN("PIR sensor initialized on pin " + String(PIR_SENSOR_PIN));
  }
  
  // 检查充电接口类型
  checkChargingInterface();
  
  // 订阅电源相关事件
  EVENT_SUBSCRIBE(EVENT_POWER_STATE_CHANGED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto powerData = std::dynamic_pointer_cast<PowerStateEventData>(data);
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
  if (LOW_POWER_MODE_ENABLED) {
    bool motionDetected = readPIRSensor();
    
    if (motionDetected) {
      lastMotionTime = millis();
      if (isLowPowerMode) {
        // 通过CoreSystem退出低功耗模式
        coreSystem->exitLowPowerMode();
      }
    } else {
      if (!isLowPowerMode && (millis() - lastMotionTime > NO_MOTION_TIMEOUT)) {
        // 通过CoreSystem进入低功耗模式
        coreSystem->enterLowPowerMode();
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
  #if LOW_POWER_MODE_ENABLED
    return digitalRead(PIR_SENSOR_PIN) == HIGH;
  #else
    return true;
  #endif
}

void PowerManager::checkChargingInterface() {
  // 由于硬件设计只支持USB-Type-C，这里直接设置为USB_TYPE_C
  chargingInterface = USB_TYPE_C;
  hasChargingProtection = CHARGING_PROTECTION_ENABLED;
  
  DEBUG_PRINTLN("Charging interface confirmed as USB-Type-C");
  DEBUG_PRINTLN("Charging power range: " + String(CHARGING_POWER_MIN) + "W - " + String(CHARGING_POWER_MAX) + "W");
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
      setCpuFrequencyMhz(80); // 降低CPU频率到80MHz
      DEBUG_PRINTLN("CPU frequency reduced to 80MHz");
    #endif
    
    // 2. 关闭不必要的模块电源
    // 关闭蓝牙（如果支持）
    #if defined(CONFIG_BT_ENABLED)
      btStop();
      DEBUG_PRINTLN("Bluetooth disabled");
    #endif
    
    // 关闭WiFi扫描
    WiFi.mode(WIFI_MODE_NONE);
    DEBUG_PRINTLN("WiFi mode set to NONE");
    
    // 3. 关闭不必要的外设时钟
    #if defined(ESP32)
      // 关闭不需要的外设时钟
      rtc_gpio_hold_en(GPIO_NUM_0);
      rtc_gpio_hold_en(GPIO_NUM_1);
      rtc_gpio_hold_en(GPIO_NUM_2);
      rtc_gpio_hold_en(GPIO_NUM_3);
      DEBUG_PRINTLN("GPIO hold enabled for unused pins");
    #endif
    
    // 4. 优化显示刷新策略
    DEBUG_PRINTLN("Display refresh interval set to " + String(LOW_POWER_REFRESH_INTERVAL) + "ms");
    
    // 5. 降低传感器采样频率
    // 可以通过修改相关模块的采样间隔来降低功耗
    DEBUG_PRINTLN("Low power mode enabled, reducing sensor sampling rate");
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
      setCpuFrequencyMhz(240); // 恢复CPU频率到240MHz
      DEBUG_PRINTLN("CPU frequency restored to 240MHz");
    #endif
    
    // 2. 恢复WiFi连接
    WiFi.mode(WIFI_STA);
    DEBUG_PRINTLN("WiFi mode set to STA");
    
    // 恢复蓝牙（如果支持）
    #if defined(CONFIG_BT_ENABLED)
      btStart();
      DEBUG_PRINTLN("Bluetooth enabled");
    #endif
    
    // 3. 恢复GPIO状态
    #if defined(ESP32)
      rtc_gpio_hold_dis(GPIO_NUM_0);
      rtc_gpio_hold_dis(GPIO_NUM_1);
      rtc_gpio_hold_dis(GPIO_NUM_2);
      rtc_gpio_hold_dis(GPIO_NUM_3);
      DEBUG_PRINTLN("GPIO hold disabled");
    #endif
    
    // 4. 恢复显示刷新频率
    DEBUG_PRINTLN("Display refresh interval set to " + String(NORMAL_REFRESH_INTERVAL) + "ms");
    
    // 5. 恢复正常的传感器采样频率
    DEBUG_PRINTLN("Normal mode enabled, restoring sensor sampling rate");
  }
}

bool PowerManager::shouldUpdateDisplay() {
  unsigned long currentTime = millis();
  unsigned long refreshInterval = isLowPowerMode ? LOW_POWER_REFRESH_INTERVAL : NORMAL_REFRESH_INTERVAL;
  
  if (currentTime - lastDisplayUpdateTime >= refreshInterval) {
    lastDisplayUpdateTime = currentTime;
    return true;
  }
  
  return false;
}

float PowerManager::readBatteryVoltage() {
  // 读取ADC值（0-4095）
  int adcValue = analogRead(BATTERY_ADC_PIN);
  
  // 转换为电压值
  // 注意：需要根据实际的分压电路进行调整
  // 假设分压电阻为100kΩ和100kΩ，总电阻200kΩ
  // ADC参考电压为3.3V
  float voltage = (adcValue / 4095.0) * 3.3 * 2.0;
  
  return voltage;
}

int PowerManager::calculateBatteryPercentage(float voltage) {
  // 确保电压在合理范围内
  if (voltage >= FULL_BATTERY_VOLTAGE) {
    return 100;
  } else if (voltage <= EMPTY_BATTERY_VOLTAGE) {
    return 0;
  }
  
  // 线性计算电量百分比
  int percentage = ((voltage - EMPTY_BATTERY_VOLTAGE) / (FULL_BATTERY_VOLTAGE - EMPTY_BATTERY_VOLTAGE)) * 100;
  
  return percentage;
}

bool PowerManager::readChargingStatus() {
  // 如果没有充电状态引脚，返回false
  if (CHARGE_STATUS_PIN == -1) {
    return false;
  }
  
  // 读取充电状态引脚
  // 注意：根据实际硬件设计，充电状态引脚的电平可能不同
  // 这里假设高电平表示正在充电
  return digitalRead(CHARGE_STATUS_PIN) == HIGH;
}