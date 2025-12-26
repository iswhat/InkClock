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
#define NTP_SERVER_BACKUP "ntp.tencent.com" // 备用NTP服务器
#define TIME_ZONE_OFFSET 8 // 时区偏移，单位小时（中国为+8）
#define DAYLIGHT_SAVING_TIME 0 // 夏令时，0表示不使用

// 地理位置配置
#define AUTO_DETECT_LOCATION true // 是否自动检测地理位置
#define GEO_CITY_ID "your_city_id" // 城市ID
#define GEO_CITY_NAME "your_city_name" // 城市名称
#define GEO_LATITUDE 0.0 // 纬度
#define GEO_LONGITUDE 0.0 // 经度

// 天气配置
#define WEATHER_UPDATE_INTERVAL 3600000 // 天气更新间隔，单位毫秒（1小时）
// 主天气API：使用公共免密钥的wttr.in API
#define WEATHER_API_URL "https://wttr.in/" // 主天气API URL（公共免密钥）
// 备用天气API：使用公共免密钥的open-meteo.com API
#define WEATHER_API_URL_BACKUP "https://api.open-meteo.com/v1/forecast" // 备用天气API URL（公共免密钥）
// 次备用天气API：使用需要申请的OpenWeatherMap API
#define WEATHER_API_URL_SECONDARY_BACKUP "https://api.openweathermap.org/data/2.5/forecast" // 次备用天气API URL
#define WEATHER_API_KEY "your_weather_api_key" // 天气API密钥（仅用于次备用API）
// 次备用天气API：使用需要申请的WeatherAPI API
#define WEATHER_API_URL_TERTIARY_BACKUP "https://api.weatherapi.com/v1/forecast.json" // 次备用天气API URL
#define WEATHER_API_KEY_BACKUP "your_weatherapi_key" // 备用天气API密钥（仅用于次备用API）

// 传感器配置
#define SENSOR_UPDATE_INTERVAL 60000 // 传感器更新间隔，单位毫秒
#define DHT_PIN 4 // DHT22传感器引脚
#define SHT30_ADDRESS 0x44 // SHT30传感器I2C地址
#define GAS_SENSOR_PIN 34 // 气体传感器模拟输入引脚
#define FLAME_SENSOR_PIN 35 // 火焰传感器数字输入引脚
#define LIGHT_SENSOR_PIN 36 // 光照传感器模拟输入引脚
#define GAS_ALARM_THRESHOLD 800 // 气体浓度报警阈值
#define FLAME_ALARM_THRESHOLD 1 // 火焰检测报警阈值

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

// 数据更新间隔
#define DATA_UPDATE_INTERVAL 300000 // 数据更新间隔，单位毫秒（5分钟）

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

// 低功耗模式配置
#define LOW_POWER_MODE_ENABLED true // 是否启用低功耗模式
#define PIR_SENSOR_PIN 5 // 人体感应传感器引脚
#define NO_MOTION_TIMEOUT 300000 // 无动作超时时间，单位毫秒（5分钟）
#define LOW_POWER_REFRESH_INTERVAL 3600000 // 低功耗模式下的刷新间隔，单位毫秒（1小时）
#define NORMAL_REFRESH_INTERVAL 60000 // 正常模式下的刷新间隔，单位毫秒（1分钟）

// 内容刷新间隔配置
#define CLOCK_REFRESH_INTERVAL 60000 // 时钟刷新间隔，单位毫秒（1分钟）
#define WEATHER_REFRESH_INTERVAL 3600000 // 天气刷新间隔，单位毫秒（1小时）
#define SENSOR_REFRESH_INTERVAL 300000 // 传感器数据刷新间隔，单位毫秒（5分钟）
#define STOCK_REFRESH_INTERVAL 600000 // 股票数据刷新间隔，单位毫秒（10分钟）
#define MESSAGE_REFRESH_INTERVAL 10000 // 消息刷新间隔，单位毫秒（10秒）
#define CALENDAR_REFRESH_INTERVAL 3600000 // 日历刷新间隔，单位毫秒（1小时）

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

// 硬件型号定义
enum HardwareModel {
  HARDWARE_MODEL_ESP32_C3_DEFAULT,
  HARDWARE_MODEL_ESP32_S3_DEFAULT,
  HARDWARE_MODEL_ESP32_C6_DEFAULT,
  HARDWARE_MODEL_ESP32_C6_CUSTOM,
  HARDWARE_MODEL_ESP32_S2_DEFAULT,
  HARDWARE_MODEL_ESP32_WROOM_32,
  HARDWARE_MODEL_ESP32_S3_PRO,
  HARDWARE_MODEL_ESP32_C3_SUPERMINI,
  HARDWARE_MODEL_ESP32_PRO_S3,
  HARDWARE_MODEL_ESP32_S3_WROOM_1 // 支持触摸和摄像头的ESP32-S3型号
};

// 当前硬件型号
#define CURRENT_HARDWARE_MODEL HARDWARE_MODEL_ESP32_C6_DEFAULT

// 墨水屏引脚配置
#if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_C3_DEFAULT
  // ESP32-C3默认引脚配置
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 2
  #define CHARGE_STATUS_PIN -1
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT
  // ESP32-S3默认引脚配置
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 4
  #define CHARGE_STATUS_PIN 5
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_C6_DEFAULT
  // ESP32-C6默认引脚配置
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 2
  #define CHARGE_STATUS_PIN -1
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_C6_CUSTOM
  // ESP32-C6自定义引脚配置
  #define EINK_SCK 5
  #define EINK_MOSI 6
  #define EINK_MISO 7
  #define EINK_CS 8
  #define EINK_DC 9
  #define EINK_RST 10
  #define EINK_BUSY 11
  
  #define BATTERY_ADC_PIN 2
  #define CHARGE_STATUS_PIN 3
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S2_DEFAULT
  // ESP32-S2默认引脚配置
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 4
  #define CHARGE_STATUS_PIN 5
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_WROOM_32
  // ESP32-WROOM-32默认引脚配置
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 2
  #define CHARGE_STATUS_PIN -1
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_PRO
  // ESP32-S3-Pro默认引脚配置
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 4
  #define CHARGE_STATUS_PIN 5
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_C3_SUPERMINI
  // ESP32-C3-Supermini默认引脚配置
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 2
  #define CHARGE_STATUS_PIN -1
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_PRO_S3
  // ESP32-Pro-S3默认引脚配置
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 4
  #define CHARGE_STATUS_PIN 5
  
#elif CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
  // ESP32-S3-WROOM-1引脚配置（支持触摸和摄像头）
  #define EINK_SCK 12
  #define EINK_MOSI 13
  #define EINK_MISO 14
  #define EINK_CS 15
  #define EINK_DC 16
  #define EINK_RST 17
  #define EINK_BUSY 18
  
  #define BATTERY_ADC_PIN 4
  #define CHARGE_STATUS_PIN 5
  
  // 触摸引脚配置
  #define TOUCH_PIN_0 0
  #define TOUCH_PIN_1 1
  #define TOUCH_PIN_2 2
  #define TOUCH_PIN_3 3
  
  // 摄像头引脚配置
  #define CAMERA_PIN_PWDN 32
  #define CAMERA_PIN_RESET -1
  #define CAMERA_PIN_XCLK 0
  #define CAMERA_PIN_SIOD 26
  #define CAMERA_PIN_SIOC 27
  
  #define CAMERA_PIN_D7 35
  #define CAMERA_PIN_D6 34
  #define CAMERA_PIN_D5 39
  #define CAMERA_PIN_D4 36
  #define CAMERA_PIN_D3 21
  #define CAMERA_PIN_D2 19
  #define CAMERA_PIN_D1 18
  #define CAMERA_PIN_D0 5
  #define CAMERA_PIN_VSYNC 25
  #define CAMERA_PIN_HREF 23
  #define CAMERA_PIN_PCLK 22
  
#endif

// SD卡引脚配置（增强版）
#define SD_CS 28
#define SD_SCK 25
#define SD_MOSI 26
#define SD_MISO 27

// API管理器配置
#define API_DEFAULT_TIMEOUT 10000 // API默认超时时间，单位毫秒
#define API_DEFAULT_CACHE_TIME 3600000 // API默认缓存时间，单位毫秒（1小时）
#define API_CACHE_CLEANUP_INTERVAL 3600000 // API缓存清理间隔，单位毫秒（1小时）

// 调试配置
#define DEBUG_ENABLED true // 是否启用调试信息
#define DEBUG_SERIAL_BAUD 115200 // 调试串口波特率

// 主题类型枚举
enum ThemeType {
  THEME_DEFAULT,
  THEME_LARGE,
  THEME_COMPACT,
  THEME_MINIMAL
};

// 当前使用的主题
#define CURRENT_THEME THEME_DEFAULT

// 定义墨水屏类型枚举
enum EinkDisplayType {
  // 标准墨水屏
  EINK_154_INCH,
  EINK_213_INCH,
  EINK_266_INCH,
  EINK_27_INCH,
  EINK_29_INCH,
  EINK_312_INCH,
  EINK_42_INCH,
  EINK_437_INCH,
  EINK_54_INCH,
  EINK_583_INCH,
  EINK_60_INCH,
  EINK_75_INCH,
  EINK_78_INCH,
  EINK_103_INCH,
  EINK_1248_INCH,
  
  // 电子价签
  ESL_154_INCH_DUAL,
  ESL_213_INCH_DUAL,
  ESL_266_INCH_DUAL,
  ESL_29_INCH_DUAL,
  ESL_312_INCH_DUAL,
  ESL_42_INCH_COLOR,
  ESL_583_INCH_COLOR,
  
  // 二手阅读器屏幕
  READER_6_INCH_MONO,
  READER_78_INCH_MONO,
  READER_103_INCH_MONO,
  READER_6_INCH_COLOR,
  READER_78_INCH_COLOR,
  READER_103_INCH_COLOR
};

// 主题配置
// 默认主题字体大小
#define THEME_DEFAULT_CLOCK_SIZE_42 5
#define THEME_DEFAULT_DATE_SIZE_42 2
#define THEME_DEFAULT_WEATHER_SIZE_42 1
#define THEME_DEFAULT_SENSOR_SIZE_42 1
#define THEME_DEFAULT_BATTERY_SIZE_42 2
#define THEME_DEFAULT_MESSAGE_SIZE_42 2

#define THEME_DEFAULT_CLOCK_SIZE_75 8
#define THEME_DEFAULT_DATE_SIZE_75 3
#define THEME_DEFAULT_WEATHER_SIZE_75 2
#define THEME_DEFAULT_SENSOR_SIZE_75 2
#define THEME_DEFAULT_BATTERY_SIZE_75 3
#define THEME_DEFAULT_MESSAGE_SIZE_75 3

// 大字体主题
#define THEME_LARGE_CLOCK_SIZE_42 6
#define THEME_LARGE_DATE_SIZE_42 3
#define THEME_LARGE_WEATHER_SIZE_42 2
#define THEME_LARGE_SENSOR_SIZE_42 2
#define THEME_LARGE_BATTERY_SIZE_42 3
#define THEME_LARGE_MESSAGE_SIZE_42 3

#define THEME_LARGE_CLOCK_SIZE_75 10
#define THEME_LARGE_DATE_SIZE_75 4
#define THEME_LARGE_WEATHER_SIZE_75 3
#define THEME_LARGE_SENSOR_SIZE_75 3
#define THEME_LARGE_BATTERY_SIZE_75 4
#define THEME_LARGE_MESSAGE_SIZE_75 4

// 紧凑主题
#define THEME_COMPACT_CLOCK_SIZE_42 4
#define THEME_COMPACT_DATE_SIZE_42 1
#define THEME_COMPACT_WEATHER_SIZE_42 1
#define THEME_COMPACT_SENSOR_SIZE_42 1
#define THEME_COMPACT_BATTERY_SIZE_42 1
#define THEME_COMPACT_MESSAGE_SIZE_42 1

#define THEME_COMPACT_CLOCK_SIZE_75 6
#define THEME_COMPACT_DATE_SIZE_75 2
#define THEME_COMPACT_WEATHER_SIZE_75 1
#define THEME_COMPACT_SENSOR_SIZE_75 1
#define THEME_COMPACT_BATTERY_SIZE_75 2
#define THEME_COMPACT_MESSAGE_SIZE_75 2

// 极简主题
#define THEME_MINIMAL_CLOCK_SIZE_42 7
#define THEME_MINIMAL_DATE_SIZE_42 0
#define THEME_MINIMAL_WEATHER_SIZE_42 0
#define THEME_MINIMAL_SENSOR_SIZE_42 0
#define THEME_MINIMAL_BATTERY_SIZE_42 1
#define THEME_MINIMAL_MESSAGE_SIZE_42 1

#define THEME_MINIMAL_CLOCK_SIZE_75 12
#define THEME_MINIMAL_DATE_SIZE_75 0
#define THEME_MINIMAL_WEATHER_SIZE_75 0
#define THEME_MINIMAL_SENSOR_SIZE_75 0
#define THEME_MINIMAL_BATTERY_SIZE_75 2
#define THEME_MINIMAL_MESSAGE_SIZE_75 2

// Web客户端配置
#define WEB_SERVER_URL "https://your-webserver-url.com/api.php" // 主Web服务器URL
#define WEB_SERVER_URL_BACKUP "https://backup-webserver-url.com/api.php" // 备用Web服务器URL
#define WEB_SERVER_URL_SECONDARY_BACKUP "https://secondary-backup-webserver-url.com/api.php" // 次备用Web服务器URL
#define API_KEY "your_secret_key_here" // Web服务器API密钥

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