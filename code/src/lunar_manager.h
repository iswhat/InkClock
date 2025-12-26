#ifndef LUNAR_MANAGER_H
#define LUNAR_MANAGER_H

#include <Arduino.h>
#include "config.h"
#include "web_client.h"

// 节日信息结构
typedef struct {
  String name; // 节日名称
  String type; // 节日类型：solar(公历), lunar(农历), other(其他)
} FestivalInfo;

// 黄历信息结构
typedef struct {
  String yearGanZhi; // 年干支
  String monthGanZhi; // 月干支
  String dayGanZhi; // 日干支
  String animal; // 生肖
  String yi; // 宜
  String ji; // 忌
  String xiangChong; // 相冲
  String xingXiu; // 星宿
  String liuYao; // 六曜
  String pengZu; // 彭祖百忌
  String wuxing; // 五行
} LunarCalendarInfo;

// 完整农历信息结构
typedef struct {
  String lunarDate; // 农历日期，如"正月初一"
  String lunarMonth; // 农历月份，如"正月"
  String lunarDay; // 农历日，如"初一"
  String solarTerm; // 节气
  FestivalInfo festival; // 节日信息
  LunarCalendarInfo lunarCalendar; // 黄历信息
} LunarInfo;

class LunarManager {
public:
  LunarManager();
  ~LunarManager();
  
  void init();
  void update();
  void loop();
  
  // 获取农历信息
  LunarInfo getLunarInfo(int year, int month, int day);
  
  // 获取指定日期的节日列表
  FestivalInfo getFestival(int year, int month, int day);
  
  // 获取指定日期的黄历信息
  LunarCalendarInfo getLunarCalendar(int year, int month, int day);
  
  // 获取农历日期字符串
  String getLunarDateString(int year, int month, int day);
  
  // 获取节气
  String getSolarTerm(int year, int month, int day);
  
private:
  // Web客户端
  WebClient webClient;
  
  // 缓存的农历信息
  LunarInfo cachedLunarInfo;
  
  // 缓存时间戳
  unsigned long cacheTimestamp;
  
  // 最后更新时间
  unsigned long lastUpdate;
  
  // 缓存有效期（毫秒）
  static const unsigned long CACHE_DURATION = 86400000; // 1天
  
  // API配置
  static const String LUNAR_API_URL;
  
  // 私有方法
  bool fetchLunarData(int year, int month, int day);
  LunarInfo parseLunarData(const String& jsonData);
  String getGanZhi(int year, int month, int day);
  String getAnimal(int year);
};

#endif // LUNAR_MANAGER_H
