#include "display_manager.h"
#include "time_manager.h"
#include "weather_manager.h"
#include "sensor_manager.h"
#include "stock_manager.h"
#include "message_manager.h"
#include "power_manager.h"

// 外部全局对象
extern TimeManager timeManager;
extern WeatherManager weatherManager;
extern SensorManager sensorManager;
extern StockManager stockManager;
extern MessageManager messageManager;
extern PowerManager powerManager;

DisplayManager::DisplayManager() {
  displayDriver = nullptr;
  currentRightPage = RIGHT_PAGE_CALENDAR;
  currentClockMode = CLOCK_MODE_DIGITAL;
  width = 0;
  height = 0;
  leftPanelWidth = 0;
  rightPanelWidth = 0;
  lastMessageCount = 0;
  lastBatteryPercentage = 100;
  
  // 初始化内容类型最后更新时间
  lastClockUpdateTime = 0;
  lastWeatherUpdateTime = 0;
  lastSensorUpdateTime = 0;
  lastStockUpdateTime = 0;
  lastMessageUpdateTime = 0;
  lastCalendarUpdateTime = 0;
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
  
  // 绘制左侧面板
  drawLeftPanel();
  
  // 绘制右侧面板
  drawRightPanel();
  
  // 刷新整个屏幕
  displayDriver->update();
}

void DisplayManager::updateDisplayPartial() {
  if (displayDriver == nullptr) {
    return;
  }
  
  unsigned long currentTime = millis();
  bool isLowPowerMode = powerManager.getLowPowerMode();
  
  // 根据低功耗模式调整刷新间隔倍率
  int refreshMultiplier = isLowPowerMode ? 6 : 1; // 低功耗模式下刷新间隔延长6倍
  
  // 1. 更新时钟区域
  if (currentTime - lastClockUpdateTime >= CLOCK_REFRESH_INTERVAL * refreshMultiplier) {
    // 更新时钟区域
    lastClockUpdateTime = currentTime;
  }
  
  // 2. 更新天气信息
  if (currentTime - lastWeatherUpdateTime >= WEATHER_REFRESH_INTERVAL * refreshMultiplier) {
    // 更新天气信息
    lastWeatherUpdateTime = currentTime;
  }
  
  // 3. 更新传感器数据
  if (currentTime - lastSensorUpdateTime >= SENSOR_REFRESH_INTERVAL * refreshMultiplier) {
    // 更新传感器数据
    lastSensorUpdateTime = currentTime;
  }
  
  // 4. 更新电池信息
  int batteryPercentage = powerManager.getBatteryPercentage();
  if (abs(batteryPercentage - lastBatteryPercentage) > 5) {
    // 更新电池信息
    lastBatteryPercentage = batteryPercentage;
  }
  
  // 5. 更新消息通知
  int messageCount = messageManager.getUnreadMessageCount();
  if (messageCount != lastMessageCount || currentTime - lastMessageUpdateTime >= MESSAGE_REFRESH_INTERVAL * refreshMultiplier) {
    // 更新消息通知
    lastMessageCount = messageCount;
    lastMessageUpdateTime = currentTime;
  }
  
  // 6. 更新右侧面板内容
  if (currentRightPage == RIGHT_PAGE_STOCK && currentTime - lastStockUpdateTime >= STOCK_REFRESH_INTERVAL * refreshMultiplier) {
    // 更新右侧面板
    lastStockUpdateTime = currentTime;
  } else if (currentRightPage == RIGHT_PAGE_CALENDAR && currentTime - lastCalendarUpdateTime >= CALENDAR_REFRESH_INTERVAL * refreshMultiplier) {
    // 更新右侧面板
    lastCalendarUpdateTime = currentTime;
  }
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
  currentClockMode = (currentClockMode == CLOCK_MODE_DIGITAL) ? CLOCK_MODE_ANALOG : CLOCK_MODE_DIGITAL;
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
  
  // 绘制左侧面板背景
  displayDriver->fillRect(0, 0, leftPanelWidth, height, GxEPD_WHITE);
  
  // 绘制分割线
  displayDriver->fillRect(leftPanelWidth - 1, 0, 1, height, GxEPD_BLACK);
  
  // 获取各种数据
  String timeStr = timeManager.getTimeString();
  String dateStr = timeManager.getDateString();
  WeatherData weather = weatherManager.getWeatherData();
  SensorData sensor = sensorManager.getSensorData();
  float batteryVoltage = powerManager.getBatteryVoltage();
  int batteryPercentage = powerManager.getBatteryPercentage();
  bool isCharging = powerManager.getChargingStatus();
  int messageCount = messageManager.getUnreadMessageCount();
  
  // 绘制时钟（根据当前时钟模式）
  if (currentClockMode == CLOCK_MODE_DIGITAL) {
    drawDigitalClock(20, 60, timeStr, dateStr);
  } else {
    // 获取当前时间的时、分、秒
    int hour = timeStr.substring(0, 2).toInt();
    int minute = timeStr.substring(3, 5).toInt();
    int second = timeStr.substring(6, 8).toInt();
    drawAnalogClock(leftPanelWidth / 2, 120, hour, minute, second);
  }
  
  // 绘制天气信息
  drawWeather(20, height < 400 ? 140 : 220, weather.city, String(weather.temp) + "°C", 
              weather.condition, "", "");
  
  // 绘制室内温湿度
  drawSensorData(20, height < 400 ? 220 : 340, sensor.temperature, sensor.humidity);
  
  // 绘制电池信息
  drawBatteryInfo(20, height < 400 ? 280 : 440, batteryVoltage, batteryPercentage, isCharging);
  
  // 绘制消息通知
  drawMessageNotification(20, height < 400 ? 320 : 500, messageCount);
}

void DisplayManager::drawRightPanel() {
  if (displayDriver == nullptr) {
    return;
  }
  
  // 绘制右侧面板背景
  displayDriver->fillRect(leftPanelWidth, 0, rightPanelWidth, height, GxEPD_WHITE);
  
  // 根据当前右侧页面绘制不同内容
  switch (currentRightPage) {
    case RIGHT_PAGE_CALENDAR:
      drawCalendarPage(leftPanelWidth + 20, 20);
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
  }
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

void DisplayManager::drawAnalogClock(int x, int y, int hour, int minute, int second) {
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
  
  // 绘制时针
  float hourAngle = (hour % 12 + minute / 60.0) * PI / 6 - PI / 2;
  int hourX = x + cos(hourAngle) * (radius - 20);
  int hourY = y + sin(hourAngle) * (radius - 20);
  displayDriver->drawLine(x, y, hourX, hourY, GxEPD_BLACK);
  
  // 绘制分针
  float minuteAngle = (minute + second / 60.0) * PI / 30 - PI / 2;
  int minuteX = x + cos(minuteAngle) * (radius - 10);
  int minuteY = y + sin(minuteAngle) * (radius - 10);
  displayDriver->drawLine(x, y, minuteX, minuteY, GxEPD_BLACK);
  
  // 绘制秒针
  float secondAngle = second * PI / 30 - PI / 2;
  int secondX = x + cos(secondAngle) * (radius - 5);
  int secondY = y + sin(secondAngle) * (radius - 5);
  displayDriver->drawLine(x, y, secondX, secondY, GxEPD_RED);
  
  // 绘制中心点
  displayDriver->drawRect(x - 2, y - 2, 4, 4, GxEPD_BLACK);
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
    displayDriver->drawString(x, y, String(messageCount) + "条新消息", GxEPD_RED, GxEPD_WHITE, textSize);
    
    // 绘制红色圆点提示
    displayDriver->fillRect(x + (height < 400 ? 18 : 27), y - (height < 400 ? 2 : 3), 
                           height < 400 ? 6 : 10, height < 400 ? 6 : 10, GxEPD_RED);
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
  
  // 绘制温度
  displayDriver->drawString(x, y + (height < 400 ? 20 : 40), temp, GxEPD_BLACK, GxEPD_WHITE, tempSize);
  
  // 绘制天气状况
  displayDriver->drawString(x, y + (height < 400 ? 50 : 100), condition, GxEPD_BLACK, GxEPD_WHITE, textSize);
}

void DisplayManager::drawSensorData(int x, int y, float temperature, float humidity) {
  if (displayDriver == nullptr) {
    return;
  }
  
  int titleSize = height < 400 ? 2 : 3;
  int dataSize = height < 400 ? 1 : 2;
  
  // 绘制标题
  displayDriver->drawString(x, y, "室内温湿度", GxEPD_BLACK, GxEPD_WHITE, titleSize);
  
  // 绘制温度
  displayDriver->drawString(x, y + (height < 400 ? 30 : 50), "温度: " + String(temperature) + "°C", 
                         GxEPD_BLACK, GxEPD_WHITE, dataSize);
  
  // 绘制湿度
  displayDriver->drawString(x, y + (height < 400 ? 50 : 90), "湿度: " + String(humidity) + "%", 
                         GxEPD_BLACK, GxEPD_WHITE, dataSize);
}

// 其他绘制方法的实现可以从eink_display.cpp迁移过来，这里省略...