#ifndef WEATHER_MANAGER_H
#define WEATHER_MANAGER_H

#include <Arduino.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "../coresystem/config.h"
#include "../coresystem/data_types.h"
#include "api_manager.h"



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
  
  // 获取天气数据（惰性计算）
  WeatherData getWeatherData();
  ForecastData getForecastData(int index);
  
  // 强制更新数据
  void forceUpdate();
  
  // 检查数据是否需要更新
  bool isDataStale();
  
  // 公共方法
  String getWeatherIcon(String condition);
  bool hasValidData();
  void setDefaultWeatherData();
  
private:
  // HTTPS客户端
  WiFiClientSecure client;
  
  // 天气数据
  WeatherData currentWeather;
  ForecastData forecastData[5]; // 未来5天天气预报
  
  // 更新标志
  unsigned long lastUpdate;
  bool dataUpdated;
  bool dataRequested;
  
  // 私有方法
  bool fetchWeatherData();
  void parseWeatherData(String json);
  bool parseWeatherDataBackup(String json);
  String convertWindSpeed(float speed);
  String convertWindDirection(float deg);
};

#endif // WEATHER_MANAGER_H