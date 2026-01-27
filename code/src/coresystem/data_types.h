/**
 * @file data_types.h
 * @brief 数据类型定义
 * @author iswhat
 * @date 2026-01-26
 * @version 1.0
 */

#ifndef DATA_TYPES_H
#define DATA_TYPES_H

#include <Arduino.h>

// 时间数据结构
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int weekday;
  String dateString;
  String timeString;
  String lunarDate;
  bool isLeapYear;
  String solarTerm;
} TimeData;

// 天气数据结构
typedef struct {
  String city;
  float temp;
  float feelsLike;
  int humidity;
  int pressure;
  int windSpeed;
  String condition;
  String weatherIcon;
  int uvIndex;
  String uvIndexLevel;
  int visibility;
  float airQuality;
  String airQualityLevel;
  float aqi;
  int sunrise;
  int sunset;
} WeatherData;

// 传感器数据结构
typedef struct {
  float temperature;
  float humidity;
  float pressure;
  float altitude;
  float light;
  float co2;
  float voc;
  float pm25;
  float pm10;
  float no2;
  float so2;
  float co;
  float o3;
  float ch2o;
  float noise;
  float soilMoisture;
  float soilTemperature;
  bool motionDetected;
  int gasLevel;
  bool flameDetected;
  int lightLevel;
  bool valid;
} SensorData;

// 消息优先级枚举
enum MessagePriority {
  MESSAGE_PRIORITY_LOW,
  MESSAGE_PRIORITY_NORMAL,
  MESSAGE_PRIORITY_HIGH,
  MESSAGE_PRIORITY_URGENT
};

// 消息类别枚举
enum MessageCategory {
  MESSAGE_CATEGORY_GENERAL,
  MESSAGE_CATEGORY_WEATHER,
  MESSAGE_CATEGORY_STOCK,
  MESSAGE_CATEGORY_SENSOR,
  MESSAGE_CATEGORY_SYSTEM,
  MESSAGE_CATEGORY_NOTIFICATION,
  MESSAGE_CATEGORY_ALARM
};

// 消息数据结构
typedef struct {
  String id;
  String content;
  String sender;
  String receiver;
  String timestamp;
  MessagePriority priority;
  MessageCategory category;
  bool read;
  bool archived;
  bool valid;
} MessageData;



#endif // DATA_TYPES_H