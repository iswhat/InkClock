#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "../drivers/displays/display_driver.h"
#include "../core/config.h"

// 右侧页面类型枚举
enum RightPageType {
  RIGHT_PAGE_CALENDAR,
  RIGHT_PAGE_STOCK,
  RIGHT_PAGE_MESSAGE,
  RIGHT_PAGE_PLUGIN,
  RIGHT_PAGE_PLUGIN_MANAGE,
  RIGHT_PAGE_SETTING
};

// 时钟显示模式枚举
enum ClockMode {
  CLOCK_MODE_DIGITAL,
  CLOCK_MODE_ANALOG
};

// 显示管理器类，处理显示逻辑
class DisplayManager {
public:
  DisplayManager();
  ~DisplayManager();
  
  // 初始化显示管理器
  bool init();
  
  // 设置显示驱动
  void setDisplayDriver(IDisplayDriver* driver);
  
  // 显示启动画面
  void showSplashScreen();
  
  // 完整更新显示
  void updateDisplay();
  
  // 局部更新显示
  void updateDisplayPartial();
  
  // 显示消息
  void showMessage(String message, uint32_t duration = 3000);
  
  // 切换右侧页面
  void switchRightPage(RightPageType page);
  
  // 切换时钟模式
  void toggleClockMode();
  
  // 获取当前右侧页面
  RightPageType getCurrentRightPage() const;
  
  // 获取当前时钟模式
  ClockMode getCurrentClockMode() const;
  
  // 获取屏幕宽度
  int16_t getWidth() const;
  
  // 获取屏幕高度
  int16_t getHeight() const;
  
  // 秒针显示设置
  bool getShowSeconds() const { return showSeconds; }
  void setShowSeconds(bool show) { showSeconds = show; }
  
private:
  // 显示驱动
  IDisplayDriver* displayDriver;
  
  // 当前右侧页面
  RightPageType currentRightPage;
  
  // 当前时钟模式
  ClockMode currentClockMode;
  
  // 显示尺寸
  uint16_t width;
  uint16_t height;
  
  // 分屏布局参数
  uint16_t leftPanelWidth;
  uint16_t rightPanelWidth;
  
  // 局部刷新优化参数
  int lastMessageCount;
  int lastBatteryPercentage;
  float lastTemperature;
  float lastHumidity;
  int lastClockSecond;
  bool showSeconds; // 是否显示秒针
  
  // 内容类型最后更新时间
  unsigned long lastClockUpdateTime;
  unsigned long lastWeatherUpdateTime;
  unsigned long lastSensorUpdateTime;
  unsigned long lastStockUpdateTime;
  unsigned long lastMessageUpdateTime;
  unsigned long lastCalendarUpdateTime;
  unsigned long lastFullRefreshTime;
  
  // 私有方法
  void drawHeader(String title);
  void drawFooter();
  void clearScreen();
  
  // 绘制左侧面板
  void drawLeftPanel();
  
  // 绘制右侧面板
  void drawRightPanel();
  
  // 绘制时钟（数字模式）
  void drawDigitalClock(int x, int y, String time, String date);
  
  // 绘制时钟（模拟模式）
  void drawAnalogClock(int x, int y, int hour, int minute, int second);
  
  // 绘制天气
  void drawWeather(int x, int y, String city, String temp, String condition, String humidity, String wind);
  
  // 绘制传感器数据
  void drawSensorData(int x, int y, float temp, float humidity);
  
  // 绘制电池信息
  void drawBatteryInfo(int x, int y, float voltage, int percentage, bool isCharging);
  
  // 绘制消息通知
  void drawMessageNotification(int x, int y, int messageCount);
  
  // 绘制日历页面
  void drawCalendarPage(int x, int y);
  
  // 绘制股票页面
  void drawStockPage(int x, int y);
  
  // 绘制消息页面
  void drawMessagePage(int x, int y);
  
  // 绘制插件页面
  void drawPluginPage(int x, int y);
  
  // 绘制插件管理页面
  void drawPluginManagePage(int x, int y);
  
  // 绘制设置页面
  void drawSettingPage(int x, int y);
  
  // 绘制股票数据
  void drawStockData(int x, int y, String code, String name, float price, float change, float changePercent);
  
  // 绘制消息
  void drawMessageItem(int x, int y, String message, String time);
  
  // 绘制消息通知内容
  void drawMessageNotificationContent(int x, int y);
};

#endif // DISPLAY_MANAGER_H