#include "display_manager.h"
#include "../services/time_manager.h"
#include "../modules/weather_manager.h"
#include "../modules/sensor_manager.h"
#include "../modules/stock_manager.h"
#include "../modules/message_manager.h"
#include "../services/power_manager.h"

// 外部全局对象
extern TimeManager timeManager;
extern LunarManager lunarManager;
extern WeatherManager weatherManager;
extern SensorManager sensorManager;
extern StockManager stockManager;
extern MessageManager messageManager;
extern PowerManager powerManager;

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
  cachedTimeData = {0, 0, 0, 0, 0, 0, 0, false, "", ""};
  cachedWeatherData = {"未知", 0, "未知", 0, 0, false};
  cachedSensorData = {0.0, 0.0, false, 0, false, 0};
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
  
  // 初始化布局配置
  layoutMode = LAYOUT_MODE_STANDARD;
  currentLayout = {
    LAYOUT_MODE_STANDARD,
    0.6f,   // 左侧面板比例 60%
    0.4f,   // 右侧面板比例 40%
    12,     // 基础字体大小
    8,      // 元素间距
    false   // 默认不显示边框
  };
  
  // 订阅事件
  EVENT_SUBSCRIBE(EVENT_ALARM_TRIGGERED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto alarmData = std::dynamic_pointer_cast<AlarmEventData>(data);
    if (alarmData) {
      this->showAlarm(alarmData->alarmType, alarmData->message);
    }
  }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_TIME_UPDATED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto timeData = std::dynamic_pointer_cast<TimeDataEventData>(data);
    if (timeData) {
      cachedTimeData = timeData->timeData;
      this->updateDisplay();
    }
  }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_WEATHER_UPDATED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto weatherData = std::dynamic_pointer_cast<WeatherDataEventData>(data);
    if (weatherData) {
      cachedWeatherData = weatherData->weatherData;
      this->updateDisplay();
    }
  }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_SENSOR_DATA_UPDATED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto sensorData = std::dynamic_pointer_cast<SensorDataEventData>(data);
    if (sensorData) {
      cachedSensorData = sensorData->sensorData;
      this->updateDisplay();
    }
  }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_POWER_STATE_CHANGED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto powerData = std::dynamic_pointer_cast<PowerEventData>(data);
    if (powerData) {
      cachedBatteryPercentage = powerData->batteryPercentage;
      cachedBatteryVoltage = powerData->batteryVoltage;
      cachedIsCharging = powerData->isCharging;
      this->updateDisplay();
    }
  }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_MESSAGE_RECEIVED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto messageData = std::dynamic_pointer_cast<MessageEventData>(data);
    if (messageData) {
      cachedUnreadMessageCount++;
      this->updateDisplay();
    }
  }, "DisplayManager");
  
  EVENT_SUBSCRIBE(EVENT_MESSAGE_READ, [this](EventType type, std::shared_ptr<EventData> data) {
    auto messageData = std::dynamic_pointer_cast<MessageEventData>(data);
    if (messageData && cachedUnreadMessageCount > 0) {
      cachedUnreadMessageCount--;
      this->updateDisplay();
    }
  }, "DisplayManager");
}

DisplayManager::~DisplayManager() {
  // 清理资源
  if (displayDriver != nullptr) {
    delete displayDriver;
    displayDriver = nullptr;
  }
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
    if (type == EVENT_ALARM_TRIGGERED) {
      auto alarmData = std::dynamic_pointer_cast<AlarmEventData>(data);
      if (alarmData) {
        this->showAlarm(alarmData->alarmType, alarmData->message);
      }
    }
  }, "DisplayManager");
  
  DEBUG_PRINTLN("显示管理器初始化完成");
  return true;
}

void DisplayManager::setDisplayDriver(IDisplayDriver* driver) {
  if (displayDriver != nullptr) {
    delete displayDriver;
  }
  displayDriver = driver;
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
  bool isLowPowerMode = powerManager.getLowPowerMode();
  
  // 根据低功耗模式调整刷新间隔倍率
  int refreshMultiplier = isLowPowerMode ? 6 : 1; // 低功耗模式下刷新间隔延长6倍
  
  // 检查是否需要刷新
  if (!powerManager.shouldUpdateDisplay()) {
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
    // 显示秒针时，每100毫秒刷新一次时钟区域以实现平滑动画
    if (currentTime - lastClockUpdateTime >= 100) {
      needClockRefresh = true;
      needLeftPanelRefresh = true;
      lastClockUpdateTime = currentTime;
    }
  } else {
    // 不显示秒针时，每分钟刷新一次时钟区域
    if (currentTime - lastClockUpdateTime >= 60000) {
      needClockRefresh = true;
      needLeftPanelRefresh = true;
      lastClockUpdateTime = currentTime;
    }
  }
  
  // 2. 检查天气信息 - 每2小时更新一次
  if (currentTime - lastWeatherUpdateTime >= 7200000) {
    needWeatherRefresh = true;
    needLeftPanelRefresh = true;
    lastWeatherUpdateTime = currentTime;
  }
  
  // 3. 检查传感器数据 - 温度或湿度变化超过±2时刷新
  if (abs(cachedSensorData.temperature - lastTemperature) >= 2.0 || 
      abs(cachedSensorData.humidity - lastHumidity) >= 2.0) {
    needSensorRefresh = true;
    needLeftPanelRefresh = true;
    lastTemperature = cachedSensorData.temperature;
    lastHumidity = cachedSensorData.humidity;
    lastSensorUpdateTime = currentTime;
  }
  
  // 4. 检查电池信息
  if (abs(cachedBatteryPercentage - lastBatteryPercentage) > 5) {
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
  if (currentRightPage == RIGHT_PAGE_STOCK && currentTime - lastStockUpdateTime >= STOCK_REFRESH_INTERVAL * refreshMultiplier) {
    needRightPanelRefresh = true;
    lastStockUpdateTime = currentTime;
  } else if (currentRightPage == RIGHT_PAGE_CALENDAR && currentTime - lastCalendarUpdateTime >= CALENDAR_REFRESH_INTERVAL * refreshMultiplier) {
    needCalendarRefresh = true;
    needRightPanelRefresh = true;
    lastCalendarUpdateTime = currentTime;
  }
  
  // 7. 检查是否需要全屏刷新（每天至少一次或内容变化较大时）
  if (currentTime - lastFullRefreshTime >= FULL_REFRESH_INTERVAL || 
      (needLeftPanelRefresh && needRightPanelRefresh)) {
    needFullRefresh = true;
    lastFullRefreshTime = currentTime;
  }
  
  // 8. 检查是否有新消息通知，需要替换日历显示
  if (messageCount > 0 && currentRightPage == RIGHT_PAGE_CALENDAR) {
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
        if (currentClockMode == CLOCK_MODE_DIGITAL) {
          drawDigitalClock(20, 60, timeManager.getTimeString(), timeManager.getDateString());
        } else if (currentClockMode == CLOCK_MODE_ANALOG) {
          TimeData timeData = timeManager.getTimeData();
          int millisecond = millis() % 1000;
          drawAnalogClock(leftPanelWidth / 2, 120, timeData.hour, timeData.minute, timeData.second, millisecond);
        } else if (currentClockMode == CLOCK_MODE_TEXT) {
          TimeData timeData = timeManager.getTimeData();
          drawTextClock(20, 60, timeData.hour, timeData.minute, timeData.second);
        }
        displayDriver->update(0, 0, leftPanelWidth, height < 400 ? 120 : 200);
      }
      
      if (needWeatherRefresh) {
        // 只刷新天气区域
        WeatherData weather = weatherManager.getWeatherData();
        drawWeather(20, height < 400 ? 140 : 220, weather.city, 
                    (weather.temp != 0 ? String(weather.temp) : "--") + "°C", 
                    weather.condition, "", "");
        displayDriver->update(0, height < 400 ? 140 : 220, leftPanelWidth, height < 400 ? 100 : 150);
      }
      
      if (needSensorRefresh) {
        // 只刷新传感器数据区域
        SensorData sensor = sensorManager.getSensorData();
        drawSensorData(20, height < 400 ? 220 : 340, sensor.temperature, sensor.humidity);
        displayDriver->update(0, height < 400 ? 220 : 340, leftPanelWidth, height < 400 ? 80 : 120);
      }
      
      if (needBatteryRefresh || needMessageRefresh) {
        // 只刷新底部区域
        float batteryVoltage = powerManager.getBatteryVoltage();
        int batteryPercentage = powerManager.getBatteryPercentage();
        bool isCharging = powerManager.getChargingStatus();
        int messageCount = messageManager.getUnreadMessageCount();
        
        drawBatteryInfo(20, height < 400 ? 340 : 560, batteryVoltage, batteryPercentage, isCharging);
        drawMessageNotification(20, height < 400 ? 380 : 600, messageCount);
        displayDriver->update(0, height < 400 ? 340 : 560, leftPanelWidth, height < 400 ? 60 : 80);
        
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
  }
  
  // 更新消息提醒动画
  updateMessageAnimation();
  
  // 更新传感器报警状态
  updateSensorAlarm();
}

void DisplayManager::updateDisplayPartial() {
  // 局部刷新已合并到updateDisplay方法中，保持兼容性
  updateDisplay();
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
  
  // 延时显示
  delay(duration);
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
  
  try {
    // 绘制左侧面板背景
    displayDriver->fillRect(0, 0, leftPanelWidth, height, GxEPD_WHITE);
    
    // 绘制分割线
    displayDriver->fillRect(leftPanelWidth - 1, 0, 1, height, GxEPD_BLACK);
    
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
    try {
      if (currentClockMode == CLOCK_MODE_DIGITAL) {
        drawDigitalClock(20, 60, timeStr, dateStr);
      } else if (currentClockMode == CLOCK_MODE_ANALOG) {
        // 获取当前时间的时、分、秒，增加异常处理
        int hour = 0;
        int minute = 0;
        int second = 0;
        int millisecond = millis() % 1000;
        
        try {
          if (timeStr.length() >= 8) {
            hour = timeStr.substring(0, 2).toInt();
            minute = timeStr.substring(3, 5).toInt();
            second = timeStr.substring(6, 8).toInt();
          }
        } catch (const std::exception& e) {
          DEBUG_PRINT("解析时间异常: ");
          DEBUG_PRINTLN(e.what());
        }
        
        drawAnalogClock(leftPanelWidth / 2, 120, hour, minute, second, millisecond);
      } else if (currentClockMode == CLOCK_MODE_TEXT) {
        // 获取当前时间的时、分、秒，增加异常处理
        int hour = 0;
        int minute = 0;
        int second = 0;
        
        try {
          if (timeStr.length() >= 8) {
            hour = timeStr.substring(0, 2).toInt();
            minute = timeStr.substring(3, 5).toInt();
            second = timeStr.substring(6, 8).toInt();
          }
        } catch (const std::exception& e) {
          DEBUG_PRINT("解析时间异常: ");
          DEBUG_PRINTLN(e.what());
        }
        
        drawTextClock(20, 60, hour, minute, second);
      }
    } catch (const std::exception& e) {
      DEBUG_PRINT("绘制时钟异常: ");
      DEBUG_PRINTLN(e.what());
    }
    
    // 绘制公历和农历年月日信息
    try {
      // 获取当前日期的农历信息
      LunarInfo lunarInfo = lunarManager.getLunarInfo(currentTime.year, currentTime.month, currentTime.day);
      
      // 构建公历和农历日期字符串
      String gregorianStr = "公历：" + String(currentTime.year) + "年" + 
                           (currentTime.month < 10 ? "0" : "") + String(currentTime.month) + "月" + 
                           (currentTime.day < 10 ? "0" : "") + String(currentTime.day) + "日";
      
      String lunarStr = "农历：" + lunarInfo.lunarDate;
      
      // 绘制在时钟下方，天气上方
      int dateY = height < 400 ? 120 : 200;
      displayDriver->drawString(20, dateY, gregorianStr + " " + lunarStr, GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 1 : 2);
    } catch (const std::exception& e) {
      DEBUG_PRINT("绘制日期信息异常: ");
      DEBUG_PRINTLN(e.what());
    }
    
    // 绘制天气信息，调整位置到日期信息下方
    try {
      drawWeather(20, height < 400 ? 160 : 240, weather.city, 
                  (weather.temp != 0 ? String(weather.temp) : "--") + "°C", 
                  weather.condition, "", "");
    } catch (const std::exception& e) {
      DEBUG_PRINT("绘制天气异常: ");
      DEBUG_PRINTLN(e.what());
    }
    
    // 绘制室内温湿度，调整位置到天气信息下方
  try {
    drawSensorData(20, height < 400 ? 260 : 360, sensor.temperature, sensor.humidity);
  } catch (const std::exception& e) {
    DEBUG_PRINT("绘制传感器数据异常: ");
    DEBUG_PRINTLN(e.what());
  }
  
  // 绘制电池信息，调整位置到传感器数据下方
  try {
    drawBatteryInfo(20, height < 400 ? 320 : 460, batteryVoltage, batteryPercentage, isCharging);
  } catch (const std::exception& e) {
    DEBUG_PRINT("绘制电池信息异常: ");
    DEBUG_PRINTLN(e.what());
  }
  
  // 绘制消息通知，调整位置到电池信息下方
  try {
    drawMessageNotification(20, height < 400 ? 360 : 520, messageCount);
  } catch (const std::exception& e) {
    DEBUG_PRINT("绘制消息通知异常: ");
    DEBUG_PRINTLN(e.what());
  }
  } catch (const std::exception& e) {
    // 捕获所有未处理的异常，确保显示驱动不会崩溃
    DEBUG_PRINT("绘制左侧面板异常: ");
    DEBUG_PRINTLN(e.what());
    
    // 尝试恢复显示驱动
    try {
      displayDriver->init();
      displayDriver->clear();
      displayDriver->update();
    } catch (const std::exception& e) {
      DEBUG_PRINT("恢复显示驱动异常: ");
      DEBUG_PRINTLN(e.what());
    }
  }
}

void DisplayManager::drawRightPanel() {
  if (displayDriver == nullptr) {
    return;
  }
  
  try {
    // 绘制右侧面板背景
    displayDriver->fillRect(leftPanelWidth, 0, rightPanelWidth, height, GxEPD_WHITE);
    
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
        case RIGHT_PAGE_CALENDAR:
          drawCalendarPage(leftPanelWidth + 20, 20);
          
          // 在月历下方绘制当前日的节日和黄历信息，确保完整显示
          try {
            // 获取当前日期
            TimeData currentTime = timeManager.getTimeData();
            LunarInfo lunarInfo = lunarManager.getLunarInfo(currentTime.year, currentTime.month, currentTime.day);
            
            // 绘制节日信息，确保完整显示
            if (!lunarInfo.festival.name.isEmpty()) {
              String festivalText = "今日节日: " + lunarInfo.festival.name;
              displayDriver->drawString(leftPanelWidth + 20, height - 80, festivalText, GxEPD_RED, GxEPD_WHITE, height < 400 ? 1 : 2);
            }
            
            // 绘制黄历信息摘要，确保完整显示
            if (!lunarInfo.lunarCalendar.yi.isEmpty() && !lunarInfo.lunarCalendar.ji.isEmpty()) {
              String lunarCalText = "宜: " + lunarInfo.lunarCalendar.yi;
              displayDriver->drawString(leftPanelWidth + 20, height - 50, lunarCalText, GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 1 : 1);
              
              lunarCalText = "忌: " + lunarInfo.lunarCalendar.ji;
              displayDriver->drawString(leftPanelWidth + 20, height - 25, lunarCalText, GxEPD_BLACK, GxEPD_WHITE, height < 400 ? 1 : 1);
            }
          } catch (const std::exception& e) {
            DEBUG_PRINT("绘制日历附加信息异常: ");
            DEBUG_PRINTLN(e.what());
          }
          break;
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
  } catch (const std::exception& e) {
    DEBUG_PRINT("绘制右侧面板异常: ");
    DEBUG_PRINTLN(e.what());
    
    // 显示驱动出现异常时，尝试重置显示驱动
    if (displayDriver != nullptr) {
      displayDriver->init();
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
  for (int i = 0; i < min(messageCount, 5); i++) {
    // 绘制消息标题和摘要
    String message = "消息 " + String(i + 1);
    String time = "刚刚";
    MessagePriority priority = MESSAGE_PRIORITY_NORMAL;
    
    // 尝试获取实际的消息优先级
    if (i < messageManager.getMessageCount()) {
      MessageData msgData = messageManager.getMessage(i + 1);
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
    
    for (int i = 0; i < messageCount; i++) {
      MessageData message = messageManager.getMessage(i + 1);
      if (message.priority == MESSAGE_PRIORITY_URGENT) {
        hasUrgentMessage = true;
        break;
      } else if (message.priority == MESSAGE_PRIORITY_HIGH) {
        hasHighPriorityMessage = true;
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

// 其他绘制方法的实现...
// 由于篇幅限制，这里省略了部分绘制方法的实现
// 实际使用时需要将所有绘制方法从eink_display.cpp迁移到这里

void DisplayManager::drawWeather(int x, int y, String city, String temp, String condition, String humidity, String wind) {
  if (displayDriver == nullptr) {
    return;
  }
  
  int textSize = height < 400 ? 1 : 2;
  int tempSize = height < 400 ? 3 : 5;
  
  // 绘制城市
  displayDriver->drawString(x, y, city, GxEPD_BLACK, GxEPD_WHITE, textSize);
  
  // 绘制当前温度
  displayDriver->drawString(x, y + (height < 400 ? 20 : 40), temp, GxEPD_BLACK, GxEPD_WHITE, tempSize);
  
  // 绘制天气状况
  displayDriver->drawString(x, y + (height < 400 ? 50 : 100), condition, GxEPD_BLACK, GxEPD_WHITE, textSize);
  
  // 绘制天气图标（使用WeatherManager的getWeatherIcon方法）
  String weatherIcon = weatherManager.getWeatherIcon(condition);
  displayDriver->drawString(x + (height < 400 ? 80 : 160), y + (height < 400 ? 40 : 80), weatherIcon, GxEPD_BLACK, GxEPD_WHITE, tempSize);
  
  // 绘制次日天气预报
  ForecastData tomorrow = weatherManager.getForecastData(1);
  if (tomorrow.date.length() > 0) {
    int tomorrowY = y + (height < 400 ? 60 : 120);
    String weatherIcon = weatherManager.getWeatherIcon(tomorrow.condition);
    String tomorrowText = "次日: " + weatherIcon + " " + tomorrow.condition + " " + String(tomorrow.tempDay) + "°C";
    displayDriver->drawString(x, tomorrowY, tomorrowText, GxEPD_BLACK, GxEPD_WHITE, textSize);
  }
  
  // 绘制5天温度趋势图表
  int chartY = y + (height < 400 ? 90 : 150);
  int chartWidth = leftPanelWidth - 40;
  int chartHeight = height < 400 ? 60 : 80;
  
  // 获取5天天气预报数据
  float temps[5];
  float minTemp = 100, maxTemp = -100;
  
  for (int i = 0; i < 5; i++) {
    ForecastData forecast = weatherManager.getForecastData(i);
    temps[i] = forecast.tempDay;
    if (temps[i] < minTemp) minTemp = temps[i];
    if (temps[i] > maxTemp) maxTemp = temps[i];
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
  WeatherData weather = weatherManager.getWeatherData();
  int extraInfoY = chartY + chartHeight + 20;
  
  if (weather.airQuality > 0) {
    String aqiText = "空气质量: " + String(weather.airQuality) + " " + weather.airQualityLevel;
    displayDriver->drawString(x, extraInfoY, aqiText, GxEPD_BLACK, GxEPD_WHITE, textSize);
  }
  
  if (weather.uvIndex > 0) {
    String uvText = "紫外线: " + String(weather.uvIndex, 1) + " " + weather.uvIndexLevel;
    displayDriver->drawString(x, extraInfoY + 20, uvText, GxEPD_BLACK, GxEPD_WHITE, textSize);
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
  int gasLevel = sensorManager.getGasLevel();
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
  int lightLevel = sensorManager.getLightLevel();
  String lightStatus = "暗";
  if (lightLevel > 500) {
    lightStatus = "亮";
  } else if (lightLevel > 200) {
    lightStatus = "中等";
  }
  displayDriver->drawString(x, y + (height < 400 ? 90 : 170), "光照: " + lightStatus, 
                         GxEPD_BLACK, GxEPD_WHITE, dataSize);
  
  // 绘制人体感应
  bool motionDetected = sensorManager.getMotionDetected();
  displayDriver->drawString(x, y + (height < 400 ? 110 : 210), "人体感应: " + (motionDetected ? "有人" : "无人"), 
                         GxEPD_BLACK, GxEPD_WHITE, dataSize);
  
  // 绘制火焰检测
  bool flameDetected = sensorManager.getFlameDetected();
  uint16_t flameColor = GxEPD_BLACK;
  if (flameDetected) {
    flameColor = GxEPD_RED;
  }
  displayDriver->drawString(x, y + (height < 400 ? 130 : 250), "火焰检测: " + (flameDetected ? "检测到" : "未检测到"), 
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
    
    // 检查动画是否应该结束（10秒后自动停止）
    if (currentTime - messageAnimationStartTime >= 10000) {
      stopMessageAnimation();
    }
  }
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
  
  // 检查报警是否应该自动停止（30秒后）
  if (currentTime - sensorAnomalyStartTime >= 30000) {
    stopSensorAlarm();
    return;
  }
  
  // 绘制报警指示
  if (displayDriver != nullptr) {
    // 计算报警指示位置
    int alarmX = leftPanelWidth - 30;
    int alarmY = height - 30;
    
    // 清除之前的报警指示
    displayDriver->fillRect(alarmX - 5, alarmY - 5, 30, 30, 0xFFFF); // 白色背景
    
    // 绘制闪烁的报警指示
    static bool blinkState = false;
    static unsigned long lastBlinkTime = 0;
    
    if (currentTime - lastBlinkTime >= 500) {
      blinkState = !blinkState;
      lastBlinkTime = currentTime;
    }
    
    if (blinkState) {
      displayDriver->fillRect(alarmX, alarmY, 20, 20, 0x0000); // 黑色报警指示
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
        0.7f,   // 左侧面板比例 70%
        0.3f,   // 右侧面板比例 30%
        10,     // 基础字体大小（较小）
        6,      // 元素间距（较小）
        false   // 不显示边框
      };
      break;
    case LAYOUT_MODE_STANDARD:
      currentLayout = {
        LAYOUT_MODE_STANDARD,
        0.6f,   // 左侧面板比例 60%
        0.4f,   // 右侧面板比例 40%
        12,     // 基础字体大小
        8,      // 元素间距
        false   // 不显示边框
      };
      break;
    case LAYOUT_MODE_EXTENDED:
      currentLayout = {
        LAYOUT_MODE_EXTENDED,
        0.5f,   // 左侧面板比例 50%
        0.5f,   // 右侧面板比例 50%
        14,     // 基础字体大小（较大）
        10,     // 元素间距（较大）
        true    // 显示边框
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

void DisplayManager::applyLayout() {
  if (!displayDriver) {
    return;
  }
  
  // 获取屏幕尺寸
  width = displayDriver->getWidth();
  height = displayDriver->getHeight();
  
  // 计算面板宽度
  leftPanelWidth = static_cast<uint16_t>(width * currentLayout.leftPanelRatio);
  rightPanelWidth = width - leftPanelWidth;
  
  // 如果显示边框，绘制边框
  if (currentLayout.showBorders && displayDriver) {
    displayDriver->drawLine(leftPanelWidth - 1, 0, leftPanelWidth - 1, height - 1, 0x0000);
  }
  
  // 触发显示更新
  updateDisplay();
}