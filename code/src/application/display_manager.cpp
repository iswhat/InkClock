#include "display_manager.h"
#include "time_manager.h"
#include "weather_manager.h"
#include "sensor_manager.h"
#include "stock_manager.h"
#include "message_manager.h"
#include "power_manager.h"
#include "lunar_manager.h"
#include "../coresystem/dependency_injection.h"
#include "image_decoder_config.h"
#include <vector>
#include <memory>

// 自定义字符串填充函数，替代缺少的 padStart 方法
String padStart(String str, unsigned int length, char padChar) {
  if (str.length() >= length) {
    return str;
  }
  String result = "";
  for (unsigned int i = 0; i < length - str.length(); i++) {
    result += padChar;
  }
  result += str;
  return result;
}

// 获取指定月份的第一天是星期几（0-6，0表示星期日）
int getFirstWeekdayOfMonth(int year, int month) {
  struct tm timeinfo;
  timeinfo.tm_year = year - 1900;
  timeinfo.tm_mon = month - 1;
  timeinfo.tm_mday = 1;
  timeinfo.tm_hour = 0;
  timeinfo.tm_min = 0;
  timeinfo.tm_sec = 0;
  mktime(&timeinfo);
  return timeinfo.tm_wday;
}

// 获取指定月份的天数
int getDaysInMonth(int year, int month) {
  if (month == 2) {
    // 闰年判断
    if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
      return 29;
    } else {
      return 28;
    }
  } else if (month == 4 || month == 6 || month == 9 || month == 11) {
    return 30;
  } else {
    return 31;
  }
}

DisplayManager::DisplayManager() {
  displayDriver = nullptr;
  currentRightPage = RIGHT_PAGE_CALENDAR;
  currentClockMode = CLOCK_MODE_DIGITAL;
  showSeconds = false; // 默认不显示秒针
  width = 0;
  height = 0;
  leftPanelWidth = 0;
  rightPanelWidth = 0;
  lastMessageCount = 0;
  lastBatteryPercentage = 100;
  lastTemperature = 0.0;
  lastHumidity = 0.0;
  lastClockSecond = -1;
  
  // 初始化时区管理
  currentTimeZone = {"UTC", "UTC", 0, false};
  autoTimeZoneEnabled = false;
  
  // 初始化内容类型最后更新时间
  lastClockUpdateTime = 0;
  lastWeatherUpdateTime = 0;
  lastSensorUpdateTime = 0;
  lastStockUpdateTime = 0;
  lastMessageUpdateTime = 0;
  lastCalendarUpdateTime = 0;
  lastFullRefreshTime = 0;
  
  // 初始化报警显示相关变量
  alarmShowing = false;
  currentAlarmType = "";
  currentAlarmMessage = "";
  lastAlarmUpdateTime = 0;
  alarmBlinkState = false;
  lastBlinkTime = 0;
  alarmStartTime = 0;
  
  // 初始化消息提醒动画相关变量
  messageAnimationActive = false;
  messageAnimationStartTime = 0;
  messageAnimationLastUpdate = 0;
  messageAnimationFrame = 0;
  messageAnimationDirection = true;
  
  // 初始化传感器异常检测相关变量
  sensorAnomalyDetected = false;
  sensorAnomalyType = "";
  sensorAnomalyStartTime = 0;
  sensorAlarmActive = false;
  
  // 初始化本地缓存数据
  cachedTimeData = TimeData{0, 0, 0, 0, 0, 0, 0, "", "", ""};
  cachedWeatherData = WeatherData{"未知", 0.0f, 0.0f, 0, 0, 0, "未知", "", 0, "", 0, 0.0f, "", 0.0f, 0, 0};
  cachedSensorData = SensorData{0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, false, 0, false, 0, false};
  cachedBatteryPercentage = 100;
  cachedBatteryVoltage = 0.0;
  cachedIsCharging = false;
  cachedUnreadMessageCount = 0;
  
  // 初始化传感器数据历史记录
  for (int i = 0; i < MAX_SENSOR_HISTORY; i++) {
    tempHistory[i] = 0.0;
    humHistory[i] = 0.0;
  }
  sensorHistoryIndex = 0;
  
  // 初始化GIF播放相关变量
  gifPlaying = false;
  gifStopped = false;
  gifLoopCount = -1;
  gifCurrentLoop = 0;
  gifLastFrameTime = 0;
  gifFrameInterval = 100;
  gifCurrentFrame = 0;
  gifTotalFrames = 0;
  currentGifPath = "";
  gifBuffer = nullptr;
  gifBufferSize = 0;
  
  // 初始化布局配置（左小右大 45:55）
  layoutMode = LAYOUT_MODE_STANDARD;
  currentLayout = {
    LAYOUT_MODE_STANDARD,
    0.45f,  // 左侧面板比例 45%（较小）
    0.55f,  // 右侧面板比例 55%（较大）
    12,     // 基础字体大小
    8,      // 元素间距
    false   // 默认不显示边框
  };
  
  // 订阅事件
  EVENT_SUBSCRIBE(EVENT_ALARM_TRIGGERED, [this](EventType type, std::shared_ptr<EventData> data) {
    if (data) {
      auto alarmData = static_cast<AlarmEventData*>(data.get());
      if (alarmData) {
        this->showAlarm(alarmData->alarmType, alarmData->message);
      }
    }
  }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_TIME_UPDATED, [this](EventType type, std::shared_ptr<EventData> data) {
    if (data) {
      auto timeData = static_cast<TimeDataEventData*>(data.get());
      if (timeData) {
        cachedTimeData = timeData->timeData;
        this->updateDisplay();
      }
    }
  }, "DisplayManager");
  
  // 天气更新事件暂时注释，等待正确的事件类型
  // EVENT_SUBSCRIBE(EVENT_WEATHER_UPDATED, [this](EventType type, std::shared_ptr<EventData> data) {
  //   // 移除dynamic_cast，直接使用类型转换
  //   auto weatherData = static_cast<WeatherDataEventData*>(data.get());
  //   cachedWeatherData = weatherData->weatherData;
  //   this->updateDisplay();
  // }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_SENSOR_DATA_UPDATED, [this](EventType type, std::shared_ptr<EventData> data) {
    if (data) {
      auto sensorData = static_cast<SensorDataEventData*>(data.get());
      if (sensorData) {
        cachedSensorData = sensorData->sensorData;
        this->updateDisplay();
      }
    }
  }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_POWER_STATE_CHANGED, [this](EventType type, std::shared_ptr<EventData> data) {
    if (data) {
      auto powerData = static_cast<PowerStateEventData*>(data.get());
      if (powerData) {
        cachedBatteryPercentage = powerData->batteryPercentage;
        cachedIsCharging = powerData->isCharging;
        this->updateDisplay();
      }
    }
  }, "DisplayManager");
  
  // 消息相关事件暂时注释，等待正确的事件类型和数据结构
  // EVENT_SUBSCRIBE(EVENT_MESSAGE_RECEIVED, [this](EventType type, std::shared_ptr<EventData> data) {
  //   // 移除dynamic_pointer_cast，直接使用static_cast
  //   auto messageData = static_cast<MessageEventData*>(data.get());
  //   if (messageData) {
  //     cachedUnreadMessageCount++;
  //     this->updateDisplay();
  //   }
  // }, "DisplayManager");

  // EVENT_SUBSCRIBE(EVENT_MESSAGE_READ, [this](EventType type, std::shared_ptr<EventData> data) {
  //   // 移除dynamic_pointer_cast，直接使用static_cast
  //   auto messageData = static_cast<MessageEventData*>(data.get());
  //   if (messageData && cachedUnreadMessageCount > 0) {
  //     cachedUnreadMessageCount--;
  //   }
  // }, "DisplayManager");
}



DisplayManager::~DisplayManager() {
  // Resource cleanup: unique_ptr will automatically delete displayDriver
  // No manual delete needed with smart pointer
  displayDriver.reset();
}

bool DisplayManager::init() {
  if (displayDriver == nullptr) {
    DEBUG_PRINTLN("错误：显示驱动未设置");
    return false;
  }
  
  // 初始化显示驱动
  if (!displayDriver->init()) {
    DEBUG_PRINTLN("显示驱动初始化失败");
    return false;
  }
  
  // 获取屏幕尺寸
  width = displayDriver->getWidth();
  height = displayDriver->getHeight();
  
  // 初始化分屏布局参数 - 根据屏幕宽度动态调整
  // 小屏幕（< 600像素）：左侧面板宽度约为总宽度的1/2
  // 大屏幕（>= 600像素）：左侧面板宽度约为总宽度的1/3
  if (width < 600) {
    leftPanelWidth = width / 2;
  } else {
    leftPanelWidth = width / 3;
  }
  rightPanelWidth = width - leftPanelWidth;
  
  // 订阅报警事件
  EVENT_SUBSCRIBE(EVENT_ALARM_TRIGGERED, [this](EventType type, std::shared_ptr<EventData> data) {
    if (type == EVENT_ALARM_TRIGGERED && data) {
      auto alarmData = static_cast<AlarmEventData*>(data.get());
      if (alarmData) {
        this->showAlarm(alarmData->alarmType, alarmData->message);
      }
    }
  }, "DisplayManager");
  
  DEBUG_PRINTLN("显示管理器初始化完成");
  return true;
}

void DisplayManager::setDisplayDriver(IDisplayDriver* driver) {
  // Security: Use reset() instead of manual delete with unique_ptr
  displayDriver.reset(driver);
}

void DisplayManager::showSplashScreen() {
  if (displayDriver == nullptr) {
    return;
  }
  
  clearScreen();
  
  // 根据屏幕尺寸动态调整启动画面
  int textSize;
  int titleX, titleY, versionX, versionY;
  
  if (height < 400) {
    // 小屏幕
    textSize = 2;
    titleX = width / 2 - 60;
    titleY = height / 2 - 20;
    versionX = width / 2 - 70;
    versionY = height / 2 + 10;
  } else {
    // 大屏幕
    textSize = 4;
    titleX = width / 2 - 120;
    titleY = height / 2 - 40;
    versionX = width / 2 - 150;
    versionY = height / 2 + 20;
  }
  
  displayDriver->drawString(titleX, titleY, "智能墨水屏", GxEPD_BLACK, GxEPD_WHITE, textSize);
  displayDriver->drawString(versionX, versionY, "万年历 v1.0", GxEPD_BLACK, GxEPD_WHITE, textSize);
  
  displayDriver->update();
}

void DisplayManager::updateDisplay() {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 如果处于报警状态，只更新报警显示
  if (alarmShowing) {
    updateAlarmDisplay();
    return;
  }
  
  unsigned long currentTime = millis();
  bool isLowPowerMode = false;
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  if (powerManager) {
    isLowPowerMode = powerManager->getLowPowerMode();
  }
  
  // 根据低功耗模式调整刷新间隔倍率
  int refreshMultiplier = isLowPowerMode ? 6 : 1; // 低功耗模式下刷新间隔延长6倍
  
  // 检查是否需要刷新
  if (powerManager && !powerManager->shouldUpdateDisplay()) {
    return;
  }
  
  // 标记需要刷新的区域
  bool needFullRefresh = false;
  bool needLeftPanelRefresh = false;
  bool needRightPanelRefresh = false;
  bool needClockRefresh = false;
  bool needWeatherRefresh = false;
  bool needSensorRefresh = false;
  bool needBatteryRefresh = false;
  bool needMessageRefresh = false;
  bool needCalendarRefresh = false;
  
  // 获取当前时间的秒数，用于判断是否需要刷新时钟
  int currentSecond = cachedTimeData.second;
  
  // 1. 检查时钟区域 - 更精确的控制
  if (showSeconds) {
    // 显示秒针时，每500毫秒刷新一次时钟区域，平衡动画效果和性能
    if (currentTime - lastClockUpdateTime >= ALARM_ICON_BLINK_INTERVAL) {
      needClockRefresh = true;
      needLeftPanelRefresh = true;
      lastClockUpdateTime = currentTime;
    }
  } else {
    // 不显示秒针时，每分钟刷新一次时钟区域
    if (currentTime - lastClockUpdateTime >= CLOCK_UPDATE_INTERVAL) {
      needClockRefresh = true;
      needLeftPanelRefresh = true;
      lastClockUpdateTime = currentTime;
    }
  }
  
  // 2. 检查天气信息 - 每 2 小时更新一次
  if (currentTime - lastWeatherUpdateTime >= WEATHER_UPDATE_INTERVAL) {
    needWeatherRefresh = true;
    needLeftPanelRefresh = true;
    lastWeatherUpdateTime = currentTime;
  }
  
  // 3. 检查传感器数据 - 温度或湿度变化超过±2 时刷新，或至少每 30 分钟刷新一次
  if ((abs(cachedSensorData.temperature - lastTemperature) >= TEMP_CHANGE_THRESHOLD || 
       abs(cachedSensorData.humidity - lastHumidity) >= HUMIDITY_CHANGE_THRESHOLD) ||
      (currentTime - lastSensorUpdateTime >= SENSOR_UPDATE_INTERVAL)) { // 30 分钟
    needSensorRefresh = true;
    needLeftPanelRefresh = true;
    lastTemperature = cachedSensorData.temperature;
    lastHumidity = cachedSensorData.humidity;
    lastSensorUpdateTime = currentTime;
  }
  
  // 4. 检查电池信息
  if (abs(cachedBatteryPercentage - lastBatteryPercentage) > BATTERY_CHANGE_THRESHOLD) {
    needBatteryRefresh = true;
    needLeftPanelRefresh = true;
    lastBatteryPercentage = cachedBatteryPercentage;
  }
  
  // 5. 检查消息通知
  if (cachedUnreadMessageCount != lastMessageCount) {
    needMessageRefresh = true;
    needLeftPanelRefresh = true;
    lastMessageCount = cachedUnreadMessageCount;
    lastMessageUpdateTime = currentTime;
  }
  
  // 6. 检查右侧面板内容
  if (currentRightPage == RIGHT_PAGE_STOCK && currentTime - lastStockUpdateTime >= STOCK_UPDATE_INTERVAL * refreshMultiplier) {
    needRightPanelRefresh = true;
    lastStockUpdateTime = currentTime;
  } else if (currentRightPage == RIGHT_PAGE_CALENDAR && currentTime - lastCalendarUpdateTime >= CALENDAR_UPDATE_INTERVAL * refreshMultiplier) {
    needCalendarRefresh = true;
    needRightPanelRefresh = true;
    lastCalendarUpdateTime = currentTime;
  }
  
  // 7. 检查是否需要全屏刷新（每天至少一次或内容变化较大时）
  if (currentTime - lastFullRefreshTime >= FULL_REFRESH_INTERVAL || // 每天一次
      (needLeftPanelRefresh && needRightPanelRefresh && isLowPowerMode)) {
    needFullRefresh = true;
    lastFullRefreshTime = currentTime;
  }
  
  // 8. 检查是否有新消息通知，需要替换日历显示
  if (cachedUnreadMessageCount > 0 && currentRightPage == RIGHT_PAGE_CALENDAR) {
    needRightPanelRefresh = true;
    needCalendarRefresh = true;
  }
  
  // 执行刷新
  if (needFullRefresh) {
    // 全屏刷新
    DEBUG_PRINTLN("Performing full display refresh");
    drawLeftPanel();
    drawRightPanel();
    displayDriver->update();
  } else {
    // 更精细的局部刷新
    if (needLeftPanelRefresh) {
      // 只刷新需要更新的区域
      if (needClockRefresh) {
        // 只刷新时钟区域（根据当前时钟模式）
        auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
        if (timeManager) {
          if (currentClockMode == CLOCK_MODE_DIGITAL) {
            drawDigitalClock(20, 60, timeManager->getTimeString(), timeManager->getDateString());
            // 精确计算时钟区域的刷新范围
            int clockHeight = height < 400 ? 120 : 200;
            displayDriver->update(0, 0, leftPanelWidth, clockHeight);
          } else if (currentClockMode == CLOCK_MODE_ANALOG) {
            TimeData timeData = timeManager->getTimeData();
            int millisecond = millis() % 1000;
            drawAnalogClock(leftPanelWidth / 2, 120, timeData.hour, timeData.minute, timeData.second);
            // 精确计算模拟时钟区域的刷新范围
            int clockRadius = height < 400 ? 40 : 60;
            int clockX = leftPanelWidth / 2;
            int clockY = 120;
            displayDriver->update(clockX - clockRadius - 10, clockY - clockRadius - 10, clockRadius * 2 + 20, clockRadius * 2 + 20);
          } else if (currentClockMode == CLOCK_MODE_TEXT) {
            TimeData timeData = timeManager->getTimeData();
            drawTextClock(20, 60, timeData.hour, timeData.minute, timeData.second);
            // 精确计算文字时钟区域的刷新范围
            int clockHeight = height < 400 ? 120 : 200;
            displayDriver->update(0, 0, leftPanelWidth, clockHeight);
          }
        }
      }
      
      if (needWeatherRefresh) {
        // 只刷新天气区域
        auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
        if (weatherManager) {
          WeatherData weather = weatherManager->getWeatherData();
          int weatherY = height < 400 ? 140 : 220;
          int weatherHeight = height < 400 ? 100 : 150;
          drawWeather(20, weatherY, weather.city, 
                      (weather.temp != 0 ? String(weather.temp) : "--") + "°C", 
                      weather.condition, "", "");
          displayDriver->update(0, weatherY, leftPanelWidth, weatherHeight);
        }
      }
      
      if (needSensorRefresh) {
        // 只刷新传感器数据区域
        auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
        if (sensorManager) {
          SensorData sensor = sensorManager->getSensorData();
          int sensorY = height < 400 ? 220 : 340;
          int sensorHeight = height < 400 ? 80 : 120;
          drawSensorData(20, sensorY, sensor.temperature, sensor.humidity);
          displayDriver->update(0, sensorY, leftPanelWidth, sensorHeight);
        }
      }
      
      if (needBatteryRefresh || needMessageRefresh) {
        // 只刷新底部区域
        auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
        auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
        if (powerManager && messageManager) {
          float batteryVoltage = powerManager->getBatteryVoltage();
          int batteryPercentage = powerManager->getBatteryPercentage();
          bool isCharging = powerManager->getChargingStatus();
          int messageCount = messageManager->getUnreadMessageCount();
          
          // 绘制左上角状态栏（电量和消息）
          drawStatusBar(20, 20, batteryVoltage, batteryPercentage, isCharging, messageCount);
          displayDriver->update(0, 0, leftPanelWidth, height < 400 ? 40 : 60);
          
          // 如果有新消息，启动消息提醒动画
          if (needMessageRefresh && messageCount > 0) {
            startMessageAnimation();
          }
        }
      }
    }
    
    if (needRightPanelRefresh) {
      // 只刷新右侧面板
      drawRightPanel();
      displayDriver->update(leftPanelWidth, 0, rightPanelWidth, height);
    }
  }
  
  // 更新消息提醒动画
  updateMessageAnimation();
  
  // 更新传感器报警状态
  updateSensorAlarm();
}

void DisplayManager::updateDisplayPartial() {
  if (!displayDriver) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // 获取各种数据
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  auto stockManager = DependencyInjectionContainer::getInstance()->getStockManager();
  
  // 检查是否需要刷新时钟
  bool needClockRefresh = false;
  if (timeManager) {
    TimeData timeData = timeManager->getTimeData();
    if (timeData.second != lastClockSecond) {
      needClockRefresh = true;
      lastClockSecond = timeData.second;
    }
  }
  
  // 检查是否需要刷新天气
  bool needWeatherRefresh = false;
  if (weatherManager && currentTime - lastWeatherUpdateTime >= WEATHER_UPDATE_INTERVAL) {
    needWeatherRefresh = true;
    lastWeatherUpdateTime = currentTime;
  }
  
  // 检查是否需要刷新传感器数据
  bool needSensorRefresh = false;
  if (sensorManager) {
    SensorData sensor = sensorManager->getSensorData();
    if ((abs(sensor.temperature - lastTemperature) >= 0.5 || 
         abs(sensor.humidity - lastHumidity) >= 5.0) ||
        (currentTime - lastSensorUpdateTime >= SENSOR_UPDATE_INTERVAL_FAST)) { // 5 分钟
      needSensorRefresh = true;
      lastTemperature = sensor.temperature;
      lastHumidity = sensor.humidity;
      lastSensorUpdateTime = currentTime;
    }
  }
  
  // 检查是否需要刷新电池信息
  bool needBatteryRefresh = false;
  if (powerManager) {
    int batteryPercentage = powerManager->getBatteryPercentage();
    if (abs(batteryPercentage - lastBatteryPercentage) > 5) {
      needBatteryRefresh = true;
      lastBatteryPercentage = batteryPercentage;
    }
  }
  
  // 检查是否需要刷新消息通知
  bool needMessageRefresh = false;
  if (messageManager) {
    int messageCount = messageManager->getUnreadMessageCount();
    if (messageCount != lastMessageCount) {
      needMessageRefresh = true;
      lastMessageCount = messageCount;
      lastMessageUpdateTime = currentTime;
    }
  }
  
  // 检查是否需要刷新右侧面板
  bool needRightPanelRefresh = false;
  if (currentRightPage == RIGHT_PAGE_STOCK && stockManager) {
    if (currentTime - lastStockUpdateTime >= STOCK_UPDATE_INTERVAL) { // 5 分钟
      needRightPanelRefresh = true;
      lastStockUpdateTime = currentTime;
    }
  } else if (currentRightPage == RIGHT_PAGE_CALENDAR) {
    if (currentTime - lastCalendarUpdateTime >= CALENDAR_UPDATE_INTERVAL) { // 1 小时
      needRightPanelRefresh = true;
      lastCalendarUpdateTime = currentTime;
    }
  }
  
  // 执行局部刷新
  if (needClockRefresh) {
    // 只刷新时钟区域
    if (timeManager) {
      if (currentClockMode == CLOCK_MODE_DIGITAL) {
        drawDigitalClock(20, 60, timeManager->getTimeString(), timeManager->getDateString());
        // 精确计算时钟区域的刷新范围
        int clockHeight = height < 400 ? 120 : 200;
        displayDriver->update(0, 0, leftPanelWidth, clockHeight);
      } else if (currentClockMode == CLOCK_MODE_ANALOG) {
        TimeData timeData = timeManager->getTimeData();
        drawAnalogClock(leftPanelWidth / 2, 120, timeData.hour, timeData.minute, timeData.second);
        // 精确计算模拟时钟区域的刷新范围
        int clockRadius = height < 400 ? 40 : 60;
        int clockX = leftPanelWidth / 2;
        int clockY = 120;
        displayDriver->update(clockX - clockRadius - 10, clockY - clockRadius - 10, clockRadius * 2 + 20, clockRadius * 2 + 20);
      } else if (currentClockMode == CLOCK_MODE_TEXT) {
        TimeData timeData = timeManager->getTimeData();
        drawTextClock(20, 60, timeData.hour, timeData.minute, timeData.second);
        // 精确计算文字时钟区域的刷新范围
        int clockHeight = height < 400 ? 120 : 200;
        displayDriver->update(0, 0, leftPanelWidth, clockHeight);
      }
    }
  }
  
  if (needWeatherRefresh && weatherManager) {
    // 只刷新天气区域
    WeatherData weather = weatherManager->getWeatherData();
    int weatherY = height < 400 ? 140 : 220;
    int weatherHeight = height < 400 ? 100 : 150;
    drawWeather(20, weatherY, weather.city, 
                (weather.temp != 0 ? String(weather.temp) : "--") + "°C", 
                weather.condition, "", "");
    displayDriver->update(0, weatherY, leftPanelWidth, weatherHeight);
  }
  
  if (needSensorRefresh && sensorManager) {
    // 只刷新传感器数据区域
    SensorData sensor = sensorManager->getSensorData();
    int sensorY = height < 400 ? 220 : 340;
    int sensorHeight = height < 400 ? 80 : 120;
    drawSensorData(20, sensorY, sensor.temperature, sensor.humidity);
    displayDriver->update(0, sensorY, leftPanelWidth, sensorHeight);
  }
  
  if (needBatteryRefresh || needMessageRefresh) {
    // 只刷新底部区域
    if (powerManager && messageManager) {
      float batteryVoltage = powerManager->getBatteryVoltage();
      int batteryPercentage = powerManager->getBatteryPercentage();
      bool isCharging = powerManager->getChargingStatus();
      int messageCount = messageManager->getUnreadMessageCount();
      
      // 绘制左上角状态栏（电量和消息）
      drawStatusBar(20, 20, batteryVoltage, batteryPercentage, isCharging, messageCount);
      displayDriver->update(0, 0, leftPanelWidth, height < 400 ? 40 : 60);
      
      // 如果有新消息，启动消息提醒动画
      if (needMessageRefresh && messageCount > 0) {
        startMessageAnimation();
      }
    }
  }
  
  if (needRightPanelRefresh) {
    // 只刷新右侧面板
    drawRightPanel();
    displayDriver->update(leftPanelWidth, 0, rightPanelWidth, height);
  }
  
  // 更新消息提醒动画
  updateMessageAnimation();
  
  // 更新传感器报警状态
  updateSensorAlarm();
}

void DisplayManager::showAlarm(String alarmType, String message) {
  #if ENABLE_ALARM_DISPLAY
  DEBUG_PRINTLN("显示报警信息...");
  
  alarmShowing = true;
  currentAlarmType = alarmType;
  currentAlarmMessage = message;
  lastAlarmUpdateTime = millis();
  lastBlinkTime = millis();
  alarmBlinkState = true;
  alarmStartTime = millis(); // 记录报警开始时间
  
  // 立即更新显示
  updateAlarmDisplay();
  #endif
}

void DisplayManager::hideAlarm() {
  #if ENABLE_ALARM_DISPLAY
  DEBUG_PRINTLN("隐藏报警信息...");
  
  alarmShowing = false;
  currentAlarmType = "";
  currentAlarmMessage = "";
  
  // 立即更新显示，恢复正常界面
  updateDisplay();
  #endif
}

void DisplayManager::updateAlarmDisplay() {
  #if ENABLE_ALARM_DISPLAY
  if (!alarmShowing || displayDriver == nullptr) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // 检查是否需要自动恢复（超过报警超时时间）
  if (currentTime - alarmStartTime >= ALARM_TIMEOUT) {
    hideAlarm();
    return;
  }
  
  // 检查是否需要刷新闪烁效果
  if (currentTime - lastBlinkTime >= ALARM_BLINK_INTERVAL) {
    alarmBlinkState = !alarmBlinkState;
    lastBlinkTime = currentTime;
  }
  
  // 清除屏幕
  displayDriver->clear();
  
  // 设置报警文字大小
  // 注意：IDisplayDriver接口中没有setFontSize方法，我们直接在drawString中指定大小
  
  // 居中显示报警信息
  String fullMessage = currentAlarmType + "\n" + currentAlarmMessage;
  
  // 计算文字位置
  int16_t x = (width - displayDriver->measureTextWidth(fullMessage, ALARM_TEXT_SIZE)) / 2;
  int16_t y = (height - displayDriver->measureTextHeight(fullMessage, ALARM_TEXT_SIZE)) / 2;
  
  // 绘制报警信息
  displayDriver->drawString(x, y, fullMessage, alarmBlinkState ? BLACK : WHITE, alarmBlinkState ? WHITE : BLACK, ALARM_TEXT_SIZE);
  
  // 根据配置选择刷新方式
  #if ALARM_FULL_REFRESH
    displayDriver->update();
  #else
    // 局部刷新
    displayDriver->update(0, 0, width, height);
  #endif
  
  lastAlarmUpdateTime = currentTime;
  #endif
}

void DisplayManager::showMessage(String message, uint32_t duration) {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 非阻塞方式显示消息
  static bool isMessageShowing = false;
  static unsigned long messageStartTime = 0;
  static String currentMessage = "";
  static uint32_t messageDuration = 0;
  
  if (!isMessageShowing) {
    // 开始显示消息
    isMessageShowing = true;
    messageStartTime = millis();
    currentMessage = message;
    messageDuration = duration;
    
    // 保存当前显示内容
    // TODO: 实现显示内容保存和恢复
    
    // 显示消息
    clearScreen();
    
    int textSize;
    int messageX, messageY;
    
    if (height < 400) {
      // 小屏幕
      textSize = 2;
      messageX = 20;
      messageY = height / 2 - 20;
    } else {
      // 大屏幕
      textSize = 3;
      messageX = 40;
      messageY = height / 2 - 40;
    }
    
    displayDriver->drawString(messageX, messageY, message, GxEPD_BLACK, GxEPD_WHITE, textSize);
    displayDriver->update();
    
    DEBUG_PRINTF("显示消息: %s, 持续时间: %lu毫秒\n", message.c_str(), duration);
  } else {
    // 检查消息是否需要结束
    if (millis() - messageStartTime >= messageDuration) {
      isMessageShowing = false;
      // 恢复之前的显示内容
      // TODO: 实现显示内容恢复
      updateDisplay();
      DEBUG_PRINTLN("消息显示结束，恢复正常显示");
    }
  }
}

void DisplayManager::switchRightPage(RightPageType page) {
  currentRightPage = page;
  updateDisplay();
}

void DisplayManager::toggleClockMode() {
  switch (currentClockMode) {
    case CLOCK_MODE_DIGITAL:
      currentClockMode = CLOCK_MODE_ANALOG;
      break;
    case CLOCK_MODE_ANALOG:
      currentClockMode = CLOCK_MODE_TEXT;
      break;
    case CLOCK_MODE_TEXT:
      currentClockMode = CLOCK_MODE_DIGITAL;
      break;
  }
  updateDisplay();
}

RightPageType DisplayManager::getCurrentRightPage() const {
  return currentRightPage;
}

ClockMode DisplayManager::getCurrentClockMode() const {
  return currentClockMode;
}

int16_t DisplayManager::getWidth() const {
  return width;
}

int16_t DisplayManager::getHeight() const {
  return height;
}

void DisplayManager::drawHeader(String title) {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 根据屏幕尺寸动态调整标题栏高度
  int headerHeight;
  int textSize;
  int cursorX, cursorY;
  
  if (height < 400) {
    // 小屏幕
    headerHeight = 30;
    textSize = 2;
    cursorX = 15;
    cursorY = 20;
  } else {
    // 大屏幕
    headerHeight = 40;
    textSize = 3;
    cursorX = 20;
    cursorY = 28;
  }
  
  // 绘制标题栏背景
  displayDriver->fillRect(0, 0, width, headerHeight, GxEPD_BLACK);
  
  // 绘制标题
  displayDriver->drawString(cursorX, cursorY, title, GxEPD_WHITE, GxEPD_BLACK, textSize);
}

void DisplayManager::drawFooter() {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 根据屏幕尺寸动态调整页脚位置和字体大小
  int textSize;
  int cursorX, cursorY;
  
  if (height < 400) {
    // 小屏幕
    textSize = 1;
    cursorX = 15;
    cursorY = height - 10;
  } else {
    // 大屏幕
    textSize = 1;
    cursorX = 20;
    cursorY = height - 20;
  }
  
  // 绘制页脚
  displayDriver->drawString(cursorX, cursorY, "家用网络智能墨水屏万年历 v1.0", GxEPD_GRAY2, GxEPD_WHITE, textSize);
}

void DisplayManager::clearScreen() {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->clear();
}

void DisplayManager::drawLeftPanel() {
  if (displayDriver == nullptr) {
    return;
  }
  
  switch (currentLayout.screenSize) {
    case SCREEN_SIZE_2_9_INCH:
      drawUI_2_9inch();
      break;
    case SCREEN_SIZE_2_13_INCH:
      drawUI_2_13inch();
      break;
    case SCREEN_SIZE_1_54_INCH:
      drawUI_1_54inch();
      break;
    case SCREEN_SIZE_4_2_INCH:
      drawUI_4_2inch();
      break;
    case SCREEN_SIZE_3_7_INCH:
      drawUI_3_7inch();
      break;
    case SCREEN_SIZE_2_66_INCH:
      drawUI_2_66inch();
      break;
    case SCREEN_SIZE_1_44_INCH:
      drawUI_1_44inch();
      break;
    case SCREEN_SIZE_1_02_INCH:
      drawUI_1_02inch();
      break;
    case SCREEN_SIZE_7_5_INCH:
      drawUI_7_5inch();
      break;
    case SCREEN_SIZE_7_3_INCH:
      drawUI_7_3inch();
      break;
    case SCREEN_SIZE_6_INCH:
      drawUI_6inch();
      break;
    case SCREEN_SIZE_5_83_INCH:
      drawUI_5_83inch();
      break;
    default:
      drawLeftPanelDefault();
      break;
  }
}

void DisplayManager::drawLeftPanelDefault() {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 根据屏幕方向绘制左侧面板背景
  if (currentLayout.orientation == ORIENTATION_HORIZONTAL) {
    // 横向布局：左右分栏
    displayDriver->fillRect(0, 0, leftPanelWidth, height, GxEPD_WHITE);
  } else {
    // 竖向布局：上下分栏
    displayDriver->fillRect(0, 0, width, leftPanelHeight, GxEPD_WHITE);
  }
  
  // 绘制分割线（已在 applyLayout 中绘制）
  
  // 获取各种数据，使用本地缓存的数据
  String timeStr = "--:--:--";
  String dateStr = "YYYY-MM-DD";
  
  // 从缓存中获取数据
  TimeData currentTime = cachedTimeData;
  WeatherData weather = cachedWeatherData;
  SensorData sensor = cachedSensorData;
  float batteryVoltage = cachedBatteryVoltage;
  int batteryPercentage = cachedBatteryPercentage;
  bool isCharging = cachedIsCharging;
  int messageCount = cachedUnreadMessageCount;
  
  // 构建时间字符串
  timeStr = String(currentTime.hour < 10 ? "0" : "") + String(currentTime.hour) + ":" + 
            String(currentTime.minute < 10 ? "0" : "") + String(currentTime.minute) + ":" + 
            String(currentTime.second < 10 ? "0" : "") + String(currentTime.second);
  
  // 构建日期字符串
  dateStr = String(currentTime.year) + "-" + 
            String(currentTime.month < 10 ? "0" : "") + String(currentTime.month) + "-" + 
            String(currentTime.day < 10 ? "0" : "") + String(currentTime.day);
    
    // 绘制时钟（根据当前时钟模式）
    if (currentClockMode == CLOCK_MODE_DIGITAL) {
      // 数字时钟 - 占用左侧 35% 高度，时间位置下移
      int clockSectionHeight = leftPanelHeight * 0.35;
      int startY = clockSectionHeight * 0.15;  // 距离顶部 15% 的间距
      drawDigitalClock(20, startY + 30, timeStr, dateStr);  // Y 坐标下移
    } else if (currentClockMode == CLOCK_MODE_ANALOG) {
      // 模拟时钟 - 占用左侧 35% 高度，位置下移
      int clockSectionHeight = leftPanelHeight * 0.35;
      int centerX = leftPanelWidth / 2;
      int centerY = (clockSectionHeight / 2) + 30;  // 位置下移
      int radius = min(leftPanelWidth, clockSectionHeight) / 2 - 10;
      
      // 获取当前时间的时、分、秒
      int hour = 0;
      int minute = 0;
      int second = 0;
      int millisecond = millis() % 1000;
      
      if (timeStr.length() >= 8) {
        hour = timeStr.substring(0, 2).toInt();
        minute = timeStr.substring(3, 5).toInt();
        second = timeStr.substring(6, 8).toInt();
      }
      
      drawAnalogClock(centerX, centerY, hour, minute, second, millisecond, radius);
    } else if (currentClockMode == CLOCK_MODE_TEXT) {
      // 文字时钟 - 占用左侧 35% 高度，位置下移
      int clockSectionHeight = leftPanelHeight * 0.35;
      int startY = clockSectionHeight * 0.15;  // 距离顶部 15% 的间距
      
      // 获取当前时间的时、分、秒
      int hour = 0;
      int minute = 0;
      int second = 0;
      
      if (timeStr.length() >= 8) {
        hour = timeStr.substring(0, 2).toInt();
        minute = timeStr.substring(3, 5).toInt();
        second = timeStr.substring(6, 8).toInt();
      }
      
      drawTextClock(20, startY + 30, hour, minute, second);  // Y 坐标下移
    }
    
    // 绘制公历和农历年月日信息（合并到一行）
    // 获取当前日期的农历信息
    LunarInfo lunarInfo;
    auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
    if (lunarManager) {
      lunarInfo = lunarManager->getLunarInfo(currentTime.year, currentTime.month, currentTime.day);
    }
    
    // 构建公历和农历日期字符串（合并到一行）
    String gregorianStr = String(currentTime.year) + "-" + 
                         (currentTime.month < 10 ? "0" : "") + String(currentTime.month) + "-" + 
                         (currentTime.day < 10 ? "0" : "") + String(currentTime.day);
    
    String lunarStr = lunarInfo.lunarDate;
    
    // 绘制在时钟下方（仍在 35% 区域内，一行显示）
    int clockSectionHeight = leftPanelHeight * 0.35;
    int dateY = clockSectionHeight * 0.15 + 80;  // 时间下方
    String combinedDate = gregorianStr + " | " + lunarStr;
    displayDriver->drawString(20, dateY, combinedDate, GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 1 : 2);
    
    // 绘制天气信息，调整位置到 35% 区域下方
    drawWeather(20, clockSectionHeight + 10, weather.city,
                (weather.temp != 0 ? String(weather.temp) : "--") + "°C", 
                weather.condition, "", "");
    
    // 绘制室内温湿度，调整位置到天气信息下方
  drawSensorData(20, height < 400 ? 180 : 280, sensor.temperature, sensor.humidity);
  
  // 绘制左上角状态栏（电量和消息）
  drawStatusBar(20, 20, batteryVoltage, batteryPercentage, isCharging, messageCount);
}

void DisplayManager::drawRightPanel() {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 根据屏幕尺寸调用对应的 UI 绘制函数
  switch (currentLayout.screenSize) {
    case SCREEN_SIZE_7_5_INCH:
      drawRightPanel_7_5inch();
      break;
    case SCREEN_SIZE_7_3_INCH:
      drawRightPanel_7_3inch();
      break;
    case SCREEN_SIZE_6_INCH:
      drawRightPanel_6inch();
      break;
    case SCREEN_SIZE_5_83_INCH:
      drawRightPanel_5_83inch();
      break;
    default:
      // 其他尺寸使用原有绘制逻辑
      drawRightPanelDefault();
      break;
  }
}

void DisplayManager::drawRightPanelDefault() {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 根据屏幕方向绘制右侧面板背景
  if (currentLayout.orientation == ORIENTATION_HORIZONTAL) {
    // 横向布局：左右分栏
    displayDriver->fillRect(leftPanelWidth, 0, rightPanelWidth, height, GxEPD_WHITE);
  } else {
    // 竖向布局：上下分栏
    displayDriver->fillRect(0, leftPanelHeight, width, rightPanelHeight, GxEPD_WHITE);
  }
  
  // 检查是否有新消息，如果有且当前页面是日历，则显示消息通知
  int messageCount = cachedUnreadMessageCount;
  bool showMessageNotification = (messageCount > 0 && currentRightPage == RIGHT_PAGE_CALENDAR);
  
  // 根据当前右侧页面绘制不同内容
  if (showMessageNotification) {
    // 当有消息通知时，替换日历显示为消息内容
    drawMessageNotificationContent(leftPanelWidth + 20, 20);
  } else {
    // 正常显示当前页面内容
    switch (currentRightPage) {
      case RIGHT_PAGE_CALENDAR: {
        drawCalendarPage(leftPanelWidth + 20, 30);
        
        // 在月历下方绘制当前日的节日和黄历信息，确保完整显示（上下排列的两个独立模块）
        // 获取当前日期
        auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
        auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
        if (timeManager && lunarManager) {
          TimeData currentTime = timeManager->getTimeData();
          LunarInfo lunarInfo = lunarManager->getLunarInfo(currentTime.year, currentTime.month, currentTime.day);
          
          // 绘制节日模块（紧凑布局，独立模块）
          if (!lunarInfo.festival.name.isEmpty()) {
            // 节日信息一行显示：节日名称 · 日期 · 倒计时
            String festivalLine = lunarInfo.festival.name + " · 4 月 5 日 · 还有 18 天";
            displayDriver->drawString(leftPanelWidth + 20, height - 85, festivalLine, GxEPD_RED, GxEPD_WHITE, height < 400 ? 1 : 2);
          }
          
          // 绘制黄历模块（紧凑布局，独立模块，上下排列）
          if (!lunarInfo.lunarCalendar.yi.isEmpty() && !lunarInfo.lunarCalendar.ji.isEmpty()) {
            // 黄历标题
            displayDriver->drawString(leftPanelWidth + 20, height - 120, "📜 黄历", GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 2 : 3);
            // 宜（一行显示）
            String yiText = "✅ 宜：" + lunarInfo.lunarCalendar.yi;
            displayDriver->drawString(leftPanelWidth + 20, height - 95, yiText, GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 1 : 2);
            // 忌（一行显示）
            String jiText = "❌ 忌：" + lunarInfo.lunarCalendar.ji;
            displayDriver->drawString(leftPanelWidth + 20, height - 75, jiText, GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 1 : 2);
          }
        }
        
        break;
      }
      case RIGHT_PAGE_STOCK:
        drawStockPage(leftPanelWidth + 20, 20);
        break;
      case RIGHT_PAGE_MESSAGE:
        drawMessagePage(leftPanelWidth + 20, 20);
        break;
      case RIGHT_PAGE_PLUGIN:
        drawPluginPage(leftPanelWidth + 20, 20);
        break;
      case RIGHT_PAGE_PLUGIN_MANAGE:
        drawPluginManagePage(leftPanelWidth + 20, 20);
        break;
      case RIGHT_PAGE_SETTING:
        drawSettingPage(leftPanelWidth + 20, 20);
        break;
      default:
        // 绘制默认页面
        int textSize = height < 400 ? 2 : 3;
        displayDriver->drawString(leftPanelWidth + 20, 20, "页面未定义", GxEPD_BLACK, GxEPD_WHITE, textSize);
        break;
    }
  }
}

void DisplayManager::drawMessageNotificationContent(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 绘制消息通知标题
  int titleSize = height < 400 ? 3 : 4;
  displayDriver->drawString(x, y, "新消息通知", GxEPD_RED, GxEPD_WHITE, titleSize);
  
  // 获取最新的消息（使用缓存）
  int messageCount = cachedUnreadMessageCount;
  displayDriver->drawString(x, y + (height < 400 ? 30 : 50), String(messageCount) + "条未读消息", GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 2 : 3);
  
  // 绘制消息列表
  int messageY = y + (height < 400 ? 60 : 100);
  int messageItemHeight = height < 400 ? 40 : 60;
  
  // 这里假设MessageManager有获取消息列表的方法
  // 实际项目中需要根据具体实现调整
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  for (int i = 0; i < min(messageCount, 5); i++) {
    // 绘制消息标题和摘要
    String message = "消息 " + String(i + 1);
    String time = "刚刚";
    MessagePriority priority = MESSAGE_PRIORITY_NORMAL;
    
    // 尝试获取实际的消息优先级
    if (messageManager && i < messageManager->getMessageCount()) {
      MessageData msgData = messageManager->getMessage(String(i + 1));
      if (msgData.valid) {
        priority = msgData.priority;
      }
    }
    
    drawMessageItem(x, messageY, message, time, priority);
    messageY += messageItemHeight;
  }
  
  // 绘制提示信息
  displayDriver->drawString(x, height - 30, "点击按钮切换回日历", GxEPD_GRAY2, GxEPD_WHITE, height < 400 ? 1 : 2);
}

void DisplayManager::drawDigitalClock(int x, int y, String time, String date) {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 根据当前主题获取字体大小
  int clockSize, dateSize;
  
  if (height < 400) {
    // 小屏幕
    clockSize = 4;
    dateSize = 1;
  } else {
    // 大屏幕
    clockSize = 7;
    dateSize = 2;
  }
  
  // 绘制时间
  displayDriver->drawString(x, y, time, GxEPD_BLACK, GxEPD_WHITE, clockSize);
  
  // 绘制日期
  if (dateSize > 0) {
    int dateY = height < 400 ? y + 50 + (clockSize - 5) * 8 : y + 90 + (clockSize - 8) * 12;
    displayDriver->drawString(x, dateY, date, GxEPD_RED, GxEPD_WHITE, dateSize);
  }
}

void DisplayManager::drawAnalogClock(int x, int y, int hour, int minute, int second, int millisecond) {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 时钟半径
  int radius = height < 400 ? 40 : 60;
  
  // 绘制时钟外圆
  displayDriver->drawRect(x - radius, y - radius, radius * 2, radius * 2, GxEPD_BLACK);
  
  // 绘制时钟刻度
  for (int i = 0; i < 12; i++) {
    float angle = i * PI / 6 - PI / 2;
    int x1 = x + cos(angle) * (radius - 5);
    int y1 = y + sin(angle) * (radius - 5);
    int x2 = x + cos(angle) * radius;
    int y2 = y + sin(angle) * radius;
    displayDriver->drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  }
  
  // 计算精确的角度（支持平滑动画）
  float totalSeconds = hour * 3600 + minute * 60 + second + millisecond / 1000.0;
  float hourAngle = (totalSeconds / 43200.0) * 2 * PI - PI / 2;
  float minuteAngle = (totalSeconds / 3600.0) * 2 * PI - PI / 2;
  float secondAngle = (totalSeconds / 60.0) * 2 * PI - PI / 2;
  
  // 绘制时针
  int hourX = x + cos(hourAngle) * (radius - 20);
  int hourY = y + sin(hourAngle) * (radius - 20);
  displayDriver->drawLine(x, y, hourX, hourY, GxEPD_BLACK);
  
  // 绘制分针
  int minuteX = x + cos(minuteAngle) * (radius - 10);
  int minuteY = y + sin(minuteAngle) * (radius - 10);
  displayDriver->drawLine(x, y, minuteX, minuteY, GxEPD_BLACK);
  
  // 绘制秒针 - 仅当showSeconds为true时显示
  if (showSeconds) {
    int secondX = x + cos(secondAngle) * (radius - 5);
    int secondY = y + sin(secondAngle) * (radius - 5);
    displayDriver->drawLine(x, y, secondX, secondY, GxEPD_RED);
  }
  
  // 绘制中心点
  displayDriver->drawRect(x - 2, y - 2, 4, 4, GxEPD_BLACK);
}

void DisplayManager::drawTextClock(int x, int y, int hour, int minute, int second) {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 根据屏幕尺寸设置字体大小
  int textSize = height < 400 ? 2 : 3;
  int lineHeight = height < 400 ? 30 : 40;
  
  // 构建文字时钟内容
  String text;
  
  // 上午/下午
  String period = hour < 12 ? "上午" : "下午";
  
  // 小时转换为12小时制
  int hour12 = hour % 12;
  if (hour12 == 0) hour12 = 12;
  
  // 构建时间文字描述
  text = "现在是" + period + "" + String(hour12) + "点";
  
  if (minute > 0) {
    text += String(minute) + "分";
  }
  
  if (showSeconds && second > 0) {
    text += String(second) + "秒";
  }
  
  // 绘制文字时钟
  displayDriver->drawString(x, y, text, GxEPD_BLACK, GxEPD_WHITE, textSize);
  
  // 绘制时区信息
  String timezoneText = "时区: " + currentTimeZone.abbreviation;
  displayDriver->drawString(x, y + lineHeight, timezoneText, GxEPD_GRAY2, GxEPD_WHITE, textSize - 1);
  
  // 绘制日期信息
  TimeData currentTime = cachedTimeData;
  String dateText = String(currentTime.year) + "年" + 
                    String(currentTime.month) + "月" + 
                    String(currentTime.day) + "日";
  displayDriver->drawString(x, y + lineHeight * 2, dateText, GxEPD_RED, GxEPD_WHITE, textSize - 1);
}

void DisplayManager::drawBatteryInfo(int x, int y, float voltage, int percentage, bool isCharging) {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 根据电量百分比设置不同颜色
  uint16_t batteryColor = GxEPD_BLACK;
  if (percentage < 20) {
    batteryColor = GxEPD_RED;
  } 
  
  int batteryX = x;
  int batteryY = y;
  int batteryWidth = height < 400 ? 30 : 50;
  int batteryHeight = height < 400 ? 15 : 25;
  
  // 绘制电池图标
  displayDriver->drawRect(batteryX, batteryY, batteryWidth, batteryHeight, GxEPD_BLACK);
  displayDriver->drawRect(batteryX + batteryWidth, batteryY + (height < 400 ? 3 : 5), 
                         (height < 400 ? 4 : 6), batteryHeight - (height < 400 ? 6 : 10), GxEPD_BLACK);
  
  // 绘制电池电量
  int batteryLevelWidth = (batteryWidth - (height < 400 ? 4 : 6)) * percentage / 100;
  displayDriver->fillRect(batteryX + (height < 400 ? 2 : 3), batteryY + (height < 400 ? 2 : 3), 
                         batteryLevelWidth, batteryHeight - (height < 400 ? 4 : 6), batteryColor);
  
  // 绘制电量文字
  int textSize = height < 400 ? 2 : 3;
  int textX = batteryX + batteryWidth + (height < 400 ? 10 : 15);
  int textY = y + (height < 400 ? 12 : 20);
  displayDriver->drawString(textX, textY, String(percentage) + "%", batteryColor, GxEPD_WHITE, textSize);
  
  // 绘制充电状态或电压
  int statusY = y + (height < 400 ? 30 : 50);
  displayDriver->drawString(x, statusY, isCharging ? "充电中" : String(voltage, 1) + "V", GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 1 : 2);
}

void DisplayManager::drawMessageNotification(int x, int y, int messageCount) {
  if (displayDriver == nullptr) {
    return;
  }
  
  int textSize = height < 400 ? 2 : 3;
  
  if (messageCount > 0) {
    // 检查是否有高优先级消息
    bool hasUrgentMessage = false;
    bool hasHighPriorityMessage = false;
    
    auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
    if (messageManager) {
      for (int i = 0; i < messageCount; i++) {
        MessageData message = messageManager->getMessage(String(i + 1));
        if (message.priority == MESSAGE_PRIORITY_URGENT) {
          hasUrgentMessage = true;
          break;
        } else if (message.priority == MESSAGE_PRIORITY_HIGH) {
          hasHighPriorityMessage = true;
        }
      }
    }
    
    // 根据优先级设置颜色
    uint16_t textColor = GxEPD_RED;
    uint16_t dotColor = GxEPD_RED;
    
    if (hasUrgentMessage) {
      textColor = GxEPD_RED;
      dotColor = GxEPD_RED;
    } else if (hasHighPriorityMessage) {
      textColor = GxEPD_RED;
      dotColor = GxEPD_RED;
    } else {
      textColor = GxEPD_BLACK;
      dotColor = GxEPD_BLACK;
    }
    
    displayDriver->drawString(x, y, String(messageCount) + "条新消息", textColor, GxEPD_WHITE, textSize);
    
    // 绘制圆点提示
    displayDriver->fillRect(x + (height < 400 ? 18 : 27), y - (height < 400 ? 2 : 3), 
                           height < 400 ? 6 : 10, height < 400 ? 6 : 10, dotColor);
  } else {
    displayDriver->drawString(x, y, "无新消息", GxEPD_BLACK, GxEPD_WHITE, textSize);
  }
}

// 绘制左上角状态栏（电量和消息）
void DisplayManager::drawStatusBar(int x, int y, float voltage, int percentage, bool isCharging, int messageCount) {
  if (displayDriver == nullptr) {
    return;
  }
  
  int batteryWidth = height < 400 ? 24 : 28;
  int batteryHeight = height < 400 ? 12 : 14;
  int textSize = height < 400 ? 1 : 2;
  
  // 绘制电池图标
  displayDriver->drawRect(x, y, batteryWidth, batteryHeight, GxEPD_BLACK);
  // 电池正极
  displayDriver->drawRect(x + batteryWidth, y + (height < 400 ? 2 : 3), 
                         (height < 400 ? 3 : 4), batteryHeight - (height < 400 ? 4 : 6), GxEPD_BLACK);
  
  // 绘制电池电量
  int batteryLevelWidth = (batteryWidth - (height < 400 ? 4 : 6)) * percentage / 100;
  displayDriver->fillRect(x + (height < 400 ? 2 : 3), y + (height < 400 ? 2 : 3), 
                         batteryLevelWidth, batteryHeight - (height < 400 ? 4 : 6), GxEPD_BLACK);
  
  // 绘制电量百分比文字
  int textX = x + batteryWidth + (height < 400 ? 8 : 10);
  int textY = y + (height < 400 ? 10 : 12);
  displayDriver->drawString(textX, textY, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, textSize);
  
  // 绘制消息数量（在电量右侧）
  if (messageCount > 0) {
    int messageX = textX + (height < 400 ? 40 : 50);
    displayDriver->drawString(messageX, textY, "🔔 " + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, textSize);
  }
}

// 其他绘制方法的实现...
// 由于篇幅限制，这里省略了部分绘制方法的实现
// 实际使用时需要将所有绘制方法从eink_display.cpp迁移到这里

void DisplayManager::drawWeather(int x, int y, String city, String temp, String condition, String humidity, String wind) {
  if (displayDriver == nullptr) {
    return;
  }
  
  int textSize = height < 400 ? 1 : 2;
  int tempSize = height < 400 ? 3 : 4;
  
  // 绘制城市
  displayDriver->drawString(x, y, city, GxEPD_BLACK, GxEPD_WHITE, textSize);
  
  // 绘制当前温度
  displayDriver->drawString(x, y + (height < 400 ? 15 : 25), temp, GxEPD_BLACK, GxEPD_WHITE, tempSize);
  
  // 绘制天气状况
  displayDriver->drawString(x, y + (height < 400 ? 35 : 55), condition, GxEPD_BLACK, GxEPD_WHITE, textSize);
  
  // 绘制天气图标
  String weatherIcon = "☀️";
  ForecastData tomorrow, dayAfter;
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (weatherManager) {
    weatherIcon = weatherManager->getWeatherIcon(condition);
    tomorrow = weatherManager->getForecastData(1);
    dayAfter = weatherManager->getForecastData(2);
  }
  displayDriver->drawString(x + (height < 400 ? 60 : 120), y + (height < 400 ? 25 : 45), weatherIcon, GxEPD_BLACK, GxEPD_WHITE, tempSize);
  
  // 一行显示今天、明天、后天天气预报（三个并列）
  if (weatherManager && tomorrow.date.length() > 0) {
    int forecastY = y + (height < 400 ? 55 : 85);
    String tomorrowIcon = weatherManager->getWeatherIcon(tomorrow.condition);
    String dayAfterIcon = "☀️";
    if (dayAfter.date.length() > 0) {
      dayAfterIcon = weatherManager->getWeatherIcon(dayAfter.condition);
    }
    
    // 绘制三个预报（一行显示）
    String forecastText = "今天 " + tomorrowIcon + " " + temp + "° | 明天 " + tomorrowIcon + " " + String(tomorrow.tempDay) + "°";
    if (dayAfter.date.length() > 0) {
      forecastText += " | 后天 " + dayAfterIcon + " " + String(dayAfter.tempDay) + "°";
    }
    displayDriver->drawString(x, forecastY, forecastText, GxEPD_BLACK, GxEPD_WHITE, textSize);
  }
  
  // 绘制 5 天温度趋势图表
  int chartY = y + (height < 400 ? 90 : 150);
  int chartWidth = leftPanelWidth - 40;
  int chartHeight = height < 400 ? 60 : 80;
  
  // 获取5天天气预报数据
  float temps[5] = {0};
  float minTemp = 100, maxTemp = -100;
  
  if (weatherManager) {
    for (int i = 0; i < 5; i++) {
      ForecastData forecast = weatherManager->getForecastData(i);
      temps[i] = forecast.tempDay;
      if (temps[i] < minTemp) minTemp = temps[i];
      if (temps[i] > maxTemp) maxTemp = temps[i];
    }
  }
  
  // 计算温度范围（添加一些边距）
  float tempRange = maxTemp - minTemp;
  if (tempRange == 0) tempRange = 10; // 防止除零
  
  // 绘制图表边框
  displayDriver->drawRect(x, chartY, chartWidth, chartHeight, GxEPD_BLACK);
  
  // 绘制温度趋势线
  for (int i = 0; i < 4; i++) {
    if (temps[i] == 0 || temps[i+1] == 0) continue;
    
    int x1 = x + (i * chartWidth) / 4;
    int y1 = chartY + chartHeight - static_cast<int>(((temps[i] - minTemp) / tempRange) * chartHeight);
    int x2 = x + ((i+1) * chartWidth) / 4;
    int y2 = chartY + chartHeight - static_cast<int>(((temps[i+1] - minTemp) / tempRange) * chartHeight);
    
    displayDriver->drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  }
  
  // 绘制温度点
  for (int i = 0; i < 5; i++) {
    if (temps[i] == 0) continue;
    
    int px = x + (i * chartWidth) / 4;
    int py = chartY + chartHeight - static_cast<int>(((temps[i] - minTemp) / tempRange) * chartHeight);
    
    // 绘制点
    displayDriver->drawRect(px - 2, py - 2, 4, 4, GxEPD_BLACK);
    
    // 绘制温度值
    displayDriver->drawString(px - 10, py - 15, String(temps[i], 0) + "°", GxEPD_BLACK, GxEPD_WHITE, textSize - 1);
  }
  
  // 绘制图表标题
  displayDriver->drawString(x, chartY - 20, "5天温度趋势", GxEPD_BLACK, GxEPD_WHITE, textSize - 1);
  
  // 绘制空气质量和紫外线指数
  WeatherData weather;
  int extraInfoY = chartY + chartHeight + 20;
  if (weatherManager) {
    weather = weatherManager->getWeatherData();
    if (weather.airQuality > 0) {
      String aqiText = "空气质量: " + String(weather.airQuality) + " " + weather.airQualityLevel;
      displayDriver->drawString(x, extraInfoY, aqiText, GxEPD_BLACK, GxEPD_WHITE, textSize);
    }
    
    if (weather.uvIndex > 0) {
      String uvText = "紫外线: " + String(weather.uvIndex, 1) + " " + weather.uvIndexLevel;
      displayDriver->drawString(x, extraInfoY + 20, uvText, GxEPD_BLACK, GxEPD_WHITE, textSize);
    }
  }
}


void DisplayManager::drawSensorData(int x, int y, float temperature, float humidity) {
  if (displayDriver == nullptr) {
    return;
  }
  
  int titleSize = height < 400 ? 2 : 3;
  int dataSize = height < 400 ? 1 : 2;
  
  // 更新传感器数据历史记录
  tempHistory[sensorHistoryIndex] = temperature;
  humHistory[sensorHistoryIndex] = humidity;
  sensorHistoryIndex = (sensorHistoryIndex + 1) % MAX_SENSOR_HISTORY;
  
  // 检查传感器数据异常
  checkSensorAnomalies(temperature, humidity);
  
  // 绘制标题
  displayDriver->drawString(x, y, "室内环境监测", GxEPD_BLACK, GxEPD_WHITE, titleSize);
  
  // 绘制温度
  displayDriver->drawString(x, y + (height < 400 ? 30 : 50), "温度: " + String(temperature) + "°C", 
                         GxEPD_BLACK, GxEPD_WHITE, dataSize);
  
  // 绘制湿度
  displayDriver->drawString(x, y + (height < 400 ? 50 : 90), "湿度: " + String(humidity) + "%", 
                         GxEPD_BLACK, GxEPD_WHITE, dataSize);
  
  // 绘制气体传感器数据
  int gasLevel = 0;
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (sensorManager) {
    gasLevel = sensorManager->getGasLevel();
  }
  String gasStatus = "正常";
  uint16_t gasColor = GxEPD_BLACK;
  if (gasLevel > 800) {
    gasStatus = "异常";
    gasColor = GxEPD_RED;
  } else if (gasLevel > 500) {
    gasStatus = "警告";
    gasColor = GxEPD_RED;
  }
  displayDriver->drawString(x, y + (height < 400 ? 70 : 130), "空气质量: " + gasStatus, 
                         gasColor, GxEPD_WHITE, dataSize);
  
  // 绘制光照强度
  int lightLevel = 0;
  if (sensorManager) {
    lightLevel = sensorManager->getLightLevel();
  }
  String lightStatus = "暗";
  if (lightLevel > 500) {
    lightStatus = "亮";
  } else if (lightLevel > 200) {
    lightStatus = "中等";
  }
  displayDriver->drawString(x, y + (height < 400 ? 90 : 170), "光照: " + lightStatus, 
                         GxEPD_BLACK, GxEPD_WHITE, dataSize);
  
  // 绘制人体感应
  bool motionDetected = false;
  if (sensorManager) {
    motionDetected = sensorManager->getMotionDetected();
  }
  displayDriver->drawString(x, y + (height < 400 ? 110 : 210), String("人体感应: ") + (motionDetected ? "有人" : "无人"), 
                         GxEPD_BLACK, GxEPD_WHITE, dataSize);
  
  // 绘制火焰检测
  bool flameDetected = false;
  if (sensorManager) {
    flameDetected = sensorManager->getFlameDetected();
  }
  uint16_t flameColor = GxEPD_BLACK;
  if (flameDetected) {
    flameColor = GxEPD_RED;
  }
  displayDriver->drawString(x, y + (height < 400 ? 130 : 250), String("火焰检测: ") + (flameDetected ? "检测到" : "未检测到"), 
                         flameColor, GxEPD_WHITE, dataSize);
  
  // 绘制传感器数据趋势图表
  int chartY = y + (height < 400 ? 150 : 290);
  int chartWidth = leftPanelWidth - 40;
  int chartHeight = height < 400 ? 60 : 80;
  
  // 计算温度范围
  float minTemp = 100, maxTemp = -100;
  float minHum = 100, maxHum = -100;
  
  for (int i = 0; i < MAX_SENSOR_HISTORY; i++) {
    if (tempHistory[i] < minTemp) minTemp = tempHistory[i];
    if (tempHistory[i] > maxTemp) maxTemp = tempHistory[i];
    if (humHistory[i] < minHum) minHum = humHistory[i];
    if (humHistory[i] > maxHum) maxHum = humHistory[i];
  }
  
  // 添加一些边距
  minTemp -= 2;
  maxTemp += 2;
  minHum -= 5;
  maxHum += 5;
  
  // 计算温度和湿度范围（防止除零）
  float tempRange = maxTemp - minTemp;
  if (tempRange == 0) tempRange = 10;
  
  float humRange = maxHum - minHum;
  if (humRange == 0) humRange = 20;
  
  // 绘制图表边框
  displayDriver->drawRect(x, chartY, chartWidth, chartHeight, GxEPD_BLACK);
  
  // 绘制温度趋势线
  for (int i = 0; i < MAX_SENSOR_HISTORY - 1; i++) {
    int x1 = x + (i * chartWidth) / (MAX_SENSOR_HISTORY - 1);
    int y1 = chartY + chartHeight - static_cast<int>(((tempHistory[i] - minTemp) / tempRange) * chartHeight);
    int x2 = x + ((i+1) * chartWidth) / (MAX_SENSOR_HISTORY - 1);
    int y2 = chartY + chartHeight - static_cast<int>(((tempHistory[i+1] - minTemp) / tempRange) * chartHeight);
    
    displayDriver->drawLine(x1, y1, x2, y2, GxEPD_RED);
  }
  
  // 绘制湿度趋势线
  for (int i = 0; i < MAX_SENSOR_HISTORY - 1; i++) {
    int x1 = x + (i * chartWidth) / (MAX_SENSOR_HISTORY - 1);
    int y1 = chartY + chartHeight - static_cast<int>(((humHistory[i] - minHum) / humRange) * chartHeight);
    int x2 = x + ((i+1) * chartWidth) / (MAX_SENSOR_HISTORY - 1);
    int y2 = chartY + chartHeight - static_cast<int>(((humHistory[i+1] - minHum) / humRange) * chartHeight);
    
    displayDriver->drawLine(x1, y1, x2, y2, GxEPD_BLUE);
  }
  
  // 绘制温度点
  for (int i = 0; i < MAX_SENSOR_HISTORY; i++) {
    int px = x + (i * chartWidth) / (MAX_SENSOR_HISTORY - 1);
    int py = chartY + chartHeight - static_cast<int>(((tempHistory[i] - minTemp) / tempRange) * chartHeight);
    
    displayDriver->drawRect(px - 2, py - 2, 4, 4, GxEPD_RED);
  }
  
  // 绘制湿度点
  for (int i = 0; i < MAX_SENSOR_HISTORY; i++) {
    int px = x + (i * chartWidth) / (MAX_SENSOR_HISTORY - 1);
    int py = chartY + chartHeight - static_cast<int>(((humHistory[i] - minHum) / humRange) * chartHeight);
    
    displayDriver->drawRect(px - 1, py - 1, 2, 2, GxEPD_BLUE);
  }
  
  // 绘制图表标题
  displayDriver->drawString(x, chartY - 20, "温湿度趋势", GxEPD_BLACK, GxEPD_WHITE, dataSize);
  
  // 绘制图例
  displayDriver->fillRect(x + chartWidth - 60, chartY - 15, 8, 8, GxEPD_RED);
  displayDriver->drawString(x + chartWidth - 50, chartY - 15, "温度", GxEPD_BLACK, GxEPD_WHITE, dataSize - 1);
  
  displayDriver->fillRect(x + chartWidth - 30, chartY - 15, 8, 8, GxEPD_BLUE);
  displayDriver->drawString(x + chartWidth - 20, chartY - 15, "湿度", GxEPD_BLACK, GxEPD_WHITE, dataSize - 1);
}

// 其他绘制方法的实现可以从eink_display.cpp迁移过来，这里省略...

// 时区管理方法实现
void DisplayManager::setTimeZone(const TimeZone& tz) {
  currentTimeZone = tz;
  DEBUG_PRINTLN("时区已设置: " + tz.name + " (" + tz.abbreviation + ")");
  updateDisplay();
}

DisplayManager::TimeZone DisplayManager::getCurrentTimeZone() const {
  return currentTimeZone;
}

void DisplayManager::autoDetectTimeZone() {
  // 自动检测时区（简化实现）
  // 实际项目中可以通过网络或系统时间获取时区信息
  TimeZone defaultTz = {"中国标准时间", "CST", 8, false};
  setTimeZone(defaultTz);
  autoTimeZoneEnabled = true;
  DEBUG_PRINTLN("时区已自动检测并设置");
}

// 绘制消息项（支持优先级显示）
void DisplayManager::drawMessageItem(int x, int y, String message, String time, MessagePriority priority) {
  if (displayDriver == nullptr) {
    return;
  }
  
  int textSize = height < 400 ? 1 : 2;
  int lineHeight = height < 400 ? 20 : 30;
  
  // 根据优先级设置颜色
  uint16_t textColor = GxEPD_BLACK;
  uint16_t priorityColor = GxEPD_GRAY2;
  
  switch (priority) {
    case MESSAGE_PRIORITY_URGENT:
      textColor = GxEPD_RED;
      priorityColor = GxEPD_RED;
      break;
    case MESSAGE_PRIORITY_HIGH:
      textColor = GxEPD_RED;
      priorityColor = GxEPD_RED;
      break;
    case MESSAGE_PRIORITY_NORMAL:
      textColor = GxEPD_BLACK;
      priorityColor = GxEPD_GRAY2;
      break;
    case MESSAGE_PRIORITY_LOW:
      textColor = GxEPD_GRAY2;
      priorityColor = GxEPD_GRAY2;
      break;
  }
  
  // 绘制优先级指示器
  displayDriver->fillRect(x - 15, y + 5, 8, 8, priorityColor);
  
  // 绘制消息内容
  displayDriver->drawString(x, y, message, textColor, GxEPD_WHITE, textSize);
  
  // 绘制时间
  displayDriver->drawString(x, y + lineHeight, time, GxEPD_GRAY2, GxEPD_WHITE, textSize - 1);
}

// 消息提醒动画方法实现
void DisplayManager::startMessageAnimation() {
  messageAnimationActive = true;
  messageAnimationStartTime = millis();
  messageAnimationLastUpdate = millis();
  messageAnimationFrame = 0;
  messageAnimationDirection = true;
  DEBUG_PRINTLN("消息提醒动画已启动");
}

void DisplayManager::stopMessageAnimation() {
  messageAnimationActive = false;
  messageAnimationFrame = 0;
  DEBUG_PRINTLN("消息提醒动画已停止");
}

void DisplayManager::updateMessageAnimation() {
  if (!messageAnimationActive) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // 每50毫秒更新一帧动画
  if (currentTime - messageAnimationLastUpdate >= 50) {
    messageAnimationLastUpdate = currentTime;
    
    // 更新动画帧
    if (messageAnimationDirection) {
      messageAnimationFrame++;
      if (messageAnimationFrame >= 10) {
        messageAnimationDirection = false;
      }
    } else {
      messageAnimationFrame--;
      if (messageAnimationFrame <= 0) {
        messageAnimationDirection = true;
      }
    }
    
    // 绘制动画效果
    if (displayDriver != nullptr) {
      // 计算动画位置和大小
      int animationX = leftPanelWidth - 40;
      int animationY = 20;
      int animationSize = 20 + messageAnimationFrame * 2;
      
      // 清除之前的动画
      displayDriver->fillRect(animationX - 5, animationY - 5, 40, 40, GxEPD_WHITE);
      
      // 绘制新的动画帧（闪烁效果）
      uint16_t color = messageAnimationFrame % 2 == 0 ? GxEPD_RED : GxEPD_WHITE;
      displayDriver->fillRect(animationX, animationY, animationSize, animationSize, color);
      
      // 局部更新显示
      displayDriver->update(animationX - 10, animationY - 10, 50, 50);
    }
    
    // 检查动画是否应该结束（10 秒后自动停止）
    if (currentTime - messageAnimationStartTime >= MESSAGE_ANIMATION_DURATION) {
      stopMessageAnimation();
    }
  }
}

// 图片显示功能实现
bool DisplayManager::drawImage(String imagePath, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("绘制图片: ");
  DEBUG_PRINTLN(imagePath);
  
  // 实现从SPIFFS绘制图片
  // 注意：实际实现需要使用合适的图片解码库，如TJpgDec或PNGdec
  
  // 这里只是一个示例实现，实际使用时需要根据图片格式和解码库调整
  DEBUG_PRINTLN("图片绘制功能待实现");
  return false;
}

bool DisplayManager::drawImageFromBuffer(uint8_t* buffer, int bufferSize, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINTLN("从缓冲区绘制图片");
  
  // 实现从缓冲区绘制图片
  // 注意：实际实现需要使用合适的图片解码库，如TJpgDec或PNGdec
  
  DEBUG_PRINTLN("缓冲区图片绘制功能待实现");
  return false;
}

bool DisplayManager::drawImageFromURL(String url, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("从URL绘制图片: ");
  DEBUG_PRINTLN(url);
  
  // 实现从URL绘制图片
  // 注意：需要先下载图片到缓冲区，然后调用drawImageFromBuffer
  
  DEBUG_PRINTLN("URL图片绘制功能待实现");
  return false;
}

// GIF显示功能实现
bool DisplayManager::drawGIF(String gifPath, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("绘制GIF图片: ");
  DEBUG_PRINTLN(gifPath);
  
  // 实现从SPIFFS绘制GIF
  // 注意：需要引入GIF解码库，如GIFDecoder或TFT_eSPI的GIF功能
  
  DEBUG_PRINTLN("GIF绘制功能待实现");
  return false;
}

bool DisplayManager::drawGIFFromBuffer(uint8_t* buffer, int bufferSize, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINTLN("从缓冲区绘制GIF图片");
  
  // 实现从缓冲区绘制GIF
  // 注意：需要引入GIF解码库，如GIFDecoder或TFT_eSPI的GIF功能
  
  DEBUG_PRINTLN("缓冲区GIF绘制功能待实现");
  return false;
}

bool DisplayManager::drawGIFFromURL(String url, int x, int y, int width, int height) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("从URL绘制GIF图片: ");
  DEBUG_PRINTLN(url);
  
  // 实现从URL绘制GIF
  // 注意：需要先下载GIF到缓冲区，然后调用drawGIFFromBuffer
  
  DEBUG_PRINTLN("URL GIF绘制功能待实现");
  return false;
}

bool DisplayManager::drawAnimatedGIF(String gifPath, int x, int y, int width, int height, int loopCount) {
  if (displayDriver == nullptr) {
    return false;
  }
  
  DEBUG_PRINT("绘制动画GIF: ");
  DEBUG_PRINTLN(gifPath);
  
  // 初始化GIF播放参数
  gifPlaying = true;
  gifStopped = false;
  this->gifLoopCount = loopCount;
  gifCurrentLoop = 0;
  gifCurrentFrame = 0;
  gifLastFrameTime = millis();
  currentGifPath = gifPath;
  
  // 实现动画GIF绘制
  // 1. 使用GIF解码库解析GIF文件
  // 2. 获取GIF总帧数和每帧间隔
  // 3. 循环绘制每一帧，使用局部刷新提高效率
  // 4. 根据loopCount控制循环次数
  
  DEBUG_PRINTLN("动画GIF绘制功能待实现");
  
  // 目前未引入GIF解码库，直接返回
  gifPlaying = false;
  return false;
}

void DisplayManager::stopGIF() {
  DEBUG_PRINTLN("停止GIF播放");
  gifStopped = true;
  gifPlaying = false;
}

bool DisplayManager::isGIFPlaying() {
  return gifPlaying;
}

// 传感器异常检测和报警方法实现
void DisplayManager::checkSensorAnomalies(float temperature, float humidity) {
  // 定义异常阈值
  const float TEMP_MIN = 0.0;
  const float TEMP_MAX = 40.0;
  const float HUM_MIN = 20.0;
  const float HUM_MAX = 80.0;
  
  // 检查温度异常
  if (temperature < TEMP_MIN || temperature > TEMP_MAX) {
    String anomalyType = "温度异常: " + String(temperature) + "°C";
    startSensorAlarm(anomalyType);
    return;
  }
  
  // 检查湿度异常
  if (humidity < HUM_MIN || humidity > HUM_MAX) {
    String anomalyType = "湿度异常: " + String(humidity) + "%";
    startSensorAlarm(anomalyType);
    return;
  }
  
  // 检查温度变化率异常（基于历史数据）
  if (MAX_SENSOR_HISTORY > 1) {
    int prevIndex = (sensorHistoryIndex - 1 + MAX_SENSOR_HISTORY) % MAX_SENSOR_HISTORY;
    float tempDiff = abs(temperature - tempHistory[prevIndex]);
    
    // 如果温度变化超过5°C，触发异常
    if (tempDiff > 5.0) {
      String anomalyType = "温度突变: " + String(tempDiff) + "°C";
      startSensorAlarm(anomalyType);
      return;
    }
  }
  
  // 没有异常，停止报警
  if (sensorAlarmActive) {
    stopSensorAlarm();
  }
}

void DisplayManager::startSensorAlarm(String anomalyType) {
  sensorAnomalyDetected = true;
  sensorAnomalyType = anomalyType;
  sensorAnomalyStartTime = millis();
  sensorAlarmActive = true;
  
  DEBUG_PRINTLN("传感器报警已启动: " + anomalyType);
  
  // 触发报警事件
  auto alarmData = std::make_shared<AlarmEventData>("传感器异常", anomalyType);
  EVENT_PUBLISH(EVENT_ALARM_TRIGGERED, alarmData);
}

void DisplayManager::stopSensorAlarm() {
  sensorAnomalyDetected = false;
  sensorAnomalyType = "";
  sensorAlarmActive = false;
  
  DEBUG_PRINTLN("传感器报警已停止");
}

void DisplayManager::updateSensorAlarm() {
  if (!sensorAlarmActive) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // 检查报警是否应该自动停止（30 秒后）
  if (currentTime - sensorAnomalyStartTime >= SENSOR_ANOMALY_TIMEOUT) {
    stopSensorAlarm();
    return;
  }
  
  // 绘制报警指示
  if (displayDriver != nullptr) {
    // 计算报警指示位置
    int alarmX = leftPanelWidth - 30;
    int alarmY = height - 30;
    
    // 清除之前的报警指示
    displayDriver->fillRect(alarmX - 5, alarmY - 5, 30, 30, COLOR_WHITE);
    
    // 绘制闪烁的报警指示
    static bool blinkState = false;
    static unsigned long lastBlinkTime = 0;
    
    if (currentTime - lastBlinkTime >= ALARM_ICON_BLINK_INTERVAL) {
      blinkState = !blinkState;
      lastBlinkTime = currentTime;
    }
    
    if (blinkState) {
      displayDriver->fillRect(alarmX, alarmY, 20, 20, COLOR_BLACK);
    }
    
    // 局部更新显示
    displayDriver->update(alarmX - 10, alarmY - 10, 40, 40);
  }
}

// 布局管理方法
void DisplayManager::setLayoutMode(LayoutMode mode) {
  layoutMode = mode;
  
  // 根据布局模式设置布局配置
  switch (mode) {
    case LAYOUT_MODE_COMPACT:
      currentLayout = {
        LAYOUT_MODE_COMPACT,
        0.4f,   // 左侧面板比例 40%（较小）
        0.6f,   // 右侧面板比例 60%（较大）
        10,     // 基础字体大小（较小）
        6,      // 元素间距（较小）
        false,  // 不显示边框
        currentLayout.orientation  // 保持当前屏幕方向
      };
      break;
    case LAYOUT_MODE_STANDARD:
      currentLayout = {
        LAYOUT_MODE_STANDARD,
        0.45f,  // 左侧面板比例 45%（较小）
        0.55f,  // 右侧面板比例 55%（较大，用于日历/消息/图片）
        12,     // 基础字体大小
        8,      // 元素间距
        false,  // 不显示边框
        currentLayout.orientation  // 保持当前屏幕方向
      };
      break;
    case LAYOUT_MODE_EXTENDED:
      currentLayout = {
        LAYOUT_MODE_EXTENDED,
        0.5f,   // 左侧面板比例 50%
        0.5f,   // 右侧面板比例 50%
        14,     // 基础字体大小（较大）
        10,     // 元素间距（较大）
        true,   // 显示边框
        currentLayout.orientation  // 保持当前屏幕方向
      };
      break;
    case LAYOUT_MODE_CUSTOM:
      // 保持当前自定义配置
      currentLayout.mode = LAYOUT_MODE_CUSTOM;
      break;
  }
  
  // 应用布局
  applyLayout();
}

LayoutMode DisplayManager::getLayoutMode() const {
  return layoutMode;
}

void DisplayManager::setCustomLayout(float leftPanelRatio, float rightPanelRatio) {
  // 确保比例有效
  leftPanelRatio = constrain(leftPanelRatio, 0.1f, 0.9f);
  rightPanelRatio = constrain(rightPanelRatio, 0.1f, 0.9f);
  
  // 调整比例，确保总和为1.0
  float total = leftPanelRatio + rightPanelRatio;
  if (total != 1.0f) {
    leftPanelRatio /= total;
    rightPanelRatio /= total;
  }
  
  // 更新布局配置
  currentLayout = {
    LAYOUT_MODE_CUSTOM,
    leftPanelRatio,
    rightPanelRatio,
    currentLayout.fontSize,
    currentLayout.spacing,
    currentLayout.showBorders
  };
  
  layoutMode = LAYOUT_MODE_CUSTOM;
  
  // 应用布局
  applyLayout();
}

LayoutConfig DisplayManager::getCurrentLayout() const {
  return currentLayout;
}

// 屏幕方向管理方法实现
void DisplayManager::setScreenOrientation(ScreenOrientation orientation) {
  currentLayout.orientation = orientation;
  applyLayout();
}

ScreenOrientation DisplayManager::getScreenOrientation() const {
  return currentLayout.orientation;
}

void DisplayManager::toggleScreenOrientation() {
  // 切换屏幕方向
  if (currentLayout.orientation == ORIENTATION_HORIZONTAL) {
    setScreenOrientation(ORIENTATION_VERTICAL);
  } else {
    setScreenOrientation(ORIENTATION_HORIZONTAL);
  }
}

// 屏幕尺寸管理方法实现
void DisplayManager::setScreenSize(ScreenSize size) {
  currentLayout.screenSize = size;
  
  // 根据屏幕尺寸自动调整布局
  // 3 寸以下只支持竖屏单栏
  if (size == SCREEN_SIZE_2_9_INCH || size == SCREEN_SIZE_2_66_INCH || 
      size == SCREEN_SIZE_2_13_INCH || size == SCREEN_SIZE_1_54_INCH || 
      size == SCREEN_SIZE_1_44_INCH || size == SCREEN_SIZE_1_02_INCH) {
    // 3 寸以下强制竖向布局
    if (currentLayout.orientation != ORIENTATION_VERTICAL) {
      setScreenOrientation(ORIENTATION_VERTICAL);
    }
  } else if (size == SCREEN_SIZE_3_7_INCH || size == SCREEN_SIZE_4_2_INCH) {
    // 3-4.2 寸默认竖向，允许切换
    if (currentLayout.orientation != ORIENTATION_VERTICAL) {
      setScreenOrientation(ORIENTATION_VERTICAL);
    }
  } else {
    // 4.2 寸以上默认横向
    if (currentLayout.orientation != ORIENTATION_HORIZONTAL) {
      setScreenOrientation(ORIENTATION_HORIZONTAL);
    }
  }
  
  applyLayout();
}

ScreenSize DisplayManager::getScreenSize() const {
  return currentLayout.screenSize;
}

void DisplayManager::getNextScreenSize() {
  // 切换到下一个屏幕尺寸
  switch (currentLayout.screenSize) {
    case SCREEN_SIZE_7_5_INCH:
      setScreenSize(SCREEN_SIZE_4_2_INCH);
      break;
    case SCREEN_SIZE_4_2_INCH:
      setScreenSize(SCREEN_SIZE_2_9_INCH);
      break;
    case SCREEN_SIZE_2_9_INCH:
      setScreenSize(SCREEN_SIZE_7_5_INCH);
      break;
  }
}

void DisplayManager::applyLayout() {
  if (!displayDriver) {
    return;
  }
  
  // 根据屏幕尺寸获取屏幕尺寸
  switch (currentLayout.screenSize) {
    case SCREEN_SIZE_7_5_INCH:
      width = 800;
      height = 480;
      break;
    case SCREEN_SIZE_7_3_INCH:
      width = 1920;
      height = 1080;
      break;
    case SCREEN_SIZE_6_INCH:
      width = 1448;
      height = 1072;
      break;
    case SCREEN_SIZE_5_83_INCH:
      width = 1448;
      height = 1072;
      break;
    case SCREEN_SIZE_4_2_INCH:
      width = 400;
      height = 300;
      break;
    case SCREEN_SIZE_3_7_INCH:
      width = 480;
      height = 280;
      break;
    case SCREEN_SIZE_2_9_INCH:
      width = 296;
      height = 128;
      break;
    case SCREEN_SIZE_2_66_INCH:
      width = 296;
      height = 152;
      break;
    case SCREEN_SIZE_2_13_INCH:
      width = 250;
      height = 122;
      break;
    case SCREEN_SIZE_1_54_INCH:
      width = 200;
      height = 200;
      break;
    case SCREEN_SIZE_1_44_INCH:
      width = 128;
      height = 128;
      break;
    case SCREEN_SIZE_1_02_INCH:
      width = 80;
      height = 128;
      break;
    default:
      // 从驱动获取实际尺寸
      width = displayDriver->getWidth();
      height = displayDriver->getHeight();
      break;
  }
  
  // 根据屏幕方向计算面板尺寸
  if (currentLayout.orientation == ORIENTATION_HORIZONTAL) {
    // 横向布局：左右分栏
    leftPanelWidth = static_cast<uint16_t>(width * currentLayout.leftPanelRatio);
    rightPanelWidth = width - leftPanelWidth;
    leftPanelHeight = height;
    rightPanelHeight = height;
  } else {
    // 竖向布局：上下分栏
    leftPanelWidth = width;
    rightPanelWidth = width;
    leftPanelHeight = static_cast<uint16_t>(height * currentLayout.leftPanelRatio);
    rightPanelHeight = height - leftPanelHeight;
  }
  
  // 如果显示边框，绘制边框
  if (currentLayout.showBorders && displayDriver) {
    if (currentLayout.orientation == ORIENTATION_HORIZONTAL) {
      // 横向布局：绘制垂直分割线
      displayDriver->drawLine(leftPanelWidth - 1, 0, leftPanelWidth - 1, height - 1, COLOR_DIVIDER_LINE);
    } else {
      // 竖向布局：绘制水平分割线
      displayDriver->drawLine(0, leftPanelHeight - 1, width - 1, leftPanelHeight - 1, COLOR_DIVIDER_LINE);
    }
  }
  
  // 触发显示更新
  updateDisplay();
}

// 日历页面绘制方法
void DisplayManager::drawCalendarPage(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->drawString(x, y, "日历页面", GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, y + 30, "功能待实现", GxEPD_GRAY2, GxEPD_WHITE, 1);
}

// 股票页面绘制方法
void DisplayManager::drawStockPage(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->drawString(x, y, "股票页面", GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, y + 30, "功能待实现", GxEPD_GRAY2, GxEPD_WHITE, 1);
}

// 消息页面绘制方法
void DisplayManager::drawMessagePage(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->drawString(x, y, "消息页面", GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, y + 30, "功能待实现", GxEPD_GRAY2, GxEPD_WHITE, 1);
}

// 插件页面绘制方法
void DisplayManager::drawPluginPage(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->drawString(x, y, "插件页面", GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, y + 30, "功能待实现", GxEPD_GRAY2, GxEPD_WHITE, 1);
}

// 插件管理页面绘制方法
void DisplayManager::drawPluginManagePage(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->drawString(x, y, "插件管理页面", GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, y + 30, "功能待实现", GxEPD_GRAY2, GxEPD_WHITE, 1);
}

// 设置页面绘制方法
void DisplayManager::drawSettingPage(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->drawString(x, y, "设置页面", GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, y + 30, "功能待实现", GxEPD_GRAY2, GxEPD_WHITE, 1);
}

// ========== 小屏幕 UI 绘制函数（3 寸以下，竖屏单栏） ==========

// 2.9 寸 (296x128) UI 绘制
void DisplayManager::drawUI_2_9inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 清屏
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  // 绘制各个组件
  drawStatusBar_2_9inch();
  drawTime_2_9inch();
  drawDate_2_9inch();
  drawWeather_2_9inch();
  drawSensor_2_9inch();
  
  // 刷新显示
  displayDriver->update();
}

void DisplayManager::drawStatusBar_2_9inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) {
    return;
  }
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  // 绘制电池图标 (18x9px)
  int x = 2, y = 2;
  displayDriver->drawRect(x, y, 18, 9, GxEPD_BLACK);
  displayDriver->drawRect(x + 18, y + 2, 2, 5, GxEPD_BLACK);
  
  // 绘制电量
  int levelWidth = 14 * percentage / 100;
  displayDriver->fillRect(x + 2, y + 2, levelWidth, 5, GxEPD_BLACK);
  
  // 绘制百分比文字
  displayDriver->drawString(x + 22, y, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  // 绘制消息数量
  if (messageCount > 0) {
    displayDriver->drawString(x + 50, y, "🔔 " + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, 1);
  }
  
  // 绘制分隔线
  displayDriver->drawLine(0, 13, width, 13, GxEPD_GRAY2);
}

void DisplayManager::drawTime_2_9inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0');
  
  // 绘制时间 (22px 粗体)
  displayDriver->drawString(0, 18, timeStr, GxEPD_BLACK, GxEPD_WHITE, 3);
}

void DisplayManager::drawDate_2_9inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  
  // 公历日期
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  displayDriver->drawString(0, 45, dateStr + " " + weekStr, GxEPD_BLACK, GxEPD_WHITE, 1);
  
  // 农历日期
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    displayDriver->drawString(0, 56, lunar.lunarMonth + "月" + lunar.lunarDay, GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawWeather_2_9inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) {
    return;
  }
  
  WeatherData weather = weatherManager->getWeatherData();
  
  // 绘制当前温度 (14px 粗体)
  displayDriver->drawString(0, 65, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 绘制天气预报（三天横向排列）
  int forecastY = 85;
  for (int i = 0; i < 3 && i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      int x = i * 95;
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(x, forecastY, icon, GxEPD_BLACK, GxEPD_WHITE, 2);
      displayDriver->drawString(x + 20, forecastY, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 1);
    }
  }
}

void DisplayManager::drawSensor_2_9inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) {
    return;
  }
  
  SensorData sensor = sensorManager->getSensorData();
  
  // 绘制分隔线
  displayDriver->drawLine(0, height - 22, width, height - 22, GxEPD_GRAY2);
  
  // 绘制传感器数据（底部横向三列）
  int y = height - 18;
  if (sensor.valid) {
    displayDriver->drawString(5, y, "🌡️" + String(sensor.temperature, 1) + "°", GxEPD_BLACK, GxEPD_WHITE, 1);
    displayDriver->drawString(width / 3, y, "💧" + String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
    displayDriver->drawString(width * 2 / 3, y, "🌿优", GxEPD_BLACK, GxEPD_WHITE, 1);
  } else {
    displayDriver->drawString(5, y, "传感器数据无效", GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

// ========== 小屏幕 UI 绘制函数（2.13 寸，竖屏单栏） ==========

void DisplayManager::drawUI_2_13inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  drawStatusBar_2_13inch();
  drawTime_2_13inch();
  drawDate_2_13inch();
  drawWeather_2_13inch();
  drawSensor_2_13inch();
  
  displayDriver->update();
}

void DisplayManager::drawStatusBar_2_13inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) {
    return;
  }
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 2, y = 2;
  displayDriver->drawRect(x, y, 16, 8, GxEPD_BLACK);
  displayDriver->drawRect(x + 16, y + 2, 2, 4, GxEPD_BLACK);
  
  int levelWidth = 12 * percentage / 100;
  displayDriver->fillRect(x + 2, y + 2, levelWidth, 4, GxEPD_BLACK);
  
  displayDriver->drawString(x + 20, y, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (messageCount > 0) {
    displayDriver->drawString(x + 45, y, "🔔" + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, 1);
  }
  
  displayDriver->drawLine(0, 12, width, 12, GxEPD_GRAY2);
}

void DisplayManager::drawTime_2_13inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0');
  
  displayDriver->drawString(0, 14, timeStr, GxEPD_BLACK, GxEPD_WHITE, 3);
}

void DisplayManager::drawDate_2_13inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  displayDriver->drawString(0, 38, dateStr + " " + weekStr, GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    displayDriver->drawString(0, 48, lunar.lunarMonth + "月" + lunar.lunarDay, GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawWeather_2_13inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) {
    return;
  }
  
  WeatherData weather = weatherManager->getWeatherData();
  
  displayDriver->drawString(0, 56, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  int forecastY = 72;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      int x = i * 80;
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(x, forecastY, icon, GxEPD_BLACK, GxEPD_WHITE, 1);
      displayDriver->drawString(x + 15, forecastY, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 1);
    }
  }
}

void DisplayManager::drawSensor_2_13inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) {
    return;
  }
  
  SensorData sensor = sensorManager->getSensorData();
  
  displayDriver->drawLine(0, height - 18, width, height - 18, GxEPD_GRAY2);
  
  int y = height - 14;
  if (sensor.valid) {
    displayDriver->drawString(5, y, "🌡️" + String(sensor.temperature, 1) + "°", GxEPD_BLACK, GxEPD_WHITE, 1);
    displayDriver->drawString(width / 3, y, "💧" + String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
    displayDriver->drawString(width * 2 / 3, y, "🌿优", GxEPD_BLACK, GxEPD_WHITE, 1);
  } else {
    displayDriver->drawString(5, y, "传感器数据无效", GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

// ========== 小屏幕 UI 绘制函数（1.54 寸，竖屏单栏，方形） ==========

void DisplayManager::drawUI_1_54inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  drawStatusBar_1_54inch();
  drawTime_1_54inch();
  drawDate_1_54inch();
  drawWeather_1_54inch();
  drawSensor_1_54inch();
  
  displayDriver->update();
}

void DisplayManager::drawStatusBar_1_54inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) {
    return;
  }
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 2, y = 2;
  displayDriver->drawRect(x, y, 14, 7, GxEPD_BLACK);
  displayDriver->drawRect(x + 14, y + 2, 2, 3, GxEPD_BLACK);
  
  int levelWidth = 10 * percentage / 100;
  displayDriver->fillRect(x + 2, y + 2, levelWidth, 3, GxEPD_BLACK);
  
  displayDriver->drawString(x + 18, y, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (messageCount > 0) {
    displayDriver->drawString(x + 45, y, "🔔" + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, 1);
  }
  
  displayDriver->drawLine(0, 10, width, 10, GxEPD_GRAY2);
}

void DisplayManager::drawTime_1_54inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0');
  
  displayDriver->drawString(0, 14, timeStr, GxEPD_BLACK, GxEPD_WHITE, 3);
}

void DisplayManager::drawDate_1_54inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  
  String dateStr = padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  displayDriver->drawString(0, 40, dateStr + " " + weekStr, GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    displayDriver->drawString(0, 52, lunar.lunarMonth + lunar.lunarDay, GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawWeather_1_54inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) {
    return;
  }
  
  WeatherData weather = weatherManager->getWeatherData();
  
  displayDriver->drawString(0, 65, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  ForecastData forecast = weatherManager->getForecastData(0);
  if (forecast.date.length() > 0) {
    String icon = weatherManager->getWeatherIcon(forecast.condition);
    displayDriver->drawString(50, 65, icon, GxEPD_BLACK, GxEPD_WHITE, 2);
  }
}

void DisplayManager::drawSensor_1_54inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) {
    return;
  }
  
  SensorData sensor = sensorManager->getSensorData();
  
  displayDriver->drawLine(0, height - 16, width, height - 16, GxEPD_GRAY2);
  
  int y = height - 12;
  if (sensor.valid) {
    displayDriver->drawString(5, y, "🌡️" + String(sensor.temperature, 1) + "°", GxEPD_BLACK, GxEPD_WHITE, 1);
    displayDriver->drawString(width / 2, y, "💧" + String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  } else {
    displayDriver->drawString(5, y, "传感器数据无效", GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

// ========== 大屏幕 UI 绘制函数（3 寸以上，横屏双栏） ==========

// 7.5 寸 (800x480) UI 绘制
void DisplayManager::drawUI_7_5inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 清屏
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  // 计算左右面板位置
  leftPanelWidth = width * 0.45;
  int rightPanelX = leftPanelWidth;
  
  // 绘制左侧面板背景
  displayDriver->fillRect(0, 0, leftPanelWidth, height, GxEPD_WHITE);
  displayDriver->drawLine(leftPanelWidth, 0, leftPanelWidth, height, GxEPD_GRAY2);
  
  // 绘制右侧面板背景（浅灰色）
  displayDriver->fillRect(rightPanelX, 0, width - rightPanelX, height, GxEPD_GRAY2);
  
  // 绘制左侧内容
  drawLeftPanel_7_5inch();
  
  // 绘制右侧内容
  drawRightPanel_7_5inch();
  
  // 刷新显示
  displayDriver->update();
}

void DisplayManager::drawLeftPanel_7_5inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  int x = 20, y = 20;
  
  // 状态栏
  drawStatusBar_7_5inch();
  y = 50;
  
  // 时间显示（64px 粗体）
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (timeManager) {
    TimeData time = timeManager->getTimeData();
    String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0') + ":" + padStart(String(time.second), 2, '0');
    displayDriver->drawString(x, y, timeStr, GxEPD_BLACK, GxEPD_WHITE, 6);
  }
  
  // 日期和农历（16px）
  y += 70;
  drawDate_7_5inch(x, y);
  
  // 天气区域（蓝色渐变背景）
  y += 30;
  drawWeather_7_5inch(x, y);
  
  // 传感器区域（绿色渐变背景）
  y += 100;
  drawSensor_7_5inch(x, y);
}

void DisplayManager::drawStatusBar_7_5inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) {
    return;
  }
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 20, y = 20;
  
  // 绘制电池图标 (32x16px)
  displayDriver->drawRect(x, y, 32, 16, GxEPD_BLACK);
  displayDriver->drawRect(x + 32, y + 4, 3, 8, GxEPD_BLACK);
  
  // 绘制电量
  int levelWidth = 26 * percentage / 100;
  displayDriver->fillRect(x + 3, y + 3, levelWidth, 10, GxEPD_BLACK);
  
  // 绘制百分比文字
  displayDriver->drawString(x + 38, y + 2, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 绘制消息数量
  if (messageCount > 0) {
    displayDriver->drawString(x + 90, y + 2, "🔔 " + String(messageCount) + "条", GxEPD_BLACK, GxEPD_WHITE, 2);
  }
}

void DisplayManager::drawDate_7_5inch(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  
  // 公历日期
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  
  // 农历日期
  String lunarStr = "";
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    lunarStr = " | " + lunar.lunarMonth + "月" + lunar.lunarDay;
  }
  
  displayDriver->drawString(x, y, dateStr + " " + weekStr + lunarStr, GxEPD_BLACK, GxEPD_WHITE, 2);
}

void DisplayManager::drawWeather_7_5inch(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) {
    return;
  }
  
  WeatherData weather = weatherManager->getWeatherData();
  
  // 绘制背景框（蓝色渐变效果用单色代替）
  displayDriver->fillRect(x, y, 320, 100, GxEPD_GRAY2);
  displayDriver->drawRect(x, y, 320, 100, GxEPD_BLUE);
  
  // 城市
  displayDriver->drawString(x + 10, y + 5, "📍 北京市", GxEPD_BLUE, GxEPD_WHITE, 2);
  
  // 当前温度（32px 粗体）
  displayDriver->drawString(x + 10, y + 25, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 4);
  
  // 天气状况
  displayDriver->drawString(x + 10, y + 60, weather.condition, GxEPD_GRAY2, GxEPD_WHITE, 2);
  
  // 天气预报（三天横向排列）
  int forecastX = x + 150;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      String dayStr = (i == 0) ? "今天" : ((i == 1) ? "明天" : "后天");
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(forecastX + i * 55, y + 5, dayStr, GxEPD_GRAY2, GxEPD_WHITE, 1);
      displayDriver->drawString(forecastX + i * 55, y + 20, icon, GxEPD_BLACK, GxEPD_WHITE, 3);
      displayDriver->drawString(forecastX + i * 55, y + 50, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 2);
    }
  }
}

void DisplayManager::drawSensor_7_5inch(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) {
    return;
  }
  
  SensorData sensor = sensorManager->getSensorData();
  
  // 绘制背景框（绿色渐变效果用单色代替）
  displayDriver->fillRect(x, y, 320, 80, GxEPD_GRAY2);
  displayDriver->drawRect(x, y, 320, 80, GxEPD_BLACK);
  
  // 标题
  displayDriver->drawString(x + 10, y + 5, "🌡️ 室内环境", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 传感器数据（3 列）
  displayDriver->drawString(x + 10, y + 30, "温度", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 10, y + 48, String(sensor.temperature, 1) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  displayDriver->drawString(x + 110, y + 30, "湿度", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 110, y + 48, String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  displayDriver->drawString(x + 210, y + 30, "空气质量", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 210, y + 48, "优", GxEPD_BLACK, GxEPD_WHITE, 2);
}

void DisplayManager::drawRightPanel_7_5inch() {
  if (displayDriver == nullptr) {
    return;
  }
  
  int x = leftPanelWidth + 20, y = 20;
  
  // 日历
  drawCalendar_7_5inch(x, y);
  y += 200;
  
  // 节日
  drawFestival_7_5inch(x, y);
  y += 60;
  
  // 黄历
  drawAlmanac_7_5inch(x, y);
  y += 60;
  
  // 股票信息
  drawStockInfo_7_5inch(x, y);
  y += 80;
  
  // 消息通知
  drawMessageInfo_7_5inch(x, y);
  y += 60;
  
  // 系统状态
  drawSystemStatus_7_5inch(x, y);
}

// 股票信息绘制
void DisplayManager::drawStockInfo_7_5inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto stockManager = DependencyInjectionContainer::getInstance()->getStockManager();
  
  // 绘制股票标题
  displayDriver->drawString(x, y, "📈 股票", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 示例股票数据（实际应从股票管理器获取）
  int stockY = y + 20;
  displayDriver->drawString(x, stockY, "上证指数: 3200.50 +0.8%", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, stockY + 15, "深证成指: 10500.20 +1.2%", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// 消息通知绘制
void DisplayManager::drawMessageInfo_7_5inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  int messageCount = 0;
  if (messageManager) {
    messageCount = messageManager->getUnreadMessageCount();
  }
  
  // 绘制消息标题
  displayDriver->drawString(x, y, "🔔 消息", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 消息数量
  int messageY = y + 20;
  displayDriver->drawString(x, messageY, "未读消息: " + String(messageCount) + "条", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// 系统状态绘制
void DisplayManager::drawSystemStatus_7_5inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  int batteryPercentage = 100;
  bool isCharging = false;
  if (powerManager) {
    batteryPercentage = powerManager->getBatteryPercentage();
    isCharging = powerManager->getChargingStatus();
  }
  
  // 绘制系统状态标题
  displayDriver->drawString(x, y, "⚙️ 系统", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 系统信息
  int statusY = y + 20;
  displayDriver->drawString(x, statusY, "电量: " + String(batteryPercentage) + "% " + (isCharging ? "充电中" : ""), GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, statusY + 15, "网络: 已连接", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, statusY + 30, "版本: v1.0.0", GxEPD_BLACK, GxEPD_WHITE, 1);
}

void DisplayManager::drawCalendar_7_5inch(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  
  // 绘制日历标题
  String monthTitle = "📅 " + String(time.year) + "年" + String(time.month) + "月";
  displayDriver->drawString(x, y, monthTitle, GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 绘制星期标题
  const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
  int startX = x;
  int cellWidth = 50;
  int cellHeight = 35;
  int startY = y + 30;
  
  for (int i = 0; i < 7; i++) {
    displayDriver->drawString(startX + i * cellWidth, startY, weekdays[i], GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
  
  // 计算当月第一天是星期几和总天数
  int firstWeekday = getFirstWeekdayOfMonth(time.year, time.month);
  int daysInMonth = getDaysInMonth(time.year, time.month);
  
  // 绘制日期
  int currentX = startX + firstWeekday * cellWidth;
  int currentY = startY + 20;
  
  for (int day = 1; day <= daysInMonth; day++) {
    if (day == time.day) {
      // 今天：反色显示
      displayDriver->fillRect(currentX - 2, currentY - 2, cellWidth - 2, cellHeight, GxEPD_BLACK);
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_WHITE, GxEPD_BLACK, 2);
    } else {
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_BLACK, GxEPD_WHITE, 2);
    }
    
    currentX += cellWidth;
    if (currentX > startX + 6 * cellWidth) {
      currentX = startX;
      currentY += cellHeight;
    }
  }
}

void DisplayManager::drawFestival_7_5inch(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  
  // 绘制节日标题
  displayDriver->drawString(x, y, "🎉 节日", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 示例节日数据（实际应从节日管理器获取）
  int festivalY = y + 20;
  
  // 春分示例
  displayDriver->drawString(x, festivalY, "🌸 春分 (3 月 21 日)", GxEPD_BLACK, GxEPD_WHITE, 1);
  festivalY += 15;
  
  // 清明示例
  displayDriver->drawString(x, festivalY, "🎊 清明 (4 月 5 日)", GxEPD_BLACK, GxEPD_WHITE, 1);
}

void DisplayManager::drawAlmanac_7_5inch(int x, int y) {
  if (displayDriver == nullptr) {
    return;
  }
  
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  if (!lunarManager) {
    return;
  }
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) {
    return;
  }
  
  TimeData time = timeManager->getTimeData();
  LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
  
  // 绘制黄历标题
  displayDriver->drawString(x, y, "📜 黄历", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 绘制宜忌
  int almanacY = y + 20;
  
  // 示例宜忌（实际应从农历管理器获取）
  String goodItems = "宜：祭祀 祈福 求嗣";
  String badItems = "忌：嫁娶 出行 动土";
  
  displayDriver->drawString(x, almanacY, goodItems, GxEPD_BLACK, GxEPD_WHITE, 1);
  almanacY += 15;
  displayDriver->drawString(x, almanacY, badItems, GxEPD_RED, GxEPD_WHITE, 1);
}

// ========== 4.2 寸 (400x300) UI 绘制 ==========

void DisplayManager::drawUI_4_2inch() {
  if (displayDriver == nullptr) return;
  
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  drawStatusBar_4_2inch();
  drawTime_4_2inch();
  drawDate_4_2inch();
  drawWeather_4_2inch();
  drawSensor_4_2inch();
  drawCalendar_4_2inch();
  drawFestival_4_2inch();
  drawAlmanac_4_2inch();
  
  displayDriver->update();
}

void DisplayManager::drawStatusBar_4_2inch() {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) return;
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 2, y = 2;
  displayDriver->drawRect(x, y, 18, 9, GxEPD_BLACK);
  displayDriver->drawRect(x + 18, y + 2, 2, 5, GxEPD_BLACK);
  
  int levelWidth = 14 * percentage / 100;
  displayDriver->fillRect(x + 2, y + 2, levelWidth, 5, GxEPD_BLACK);
  
  displayDriver->drawString(x + 22, y, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (messageCount > 0) {
    displayDriver->drawString(x + 50, y, "🔔" + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawTime_4_2inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0');
  
  displayDriver->drawString(0, 14, timeStr, GxEPD_BLACK, GxEPD_WHITE, 4);
}

void DisplayManager::drawDate_4_2inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  displayDriver->drawString(0, 50, dateStr + " " + weekStr, GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    displayDriver->drawString(0, 62, lunar.lunarMonth + "月" + lunar.lunarDay, GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawWeather_4_2inch() {
  if (displayDriver == nullptr) return;
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) return;
  
  WeatherData weather = weatherManager->getWeatherData();
  
  displayDriver->drawString(0, 75, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  int forecastY = 95;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      int x = i * 60;
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(x, forecastY, icon, GxEPD_BLACK, GxEPD_WHITE, 1);
      displayDriver->drawString(x + 12, forecastY, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 1);
    }
  }
}

void DisplayManager::drawSensor_4_2inch() {
  if (displayDriver == nullptr) return;
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) return;
  
  SensorData sensor = sensorManager->getSensorData();
  
  displayDriver->drawLine(0, height - 15, 180, height - 15, GxEPD_GRAY2);
  
  int y = height - 12;
  displayDriver->drawString(5, y, "🌡️" + String(sensor.temperature, 1) + "°", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(60, y, "💧" + String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(115, y, "🌿优", GxEPD_BLACK, GxEPD_WHITE, 1);
}

void DisplayManager::drawCalendar_4_2inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  int x = 185, y = 5;
  String monthTitle = "📅 " + String(time.month) + "月";
  displayDriver->drawString(x, y, monthTitle, GxEPD_BLACK, GxEPD_WHITE, 2);
  
  const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
  int startX = x;
  int cellWidth = 30;
  int startY = y + 20;
  
  for (int i = 0; i < 7; i++) {
    displayDriver->drawString(startX + i * cellWidth, startY, weekdays[i], GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
  
  int firstWeekday = getFirstWeekdayOfMonth(time.year, time.month);
  int daysInMonth = getDaysInMonth(time.year, time.month);
  
  int currentX = startX + firstWeekday * cellWidth;
  int currentY = startY + 12;
  
  for (int day = 1; day <= daysInMonth; day++) {
    if (day == time.day) {
      displayDriver->fillRect(currentX - 1, currentY - 1, cellWidth - 2, 10, GxEPD_BLACK);
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_WHITE, GxEPD_BLACK, 1);
    } else {
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_BLACK, GxEPD_WHITE, 1);
    }
    
    currentX += cellWidth;
    if (currentX > startX + 6 * cellWidth) {
      currentX = startX;
      currentY += 12;
    }
  }
}

void DisplayManager::drawFestival_4_2inch() {
  if (displayDriver == nullptr) return;
  
  int x = 185, y = 130;
  displayDriver->drawString(x, y, "🎉 节日", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, y + 12, "🌸 春分 (3月21日)", GxEPD_BLACK, GxEPD_WHITE, 1);
}

void DisplayManager::drawAlmanac_4_2inch() {
  if (displayDriver == nullptr) return;
  
  int x = 185, y = 160;
  displayDriver->drawString(x, y, "📜 黄历", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, y + 12, "宜：祭祀 祈福", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, y + 24, "忌：嫁娶 出行", GxEPD_RED, GxEPD_WHITE, 1);
}

// ========== 3.7 寸 (480x280) UI 绘制 ==========

void DisplayManager::drawUI_3_7inch() {
  if (displayDriver == nullptr) return;
  
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  drawStatusBar_3_7inch();
  drawTime_3_7inch();
  drawDate_3_7inch();
  drawWeather_3_7inch();
  drawSensor_3_7inch();
  drawCalendar_3_7inch();
  drawFestival_3_7inch();
  drawAlmanac_3_7inch();
  
  displayDriver->update();
}

void DisplayManager::drawStatusBar_3_7inch() {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) return;
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 2, y = 2;
  displayDriver->drawRect(x, y, 20, 10, GxEPD_BLACK);
  displayDriver->drawRect(x + 20, y + 2, 2, 6, GxEPD_BLACK);
  
  int levelWidth = 16 * percentage / 100;
  displayDriver->fillRect(x + 2, y + 2, levelWidth, 6, GxEPD_BLACK);
  
  displayDriver->drawString(x + 25, y, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (messageCount > 0) {
    displayDriver->drawString(x + 55, y, "🔔" + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawTime_3_7inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0');
  
  displayDriver->drawString(0, 16, timeStr, GxEPD_BLACK, GxEPD_WHITE, 4);
}

void DisplayManager::drawDate_3_7inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  displayDriver->drawString(0, 55, dateStr + " " + weekStr, GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    displayDriver->drawString(0, 68, lunar.lunarMonth + "月" + lunar.lunarDay, GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawWeather_3_7inch() {
  if (displayDriver == nullptr) return;
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) return;
  
  WeatherData weather = weatherManager->getWeatherData();
  
  displayDriver->drawString(0, 82, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  int forecastY = 105;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      int x = i * 70;
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(x, forecastY, icon, GxEPD_BLACK, GxEPD_WHITE, 1);
      displayDriver->drawString(x + 14, forecastY, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 1);
    }
  }
}

void DisplayManager::drawSensor_3_7inch() {
  if (displayDriver == nullptr) return;
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) return;
  
  SensorData sensor = sensorManager->getSensorData();
  
  displayDriver->drawLine(0, height - 15, 216, height - 15, GxEPD_GRAY2);
  
  int y = height - 12;
  displayDriver->drawString(5, y, "🌡️" + String(sensor.temperature, 1) + "°", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(75, y, "💧" + String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(145, y, "🌿优", GxEPD_BLACK, GxEPD_WHITE, 1);
}

void DisplayManager::drawCalendar_3_7inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  int x = 220, y = 5;
  String monthTitle = "📅 " + String(time.year) + "年" + String(time.month) + "月";
  displayDriver->drawString(x, y, monthTitle, GxEPD_BLACK, GxEPD_WHITE, 2);
  
  const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
  int startX = x;
  int cellWidth = 35;
  int startY = y + 22;
  
  for (int i = 0; i < 7; i++) {
    displayDriver->drawString(startX + i * cellWidth, startY, weekdays[i], GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
  
  int firstWeekday = getFirstWeekdayOfMonth(time.year, time.month);
  int daysInMonth = getDaysInMonth(time.year, time.month);
  
  int currentX = startX + firstWeekday * cellWidth;
  int currentY = startY + 14;
  
  for (int day = 1; day <= daysInMonth; day++) {
    if (day == time.day) {
      displayDriver->fillRect(currentX - 1, currentY - 1, cellWidth - 2, 12, GxEPD_BLACK);
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_WHITE, GxEPD_BLACK, 1);
    } else {
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_BLACK, GxEPD_WHITE, 1);
    }
    
    currentX += cellWidth;
    if (currentX > startX + 6 * cellWidth) {
      currentX = startX;
      currentY += 14;
    }
  }
}

void DisplayManager::drawFestival_3_7inch() {
  if (displayDriver == nullptr) return;
  
  int x = 220, y = 130;
  displayDriver->drawString(x, y, "🎉 节日", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, y + 14, "🌸 春分 (3月21日)", GxEPD_BLACK, GxEPD_WHITE, 1);
}

void DisplayManager::drawAlmanac_3_7inch() {
  if (displayDriver == nullptr) return;
  
  int x = 220, y = 165;
  displayDriver->drawString(x, y, "📜 黄历", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, y + 14, "宜：祭祀 祈福", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, y + 28, "忌：嫁娶 出行", GxEPD_RED, GxEPD_WHITE, 1);
}

// ========== 2.66 寸 (296x152) UI 绘制 ==========

void DisplayManager::drawUI_2_66inch() {
  if (displayDriver == nullptr) return;
  
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  drawStatusBar_2_66inch();
  drawTime_2_66inch();
  drawDate_2_66inch();
  drawWeather_2_66inch();
  drawSensor_2_66inch();
  
  displayDriver->update();
}

void DisplayManager::drawStatusBar_2_66inch() {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) return;
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 2, y = 2;
  displayDriver->drawRect(x, y, 16, 8, GxEPD_BLACK);
  displayDriver->drawRect(x + 16, y + 2, 2, 4, GxEPD_BLACK);
  
  int levelWidth = 12 * percentage / 100;
  displayDriver->fillRect(x + 2, y + 2, levelWidth, 4, GxEPD_BLACK);
  
  displayDriver->drawString(x + 20, y, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (messageCount > 0) {
    displayDriver->drawString(x + 45, y, "🔔" + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, 1);
  }
  
  displayDriver->drawLine(0, 12, width, 12, GxEPD_GRAY2);
}

void DisplayManager::drawTime_2_66inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0');
  
  displayDriver->drawString(0, 16, timeStr, GxEPD_BLACK, GxEPD_WHITE, 3);
}

void DisplayManager::drawDate_2_66inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  displayDriver->drawString(0, 42, dateStr + " " + weekStr, GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    displayDriver->drawString(0, 54, lunar.lunarMonth + "月" + lunar.lunarDay, GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawWeather_2_66inch() {
  if (displayDriver == nullptr) return;
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) return;
  
  WeatherData weather = weatherManager->getWeatherData();
  
  displayDriver->drawString(0, 68, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  int forecastY = 88;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      int x = i * 95;
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(x, forecastY, icon, GxEPD_BLACK, GxEPD_WHITE, 1);
      displayDriver->drawString(x + 12, forecastY, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 1);
    }
  }
}

void DisplayManager::drawSensor_2_66inch() {
  if (displayDriver == nullptr) return;
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) return;
  
  SensorData sensor = sensorManager->getSensorData();
  
  displayDriver->drawLine(0, height - 15, width, height - 15, GxEPD_GRAY2);
  
  int y = height - 12;
  displayDriver->drawString(5, y, "🌡️" + String(sensor.temperature, 1) + "°", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(width / 3, y, "💧" + String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(width * 2 / 3, y, "🌿优", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// ========== 1.44 寸 (128x128) UI 绘制 ==========

void DisplayManager::drawUI_1_44inch() {
  if (displayDriver == nullptr) return;
  
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  drawStatusBar_1_44inch();
  drawTime_1_44inch();
  drawDate_1_44inch();
  drawWeather_1_44inch();
  drawSensor_1_44inch();
  
  displayDriver->update();
}

void DisplayManager::drawStatusBar_1_44inch() {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) return;
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 2, y = 2;
  displayDriver->drawRect(x, y, 12, 6, GxEPD_BLACK);
  displayDriver->drawRect(x + 12, y + 1, 1, 4, GxEPD_BLACK);
  
  int levelWidth = 10 * percentage / 100;
  displayDriver->fillRect(x + 1, y + 1, levelWidth, 4, GxEPD_BLACK);
  
  displayDriver->drawString(x + 15, y, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (messageCount > 0) {
    displayDriver->drawString(x + 35, y, "🔔" + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawTime_1_44inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0');
  
  displayDriver->drawString(0, 12, timeStr, GxEPD_BLACK, GxEPD_WHITE, 3);
}

void DisplayManager::drawDate_1_44inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  String dateStr = padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  displayDriver->drawString(0, 38, dateStr + " " + weekStr, GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    displayDriver->drawString(0, 50, lunar.lunarMonth + lunar.lunarDay, GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawWeather_1_44inch() {
  if (displayDriver == nullptr) return;
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) return;
  
  WeatherData weather = weatherManager->getWeatherData();
  
  displayDriver->drawString(0, 62, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  int forecastY = 82;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      int x = i * 42;
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(x, forecastY, icon, GxEPD_BLACK, GxEPD_WHITE, 1);
      displayDriver->drawString(x + 10, forecastY, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 1);
    }
  }
}

void DisplayManager::drawSensor_1_44inch() {
  if (displayDriver == nullptr) return;
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) return;
  
  SensorData sensor = sensorManager->getSensorData();
  
  displayDriver->drawLine(0, height - 14, width, height - 14, GxEPD_GRAY2);
  
  int y = height - 11;
  displayDriver->drawString(5, y, "🌡️" + String(sensor.temperature, 1) + "°", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(width / 2, y, "💧" + String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// ========== 1.02 寸 (80x128) UI 绘制 ==========

void DisplayManager::drawUI_1_02inch() {
  if (displayDriver == nullptr) return;
  
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  drawStatusBar_1_02inch();
  drawTime_1_02inch();
  drawDate_1_02inch();
  drawWeather_1_02inch();
  drawSensor_1_02inch();
  
  displayDriver->update();
}

// ========== 7.3 寸 (1920x1080) UI 绘制 ==========

void DisplayManager::drawUI_7_3inch() {
  if (displayDriver == nullptr) return;
  
  // 清屏
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  // 计算左右面板位置
  leftPanelWidth = width * 0.45;
  int rightPanelX = leftPanelWidth;
  
  // 绘制左侧面板背景
  displayDriver->fillRect(0, 0, leftPanelWidth, height, GxEPD_WHITE);
  displayDriver->drawLine(leftPanelWidth, 0, leftPanelWidth, height, GxEPD_GRAY2);
  
  // 绘制右侧面板背景（浅灰色）
  displayDriver->fillRect(rightPanelX, 0, width - rightPanelX, height, GxEPD_GRAY2);
  
  // 绘制左侧内容
  drawLeftPanel_7_3inch();
  
  // 绘制右侧内容
  drawRightPanel_7_3inch();
  
  // 刷新显示
  displayDriver->update();
}

void DisplayManager::drawLeftPanel_7_3inch() {
  if (displayDriver == nullptr) return;
  
  int x = 30, y = 30;
  
  // 状态栏
  drawStatusBar_7_3inch();
  y = 80;
  
  // 时间显示（80px 粗体）
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (timeManager) {
    TimeData time = timeManager->getTimeData();
    String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0') + ":" + padStart(String(time.second), 2, '0');
    displayDriver->drawString(x, y, timeStr, GxEPD_BLACK, GxEPD_WHITE, 8);
  }
  
  // 日期和农历（20px）
  y += 100;
  drawDate_7_3inch(x, y);
  
  // 天气区域（蓝色渐变背景）
  y += 40;
  drawWeather_7_3inch(x, y);
  
  // 传感器区域（绿色渐变背景）
  y += 120;
  drawSensor_7_3inch(x, y);
}

void DisplayManager::drawStatusBar_7_3inch() {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) return;
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 30, y = 30;
  
  // 绘制电池图标 (40x20px)
  displayDriver->drawRect(x, y, 40, 20, GxEPD_BLACK);
  displayDriver->drawRect(x + 40, y + 5, 4, 10, GxEPD_BLACK);
  
  // 绘制电量
  int levelWidth = 32 * percentage / 100;
  displayDriver->fillRect(x + 4, y + 4, levelWidth, 12, GxEPD_BLACK);
  
  // 绘制百分比文字
  displayDriver->drawString(x + 48, y + 2, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 绘制消息数量
  if (messageCount > 0) {
    displayDriver->drawString(x + 120, y + 2, "🔔 " + String(messageCount) + "条", GxEPD_BLACK, GxEPD_WHITE, 3);
  }
}

void DisplayManager::drawDate_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 公历日期
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  
  // 农历日期
  String lunarStr = "";
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    lunarStr = " | " + lunar.lunarMonth + "月" + lunar.lunarDay;
  }
  
  displayDriver->drawString(x, y, dateStr + " " + weekStr + lunarStr, GxEPD_BLACK, GxEPD_WHITE, 3);
}

void DisplayManager::drawWeather_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) return;
  
  WeatherData weather = weatherManager->getWeatherData();
  
  // 绘制背景框（蓝色渐变效果用单色代替）
  displayDriver->fillRect(x, y, 800, 150, GxEPD_GRAY2);
  displayDriver->drawRect(x, y, 800, 150, GxEPD_BLUE);
  
  // 城市
  displayDriver->drawString(x + 15, y + 10, "📍 北京市", GxEPD_BLUE, GxEPD_WHITE, 3);
  
  // 当前温度（40px 粗体）
  displayDriver->drawString(x + 15, y + 40, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 5);
  
  // 天气状况
  displayDriver->drawString(x + 15, y + 90, weather.condition, GxEPD_GRAY2, GxEPD_WHITE, 3);
  
  // 天气预报（三天横向排列）
  int forecastX = x + 300;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      String dayStr = (i == 0) ? "今天" : ((i == 1) ? "明天" : "后天");
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(forecastX + i * 150, y + 10, dayStr, GxEPD_GRAY2, GxEPD_WHITE, 2);
      displayDriver->drawString(forecastX + i * 150, y + 40, icon, GxEPD_BLACK, GxEPD_WHITE, 4);
      displayDriver->drawString(forecastX + i * 150, y + 100, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 3);
    }
  }
}

void DisplayManager::drawSensor_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) return;
  
  SensorData sensor = sensorManager->getSensorData();
  
  // 绘制背景框（绿色渐变效果用单色代替）
  displayDriver->fillRect(x, y, 800, 120, GxEPD_GRAY2);
  displayDriver->drawRect(x, y, 800, 120, GxEPD_BLACK);
  
  // 标题
  displayDriver->drawString(x + 15, y + 10, "🌡️ 室内环境", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 传感器数据（3 列）
  displayDriver->drawString(x + 15, y + 45, "温度", GxEPD_GRAY2, GxEPD_WHITE, 2);
  displayDriver->drawString(x + 15, y + 75, String(sensor.temperature, 1) + "°C", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  displayDriver->drawString(x + 270, y + 45, "湿度", GxEPD_GRAY2, GxEPD_WHITE, 2);
  displayDriver->drawString(x + 270, y + 75, String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  displayDriver->drawString(x + 530, y + 45, "空气质量", GxEPD_GRAY2, GxEPD_WHITE, 2);
  displayDriver->drawString(x + 530, y + 75, "优", GxEPD_BLACK, GxEPD_WHITE, 3);
}

void DisplayManager::drawRightPanel_7_3inch() {
  if (displayDriver == nullptr) return;
  
  int x = leftPanelWidth + 30, y = 30;
  
  // 日历
  drawCalendar_7_3inch(x, y);
  y += 300;
  
  // 节日
  drawFestival_7_3inch(x, y);
  y += 80;
  
  // 黄历
  drawAlmanac_7_3inch(x, y);
  y += 80;
  
  // 股票信息
  drawStockInfo_7_3inch(x, y);
  y += 100;
  
  // 消息通知
  drawMessageInfo_7_3inch(x, y);
  y += 80;
  
  // 系统状态
  drawSystemStatus_7_3inch(x, y);
}

// 7.3寸股票信息绘制
void DisplayManager::drawStockInfo_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  // 绘制股票标题
  displayDriver->drawString(x, y, "📈 股票", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 示例股票数据
  int stockY = y + 30;
  displayDriver->drawString(x, stockY, "上证指数: 3200.50 +0.8%", GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, stockY + 25, "深证成指: 10500.20 +1.2%", GxEPD_BLACK, GxEPD_WHITE, 2);
}

// 7.3寸消息通知绘制
void DisplayManager::drawMessageInfo_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  int messageCount = 0;
  if (messageManager) {
    messageCount = messageManager->getUnreadMessageCount();
  }
  
  // 绘制消息标题
  displayDriver->drawString(x, y, "🔔 消息", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 消息数量
  int messageY = y + 30;
  displayDriver->drawString(x, messageY, "未读消息: " + String(messageCount) + "条", GxEPD_BLACK, GxEPD_WHITE, 2);
}

// 7.3寸系统状态绘制
void DisplayManager::drawSystemStatus_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  int batteryPercentage = 100;
  bool isCharging = false;
  if (powerManager) {
    batteryPercentage = powerManager->getBatteryPercentage();
    isCharging = powerManager->getChargingStatus();
  }
  
  // 绘制系统状态标题
  displayDriver->drawString(x, y, "⚙️ 系统", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 系统信息
  int statusY = y + 30;
  displayDriver->drawString(x, statusY, "电量: " + String(batteryPercentage) + "% " + (isCharging ? "充电中" : ""), GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, statusY + 25, "网络: 已连接", GxEPD_BLACK, GxEPD_WHITE, 2);
  displayDriver->drawString(x, statusY + 50, "版本: v1.0.0", GxEPD_BLACK, GxEPD_WHITE, 2);
}

// ========== 6 寸 (1448x1072) UI 绘制 ==========

void DisplayManager::drawUI_6inch() {
  if (displayDriver == nullptr) return;
  
  // 清屏
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  // 计算左右面板位置
  leftPanelWidth = width * 0.45;
  int rightPanelX = leftPanelWidth;
  
  // 绘制左侧面板背景
  displayDriver->fillRect(0, 0, leftPanelWidth, height, GxEPD_WHITE);
  displayDriver->drawLine(leftPanelWidth, 0, leftPanelWidth, height, GxEPD_GRAY2);
  
  // 绘制右侧面板背景（浅灰色）
  displayDriver->fillRect(rightPanelX, 0, width - rightPanelX, height, GxEPD_GRAY2);
  
  // 绘制左侧内容
  drawLeftPanel_6inch();
  
  // 绘制右侧内容
  drawRightPanel_6inch();
  
  // 刷新显示
  displayDriver->update();
}

void DisplayManager::drawLeftPanel_6inch() {
  if (displayDriver == nullptr) return;
  
  int x = 25, y = 25;
  
  // 状态栏
  drawStatusBar_6inch();
  y = 70;
  
  // 时间显示（60px 粗体）
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (timeManager) {
    TimeData time = timeManager->getTimeData();
    String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0') + ":" + padStart(String(time.second), 2, '0');
    displayDriver->drawString(x, y, timeStr, GxEPD_BLACK, GxEPD_WHITE, 6);
  }
  
  // 日期和农历（18px）
  y += 80;
  drawDate_6inch(x, y);
  
  // 天气区域（蓝色渐变背景）
  y += 35;
  drawWeather_6inch(x, y);
  
  // 传感器区域（绿色渐变背景）
  y += 100;
  drawSensor_6inch(x, y);
}

void DisplayManager::drawStatusBar_6inch() {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) return;
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 25, y = 25;
  
  // 绘制电池图标 (35x18px)
  displayDriver->drawRect(x, y, 35, 18, GxEPD_BLACK);
  displayDriver->drawRect(x + 35, y + 4, 3, 10, GxEPD_BLACK);
  
  // 绘制电量
  int levelWidth = 28 * percentage / 100;
  displayDriver->fillRect(x + 3, y + 3, levelWidth, 12, GxEPD_BLACK);
  
  // 绘制百分比文字
  displayDriver->drawString(x + 42, y + 2, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 绘制消息数量
  if (messageCount > 0) {
    displayDriver->drawString(x + 100, y + 2, "🔔 " + String(messageCount) + "条", GxEPD_BLACK, GxEPD_WHITE, 2);
  }
}

void DisplayManager::drawDate_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 公历日期
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  
  // 农历日期
  String lunarStr = "";
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    lunarStr = " | " + lunar.lunarMonth + "月" + lunar.lunarDay;
  }
  
  displayDriver->drawString(x, y, dateStr + " " + weekStr + lunarStr, GxEPD_BLACK, GxEPD_WHITE, 2);
}

void DisplayManager::drawWeather_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) return;
  
  WeatherData weather = weatherManager->getWeatherData();
  
  // 绘制背景框（蓝色渐变效果用单色代替）
  displayDriver->fillRect(x, y, 650, 120, GxEPD_GRAY2);
  displayDriver->drawRect(x, y, 650, 120, GxEPD_BLUE);
  
  // 城市
  displayDriver->drawString(x + 12, y + 8, "📍 北京市", GxEPD_BLUE, GxEPD_WHITE, 2);
  
  // 当前温度（30px 粗体）
  displayDriver->drawString(x + 12, y + 35, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 4);
  
  // 天气状况
  displayDriver->drawString(x + 12, y + 75, weather.condition, GxEPD_GRAY2, GxEPD_WHITE, 2);
  
  // 天气预报（三天横向排列）
  int forecastX = x + 250;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      String dayStr = (i == 0) ? "今天" : ((i == 1) ? "明天" : "后天");
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(forecastX + i * 120, y + 8, dayStr, GxEPD_GRAY2, GxEPD_WHITE, 1);
      displayDriver->drawString(forecastX + i * 120, y + 30, icon, GxEPD_BLACK, GxEPD_WHITE, 3);
      displayDriver->drawString(forecastX + i * 120, y + 80, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 2);
    }
  }
}

void DisplayManager::drawSensor_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) return;
  
  SensorData sensor = sensorManager->getSensorData();
  
  // 绘制背景框（绿色渐变效果用单色代替）
  displayDriver->fillRect(x, y, 650, 100, GxEPD_GRAY2);
  displayDriver->drawRect(x, y, 650, 100, GxEPD_BLACK);
  
  // 标题
  displayDriver->drawString(x + 12, y + 8, "🌡️ 室内环境", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 传感器数据（3 列）
  displayDriver->drawString(x + 12, y + 35, "温度", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 12, y + 60, String(sensor.temperature, 1) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  displayDriver->drawString(x + 220, y + 35, "湿度", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 220, y + 60, String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  displayDriver->drawString(x + 430, y + 35, "空气质量", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 430, y + 60, "优", GxEPD_BLACK, GxEPD_WHITE, 2);
}

void DisplayManager::drawRightPanel_6inch() {
  if (displayDriver == nullptr) return;
  
  int x = leftPanelWidth + 25, y = 25;
  
  // 日历
  drawCalendar_6inch(x, y);
  y += 250;
  
  // 节日
  drawFestival_6inch(x, y);
  y += 70;
  
  // 黄历
  drawAlmanac_6inch(x, y);
  y += 70;
  
  // 股票信息
  drawStockInfo_6inch(x, y);
  y += 80;
  
  // 消息通知
  drawMessageInfo_6inch(x, y);
  y += 60;
  
  // 系统状态
  drawSystemStatus_6inch(x, y);
}

// 6寸股票信息绘制
void DisplayManager::drawStockInfo_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  // 绘制股票标题
  displayDriver->drawString(x, y, "📈 股票", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 示例股票数据
  int stockY = y + 20;
  displayDriver->drawString(x, stockY, "上证指数: 3200.50 +0.8%", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, stockY + 15, "深证成指: 10500.20 +1.2%", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// 6寸消息通知绘制
void DisplayManager::drawMessageInfo_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  int messageCount = 0;
  if (messageManager) {
    messageCount = messageManager->getUnreadMessageCount();
  }
  
  // 绘制消息标题
  displayDriver->drawString(x, y, "🔔 消息", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 消息数量
  int messageY = y + 20;
  displayDriver->drawString(x, messageY, "未读消息: " + String(messageCount) + "条", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// 6寸系统状态绘制
void DisplayManager::drawSystemStatus_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  int batteryPercentage = 100;
  bool isCharging = false;
  if (powerManager) {
    batteryPercentage = powerManager->getBatteryPercentage();
    isCharging = powerManager->getChargingStatus();
  }
  
  // 绘制系统状态标题
  displayDriver->drawString(x, y, "⚙️ 系统", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 系统信息
  int statusY = y + 20;
  displayDriver->drawString(x, statusY, "电量: " + String(batteryPercentage) + "% " + (isCharging ? "充电中" : ""), GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, statusY + 15, "网络: 已连接", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, statusY + 30, "版本: v1.0.0", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// ========== 5.83 寸 (1448x1072) UI 绘制 ==========

void DisplayManager::drawUI_5_83inch() {
  if (displayDriver == nullptr) return;
  
  // 清屏
  displayDriver->fillRect(0, 0, width, height, GxEPD_WHITE);
  
  // 计算左右面板位置
  leftPanelWidth = width * 0.45;
  int rightPanelX = leftPanelWidth;
  
  // 绘制左侧面板背景
  displayDriver->fillRect(0, 0, leftPanelWidth, height, GxEPD_WHITE);
  displayDriver->drawLine(leftPanelWidth, 0, leftPanelWidth, height, GxEPD_GRAY2);
  
  // 绘制右侧面板背景（浅灰色）
  displayDriver->fillRect(rightPanelX, 0, width - rightPanelX, height, GxEPD_GRAY2);
  
  // 绘制左侧内容
  drawLeftPanel_5_83inch();
  
  // 绘制右侧内容
  drawRightPanel_5_83inch();
  
  // 刷新显示
  displayDriver->update();
}

void DisplayManager::drawLeftPanel_5_83inch() {
  if (displayDriver == nullptr) return;
  
  int x = 25, y = 25;
  
  // 状态栏
  drawStatusBar_5_83inch();
  y = 70;
  
  // 时间显示（55px 粗体）
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (timeManager) {
    TimeData time = timeManager->getTimeData();
    String timeStr = padStart(String(time.hour), 2, '0') + ":" + padStart(String(time.minute), 2, '0') + ":" + padStart(String(time.second), 2, '0');
    displayDriver->drawString(x, y, timeStr, GxEPD_BLACK, GxEPD_WHITE, 5);
  }
  
  // 日期和农历（18px）
  y += 75;
  drawDate_5_83inch(x, y);
  
  // 天气区域（蓝色渐变背景）
  y += 35;
  drawWeather_5_83inch(x, y);
  
  // 传感器区域（绿色渐变背景）
  y += 95;
  drawSensor_5_83inch(x, y);
}

void DisplayManager::drawStatusBar_5_83inch() {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) return;
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 25, y = 25;
  
  // 绘制电池图标 (35x18px)
  displayDriver->drawRect(x, y, 35, 18, GxEPD_BLACK);
  displayDriver->drawRect(x + 35, y + 4, 3, 10, GxEPD_BLACK);
  
  // 绘制电量
  int levelWidth = 28 * percentage / 100;
  displayDriver->fillRect(x + 3, y + 3, levelWidth, 12, GxEPD_BLACK);
  
  // 绘制百分比文字
  displayDriver->drawString(x + 42, y + 2, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 绘制消息数量
  if (messageCount > 0) {
    displayDriver->drawString(x + 100, y + 2, "🔔 " + String(messageCount) + "条", GxEPD_BLACK, GxEPD_WHITE, 2);
  }
}

void DisplayManager::drawDate_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 公历日期
  String dateStr = String(time.year) + "-" + padStart(String(time.month), 2, '0') + "-" + padStart(String(time.day), 2, '0');
  String weekStr = getWeekString(time.weekday);
  
  // 农历日期
  String lunarStr = "";
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    lunarStr = " | " + lunar.lunarMonth + "月" + lunar.lunarDay;
  }
  
  displayDriver->drawString(x, y, dateStr + " " + weekStr + lunarStr, GxEPD_BLACK, GxEPD_WHITE, 2);
}

void DisplayManager::drawWeather_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) return;
  
  WeatherData weather = weatherManager->getWeatherData();
  
  // 绘制背景框（蓝色渐变效果用单色代替）
  displayDriver->fillRect(x, y, 650, 110, GxEPD_GRAY2);
  displayDriver->drawRect(x, y, 650, 110, GxEPD_BLUE);
  
  // 城市
  displayDriver->drawString(x + 12, y + 8, "📍 北京市", GxEPD_BLUE, GxEPD_WHITE, 2);
  
  // 当前温度（28px 粗体）
  displayDriver->drawString(x + 12, y + 30, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 4);
  
  // 天气状况
  displayDriver->drawString(x + 12, y + 65, weather.condition, GxEPD_GRAY2, GxEPD_WHITE, 2);
  
  // 天气预报（三天横向排列）
  int forecastX = x + 250;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      String dayStr = (i == 0) ? "今天" : ((i == 1) ? "明天" : "后天");
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(forecastX + i * 120, y + 8, dayStr, GxEPD_GRAY2, GxEPD_WHITE, 1);
      displayDriver->drawString(forecastX + i * 120, y + 25, icon, GxEPD_BLACK, GxEPD_WHITE, 3);
      displayDriver->drawString(forecastX + i * 120, y + 70, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 2);
    }
  }
}

void DisplayManager::drawSensor_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) return;
  
  SensorData sensor = sensorManager->getSensorData();
  
  // 绘制背景框（绿色渐变效果用单色代替）
  displayDriver->fillRect(x, y, 650, 90, GxEPD_GRAY2);
  displayDriver->drawRect(x, y, 650, 90, GxEPD_BLACK);
  
  // 标题
  displayDriver->drawString(x + 12, y + 8, "🌡️ 室内环境", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 传感器数据（3 列）
  displayDriver->drawString(x + 12, y + 30, "温度", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 12, y + 55, String(sensor.temperature, 1) + "°C", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  displayDriver->drawString(x + 220, y + 30, "湿度", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 220, y + 55, String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  displayDriver->drawString(x + 430, y + 30, "空气质量", GxEPD_GRAY2, GxEPD_WHITE, 1);
  displayDriver->drawString(x + 430, y + 55, "优", GxEPD_BLACK, GxEPD_WHITE, 2);
}

void DisplayManager::drawRightPanel_5_83inch() {
  if (displayDriver == nullptr) return;
  
  int x = leftPanelWidth + 25, y = 25;
  
  // 日历
  drawCalendar_5_83inch(x, y);
  y += 240;
  
  // 节日
  drawFestival_5_83inch(x, y);
  y += 65;
  
  // 黄历
  drawAlmanac_5_83inch(x, y);
  y += 65;
  
  // 股票信息
  drawStockInfo_5_83inch(x, y);
  y += 75;
  
  // 消息通知
  drawMessageInfo_5_83inch(x, y);
  y += 55;
  
  // 系统状态
  drawSystemStatus_5_83inch(x, y);
}

// 5.83寸股票信息绘制
void DisplayManager::drawStockInfo_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  // 绘制股票标题
  displayDriver->drawString(x, y, "📈 股票", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 示例股票数据
  int stockY = y + 18;
  displayDriver->drawString(x, stockY, "上证指数: 3200.50 +0.8%", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, stockY + 14, "深证成指: 10500.20 +1.2%", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// 5.83寸消息通知绘制
void DisplayManager::drawMessageInfo_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  int messageCount = 0;
  if (messageManager) {
    messageCount = messageManager->getUnreadMessageCount();
  }
  
  // 绘制消息标题
  displayDriver->drawString(x, y, "🔔 消息", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 消息数量
  int messageY = y + 18;
  displayDriver->drawString(x, messageY, "未读消息: " + String(messageCount) + "条", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// 5.83寸系统状态绘制
void DisplayManager::drawSystemStatus_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  int batteryPercentage = 100;
  bool isCharging = false;
  if (powerManager) {
    batteryPercentage = powerManager->getBatteryPercentage();
    isCharging = powerManager->getChargingStatus();
  }
  
  // 绘制系统状态标题
  displayDriver->drawString(x, y, "⚙️ 系统", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 系统信息
  int statusY = y + 18;
  displayDriver->drawString(x, statusY, "电量: " + String(batteryPercentage) + "% " + (isCharging ? "充电中" : ""), GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, statusY + 14, "网络: 已连接", GxEPD_BLACK, GxEPD_WHITE, 1);
  displayDriver->drawString(x, statusY + 28, "版本: v1.0.0", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// 日历绘制函数
void DisplayManager::drawCalendar_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 绘制日历标题
  String monthTitle = "📅 " + String(time.year) + "年" + String(time.month) + "月";
  displayDriver->drawString(x, y, monthTitle, GxEPD_BLACK, GxEPD_WHITE, 4);
  
  // 绘制星期标题
  const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
  int startX = x;
  int cellWidth = 120;
  int cellHeight = 50;
  int startY = y + 50;
  
  for (int i = 0; i < 7; i++) {
    displayDriver->drawString(startX + i * cellWidth, startY, weekdays[i], GxEPD_GRAY2, GxEPD_WHITE, 2);
  }
  
  // 计算当月第一天是星期几和总天数
  int firstWeekday = getFirstWeekdayOfMonth(time.year, time.month);
  int daysInMonth = getDaysInMonth(time.year, time.month);
  
  // 绘制日期
  int currentX = startX + firstWeekday * cellWidth;
  int currentY = startY + 30;
  
  for (int day = 1; day <= daysInMonth; day++) {
    if (day == time.day) {
      // 今天：反色显示
      displayDriver->fillRect(currentX - 3, currentY - 3, cellWidth - 4, cellHeight, GxEPD_BLACK);
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_WHITE, GxEPD_BLACK, 3);
    } else {
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_BLACK, GxEPD_WHITE, 3);
    }
    
    currentX += cellWidth;
    if (currentX > startX + 6 * cellWidth) {
      currentX = startX;
      currentY += cellHeight;
    }
  }
}

void DisplayManager::drawCalendar_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 绘制日历标题
  String monthTitle = "📅 " + String(time.year) + "年" + String(time.month) + "月";
  displayDriver->drawString(x, y, monthTitle, GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 绘制星期标题
  const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
  int startX = x;
  int cellWidth = 90;
  int cellHeight = 40;
  int startY = y + 40;
  
  for (int i = 0; i < 7; i++) {
    displayDriver->drawString(startX + i * cellWidth, startY, weekdays[i], GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
  
  // 计算当月第一天是星期几和总天数
  int firstWeekday = getFirstWeekdayOfMonth(time.year, time.month);
  int daysInMonth = getDaysInMonth(time.year, time.month);
  
  // 绘制日期
  int currentX = startX + firstWeekday * cellWidth;
  int currentY = startY + 25;
  
  for (int day = 1; day <= daysInMonth; day++) {
    if (day == time.day) {
      // 今天：反色显示
      displayDriver->fillRect(currentX - 2, currentY - 2, cellWidth - 3, cellHeight, GxEPD_BLACK);
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_WHITE, GxEPD_BLACK, 2);
    } else {
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_BLACK, GxEPD_WHITE, 2);
    }
    
    currentX += cellWidth;
    if (currentX > startX + 6 * cellWidth) {
      currentX = startX;
      currentY += cellHeight;
    }
  }
}

void DisplayManager::drawCalendar_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 绘制日历标题
  String monthTitle = "📅 " + String(time.year) + "年" + String(time.month) + "月";
  displayDriver->drawString(x, y, monthTitle, GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 绘制星期标题
  const char* weekdays[] = {"日", "一", "二", "三", "四", "五", "六"};
  int startX = x;
  int cellWidth = 85;
  int cellHeight = 38;
  int startY = y + 38;
  
  for (int i = 0; i < 7; i++) {
    displayDriver->drawString(startX + i * cellWidth, startY, weekdays[i], GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
  
  // 计算当月第一天是星期几和总天数
  int firstWeekday = getFirstWeekdayOfMonth(time.year, time.month);
  int daysInMonth = getDaysInMonth(time.year, time.month);
  
  // 绘制日期
  int currentX = startX + firstWeekday * cellWidth;
  int currentY = startY + 23;
  
  for (int day = 1; day <= daysInMonth; day++) {
    if (day == time.day) {
      // 今天：反色显示
      displayDriver->fillRect(currentX - 2, currentY - 2, cellWidth - 3, cellHeight, GxEPD_BLACK);
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_WHITE, GxEPD_BLACK, 2);
    } else {
      displayDriver->drawString(currentX, currentY, String(day), GxEPD_BLACK, GxEPD_WHITE, 2);
    }
    
    currentX += cellWidth;
    if (currentX > startX + 6 * cellWidth) {
      currentX = startX;
      currentY += cellHeight;
    }
  }
}

// 节日绘制函数
void DisplayManager::drawFestival_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 绘制节日标题
  displayDriver->drawString(x, y, "🎉 节日", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 示例节日数据（实际应从节日管理器获取）
  int festivalY = y + 40;
  
  // 春分示例
  displayDriver->drawString(x, festivalY, "🌸 春分 (3 月 21 日)", GxEPD_BLACK, GxEPD_WHITE, 2);
  festivalY += 30;
  
  // 清明示例
  displayDriver->drawString(x, festivalY, "🎊 清明 (4 月 5 日)", GxEPD_BLACK, GxEPD_WHITE, 2);
}

void DisplayManager::drawFestival_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 绘制节日标题
  displayDriver->drawString(x, y, "🎉 节日", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 示例节日数据（实际应从节日管理器获取）
  int festivalY = y + 30;
  
  // 春分示例
  displayDriver->drawString(x, festivalY, "🌸 春分 (3月21日)", GxEPD_BLACK, GxEPD_WHITE, 1);
  festivalY += 20;
  
  // 清明示例
  displayDriver->drawString(x, festivalY, "🎊 清明 (4月5日)", GxEPD_BLACK, GxEPD_WHITE, 1);
}

void DisplayManager::drawFestival_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  // 绘制节日标题
  displayDriver->drawString(x, y, "🎉 节日", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 示例节日数据（实际应从节日管理器获取）
  int festivalY = y + 28;
  
  // 春分示例
  displayDriver->drawString(x, festivalY, "🌸 春分 (3月21日)", GxEPD_BLACK, GxEPD_WHITE, 1);
  festivalY += 18;
  
  // 清明示例
  displayDriver->drawString(x, festivalY, "🎊 清明 (4月5日)", GxEPD_BLACK, GxEPD_WHITE, 1);
}

// 黄历绘制函数
void DisplayManager::drawAlmanac_7_3inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  if (!lunarManager) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
  
  // 绘制黄历标题
  displayDriver->drawString(x, y, "📜 黄历", GxEPD_BLACK, GxEPD_WHITE, 3);
  
  // 绘制宜忌
  int almanacY = y + 40;
  
  // 示例宜忌（实际应从农历管理器获取）
  String goodItems = "宜：祭祀 祈福 求嗣";
  String badItems = "忌：嫁娶 出行 动土";
  
  displayDriver->drawString(x, almanacY, goodItems, GxEPD_BLACK, GxEPD_WHITE, 2);
  almanacY += 30;
  displayDriver->drawString(x, almanacY, badItems, GxEPD_RED, GxEPD_WHITE, 2);
}

void DisplayManager::drawAlmanac_6inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  if (!lunarManager) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
  
  // 绘制黄历标题
  displayDriver->drawString(x, y, "📜 黄历", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 绘制宜忌
  int almanacY = y + 25;
  
  // 示例宜忌（实际应从农历管理器获取）
  String goodItems = "宜：祭祀 祈福";
  String badItems = "忌：嫁娶 出行";
  
  displayDriver->drawString(x, almanacY, goodItems, GxEPD_BLACK, GxEPD_WHITE, 1);
  almanacY += 20;
  displayDriver->drawString(x, almanacY, badItems, GxEPD_RED, GxEPD_WHITE, 1);
}

void DisplayManager::drawAlmanac_5_83inch(int x, int y) {
  if (displayDriver == nullptr) return;
  
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  if (!lunarManager) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
  
  // 绘制黄历标题
  displayDriver->drawString(x, y, "📜 黄历", GxEPD_BLACK, GxEPD_WHITE, 2);
  
  // 绘制宜忌
  int almanacY = y + 23;
  
  // 示例宜忌（实际应从农历管理器获取）
  String goodItems = "宜：祭祀 祈福";
  String badItems = "忌：嫁娶 出行";
  
  displayDriver->drawString(x, almanacY, goodItems, GxEPD_BLACK, GxEPD_WHITE, 1);
  almanacY += 18;
  displayDriver->drawString(x, almanacY, badItems, GxEPD_RED, GxEPD_WHITE, 1);
}

void DisplayManager::drawStatusBar_1_02inch() {
  if (displayDriver == nullptr) return;
  
  auto powerManager = DependencyInjectionContainer::getInstance()->getPowerManager();
  auto messageManager = DependencyInjectionContainer::getInstance()->getMessageManager();
  
  if (!powerManager || !messageManager) return;
  
  int percentage = powerManager->getBatteryPercentage();
  int messageCount = messageManager->getUnreadMessageCount();
  
  int x = 2, y = 2;
  displayDriver->drawRect(x, y, 10, 5, GxEPD_BLACK);
  displayDriver->drawRect(x + 10, y + 1, 1, 3, GxEPD_BLACK);
  
  int levelWidth = 8 * percentage / 100;
  displayDriver->fillRect(x + 1, y + 1, levelWidth, 3, GxEPD_BLACK);
  
  displayDriver->drawString(x + 13, y, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (messageCount > 0) {
    displayDriver->drawString(x + 30, y, "🔔" + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawTime_1_02inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  String timeStr = String(time.hour).length() == 1 ? "0" + String(time.hour) : String(time.hour) + ":" + (String(time.minute).length() == 1 ? "0" + String(time.minute) : String(time.minute));
  
  displayDriver->drawString(0, 10, timeStr, GxEPD_BLACK, GxEPD_WHITE, 2);
}

void DisplayManager::drawDate_1_02inch() {
  if (displayDriver == nullptr) return;
  
  auto timeManager = DependencyInjectionContainer::getInstance()->getTimeManager();
  auto lunarManager = DependencyInjectionContainer::getInstance()->getLunarManager();
  
  if (!timeManager) return;
  
  TimeData time = timeManager->getTimeData();
  
  String dateStr = (String(time.month).length() == 1 ? "0" + String(time.month) : String(time.month)) + "-" + (String(time.day).length() == 1 ? "0" + String(time.day) : String(time.day));
  String weekStr = getWeekString(time.weekday);
  displayDriver->drawString(0, 28, dateStr + " " + weekStr, GxEPD_BLACK, GxEPD_WHITE, 1);
  
  if (lunarManager) {
    LunarInfo lunar = lunarManager->getLunarInfo(time.year, time.month, time.day);
    displayDriver->drawString(0, 40, lunar.lunarMonth + lunar.lunarDay, GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

void DisplayManager::drawWeather_1_02inch() {
  if (displayDriver == nullptr) return;
  
  auto weatherManager = DependencyInjectionContainer::getInstance()->getWeatherManager();
  if (!weatherManager) return;
  
  WeatherData weather = weatherManager->getWeatherData();
  
  displayDriver->drawString(0, 52, String(weather.temp, 0) + "°C", GxEPD_BLACK, GxEPD_WHITE, 1);
  
  int forecastY = 66;
  for (int i = 0; i < 3; i++) {
    ForecastData forecast = weatherManager->getForecastData(i);
    if (forecast.date.length() > 0) {
      int x = i * 26;
      String icon = weatherManager->getWeatherIcon(forecast.condition);
      displayDriver->drawString(x, forecastY, icon, GxEPD_BLACK, GxEPD_WHITE, 1);
      displayDriver->drawString(x + 8, forecastY, String(forecast.tempDay, 0) + "°", GxEPD_BLUE, GxEPD_WHITE, 1);
    }
  }
}

void DisplayManager::drawSensor_1_02inch() {
  if (displayDriver == nullptr) return;
  
  auto sensorManager = DependencyInjectionContainer::getInstance()->getSensorManager();
  if (!sensorManager) return;
  
  SensorData sensor = sensorManager->getSensorData();
  
  displayDriver->drawLine(0, height - 16, width, height - 16, GxEPD_GRAY2);
  
  int y = height - 13;
  if (sensor.valid) {
    displayDriver->drawString(5, y, "🌡️" + String(sensor.temperature, 1) + "°", GxEPD_BLACK, GxEPD_WHITE, 1);
    displayDriver->drawString(5, y + 10, "💧" + String(sensor.humidity, 0) + "%", GxEPD_BLACK, GxEPD_WHITE, 1);
  } else {
    displayDriver->drawString(5, y, "传感器数据无效", GxEPD_GRAY2, GxEPD_WHITE, 1);
  }
}

uint8_t DisplayManager::calculateFontSize(uint16_t screenWidth, uint16_t screenHeight, String text, float widthRatio) {
  float availableWidth = screenWidth * widthRatio;
  int charCount = text.length();
  float pixelsPerChar = availableWidth / charCount;
  
  if (pixelsPerChar >= 30) return 6;
  if (pixelsPerChar >= 24) return 5;
  if (pixelsPerChar >= 18) return 4;
  if (pixelsPerChar >= 14) return 3;
  if (pixelsPerChar >= 10) return 2;
  return 1;
}

uint8_t DisplayManager::calculateTimeFontSize(uint16_t screenWidth) {
  if (screenWidth >= 800) return 6;
  if (screenWidth >= 480) return 5;
  if (screenWidth >= 400) return 4;
  if (screenWidth >= 300) return 4;
  if (screenWidth >= 250) return 3;
  if (screenWidth >= 200) return 3;
  if (screenWidth >= 150) return 2;
  return 1;
}

uint8_t DisplayManager::calculateDateFontSize(uint16_t screenWidth) {
  if (screenWidth >= 800) return 2;
  if (screenWidth >= 480) return 2;
  if (screenWidth >= 400) return 2;
  if (screenWidth >= 300) return 1;
  if (screenWidth >= 250) return 1;
  if (screenWidth >= 200) return 1;
  return 1;
}

uint8_t DisplayManager::calculateWeatherFontSize(uint16_t screenWidth) {
  if (screenWidth >= 800) return 3;
  if (screenWidth >= 480) return 2;
  if (screenWidth >= 400) return 2;
  if (screenWidth >= 300) return 2;
  if (screenWidth >= 250) return 2;
  if (screenWidth >= 200) return 2;
  if (screenWidth >= 150) return 1;
  return 1;
}

DisplayManager::RefreshAreaConfig DisplayManager::getRefreshAreaConfig(ScreenSize size) {
  DisplayManager::RefreshAreaConfig config = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  
  switch (size) {
    case SCREEN_SIZE_7_5_INCH:
      config = {20, 50, 360, 100, 20, 160, 360, 60, 20, 230, 360, 30};
      break;
    case SCREEN_SIZE_7_3_INCH:
      config = {30, 80, 800, 120, 30, 220, 800, 150, 30, 380, 800, 120};
      break;
    case SCREEN_SIZE_6_INCH:
      config = {25, 70, 650, 100, 25, 180, 650, 120, 25, 300, 650, 100};
      break;
    case SCREEN_SIZE_5_83_INCH:
      config = {25, 70, 650, 90, 25, 175, 650, 110, 25, 290, 650, 90};
      break;
    case SCREEN_SIZE_4_2_INCH:
      config = {0, 14, 180, 50, 0, 75, 180, 40, 0, 285, 180, 15};
      break;
    case SCREEN_SIZE_3_7_INCH:
      config = {0, 16, 216, 50, 0, 82, 216, 40, 0, 265, 216, 15};
      break;
    case SCREEN_SIZE_2_9_INCH:
      config = {0, 18, 296, 30, 0, 55, 296, 30, 0, 110, 296, 18};
      break;
    case SCREEN_SIZE_2_66_INCH:
      config = {0, 16, 296, 30, 0, 68, 296, 30, 0, 137, 296, 15};
      break;
    case SCREEN_SIZE_2_13_INCH:
      config = {0, 14, 250, 30, 0, 56, 250, 30, 0, 106, 250, 16};
      break;
    case SCREEN_SIZE_1_54_INCH:
      config = {0, 14, 200, 30, 0, 65, 200, 30, 0, 184, 200, 16};
      break;
    case SCREEN_SIZE_1_44_INCH:
      config = {0, 12, 128, 30, 0, 62, 128, 30, 0, 114, 128, 14};
      break;
    case SCREEN_SIZE_1_02_INCH:
      config = {0, 10, 80, 25, 0, 52, 80, 25, 0, 112, 80, 16};
      break;
    default:
      config = {0, 20, 400, 60, 0, 90, 400, 50, 0, 250, 400, 30};
      break;
  }
  
  return config;
}

String DisplayManager::getWeekString(int weekday) {
  switch (weekday) {
    case 0: return "周日";
    case 1: return "周一";
    case 2: return "周二";
    case 3: return "周三";
    case 4: return "周四";
    case 5: return "周五";
    case 6: return "周六";
    default: return "";
  }
}

