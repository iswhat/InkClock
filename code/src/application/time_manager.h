#ifndef TIME_MANAGER_H
#define TIME_MANAGER_H

#include <Arduino.h>
#include <WiFiUdp.h>
#include <time.h>
#include "../coresystem/config.h"
#include "../coresystem/data_types.h"
#include "api_manager.h"


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
  String getDateTimeString(); // 获取格式化日期时间字符串，如"2023-01-01 星期日 12:34:56"
  
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
  
  // 计算精度等级
  int calculationPrecision;
  
  // 私有方法
  void updateNTPTime();
  bool sendNTPRequest(const char* serverName);
  void parseNTPTime(uint32_t unixTime);
  String getWeekdayName(int weekday);
  String getLunarDate(int year, int month, int day);
  String getSolarTerm(int year, int month, int day);
  bool isLeapYear(int year);
  int getDaysInMonth(int year, int month);
  
  // 资源感知方法
  void updateCalculationPrecision();
  int getSystemLoadLevel();
};

#endif // TIME_MANAGER_H