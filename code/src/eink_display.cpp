#include "eink_display.h"
#include "time_manager.h"
#include "weather_manager.h"
#include "sensor_manager.h"
#include "stock_manager.h"
#include "message_manager.h"

// 外部全局对象
extern TimeManager timeManager;
extern WeatherManager weatherManager;
extern SensorManager sensorManager;
extern StockManager stockManager;
extern MessageManager messageManager;
extern PowerManager powerManager;

EinkDisplay::EinkDisplay() {
  #if DISPLAY_TYPE == EINK_42_INCH
    io = GxIO_Class(SPI, EINK_CS, EINK_DC, EINK_RST);
    display = GxGDEW042Z15_Class(io, EINK_BUSY);
    width = GxGDEW042Z15_WIDTH;
    height = GxGDEW042Z15_HEIGHT;
  #elif DISPLAY_TYPE == EINK_75_INCH
    io = GxIO_Class(SPI, EINK_CS, EINK_DC, EINK_RST);
    display = GxGDEW075Z09_Class(io, EINK_BUSY);
    width = GxGDEW075Z09_WIDTH;
    height = GxGDEW075Z09_HEIGHT;
  #endif
  
  // 初始化分屏布局参数
  // 左侧面板宽度约为总宽度的1/3
  leftPanelWidth = width / 3;
  rightPanelWidth = width - leftPanelWidth;
  
  // 初始化当前页面和时钟模式
  currentRightPage = RIGHT_PAGE_CALENDAR;
  currentClockMode = CLOCK_MODE_DIGITAL;
}

EinkDisplay::~EinkDisplay() {
  // 清理资源
}

void EinkDisplay::init() {
  DEBUG_PRINTLN("初始化墨水屏...");
  
  // 初始化墨水屏
  display.init();
  
  // 初始化字体
  fonts.init(display);
  
  // 设置默认字体
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setTextColor(GxEPD_BLACK);
    display.setTextSize(1);
  #endif
  
  // 清空屏幕
  clearScreen();
  displayFullRefresh();
  
  DEBUG_PRINTLN("墨水屏初始化完成");
}

void EinkDisplay::updateDisplay() {
  DEBUG_PRINTLN("更新显示...");
  
  // 绘制左侧面板
  drawLeftPanel();
  
  // 绘制右侧面板
  drawRightPanel();
  
  // 刷新整个屏幕
  displayFullRefresh();
  
  DEBUG_PRINTLN("显示更新完成");
}

void EinkDisplay::updateLeftPanel() {
  DEBUG_PRINTLN("更新左侧面板...");
  
  // 绘制左侧面板
  drawLeftPanel();
  
  // 刷新左侧面板区域
  displayPartialRefresh(0, 0, leftPanelWidth, height);
  
  DEBUG_PRINTLN("左侧面板更新完成");
}

void EinkDisplay::updateRightPanel() {
  DEBUG_PRINTLN("更新右侧面板...");
  
  // 绘制右侧面板
  drawRightPanel();
  
  // 刷新右侧面板区域
  displayPartialRefresh(leftPanelWidth, 0, rightPanelWidth, height);
  
  DEBUG_PRINTLN("右侧面板更新完成");
}

void EinkDisplay::updateClockArea() {
  DEBUG_PRINTLN("更新时钟区域...");
  
  // 绘制左侧面板的时钟部分
  // 首先获取时间信息
  String timeStr = timeManager.getTimeString();
  String dateStr = timeManager.getDateString();
  
  // 清空时钟区域
  #if DISPLAY_TYPE == EINK_42_INCH
    display.fillRect(10, 10, leftPanelWidth - 20, 120, GxEPD_WHITE);
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.fillRect(20, 20, leftPanelWidth - 40, 180, GxEPD_WHITE);
  #endif
  
  // 根据当前时钟模式绘制时钟
  if (currentClockMode == CLOCK_MODE_DIGITAL) {
    // 绘制数字时钟
    #if DISPLAY_TYPE == EINK_42_INCH
      display.setCursor(20, 60);
      display.setTextSize(4);
      display.print(timeStr);
      
      display.setCursor(20, 100);
      display.setTextSize(1);
      display.print(dateStr);
    #elif DISPLAY_TYPE == EINK_75_INCH
      display.setCursor(40, 100);
      display.setTextSize(7);
      display.print(timeStr);
      
      display.setCursor(40, 180);
      display.setTextSize(2);
      display.print(dateStr);
    #endif
  } else {
    // 绘制模拟时钟
    // 获取当前时间的时、分、秒
    int hour = timeStr.substring(0, 2).toInt();
    int minute = timeStr.substring(3, 5).toInt();
    int second = timeStr.substring(6, 8).toInt();
    
    drawAnalogClock(leftPanelWidth / 2, 120, hour, minute, second);
  }
  
  // 刷新时钟区域
  #if DISPLAY_TYPE == EINK_42_INCH
    displayPartialRefresh(10, 10, leftPanelWidth - 20, 120);
  #elif DISPLAY_TYPE == EINK_75_INCH
    displayPartialRefresh(20, 20, leftPanelWidth - 40, 180);
  #endif
  
  DEBUG_PRINTLN("时钟区域更新完成");
}

void EinkDisplay::switchRightPage(RightPageType page) {
  DEBUG_PRINT("切换右侧页面到: ");
  DEBUG_PRINTLN(page);
  
  currentRightPage = page;
  
  // 更新右侧面板
  updateRightPanel();
}

void EinkDisplay::toggleClockMode() {
  DEBUG_PRINTLN("切换时钟模式...");
  
  // 切换时钟模式
  currentClockMode = (currentClockMode == CLOCK_MODE_DIGITAL) ? CLOCK_MODE_ANALOG : CLOCK_MODE_DIGITAL;
  
  // 更新时钟区域
  updateClockArea();
}

void EinkDisplay::showSplashScreen() {
  DEBUG_PRINTLN("显示启动画面...");
  
  clearScreen();
  
  #if DISPLAY_TYPE == EINK_42_INCH
    // 4.2寸墨水屏启动画面
    display.setCursor(width/2 - 60, height/2 - 20);
    display.setTextSize(2);
    display.print("智能墨水屏");
    display.setCursor(width/2 - 70, height/2 + 10);
    display.setTextSize(2);
    display.print("万年历 v1.0");
  #elif DISPLAY_TYPE == EINK_75_INCH
    // 7.5寸墨水屏启动画面
    display.setCursor(width/2 - 120, height/2 - 40);
    display.setTextSize(4);
    display.print("智能墨水屏");
    display.setCursor(width/2 - 150, height/2 + 20);
    display.setTextSize(3);
    display.print("万年历 v1.0");
  #endif
  
  displayFullRefresh();
  delay(SPLASH_SCREEN_DURATION);
  
  DEBUG_PRINTLN("启动画面显示完成");
}

void EinkDisplay::drawLeftPanel() {
  DEBUG_PRINTLN("绘制左侧面板...");
  
  // 绘制左侧面板背景
  display.fillRect(0, 0, leftPanelWidth, height, GxEPD_WHITE);
  
  // 绘制分割线
  display.fillRect(leftPanelWidth - 1, 0, 1, height, GxEPD_BLACK);
  
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
  #if DISPLAY_TYPE == EINK_42_INCH
    drawWeather(20, 140, weather.city, String(weather.temp) + "°C", 
                weather.condition, "", "");
  #elif DISPLAY_TYPE == EINK_75_INCH
    drawWeather(30, 220, weather.city, String(weather.temp) + "°C", 
                weather.condition, "", "");
  #endif
  
  // 绘制室内温湿度
  #if DISPLAY_TYPE == EINK_42_INCH
    drawSensorData(20, 220, sensor.temperature, sensor.humidity);
  #elif DISPLAY_TYPE == EINK_75_INCH
    drawSensorData(30, 340, sensor.temperature, sensor.humidity);
  #endif
  
  // 绘制电池信息
  #if DISPLAY_TYPE == EINK_42_INCH
    drawBatteryInfo(20, 280, batteryVoltage, batteryPercentage, isCharging);
  #elif DISPLAY_TYPE == EINK_75_INCH
    drawBatteryInfo(30, 440, batteryVoltage, batteryPercentage, isCharging);
  #endif
  
  // 绘制消息通知
  #if DISPLAY_TYPE == EINK_42_INCH
    drawMessageNotification(20, 320, messageCount);
  #elif DISPLAY_TYPE == EINK_75_INCH
    drawMessageNotification(30, 500, messageCount);
  #endif
  
  DEBUG_PRINTLN("左侧面板绘制完成");
}

void EinkDisplay::drawRightPanel() {
  DEBUG_PRINTLN("绘制右侧面板...");
  
  // 绘制右侧面板背景
  display.fillRect(leftPanelWidth, 0, rightPanelWidth, height, GxEPD_WHITE);
  
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
  
  DEBUG_PRINTLN("右侧面板绘制完成");
}

void EinkDisplay::drawDigitalClock(int x, int y, String time, String date) {
  #if DISPLAY_TYPE == EINK_42_INCH
    // 绘制时间
    display.setCursor(x, y);
    display.setTextSize(4);
    display.print(time);
    
    // 绘制日期
    display.setCursor(x, y + 40);
    display.setTextSize(1);
    display.print(date);
  #elif DISPLAY_TYPE == EINK_75_INCH
    // 绘制时间
    display.setCursor(x, y);
    display.setTextSize(7);
    display.print(time);
    
    // 绘制日期
    display.setCursor(x, y + 80);
    display.setTextSize(2);
    display.print(date);
  #endif
}

void EinkDisplay::drawAnalogClock(int x, int y, int hour, int minute, int second) {
  DEBUG_PRINTLN("绘制模拟时钟...");
  
  // 时钟半径
  #if DISPLAY_TYPE == EINK_42_INCH
    int radius = 40;
  #elif DISPLAY_TYPE == EINK_75_INCH
    int radius = 60;
  #endif
  
  // 绘制时钟外圆
  display.drawCircle(x, y, radius, GxEPD_BLACK);
  
  // 绘制时钟刻度
  for (int i = 0; i < 12; i++) {
    float angle = i * PI / 6 - PI / 2;
    int x1 = x + cos(angle) * (radius - 5);
    int y1 = y + sin(angle) * (radius - 5);
    int x2 = x + cos(angle) * radius;
    int y2 = y + sin(angle) * radius;
    display.drawLine(x1, y1, x2, y2, GxEPD_BLACK);
  }
  
  // 绘制时针
  float hourAngle = (hour % 12 + minute / 60.0) * PI / 6 - PI / 2;
  int hourX = x + cos(hourAngle) * (radius - 20);
  int hourY = y + sin(hourAngle) * (radius - 20);
  display.drawLine(x, y, hourX, hourY, GxEPD_BLACK);
  display.drawCircle(hourX, hourY, 2, GxEPD_BLACK);
  
  // 绘制分针
  float minuteAngle = (minute + second / 60.0) * PI / 30 - PI / 2;
  int minuteX = x + cos(minuteAngle) * (radius - 10);
  int minuteY = y + sin(minuteAngle) * (radius - 10);
  display.drawLine(x, y, minuteX, minuteY, GxEPD_BLACK);
  display.drawCircle(minuteX, minuteY, 2, GxEPD_BLACK);
  
  // 绘制秒针
  float secondAngle = second * PI / 30 - PI / 2;
  int secondX = x + cos(secondAngle) * (radius - 5);
  int secondY = y + sin(secondAngle) * (radius - 5);
  display.drawLine(x, y, secondX, secondY, GxEPD_RED);
  display.drawCircle(secondX, secondY, 2, GxEPD_RED);
  
  // 绘制中心点
  display.fillCircle(x, y, 3, GxEPD_BLACK);
  
  DEBUG_PRINTLN("模拟时钟绘制完成");
}

void EinkDisplay::drawBatteryInfo(int x, int y, float voltage, int percentage, bool isCharging) {
  DEBUG_PRINTLN("绘制电池信息...");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(1);
    display.print("电量: ");
    display.setTextSize(2);
    display.print(String(percentage) + "%");
    
    display.setCursor(x, y + 25);
    display.setTextSize(1);
    display.print(isCharging ? "充电中" : String(voltage, 1) + "V");
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("电量: ");
    display.setTextSize(3);
    display.print(String(percentage) + "%");
    
    display.setCursor(x, y + 40);
    display.setTextSize(2);
    display.print(isCharging ? "充电中" : String(voltage, 1) + "V");
  #endif
  
  // 绘制电池图标
  #if DISPLAY_TYPE == EINK_42_INCH
    int batteryX = x + 60;
    int batteryY = y;
    int batteryWidth = 20;
    int batteryHeight = 10;
  #elif DISPLAY_TYPE == EINK_75_INCH
    int batteryX = x + 120;
    int batteryY = y;
    int batteryWidth = 30;
    int batteryHeight = 15;
  #endif
  
  // 绘制电池外壳
  display.drawRect(batteryX, batteryY, batteryWidth, batteryHeight, GxEPD_BLACK);
  display.drawRect(batteryX + batteryWidth, batteryY + 2, 3, batteryHeight - 4, GxEPD_BLACK);
  
  // 绘制电池电量
  int batteryLevelWidth = (batteryWidth - 4) * percentage / 100;
  display.fillRect(batteryX + 2, batteryY + 2, batteryLevelWidth, batteryHeight - 4, GxEPD_BLACK);
  
  DEBUG_PRINTLN("电池信息绘制完成");
}

void EinkDisplay::drawMessageNotification(int x, int y, int messageCount) {
  DEBUG_PRINTLN("绘制消息通知...");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(1);
    display.print("消息: ");
    
    if (messageCount > 0) {
      display.setTextColor(GxEPD_RED);
      display.setTextSize(2);
      display.print("" + String(messageCount) + "条新消息");
    } else {
      display.setTextColor(GxEPD_BLACK);
      display.setTextSize(1);
      display.print("无新消息");
    }
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("消息: ");
    
    if (messageCount > 0) {
      display.setTextColor(GxEPD_RED);
      display.setTextSize(3);
      display.print("" + String(messageCount) + "条新消息");
    } else {
      display.setTextColor(GxEPD_BLACK);
      display.setTextSize(2);
      display.print("无新消息");
    }
  #endif
  
  // 恢复默认颜色
  display.setTextColor(GxEPD_BLACK);
  
  DEBUG_PRINTLN("消息通知绘制完成");
}

void EinkDisplay::updateMainPage() {
  DEBUG_PRINTLN("更新主页面...");
  
  clearScreen();
  
  // 绘制标题
  drawHeader("主页面");
  
  // 获取时间和日期
  String timeStr = timeManager.getTimeString();
  String dateStr = timeManager.getDateString();
  
  // 绘制时钟
  drawClock(20, 60, timeStr, dateStr);
  
  // 绘制天气信息
  WeatherData weather = weatherManager.getWeatherData();
  drawWeather(width - 180, 60, weather.city, String(weather.temp) + "°C", 
              weather.condition, "湿度: " + String(weather.humidity) + "%", 
              "风力: " + weather.wind);
  
  // 绘制温湿度传感器数据
  SensorData sensor = sensorManager.getSensorData();
  drawSensorData(20, height - 120, sensor.temperature, sensor.humidity);
  
  // 绘制最新消息提示
  if (messageManager.hasNewMessage()) {
    #if DISPLAY_TYPE == EINK_42_INCH
      display.setCursor(20, height - 60);
      display.setTextSize(1);
      display.setTextColor(GxEPD_RED);
      display.print("📩 有新消息");
    #elif DISPLAY_TYPE == EINK_75_INCH
      display.setCursor(20, height - 80);
      display.setTextSize(2);
      display.setTextColor(GxEPD_RED);
      display.print("📩 有新消息");
    #endif
  }
  
  // 绘制股票信息
  StockData stock = stockManager.getStockData(0);
  if (stock.valid) {
    drawStockData(width - 220, height - 120, stock.code, stock.name, 
                  stock.price, stock.change, stock.changePercent);
  }
  
  // 绘制页脚
  drawFooter();
  
  // 刷新显示
  displayFullRefresh();
  
  DEBUG_PRINTLN("主页面更新完成");
}

void EinkDisplay::updateWeatherPage() {
  DEBUG_PRINTLN("更新天气页面...");
  
  clearScreen();
  
  // 绘制标题
  drawHeader("天气预报");
  
  // 获取天气数据
  WeatherData weather = weatherManager.getWeatherData();
  
  // 绘制当前天气
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(20, 60);
    display.setTextSize(2);
    display.setTextColor(GxEPD_BLACK);
    display.print(weather.city);
    display.setCursor(20, 90);
    display.setTextSize(3);
    display.print(String(weather.temp) + "°C");
    display.setCursor(20, 130);
    display.setTextSize(1);
    display.print(weather.condition);
    display.setCursor(20, 150);
    display.print("湿度: " + String(weather.humidity) + "%");
    display.setCursor(20, 170);
    display.print("风力: " + weather.wind);
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(40, 80);
    display.setTextSize(3);
    display.setTextColor(GxEPD_BLACK);
    display.print(weather.city);
    display.setCursor(40, 130);
    display.setTextSize(5);
    display.print(String(weather.temp) + "°C");
    display.setCursor(40, 200);
    display.setTextSize(2);
    display.print(weather.condition);
    display.setCursor(40, 240);
    display.print("湿度: " + String(weather.humidity) + "%");
    display.setCursor(40, 280);
    display.print("风力: " + weather.wind);
  #endif
  
  // 绘制未来天气预报
  // TODO: 实现未来天气预报绘制
  
  // 绘制页脚
  drawFooter();
  
  // 刷新显示
  displayFullRefresh();
  
  DEBUG_PRINTLN("天气页面更新完成");
}

void EinkDisplay::updateSensorPage() {
  DEBUG_PRINTLN("更新传感器页面...");
  
  clearScreen();
  
  // 绘制标题
  drawHeader("温湿度监测");
  
  // 获取传感器数据
  SensorData sensor = sensorManager.getSensorData();
  
  // 绘制当前温湿度
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(20, 60);
    display.setTextSize(2);
    display.setTextColor(GxEPD_BLACK);
    display.print("当前温度: ");
    display.setTextSize(3);
    display.print(String(sensor.temperature) + "°C");
    
    display.setCursor(20, 120);
    display.setTextSize(2);
    display.print("当前湿度: ");
    display.setTextSize(3);
    display.print(String(sensor.humidity) + "%");
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(40, 80);
    display.setTextSize(3);
    display.setTextColor(GxEPD_BLACK);
    display.print("当前温度: ");
    display.setTextSize(5);
    display.print(String(sensor.temperature) + "°C");
    
    display.setCursor(40, 180);
    display.setTextSize(3);
    display.print("当前湿度: ");
    display.setTextSize(5);
    display.print(String(sensor.humidity) + "%");
  #endif
  
  // 绘制历史数据趋势
  // TODO: 实现历史数据趋势图绘制
  
  // 绘制页脚
  drawFooter();
  
  // 刷新显示
  displayFullRefresh();
  
  DEBUG_PRINTLN("传感器页面更新完成");
}

void EinkDisplay::updateStockPage() {
  DEBUG_PRINTLN("更新股票页面...");
  
  clearScreen();
  
  // 绘制标题
  drawHeader("股票行情");
  
  // 绘制股票数据
  for (int i = 0; i < MAX_STOCKS; i++) {
    StockData stock = stockManager.getStockData(i);
    if (stock.valid) {
      #if DISPLAY_TYPE == EINK_42_INCH
        drawStockData(20, 60 + i * 80, stock.code, stock.name, 
                      stock.price, stock.change, stock.changePercent);
      #elif DISPLAY_TYPE == EINK_75_INCH
        drawStockData(40, 80 + i * 120, stock.code, stock.name, 
                      stock.price, stock.change, stock.changePercent);
      #endif
    }
  }
  
  // 绘制页脚
  drawFooter();
  
  // 刷新显示
  displayFullRefresh();
  
  DEBUG_PRINTLN("股票页面更新完成");
}

void EinkDisplay::updateMessagePage() {
  DEBUG_PRINTLN("更新消息页面...");
  
  clearScreen();
  
  // 绘制标题
  drawHeader("消息中心");
  
  // 获取最新消息
  MessageData message = messageManager.getLatestMessage();
  
  if (message.valid) {
    #if DISPLAY_TYPE == EINK_42_INCH
      display.setCursor(20, 60);
      display.setTextSize(1);
      display.setTextColor(GxEPD_BLACK);
      display.print("发件人: " + message.sender);
      
      display.setCursor(20, 80);
      display.print("时间: " + message.time);
      
      display.setCursor(20, 100);
      display.setTextSize(2);
      display.print("内容: ");
      
      display.setCursor(20, 130);
      display.setTextSize(1);
      display.print(message.content);
    #elif DISPLAY_TYPE == EINK_75_INCH
      display.setCursor(40, 80);
      display.setTextSize(2);
      display.setTextColor(GxEPD_BLACK);
      display.print("发件人: " + message.sender);
      
      display.setCursor(40, 120);
      display.print("时间: " + message.time);
      
      display.setCursor(40, 160);
      display.setTextSize(3);
      display.print("内容: ");
      
      display.setCursor(40, 220);
      display.setTextSize(2);
      display.print(message.content);
    #endif
    
    // 标记消息为已读
    messageManager.markMessageAsRead(message.id);
  } else {
    #if DISPLAY_TYPE == EINK_42_INCH
      display.setCursor(20, 100);
      display.setTextSize(2);
      display.print("暂无消息");
    #elif DISPLAY_TYPE == EINK_75_INCH
      display.setCursor(40, 180);
      display.setTextSize(3);
      display.print("暂无消息");
    #endif
  }
  
  // 绘制页脚
  drawFooter();
  
  // 刷新显示
  displayFullRefresh();
  
  DEBUG_PRINTLN("消息页面更新完成");
}

void EinkDisplay::updateSettingPage() {
  DEBUG_PRINTLN("更新设置页面...");
  
  clearScreen();
  
  // 绘制标题
  drawHeader("设置");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(20, 60);
    display.setTextSize(1);
    display.print("1. WiFi设置");
    
    display.setCursor(20, 80);
    display.print("2. 时间设置");
    
    display.setCursor(20, 100);
    display.print("3. 天气设置");
    
    display.setCursor(20, 120);
    display.print("4. 股票设置");
    
    display.setCursor(20, 140);
    display.print("5. 音量设置");
    
    display.setCursor(20, 160);
    display.print("6. 关于");
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(40, 80);
    display.setTextSize(2);
    display.print("1. WiFi设置");
    
    display.setCursor(40, 120);
    display.print("2. 时间设置");
    
    display.setCursor(40, 160);
    display.print("3. 天气设置");
    
    display.setCursor(40, 200);
    display.print("4. 股票设置");
    
    display.setCursor(40, 240);
    display.print("5. 音量设置");
    
    display.setCursor(40, 280);
    display.print("6. 关于");
  #endif
  
  // 绘制页脚
  drawFooter();
  
  // 刷新显示
  displayFullRefresh();
  
  DEBUG_PRINTLN("设置页面更新完成");
}

void EinkDisplay::showMessage(String message, uint32_t duration) {
  DEBUG_PRINT("显示消息: ");
  DEBUG_PRINTLN(message);
  
  // 保存当前显示内容
  // TODO: 实现显示内容保存和恢复
  
  // 显示消息
  clearScreen();
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(20, height/2 - 20);
    display.setTextSize(2);
    display.setTextColor(GxEPD_BLACK);
    display.print(message);
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(40, height/2 - 40);
    display.setTextSize(3);
    display.setTextColor(GxEPD_BLACK);
    display.print(message);
  #endif
  
  displayFullRefresh();
  
  // 延时显示
  delay(duration);
  
  // 恢复之前的显示内容
  // TODO: 实现显示内容恢复
  
  // 更新当前页面
  switch (currentPage) {
    case PAGE_MAIN:
      updateMainPage();
      break;
    case PAGE_WEATHER:
      updateWeatherPage();
      break;
    case PAGE_SENSOR:
      updateSensorPage();
      break;
    case PAGE_STOCK:
      updateStockPage();
      break;
    case PAGE_MESSAGE:
      updateMessagePage();
      break;
    case PAGE_SETTING:
      updateSettingPage();
      break;
  }
  
  DEBUG_PRINTLN("消息显示完成");
}

void EinkDisplay::switchPage(PageType page) {
  DEBUG_PRINT("切换页面到: ");
  DEBUG_PRINTLN(page);
  
  currentPage = page;
  
  // 更新对应页面
  switch (page) {
    case PAGE_MAIN:
      updateMainPage();
      break;
    case PAGE_WEATHER:
      updateWeatherPage();
      break;
    case PAGE_SENSOR:
      updateSensorPage();
      break;
    case PAGE_STOCK:
      updateStockPage();
      break;
    case PAGE_MESSAGE:
      updateMessagePage();
      break;
    case PAGE_SETTING:
      updateSettingPage();
      break;
  }
}

// 私有方法
void EinkDisplay::drawHeader(String title) {
  // 绘制标题栏背景
  display.fillRect(0, 0, width, 40, GxEPD_BLACK);
  
  // 绘制标题
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(20, 25);
    display.setTextColor(GxEPD_WHITE);
    display.setTextSize(2);
    display.print(title);
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(40, 35);
    display.setTextColor(GxEPD_WHITE);
    display.setTextSize(3);
    display.print(title);
  #endif
}

void EinkDisplay::drawFooter() {
  // 绘制页脚
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(20, height - 15);
    display.setTextColor(GxEPD_GRAY2);
    display.setTextSize(1);
    display.print("家用网络智能墨水屏万年历 v1.0");
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(40, height - 30);
    display.setTextColor(GxEPD_GRAY2);
    display.setTextSize(1);
    display.print("家用网络智能墨水屏万年历 v1.0");
  #endif
}

void EinkDisplay::clearScreen() {
  display.fillScreen(GxEPD_WHITE);
  display.setTextColor(GxEPD_BLACK);
}

void EinkDisplay::displayFullRefresh() {
  display.update();
}

void EinkDisplay::displayPartialRefresh(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  // 部分刷新，根据墨水屏型号实现
  // 注意：不同型号的墨水屏部分刷新实现可能不同
  // 这里使用通用的update方法，实际使用时需要根据墨水屏型号调整
  display.update();
}

void EinkDisplay::drawCalendarPage(int x, int y) {
  DEBUG_PRINTLN("绘制日历页面...");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("万年历");
    
    // 绘制当月日历
    // TODO: 实现完整的日历绘制
    display.setCursor(x, y + 40);
    display.setTextSize(1);
    display.print("2025年12月");
    
    // 绘制星期
    display.setCursor(x, y + 60);
    display.print("日 一 二 三 四 五 六");
    
    // 绘制日期
    display.setCursor(x, y + 80);
    display.print("          1  2  3  4");
    display.setCursor(x, y + 100);
    display.print(" 5  6  7  8  9 10 11");
    display.setCursor(x, y + 120);
    display.print("12 13 14 15 16 17 18");
    display.setCursor(x, y + 140);
    display.print("19 20 21 22 23 24 25");
    display.setCursor(x, y + 160);
    display.print("26 27 28 29 30 31");
    
    // 绘制黄历信息
    display.setCursor(x, y + 200);
    display.setTextColor(GxEPD_RED);
    display.print("今日宜: 出行、祭祀");
    
    display.setCursor(x, y + 220);
    display.print("今日忌: 开市、动土");
    
    // 恢复默认颜色
    display.setTextColor(GxEPD_BLACK);
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(3);
    display.print("万年历");
    
    // 绘制当月日历
    // TODO: 实现完整的日历绘制
    display.setCursor(x, y + 60);
    display.setTextSize(2);
    display.print("2025年12月");
    
    // 绘制星期
    display.setCursor(x, y + 100);
    display.print("日     一     二     三     四     五     六");
    
    // 绘制日期
    display.setCursor(x, y + 140);
    display.print("                    1     2     3     4");
    display.setCursor(x, y + 180);
    display.print(" 5     6     7     8     9    10    11");
    display.setCursor(x, y + 220);
    display.print("12    13    14    15    16    17    18");
    display.setCursor(x, y + 260);
    display.print("19    20    21    22    23    24    25");
    display.setCursor(x, y + 300);
    display.print("26    27    28    29    30    31");
    
    // 绘制黄历信息
    display.setCursor(x, y + 360);
    display.setTextColor(GxEPD_RED);
    display.setTextSize(3);
    display.print("今日宜: 出行、祭祀、祈福");
    
    display.setCursor(x, y + 400);
    display.print("今日忌: 开市、动土、安葬");
    
    // 恢复默认颜色
    display.setTextColor(GxEPD_BLACK);
  #endif
  
  DEBUG_PRINTLN("日历页面绘制完成");
}

void EinkDisplay::drawStockPage(int x, int y) {
  DEBUG_PRINTLN("绘制股票页面...");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("股票行情");
    
    // 绘制股票列表
    for (int i = 0; i < MAX_STOCKS; i++) {
      StockData stock = stockManager.getStockData(i);
      if (stock.valid) {
        drawStockData(x, y + 40 + i * 80, stock.code, stock.name, 
                      stock.price, stock.change, stock.changePercent);
      }
    }
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(3);
    display.print("股票行情");
    
    // 绘制股票列表
    for (int i = 0; i < MAX_STOCKS; i++) {
      StockData stock = stockManager.getStockData(i);
      if (stock.valid) {
        drawStockData(x, y + 60 + i * 120, stock.code, stock.name, 
                      stock.price, stock.change, stock.changePercent);
      }
    }
  #endif
  
  DEBUG_PRINTLN("股票页面绘制完成");
}

void EinkDisplay::drawMessagePage(int x, int y) {
  DEBUG_PRINTLN("绘制消息页面...");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("消息中心");
    
    // 获取最新消息
    MessageData message = messageManager.getLatestMessage();
    
    if (message.valid) {
      display.setCursor(x, y + 40);
      display.setTextSize(1);
      display.print("发件人: " + message.sender);
      
      display.setCursor(x, y + 60);
      display.print("时间: " + message.time);
      
      display.setCursor(x, y + 80);
      display.setTextSize(2);
      display.print("内容: ");
      
      display.setCursor(x, y + 110);
      display.setTextSize(1);
      display.print(message.content);
      
      // 标记消息为已读
      messageManager.markMessageAsRead(message.id);
    } else {
      display.setCursor(x, y + 80);
      display.setTextSize(2);
      display.print("暂无消息");
    }
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(3);
    display.print("消息中心");
    
    // 获取最新消息
    MessageData message = messageManager.getLatestMessage();
    
    if (message.valid) {
      display.setCursor(x, y + 60);
      display.setTextSize(2);
      display.print("发件人: " + message.sender);
      
      display.setCursor(x, y + 100);
      display.print("时间: " + message.time);
      
      display.setCursor(x, y + 140);
      display.setTextSize(3);
      display.print("内容: ");
      
      display.setCursor(x, y + 190);
      display.setTextSize(2);
      display.print(message.content);
      
      // 标记消息为已读
      messageManager.markMessageAsRead(message.id);
    } else {
      display.setCursor(x, y + 140);
      display.setTextSize(3);
      display.print("暂无消息");
    }
  #endif
  
  DEBUG_PRINTLN("消息页面绘制完成");
}

void EinkDisplay::drawPluginPage(int x, int y) {
  DEBUG_PRINTLN("绘制插件页面...");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("插件功能");
    
    display.setCursor(x, y + 40);
    display.setTextSize(1);
    display.print("插件1: 待开发");
    
    display.setCursor(x, y + 60);
    display.print("插件2: 待开发");
    
    display.setCursor(x, y + 80);
    display.print("插件3: 待开发");
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(3);
    display.print("插件功能");
    
    display.setCursor(x, y + 60);
    display.setTextSize(2);
    display.print("1. 插件1: 待开发");
    
    display.setCursor(x, y + 100);
    display.print("2. 插件2: 待开发");
    
    display.setCursor(x, y + 140);
    display.print("3. 插件3: 待开发");
    
    display.setCursor(x, y + 180);
    display.print("4. 插件4: 待开发");
    
    display.setCursor(x, y + 220);
    display.print("5. 插件5: 待开发");
  #endif
  
  DEBUG_PRINTLN("插件页面绘制完成");
}

void EinkDisplay::drawPluginManagePage(int x, int y) {
  DEBUG_PRINTLN("绘制插件管理页面...");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("插件管理");
    
    display.setCursor(x, y + 40);
    display.setTextSize(1);
    display.print("1. 启用插件");
    
    display.setCursor(x, y + 60);
    display.print("2. 禁用插件");
    
    display.setCursor(x, y + 80);
    display.print("3. 更新插件");
    
    display.setCursor(x, y + 100);
    display.print("4. 删除插件");
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(3);
    display.print("插件管理");
    
    display.setCursor(x, y + 60);
    display.setTextSize(2);
    display.print("1. 启用插件");
    
    display.setCursor(x, y + 100);
    display.print("2. 禁用插件");
    
    display.setCursor(x, y + 140);
    display.print("3. 更新插件");
    
    display.setCursor(x, y + 180);
    display.print("4. 删除插件");
    
    display.setCursor(x, y + 220);
    display.print("5. 安装新插件");
  #endif
  
  DEBUG_PRINTLN("插件管理页面绘制完成");
}

void EinkDisplay::drawSettingPage(int x, int y) {
  DEBUG_PRINTLN("绘制设置页面...");
  
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("设置");
    
    display.setCursor(x, y + 40);
    display.setTextSize(1);
    display.print("1. WiFi设置");
    
    display.setCursor(x, y + 60);
    display.print("2. 时间设置");
    
    display.setCursor(x, y + 80);
    display.print("3. 天气设置");
    
    display.setCursor(x, y + 100);
    display.print("4. 股票设置");
    
    display.setCursor(x, y + 120);
    display.print("5. 音量设置");
    
    display.setCursor(x, y + 140);
    display.print("6. 关于");
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(3);
    display.print("设置");
    
    display.setCursor(x, y + 60);
    display.setTextSize(2);
    display.print("1. WiFi设置");
    
    display.setCursor(x, y + 100);
    display.print("2. 时间设置");
    
    display.setCursor(x, y + 140);
    display.print("3. 天气设置");
    
    display.setCursor(x, y + 180);
    display.print("4. 股票设置");
    
    display.setCursor(x, y + 220);
    display.print("5. 音量设置");
    
    display.setCursor(x, y + 260);
    display.print("6. 显示设置");
    
    display.setCursor(x, y + 300);
    display.print("7. 关于");
  #endif
  
  DEBUG_PRINTLN("设置页面绘制完成");
}

void EinkDisplay::drawClock(int x, int y, String time, String date) {
  #if DISPLAY_TYPE == EINK_42_INCH
    // 绘制时间
    display.setCursor(x, y);
    display.setTextSize(4);
    display.print(time);
    
    // 绘制日期
    display.setCursor(x, y + 40);
    display.setTextSize(1);
    display.print(date);
  #elif DISPLAY_TYPE == EINK_75_INCH
    // 绘制时间
    display.setCursor(x, y);
    display.setTextSize(7);
    display.print(time);
    
    // 绘制日期
    display.setCursor(x, y + 80);
    display.setTextSize(2);
    display.print(date);
  #endif
}

void EinkDisplay::drawWeather(int x, int y, String city, String temp, String condition, String humidity, String wind) {
  #if DISPLAY_TYPE == EINK_42_INCH
    // 绘制城市
    display.setCursor(x, y);
    display.setTextSize(1);
    display.print(city);
    
    // 绘制温度
    display.setCursor(x, y + 20);
    display.setTextSize(3);
    display.print(temp);
    
    // 绘制天气状况
    display.setCursor(x, y + 50);
    display.setTextSize(1);
    display.print(condition);
    
    // 绘制湿度
    display.setCursor(x, y + 70);
    display.setTextSize(1);
    display.print(humidity);
    
    // 绘制风力
    display.setCursor(x, y + 90);
    display.setTextSize(1);
    display.print(wind);
  #elif DISPLAY_TYPE == EINK_75_INCH
    // 绘制城市
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print(city);
    
    // 绘制温度
    display.setCursor(x, y + 40);
    display.setTextSize(5);
    display.print(temp);
    
    // 绘制天气状况
    display.setCursor(x, y + 100);
    display.setTextSize(2);
    display.print(condition);
    
    // 绘制湿度
    display.setCursor(x, y + 140);
    display.setTextSize(2);
    display.print(humidity);
    
    // 绘制风力
    display.setCursor(x, y + 180);
    display.setTextSize(2);
    display.print(wind);
  #endif
}

void EinkDisplay::drawSensorData(int x, int y, float temperature, float humidity) {
  #if DISPLAY_TYPE == EINK_42_INCH
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print("室内温湿度");
    
    display.setCursor(x, y + 30);
    display.setTextSize(1);
    display.print("温度: " + String(temperature) + "°C");
    
    display.setCursor(x, y + 50);
    display.setTextSize(1);
    display.print("湿度: " + String(humidity) + "%");
  #elif DISPLAY_TYPE == EINK_75_INCH
    display.setCursor(x, y);
    display.setTextSize(3);
    display.print("室内温湿度");
    
    display.setCursor(x, y + 50);
    display.setTextSize(2);
    display.print("温度: " + String(temperature) + "°C");
    
    display.setCursor(x, y + 90);
    display.setTextSize(2);
    display.print("湿度: " + String(humidity) + "%");
  #endif
}

void EinkDisplay::drawStockData(int x, int y, String code, String name, float price, float change, float changePercent) {
  #if DISPLAY_TYPE == EINK_42_INCH
    // 绘制股票名称和代码
    display.setCursor(x, y);
    display.setTextSize(1);
    display.print(name + " (" + code + ")");
    
    // 绘制股票价格
    display.setCursor(x, y + 20);
    display.setTextSize(2);
    display.print(String(price, 2));
    
    // 绘制涨跌额和涨跌幅
    if (change >= 0) {
      display.setTextColor(GxEPD_RED);
    } else {
      display.setTextColor(GxEPD_GREEN);
    }
    
    display.setCursor(x, y + 40);
    display.setTextSize(1);
    display.print("" + String(change, 2) + " (" + String(changePercent, 2) + "%)");
    
    // 恢复默认颜色
    display.setTextColor(GxEPD_BLACK);
  #elif DISPLAY_TYPE == EINK_75_INCH
    // 绘制股票名称和代码
    display.setCursor(x, y);
    display.setTextSize(2);
    display.print(name + " (" + code + ")");
    
    // 绘制股票价格
    display.setCursor(x, y + 40);
    display.setTextSize(3);
    display.print(String(price, 2));
    
    // 绘制涨跌额和涨跌幅
    if (change >= 0) {
      display.setTextColor(GxEPD_RED);
    } else {
      display.setTextColor(GxEPD_GREEN);
    }
    
    display.setCursor(x, y + 80);
    display.setTextSize(2);
    display.print("" + String(change, 2) + " (" + String(changePercent, 2) + "%)");
    
    // 恢复默认颜色
    display.setTextColor(GxEPD_BLACK);
  #endif
}