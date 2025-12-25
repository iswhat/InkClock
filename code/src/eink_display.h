#ifndef EINK_DISPLAY_H
#define EINK_DISPLAY_H

#include <Arduino.h>
#include <GxGDEW042Z15/GxGDEW042Z15.h> // 4.2寸三色墨水屏库
#include <GxGDEW075Z09/GxGDEW075Z09.h> // 7.5寸三色墨水屏库
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <GxFonts/GxFonts.h>
#include "config.h"

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

class EinkDisplay {
public:
  EinkDisplay();
  ~EinkDisplay();
  
  void init();
  void showSplashScreen();
  void updateDisplay();
  void updateLeftPanel();
  void updateRightPanel();
  void updateClockArea();
  void showMessage(String message, uint32_t duration = 3000);
  void switchRightPage(RightPageType page);
  void toggleClockMode();
  
  // 获取当前右侧页面
  RightPageType getCurrentRightPage() { return currentRightPage; }
  
  // 获取当前时钟模式
  ClockMode getCurrentClockMode() { return currentClockMode; }
  
private:
  // 墨水屏对象
  #if DISPLAY_TYPE == EINK_42_INCH
    GxIO_Class io;
    GxGDEW042Z15_Class display;
  #elif DISPLAY_TYPE == EINK_75_INCH
    GxIO_Class io;
    GxGDEW075Z09_Class display;
  #endif
  GxFonts fonts;
  
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
  
  // 私有方法
  void drawHeader(String title);
  void drawFooter();
  void clearScreen();
  void displayFullRefresh();
  void displayPartialRefresh(uint16_t x, uint16_t y, uint16_t w, uint16_t h);
  
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
};

#endif // EINK_DISPLAY_H