#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "../coresystem/config.h"
#include "../coresystem/event_bus.h"
#include "../coresystem/data_types.h"

// 包含显示驱动接口头文件
#include "../drivers/peripherals/display_driver.h"
// 包含 GxEPD 颜色常量定义
#include "../coresystem/gxepd_colors.h"

// 包含农历管理器头文件
#include "lunar_manager.h"

// 显示刷新时间常量定义（如果 config.h 中未定义）
#ifndef WEATHER_UPDATE_INTERVAL
#define WEATHER_UPDATE_INTERVAL      7200000UL  // 天气更新间隔：2 小时
#endif
#ifndef SENSOR_UPDATE_INTERVAL
#define SENSOR_UPDATE_INTERVAL       1800000UL  // 传感器更新间隔：30 分钟
#endif
#ifndef SENSOR_UPDATE_INTERVAL_FAST
#define SENSOR_UPDATE_INTERVAL_FAST  300000UL   // 传感器快速更新间隔：5 分钟
#endif
#ifndef STOCK_UPDATE_INTERVAL
#define STOCK_UPDATE_INTERVAL        300000UL   // 股票更新间隔：5 分钟
#endif
#ifndef CALENDAR_UPDATE_INTERVAL
#define CALENDAR_UPDATE_INTERVAL     3600000UL  // 日历更新间隔：1 小时
#endif
#ifndef FULL_REFRESH_INTERVAL
#define FULL_REFRESH_INTERVAL        86400000UL // 全屏刷新间隔：24 小时
#endif
#ifndef BATTERY_CHANGE_THRESHOLD
#define BATTERY_CHANGE_THRESHOLD     5          // 电池变化阈值：5%
#endif
#ifndef TEMP_CHANGE_THRESHOLD
#define TEMP_CHANGE_THRESHOLD        2.0f       // 温度变化阈值：2°C
#endif
#ifndef HUMIDITY_CHANGE_THRESHOLD
#define HUMIDITY_CHANGE_THRESHOLD    2.0f       // 湿度变化阈值：2%
#endif

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

// 屏幕方向枚举
enum ScreenOrientation {
  ORIENTATION_HORIZONTAL,  // 横向布局（左右分栏）
  ORIENTATION_VERTICAL     // 竖向布局（上下分栏）
};

// 墨水屏尺寸枚举（根据硬件库支持的尺寸）
enum ScreenSize {
  SCREEN_SIZE_7_5_INCH,   // 7.5 寸 (800x480)
  SCREEN_SIZE_7_3_INCH,   // 7.3 寸 (1920x1080)
  SCREEN_SIZE_6_INCH,     // 6 寸 (1448x1072)
  SCREEN_SIZE_5_83_INCH,  // 5.83 寸 (1448x1072)
  SCREEN_SIZE_4_2_INCH,   // 4.2 寸 (400x300)
  SCREEN_SIZE_3_7_INCH,   // 3.7 寸 (480x280)
  SCREEN_SIZE_2_9_INCH,   // 2.9 寸 (296x128)
  SCREEN_SIZE_2_66_INCH,  // 2.66 寸 (296x152)
  SCREEN_SIZE_2_13_INCH,  // 2.13 寸 (250x122)
  SCREEN_SIZE_1_54_INCH,  // 1.54 寸 (200x200)
  SCREEN_SIZE_1_44_INCH,  // 1.44 寸 (128x128)
  SCREEN_SIZE_1_02_INCH   // 1.02 寸 (80x128)
};

// 布局配置结构
typedef struct {
  LayoutMode mode;            // 布局模式
  float leftPanelRatio;       // 左侧面板比例 (0.0-1.0)
  float rightPanelRatio;      // 右侧面板比例 (0.0-1.0)
  uint8_t fontSize;           // 基础字体大小
  uint8_t spacing;            // 元素间距
  bool showBorders;           // 是否显示边框
  ScreenOrientation orientation;  // 屏幕方向
  ScreenSize screenSize;      // 屏幕尺寸
} LayoutConfig;

// ========================================
// 显示布局常量
// ========================================
#ifndef LEFT_PANEL_DEFAULT_WIDTH
  #define LEFT_PANEL_DEFAULT_WIDTH     152    // 左侧面板默认宽度
#endif

#ifndef LEFT_PANEL_MIN_WIDTH
  #define LEFT_PANEL_MIN_WIDTH         100    // 左侧面板最小宽度
#endif

#ifndef LEFT_PANEL_MAX_WIDTH
  #define LEFT_PANEL_MAX_WIDTH         200    // 左侧面板最大宽度
#endif

#ifndef RIGHT_PANEL_DEFAULT_WIDTH
  #define RIGHT_PANEL_DEFAULT_WIDTH    96     // 右侧面板默认宽度
#endif

#ifndef DISPLAY_BORDER_PADDING
  #define DISPLAY_BORDER_PADDING       5      // 显示边框内边距
#endif

#ifndef ELEMENT_SPACING
  #define ELEMENT_SPACING              8      // 元素间距
#endif

// ========================================
// 时间间隔常量（毫秒）
// ========================================
#ifndef CLOCK_UPDATE_INTERVAL
  #define CLOCK_UPDATE_INTERVAL        60000UL    // 时钟更新间隔：1 分钟
#endif

#ifndef MESSAGE_ANIMATION_DURATION
  #define MESSAGE_ANIMATION_DURATION   10000UL    // 消息动画持续时间：10 秒
#endif

#ifndef SENSOR_ANOMALY_TIMEOUT
  #define SENSOR_ANOMALY_TIMEOUT       30000UL    // 传感器异常超时：30 秒
#endif

#ifndef ALARM_ICON_BLINK_INTERVAL
  #define ALARM_ICON_BLINK_INTERVAL    500UL      // 报警图标闪烁间隔：0.5 秒
#endif

// ========================================
// 颜色常量（16 位 RGB565 格式）
// ========================================
#ifndef COLOR_WHITE
  #define COLOR_WHITE                  0xFFFF     // 白色
#endif

#ifndef COLOR_BLACK
  #define COLOR_BLACK                  0x0000     // 黑色
#endif

#ifndef COLOR_RED
  #define COLOR_RED                    0xF800     // 红色
#endif

#ifndef COLOR_GREEN
  #define COLOR_GREEN                  0x07E0     // 绿色
#endif

#ifndef COLOR_BLUE
  #define COLOR_BLUE                   0x001F     // 蓝色
#endif

#ifndef COLOR_YELLOW
  #define COLOR_YELLOW                 0xFFE0     // 黄色
#endif

#ifndef COLOR_ALARM_BORDER
  #define COLOR_ALARM_BORDER           COLOR_WHITE  // 报警边框颜色
#endif

#ifndef COLOR_ALARM_FILL
  #define COLOR_ALARM_FILL             COLOR_BLACK  // 报警填充颜色
#endif

#ifndef COLOR_DIVIDER_LINE
  #define COLOR_DIVIDER_LINE           COLOR_BLACK  // 分隔线颜色
#endif

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
  
  // 屏幕方向管理
  void setScreenOrientation(ScreenOrientation orientation);
  ScreenOrientation getScreenOrientation() const;
  void toggleScreenOrientation();  // 切换屏幕方向
  
  // 屏幕尺寸管理
  void setScreenSize(ScreenSize size);
  ScreenSize getScreenSize() const;
  void getNextScreenSize();  // 切换到下一个尺寸
  
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
  
  // 获取显示驱动
  IDisplayDriver* getDisplayDriver() const { return displayDriver.get(); }
  
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
  uint16_t leftPanelHeight;   // 左侧面板高度（竖向布局时使用）
  uint16_t rightPanelHeight;  // 右侧面板高度（竖向布局时使用）
  
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
  
  // GIF播放相关
  bool gifPlaying;              // GIF是否正在播放
  bool gifStopped;              // GIF是否已停止
  int gifLoopCount;             // GIF循环次数（-1表示无限循环）
  int gifCurrentLoop;           // 当前循环次数
  unsigned long gifLastFrameTime; // 上一帧播放时间
  int gifFrameInterval;         // 帧间隔时间（毫秒）
  int gifCurrentFrame;          // 当前播放帧数
  int gifTotalFrames;           // GIF总帧数
  String currentGifPath;        // 当前播放的GIF路径
  uint8_t* gifBuffer;           // GIF缓冲区
  size_t gifBufferSize;         // GIF缓冲区大小
  
  // 私有方法
  void drawHeader(String title);
  void drawFooter();
  void clearScreen();
  
  // 图片解码辅助方法
  bool decodeAndDrawJPEG(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);
  bool decodeAndDrawPNG(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);
  bool decodeAndDrawBMP(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);
  bool decodeAndDrawGIF(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);
  
  // 绘制左侧面板
  void drawLeftPanel();
  
  // 绘制右侧面板
  void drawRightPanel();
  
  // 默认面板绘制函数
  void drawLeftPanelDefault();
  void drawRightPanelDefault();
  
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
  
  // 绘制状态栏（电量和消息在左上角）
  void drawStatusBar(int x, int y, float voltage, int percentage, bool isCharging, int messageCount);
  
  // 动态字体大小计算函数
  uint8_t calculateFontSize(uint16_t screenWidth, uint16_t screenHeight, String text, float widthRatio = 0.8);
  uint8_t calculateTimeFontSize(uint16_t screenWidth);
  uint8_t calculateDateFontSize(uint16_t screenWidth);
  uint8_t calculateWeatherFontSize(uint16_t screenWidth);
  
  // 刷新区域配置结构
  typedef struct {
    int clockX, clockY, clockWidth, clockHeight;
    int weatherX, weatherY, weatherWidth, weatherHeight;
    int sensorX, sensorY, sensorWidth, sensorHeight;
  } RefreshAreaConfig;
  
  RefreshAreaConfig getRefreshAreaConfig(ScreenSize size);
  
  // ========== 小屏幕 UI 绘制函数（3 寸以下，竖屏单栏） ==========
  // 2.9 寸 (296x128) UI 绘制
  void drawUI_2_9inch();
  void drawStatusBar_2_9inch();
  void drawTime_2_9inch();
  void drawDate_2_9inch();
  void drawWeather_2_9inch();
  void drawSensor_2_9inch();
  
  // 2.13 寸 (250x122) UI 绘制
  void drawUI_2_13inch();
  void drawStatusBar_2_13inch();
  void drawTime_2_13inch();
  void drawDate_2_13inch();
  void drawWeather_2_13inch();
  void drawSensor_2_13inch();
  
  // 1.54 寸 (200x200) UI 绘制
  void drawUI_1_54inch();
  void drawStatusBar_1_54inch();
  void drawTime_1_54inch();
  void drawDate_1_54inch();
  void drawWeather_1_54inch();
  void drawSensor_1_54inch();
  
  // 4.2 寸 (400x300) UI 绘制
  void drawUI_4_2inch();
  void drawStatusBar_4_2inch();
  void drawTime_4_2inch();
  void drawDate_4_2inch();
  void drawWeather_4_2inch();
  void drawSensor_4_2inch();
  void drawCalendar_4_2inch();
  void drawFestival_4_2inch();
  void drawAlmanac_4_2inch();
  
  // 3.7 寸 (480x280) UI 绘制
  void drawUI_3_7inch();
  void drawStatusBar_3_7inch();
  void drawTime_3_7inch();
  void drawDate_3_7inch();
  void drawWeather_3_7inch();
  void drawSensor_3_7inch();
  void drawCalendar_3_7inch();
  void drawFestival_3_7inch();
  void drawAlmanac_3_7inch();
  
  // 2.66 寸 (296x152) UI 绘制
  void drawUI_2_66inch();
  void drawStatusBar_2_66inch();
  void drawTime_2_66inch();
  void drawDate_2_66inch();
  void drawWeather_2_66inch();
  void drawSensor_2_66inch();
  
  // 1.44 寸 (128x128) UI 绘制
  void drawUI_1_44inch();
  void drawStatusBar_1_44inch();
  void drawTime_1_44inch();
  void drawDate_1_44inch();
  void drawWeather_1_44inch();
  void drawSensor_1_44inch();
  
  // 1.02 寸 (80x128) UI 绘制
  void drawUI_1_02inch();
  void drawStatusBar_1_02inch();
  void drawTime_1_02inch();
  void drawDate_1_02inch();
  void drawWeather_1_02inch();
  void drawSensor_1_02inch();
  
  // ========== 大屏幕 UI 绘制函数（3 寸以上，横屏双栏） ==========
  // 7.5 寸 (800x480) UI 绘制
  void drawUI_7_5inch();
  void drawLeftPanel_7_5inch();
  void drawRightPanel_7_5inch();
  void drawStatusBar_7_5inch();
  void drawTime_7_5inch();
  void drawDate_7_5inch(int x, int y);
  void drawWeather_7_5inch(int x, int y);
  void drawSensor_7_5inch(int x, int y);
  void drawStockInfo_7_5inch(int x, int y);
  void drawMessageInfo_7_5inch(int x, int y);
  void drawSystemStatus_7_5inch(int x, int y);
  void drawCalendar_7_5inch(int x, int y);
  void drawFestival_7_5inch(int x, int y);
  void drawAlmanac_7_5inch(int x, int y);
  
  // 7.3 寸 (1920x1080) UI 绘制
  void drawUI_7_3inch();
  void drawLeftPanel_7_3inch();
  void drawRightPanel_7_3inch();
  void drawStatusBar_7_3inch();
  void drawTime_7_3inch();
  void drawDate_7_3inch(int x, int y);
  void drawWeather_7_3inch(int x, int y);
  void drawSensor_7_3inch(int x, int y);
  void drawStockInfo_7_3inch(int x, int y);
  void drawMessageInfo_7_3inch(int x, int y);
  void drawSystemStatus_7_3inch(int x, int y);
  void drawCalendar_7_3inch(int x, int y);
  void drawFestival_7_3inch(int x, int y);
  void drawAlmanac_7_3inch(int x, int y);
  
  // 6 寸 (1448x1072) UI 绘制
  void drawUI_6inch();
  void drawLeftPanel_6inch();
  void drawRightPanel_6inch();
  void drawStatusBar_6inch();
  void drawTime_6inch();
  void drawDate_6inch(int x, int y);
  void drawWeather_6inch(int x, int y);
  void drawSensor_6inch(int x, int y);
  void drawStockInfo_6inch(int x, int y);
  void drawMessageInfo_6inch(int x, int y);
  void drawSystemStatus_6inch(int x, int y);
  void drawCalendar_6inch(int x, int y);
  void drawFestival_6inch(int x, int y);
  void drawAlmanac_6inch(int x, int y);
  
  // 5.83 寸 (1448x1072) UI 绘制
  void drawUI_5_83inch();
  void drawLeftPanel_5_83inch();
  void drawRightPanel_5_83inch();
  void drawStatusBar_5_83inch();
  void drawTime_5_83inch();
  void drawDate_5_83inch(int x, int y);
  void drawWeather_5_83inch(int x, int y);
  void drawSensor_5_83inch(int x, int y);
  void drawStockInfo_5_83inch(int x, int y);
  void drawMessageInfo_5_83inch(int x, int y);
  void drawSystemStatus_5_83inch(int x, int y);
  void drawCalendar_5_83inch(int x, int y);
  void drawFestival_5_83inch(int x, int y);
  void drawAlmanac_5_83inch(int x, int y);
  
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
  
  // 辅助函数
  String getWeekString(int weekday);
  
  // 图片显示功能
  bool drawImage(String imagePath, int x, int y, int width = 0, int height = 0);
  bool drawImageFromBuffer(uint8_t* buffer, int bufferSize, int x, int y, int width = 0, int height = 0);
  bool drawImageFromURL(String url, int x, int y, int width = 0, int height = 0);
  
  // GIF显示功能
  bool drawGIF(String gifPath, int x, int y, int width = 0, int height = 0);
  bool drawGIFFromBuffer(uint8_t* buffer, int bufferSize, int x, int y, int width = 0, int height = 0);
  bool drawGIFFromURL(String url, int x, int y, int width = 0, int height = 0);
  bool drawAnimatedGIF(String gifPath, int x, int y, int width = 0, int height = 0, int loopCount = -1);
  void stopGIF();
  bool isGIFPlaying();
};

#endif // DISPLAY_MANAGER_H