#ifndef CONFIG_H
#define CONFIG_H

// 显示配置
#define DISPLAY_TYPE EINK_75_INCH // EINK_42_INCH or EINK_75_INCH
#define DISPLAY_UPDATE_INTERVAL 60000 // 显示更新间隔，单位毫秒
#define SPLASH_SCREEN_DURATION 3000 // 启动画面显示时间，单位毫秒

// WiFi配置
#define WIFI_SSID "your_ssid" // WiFi名称
#define WIFI_PASSWORD "your_password" // WiFi密码
#define WIFI_RECONNECT_INTERVAL 5000 // 重连间隔，单位毫秒

// 时间配置
#define NTP_SERVER "ntp.aliyun.com" // NTP服务器
#define TIME_ZONE_OFFSET 8 // 时区偏移，单位小时（中国为+8）
#define DAYLIGHT_SAVING_TIME 0 // 夏令时，0表示不使用

// 天气配置
#define WEATHER_UPDATE_INTERVAL 3600000 // 天气更新间隔，单位毫秒（1小时）
#define WEATHER_API_KEY "your_weather_api_key" // 天气API密钥
#define WEATHER_CITY_ID "your_city_id" // 城市ID
#define WEATHER_API_URL "https://api.openweathermap.org/data/2.5/forecast" // 天气API URL

// 传感器配置
#define SENSOR_UPDATE_INTERVAL 60000 // 传感器更新间隔，单位毫秒
#define DHT_PIN 4 // DHT22传感器引脚
#define SHT30_ADDRESS 0x44 // SHT30传感器I2C地址

// 音频配置
#define AUDIO_SAMPLE_RATE 44100 // 音频采样率
#define AUDIO_RECORD_DURATION 10 // 录音时长，单位秒
#define AUDIO_VOLUME 8 // 音量，0-10
#define I2S_BCLK 19 // I2S时钟引脚
#define I2S_LRC 20 // I2S左右声道时钟引脚
#define I2S_DIN 21 // I2S数据输入引脚
#define I2S_DOUT 22 // I2S数据输出引脚

// 按键配置
#define BUTTON_COUNT 4 // 按键数量
#define BUTTON_PINS {0, 1, 2, 3} // 按键引脚
#define BUTTON_DEBOUNCE_TIME 50 // 按键消抖时间，单位毫秒
#define BUTTON_LONG_PRESS_TIME 1000 // 长按时间，单位毫秒

// 消息配置
#define MESSAGE_UPDATE_INTERVAL 60000 // 消息更新间隔，单位毫秒
#define MAX_MESSAGES 10 // 最大消息数量
#define MESSAGE_DISPLAY_DURATION 5000 // 消息显示时长，单位毫秒

// 股票配置
#define STOCK_UPDATE_INTERVAL 600000 // 股票更新间隔，单位毫秒（10分钟）
#define MAX_STOCKS 3 // 最大股票数量
#define STOCK_CODES {"600000", "000001", "300001"} // 股票代码列表

// 插件配置
#define PLUGIN_UPDATE_INTERVAL 300000 // 插件更新间隔，单位毫秒（5分钟）
#define MAX_PLUGINS 5 // 最大插件数量

// 电源配置
#define LOW_BATTERY_THRESHOLD 3.5 // 低电量阈值，单位伏特
#define BATTERY_UPDATE_INTERVAL 300000 // 电池电量更新间隔，单位毫秒（5分钟）
#define FULL_BATTERY_VOLTAGE 4.2 // 满电电压，单位伏特
#define EMPTY_BATTERY_VOLTAGE 3.3 // 空电电压，单位伏特

// 电池检测引脚配置（ESP32-C3）
#if defined(ESP32_C3)
  #define BATTERY_ADC_PIN 2 // 电池电量检测ADC引脚
  #define CHARGE_STATUS_PIN -1 // 充电状态检测引脚（-1表示不使用）
#endif

// 电池检测引脚配置（ESP32-S3）
#if defined(ESP32_S3)
  #define BATTERY_ADC_PIN 4 // 电池电量检测ADC引脚
  #define CHARGE_STATUS_PIN 5 // 充电状态检测引脚
#endif

// 墨水屏引脚配置（ESP32-C3）
#if defined(ESP32_C3)
  #if DISPLAY_TYPE == EINK_42_INCH
    #define EINK_SCK 12
    #define EINK_MOSI 13
    #define EINK_MISO 14
    #define EINK_CS 15
    #define EINK_DC 16
    #define EINK_RST 17
    #define EINK_BUSY 18
  #elif DISPLAY_TYPE == EINK_75_INCH
    #define EINK_SCK 12
    #define EINK_MOSI 13
    #define EINK_MISO 14
    #define EINK_CS 15
    #define EINK_DC 16
    #define EINK_RST 17
    #define EINK_BUSY 18
  #endif
#endif

// 墨水屏引脚配置（ESP32-S3）
#if defined(ESP32_S3)
  #if DISPLAY_TYPE == EINK_42_INCH
    #define EINK_SCK 12
    #define EINK_MOSI 13
    #define EINK_MISO 14
    #define EINK_CS 15
    #define EINK_DC 16
    #define EINK_RST 17
    #define EINK_BUSY 18
  #elif DISPLAY_TYPE == EINK_75_INCH
    #define EINK_SCK 12
    #define EINK_MOSI 13
    #define EINK_MISO 14
    #define EINK_CS 15
    #define EINK_DC 16
    #define EINK_RST 17
    #define EINK_BUSY 18
  #endif
#endif

// SD卡引脚配置（增强版）
#define SD_CS 28
#define SD_SCK 25
#define SD_MOSI 26
#define SD_MISO 27

// 调试配置
#define DEBUG_ENABLED true // 是否启用调试信息
#define DEBUG_SERIAL_BAUD 115200 // 调试串口波特率

// 定义墨水屏类型枚举
enum EinkDisplayType {
  EINK_42_INCH,
  EINK_75_INCH
};

// 调试宏
#if DEBUG_ENABLED
  #define DEBUG_PRINT(...) Serial.print(__VA_ARGS__)
  #define DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
  #define DEBUG_WRITE(...) Serial.write(__VA_ARGS__)
#else
  #define DEBUG_PRINT(...) do {} while (0)
  #define DEBUG_PRINTLN(...) do {} while (0)
  #define DEBUG_WRITE(...) do {} while (0)
#endif

#endif // CONFIG_H