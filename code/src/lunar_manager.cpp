#include "lunar_manager.h"
#include <ArduinoJson.h>

// 外部全局对象
extern WiFiManager wifiManager;

// API配置 - 使用RollToolsApi的万年历接口
const String LunarManager::LUNAR_API_URL = "https://api.rolltools.cn/api/lunar?date=";

LunarManager::LunarManager() {
  // 初始化缓存
  cacheTimestamp = 0;
  lastUpdate = 0;
}

LunarManager::~LunarManager() {
  // 清理资源
}

void LunarManager::init() {
  DEBUG_PRINTLN("初始化农历管理器...");
  
  // 初始化Web客户端
  webClient.init();
  
  DEBUG_PRINTLN("农历管理器初始化完成");
}

void LunarManager::update() {
  // 定期更新农历数据
  if (wifiManager.isConnected()) {
    // 每天更新一次
    unsigned long now = millis();
    if (now - lastUpdate > 86400000) { // 24小时
      lastUpdate = now;
      // 更新当前日期的农历信息
      time_t nowTime = time(nullptr);
      struct tm *timeinfo = localtime(&nowTime);
      getLunarInfo(timeinfo->tm_year + 1900, timeinfo->tm_mon + 1, timeinfo->tm_mday);
    }
  }
}

void LunarManager::loop() {
  // 处理Web客户端的循环任务
  webClient.loop();
}

LunarInfo LunarManager::getLunarInfo(int year, int month, int day) {
  // 检查缓存是否有效
  unsigned long now = millis();
  if (cacheTimestamp > 0 && now - cacheTimestamp < CACHE_DURATION) {
    return cachedLunarInfo;
  }
  
  // 尝试从API获取数据
  if (fetchLunarData(year, month, day)) {
    cacheTimestamp = now;
    return cachedLunarInfo;
  }
  
  // 如果API请求失败，返回默认值
  LunarInfo defaultInfo;
  defaultInfo.lunarDate = "正月初一";
  defaultInfo.lunarMonth = "正月";
  defaultInfo.lunarDay = "初一";
  defaultInfo.solarTerm = "";
  defaultInfo.festival.name = "";
  defaultInfo.festival.type = "";
  return defaultInfo;
}

FestivalInfo LunarManager::getFestival(int year, int month, int day) {
  LunarInfo lunarInfo = getLunarInfo(year, month, day);
  return lunarInfo.festival;
}

LunarCalendarInfo LunarManager::getLunarCalendar(int year, int month, int day) {
  LunarInfo lunarInfo = getLunarInfo(year, month, day);
  return lunarInfo.lunarCalendar;
}

String LunarManager::getLunarDateString(int year, int month, int day) {
  LunarInfo lunarInfo = getLunarInfo(year, month, day);
  return lunarInfo.lunarDate;
}

String LunarManager::getSolarTerm(int year, int month, int day) {
  LunarInfo lunarInfo = getLunarInfo(year, month, day);
  return lunarInfo.solarTerm;
}

bool LunarManager::fetchLunarData(int year, int month, int day) {
  // 构建请求URL
  char dateStr[11];
  snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d", year, month, day);
  String url = LUNAR_API_URL + String(dateStr);
  
  DEBUG_PRINT("获取农历数据: ");
  DEBUG_PRINTLN(url);
  
  // 发送HTTP请求
  String response = webClient.get(url);
  if (response.isEmpty()) {
    DEBUG_PRINTLN("获取农历数据失败，响应为空");
    return false;
  }
  
  // 解析JSON响应
  LunarInfo lunarInfo = parseLunarData(response);
  if (lunarInfo.lunarDate.isEmpty()) {
    DEBUG_PRINTLN("解析农历数据失败");
    return false;
  }
  
  // 保存到缓存
  cachedLunarInfo = lunarInfo;
  return true;
}

LunarInfo LunarManager::parseLunarData(const String& jsonData) {
  LunarInfo lunarInfo;
  
  // 解析JSON响应
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, jsonData);
  
  if (error) {
    DEBUG_PRINT("JSON解析失败: ");
    DEBUG_PRINTLN(error.c_str());
    return lunarInfo;
  }
  
  // 检查响应状态
  if (doc["code"].as<int>() != 0) {
    DEBUG_PRINT("API请求失败: ");
    DEBUG_PRINTLN(doc["msg"].as<String>());
    return lunarInfo;
  }
  
  // 解析农历日期
  JsonObject data = doc["data"];
  
  // 获取农历日期信息
  lunarInfo.lunarDate = data["lunar_date"].as<String>();
  lunarInfo.lunarMonth = data["lunar_month"].as<String>();
  lunarInfo.lunarDay = data["lunar_day"].as<String>();
  
  // 获取节气信息
  if (data.containsKey("solar_term")) {
    lunarInfo.solarTerm = data["solar_term"].as<String>();
  }
  
  // 获取节日信息
  if (data.containsKey("festival")) {
    lunarInfo.festival.name = data["festival"].as<String>();
    lunarInfo.festival.type = data["festival_type"].as<String>();
  }
  
  // 获取黄历信息
  if (data.containsKey("lunar_calendar")) {
    JsonObject lunarCalendar = data["lunar_calendar"];
    
    if (lunarCalendar.containsKey("year_ganzhi")) {
      lunarInfo.lunarCalendar.yearGanZhi = lunarCalendar["year_ganzhi"].as<String>();
    }
    
    if (lunarCalendar.containsKey("month_ganzhi")) {
      lunarInfo.lunarCalendar.monthGanZhi = lunarCalendar["month_ganzhi"].as<String>();
    }
    
    if (lunarCalendar.containsKey("day_ganzhi")) {
      lunarInfo.lunarCalendar.dayGanZhi = lunarCalendar["day_ganzhi"].as<String>();
    }
    
    if (lunarCalendar.containsKey("animal")) {
      lunarInfo.lunarCalendar.animal = lunarCalendar["animal"].as<String>();
    }
    
    if (lunarCalendar.containsKey("yi")) {
      lunarInfo.lunarCalendar.yi = lunarCalendar["yi"].as<String>();
    }
    
    if (lunarCalendar.containsKey("ji")) {
      lunarInfo.lunarCalendar.ji = lunarCalendar["ji"].as<String>();
    }
    
    if (lunarCalendar.containsKey("xiang_chong")) {
      lunarInfo.lunarCalendar.xiangChong = lunarCalendar["xiang_chong"].as<String>();
    }
    
    if (lunarCalendar.containsKey("xingxiu")) {
      lunarInfo.lunarCalendar.xingXiu = lunarCalendar["xingxiu"].as<String>();
    }
    
    if (lunarCalendar.containsKey("liuyao")) {
      lunarInfo.lunarCalendar.liuYao = lunarCalendar["liuyao"].as<String>();
    }
    
    if (lunarCalendar.containsKey("pengzu")) {
      lunarInfo.lunarCalendar.pengZu = lunarCalendar["pengzu"].as<String>();
    }
    
    if (lunarCalendar.containsKey("wuxing")) {
      lunarInfo.lunarCalendar.wuxing = lunarCalendar["wuxing"].as<String>();
    }
  }
  
  return lunarInfo;
}

String LunarManager::getGanZhi(int year, int month, int day) {
  // 简化的干支计算，实际应用中使用API数据
  return "甲子";
}

String LunarManager::getAnimal(int year) {
  // 简化的生肖计算，实际应用中使用API数据
  const char* animals[] = {"鼠", "牛", "虎", "兔", "龙", "蛇", "马", "羊", "猴", "鸡", "狗", "猪"};
  int index = (year - 1900) % 12;
  return String(animals[index]);
}
