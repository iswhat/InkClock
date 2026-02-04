#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "../coresystem/arduino_compat.h"
#endif
#include "../coresystem/config.h"
#include "../coresystem/event_bus.h"
#include "../coresystem/data_types.h"

// 包含显示驱动接口头文件
#include "../drivers/peripherals/display_driver.h"
// 包含GxEPD颜色常量定义
#include "../coresystem/gxepd_colors.h"

// 包含农历管理器头文件
#include "lunar_manager.h"

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
  CLOCK_MODE_ANALOG,
  CLOCK_MODE_TEXT
};

// 布局模式枚举
enum LayoutMode {
  LAYOUT_MODE_COMPACT,   // 紧凑模式，适用于小屏幕
  LAYOUT_MODE_STANDARD,  // 标准模式，适用于中等屏幕
  LAYOUT_MODE_EXTENDED,  // 扩展模式，适用于大屏幕
  LAYOUT_MODE_CUSTOM     // 自定义模式，用户可调整比例
};

// 布局配置结构
typedef struct {
  LayoutMode mode;        // 布局模式
  float leftPanelRatio;   // 左侧面板比例 (0.0-1.0)
  float rightPanelRatio;  // 右侧面板比例 (0.0-1.0)
  uint8_t fontSize;       // 基础字体大小
  uint8_t spacing;        // 元素间距
  bool showBorders;       // 是否显示边框
} LayoutConfig;

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
  
  // 消息提醒动画
  void startMessageAnimation();
  void stopMessageAnimation();
  void updateMessageAnimation();
  
  // 传感器异常检测和报警
  void checkSensorAnomalies(float temperature, float humidity);
  void startSensorAlarm(String anomalyType);
  void stopSensorAlarm();
  void updateSensorAlarm();
  
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
  
  // 设置刷新间隔
  void setRefreshInterval(unsigned long interval) { /* 空实现，用于兼容 */ }
  
  // 布局管理
  void setLayoutMode(LayoutMode mode);
  LayoutMode getLayoutMode() const;
  void setCustomLayout(float leftPanelRatio, float rightPanelRatio);
  LayoutConfig getCurrentLayout() const;
  void applyLayout();
  
  // 时区管理
  struct TimeZone {
    String name;
    String abbreviation;
    int offset;
    bool useDST;
  };
  
  // 设置时区
  void setTimeZone(const TimeZone& tz);
  
  // 获取当前时区
  TimeZone getCurrentTimeZone() const;
  
  // 自动检测时区
  void autoDetectTimeZone();
  
  // 秒针显示设置
  bool getShowSeconds() const { return showSeconds; }
  void setShowSeconds(bool show) { showSeconds = show; }
  
  // 报警显示相关方法
  void showAlarm(String alarmType, String message);
  void hideAlarm();
  bool isAlarmShowing() const { return alarmShowing; }
  void updateAlarmDisplay();
  
private:
  // Security: Use smart pointer to prevent memory leaks and double-free
  std::unique_ptr<IDisplayDriver> displayDriver;
  
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
  
  // 布局配置
  LayoutConfig currentLayout;
  LayoutMode layoutMode;
  
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
  
  // 报警显示相关变量
  bool alarmShowing;
  String currentAlarmType;
  String currentAlarmMessage;
  unsigned long lastAlarmUpdateTime;
  bool alarmBlinkState;
  unsigned long lastBlinkTime;
  unsigned long alarmStartTime;
  
  // 消息提醒动画相关变量
  bool messageAnimationActive;
  unsigned long messageAnimationStartTime;
  unsigned long messageAnimationLastUpdate;
  int messageAnimationFrame;
  bool messageAnimationDirection;
  
  // 传感器异常检测相关变量
  bool sensorAnomalyDetected;
  String sensorAnomalyType;
  unsigned long sensorAnomalyStartTime;
  bool sensorAlarmActive;
  
  // 时区管理
  TimeZone currentTimeZone;
  bool autoTimeZoneEnabled;
  
  // 本地缓存数据
  TimeData cachedTimeData;
  WeatherData cachedWeatherData;
  SensorData cachedSensorData;
  int cachedBatteryPercentage;
  float cachedBatteryVoltage;
  bool cachedIsCharging;
  int cachedUnreadMessageCount;
  
  // 传感器数据历史记录（用于趋势图表）
  static const int MAX_SENSOR_HISTORY = 10;
  float tempHistory[MAX_SENSOR_HISTORY];
  float humHistory[MAX_SENSOR_HISTORY];
  int sensorHistoryIndex;
  
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
  void drawAnalogClock(int x, int y, int hour, int minute, int second, int millisecond = 0);
  
  // 绘制时钟（文字模式）
  void drawTextClock(int x, int y, int hour, int minute, int second);
  
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
  void drawMessageItem(int x, int y, String message, String time, MessagePriority priority = MESSAGE_PRIORITY_NORMAL);
  
  // 绘制消息通知内容
  void drawMessageNotificationContent(int x, int y);
};

#endif // DISPLAY_MANAGER_H