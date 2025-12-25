#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include "config.h"

// 时间数据结构
typedef struct {
  int year;
  int month;
  int day;
  int hour;
  int minute;
  int second;
  int weekday; // 0-6, 0表示周日
  bool isLeapYear; // 是否是闰年
  String lunarDate; // 农历日期
  String solarTerm; // 节气
} TimeData;

class TimeManager {
public:
  TimeManager();
  ~TimeManager();
  
  void init();
  void update();
  void loop();
  
  // 获取时间数据
  TimeData getTimeData() { return currentTime; }
  String getTimeString(); // 获取格式化时间字符串，如"12:34:56"
  String getDateString(); // 获取格式化日期字符串，如"2023-01-01 星期日"
  String getLunarDateString(); // 获取农历日期字符串
  
  // 设置时间
  void setTime(int hour, int minute, int second);
  void setDate(int year, int month, int day);
  
private:
  // NTP客户端
  WiFiUDP ntpUDP;
  
  // 当前时间
  TimeData currentTime;
  
  // 时间更新标志
  bool timeUpdated;
  unsigned long lastUpdate;
  
  // 私有方法
  void updateNTPTime();
  void parseNTPTime(uint32_t ntpTime);
  String getWeekdayName(int weekday);
  String getLunarDate(int year, int month, int day);
  String getSolarTerm(int year, int month, int day);
  bool isLeapYear(int year);
  int getDaysInMonth(int year, int month);
};

#endif // TIME_MANAGER_H