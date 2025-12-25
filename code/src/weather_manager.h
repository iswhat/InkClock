#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "config.h"

// 天气数据结构
typedef struct {
  String city;          // 城市名称
  float temp;           // 当前温度（摄氏度）
  int humidity;         // 湿度（%）
  String condition;     // 天气状况（如"晴"、"多云"等）
  String wind;          // 风力风向
  float tempMin;        // 最低温度
  float tempMax;        // 最高温度
  int pressure;         // 气压
  int visibility;       // 能见度
  long sunrise;         // 日出时间（Unix时间戳）
  long sunset;          // 日落时间（Unix时间戳）
  bool valid;           // 数据是否有效
} WeatherData;

// 天气预报数据结构
typedef struct {
  String date;          // 日期
  float tempDay;        // 白天温度
  float tempNight;      // 夜间温度
  String condition;     // 天气状况
  String wind;          // 风力风向
  int humidity;         // 湿度
} ForecastData;

class WeatherManager {
public:
  WeatherManager();
  ~WeatherManager();
  
  void init();
  void update();
  void loop();
  
  // 获取天气数据
  WeatherData getWeatherData() { return currentWeather; }
  ForecastData getForecastData(int index);
  
private:
  // HTTPS客户端
  WiFiClientSecure client;
  
  // 天气数据
  WeatherData currentWeather;
  ForecastData forecastData[5]; // 未来5天天气预报
  
  // 更新标志
  unsigned long lastUpdate;
  bool dataUpdated;
  
  // 私有方法
  bool fetchWeatherData();
  void parseWeatherData(String json);
  String getWeatherIcon(String condition);
  String convertWindSpeed(float speed);
  String convertWindDirection(float deg);
};

#endif // WEATHER_MANAGER_H