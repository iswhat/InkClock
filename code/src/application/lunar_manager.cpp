#include "lunar_manager.h"
#include <ArduinoJson.h>
#include "application/api_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;
extern APIManager apiManager;

// API配置 - 使用RollToolsApi的万年历接口
const String LunarManager::LUNAR_API_URL = "https://api.rolltools.cn/api/lunar?date="; // 主农历API（公共免密钥）
const String LunarManager::LUNAR_API_URL_BACKUP = "https://api.vvhan.com/api/lunar?date="; // 备用农历API（公共免密钥）
const String LunarManager::LUNAR_API_URL_SECONDARY_BACKUP = "https://api.66mz8.com/api/lunar.php?date="; // 次备用农历API（公共免密钥）

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
  
  // 尝试使用主API
  String url = LUNAR_API_URL + String(dateStr);
  DEBUG_PRINT("获取农历数据: ");
  DEBUG_PRINTLN(url);
  
  ApiResponse apiResponse = apiManager.get(url, API_TYPE_LUNAR, 86400000); // 缓存24小时
  
  if (apiResponse.status == API_STATUS_SUCCESS || apiResponse.status == API_STATUS_CACHED) {
    String response = apiResponse.response;
    if (!response.isEmpty()) {
      LunarInfo lunarInfo = parseLunarData(response);
      if (!lunarInfo.lunarDate.isEmpty()) {
        cachedLunarInfo = lunarInfo;
        return true;
      }
    }
  }
  
  DEBUG_PRINTLN("主API获取农历数据失败: " + apiResponse.error);
  
  // 尝试使用备用API
  String backupUrl = LUNAR_API_URL_BACKUP + String(dateStr);
  DEBUG_PRINT("尝试使用备用API获取农历数据: ");
  DEBUG_PRINTLN(backupUrl);
  
  ApiResponse backupApiResponse = apiManager.get(backupUrl, API_TYPE_LUNAR, 86400000);
  
  if (backupApiResponse.status == API_STATUS_SUCCESS || backupApiResponse.status == API_STATUS_CACHED) {
    String backupResponse = backupApiResponse.response;
    if (!backupResponse.isEmpty()) {
      LunarInfo lunarInfo = parseLunarDataBackup(backupResponse);
      if (!lunarInfo.lunarDate.isEmpty()) {
        cachedLunarInfo = lunarInfo;
        return true;
      }
    }
  }
  
  DEBUG_PRINTLN("备用API获取农历数据失败: " + backupApiResponse.error);
  
  // 尝试使用次备用API
  String secondaryBackupUrl = LUNAR_API_URL_SECONDARY_BACKUP + String(dateStr);
  DEBUG_PRINT("尝试使用次备用API获取农历数据: ");
  DEBUG_PRINTLN(secondaryBackupUrl);
  
  ApiResponse secondaryBackupApiResponse = apiManager.get(secondaryBackupUrl, API_TYPE_LUNAR, 86400000);
  
  if (secondaryBackupApiResponse.status == API_STATUS_SUCCESS || secondaryBackupApiResponse.status == API_STATUS_CACHED) {
    String secondaryBackupResponse = secondaryBackupApiResponse.response;
    if (!secondaryBackupResponse.isEmpty()) {
      LunarInfo lunarInfo = parseLunarDataSecondaryBackup(secondaryBackupResponse);
      if (!lunarInfo.lunarDate.isEmpty()) {
        cachedLunarInfo = lunarInfo;
        return true;
      }
    }
  }
  
  DEBUG_PRINTLN("次备用API获取农历数据失败: " + secondaryBackupApiResponse.error);
  return false;
}

LunarInfo LunarManager::parseLunarData(const String& jsonData) {
  LunarInfo lunarInfo;
  
  // 解析JSON响应
  JsonDocument doc;
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

LunarInfo LunarManager::parseLunarDataBackup(const String& jsonData) {
  LunarInfo lunarInfo;
  
  // 解析JSON响应（备用API格式）
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  
  if (error) {
    DEBUG_PRINT("备用API JSON解析失败: ");
    DEBUG_PRINTLN(error.c_str());
    return lunarInfo;
  }
  
  // 检查响应状态
  if (doc.containsKey("success") && !doc["success"].as<bool>()) {
    DEBUG_PRINT("备用API请求失败: ");
    DEBUG_PRINTLN(doc["message"].as<String>());
    return lunarInfo;
  }
  
  // 解析农历日期信息
  if (doc.containsKey("lunar")) {
    JsonObject lunar = doc["lunar"];
    
    // 构建农历日期字符串
    lunarInfo.lunarMonth = lunar["lMonthCn"].as<String>();
    lunarInfo.lunarDay = lunar["dayCn"].as<String>();
    lunarInfo.lunarDate = lunarInfo.lunarMonth + lunarInfo.lunarDay;
    
    // 解析节气
    if (lunar.containsKey("term")) {
      lunarInfo.solarTerm = lunar["term"].as<String>();
    }
    
    // 解析节日信息
    if (lunar.containsKey("festival")) {
      String festivalName = lunar["festival"].as<String>();
      if (festivalName.length() > 0) {
        lunarInfo.festival.name = festivalName;
        lunarInfo.festival.type = "other";
      }
    }
    
    // 解析黄历信息
    if (lunar.containsKey("lYear")) {
      lunarInfo.lunarCalendar.yearGanZhi = lunar["lYear"].as<String>();
    }
    if (lunar.containsKey("lMonth")) {
      lunarInfo.lunarCalendar.monthGanZhi = lunar["lMonth"].as<String>();
    }
    if (lunar.containsKey("lDay")) {
      lunarInfo.lunarCalendar.dayGanZhi = lunar["lDay"].as<String>();
    }
    if (lunar.containsKey("Animal")) {
      lunarInfo.lunarCalendar.animal = lunar["Animal"].as<String>();
    }
    if (lunar.containsKey("yi")) {
      lunarInfo.lunarCalendar.yi = lunar["yi"].as<String>();
    }
    if (lunar.containsKey("ji")) {
      lunarInfo.lunarCalendar.ji = lunar["ji"].as<String>();
    }
    if (lunar.containsKey("chong")) {
      lunarInfo.lunarCalendar.xiangChong = lunar["chong"].as<String>();
    }
  }
  
  return lunarInfo;
}

LunarInfo LunarManager::parseLunarDataSecondaryBackup(const String& jsonData) {
  LunarInfo lunarInfo;
  
  // 解析JSON响应（次备用API格式，api.66mz8.com）
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, jsonData);
  
  if (error) {
    DEBUG_PRINT("次备用API JSON解析失败: ");
    DEBUG_PRINTLN(error.c_str());
    return lunarInfo;
  }
  
  // 检查响应状态
  if (doc["code"].as<int>() != 1) {
    DEBUG_PRINT("次备用API请求失败: ");
    DEBUG_PRINTLN(doc["msg"].as<String>());
    return lunarInfo;
  }
  
  // 解析农历日期信息
  JsonObject data = doc["data"];
  
  // 构建农历日期字符串
  lunarInfo.lunarMonth = data["lunar_month"].as<String>();
  lunarInfo.lunarDay = data["lunar_day"].as<String>();
  lunarInfo.lunarDate = lunarInfo.lunarMonth + lunarInfo.lunarDay;
  
  // 解析节气
  if (data.containsKey("jieqi")) {
    lunarInfo.solarTerm = data["jieqi"].as<String>();
  }
  
  // 解析节日信息
  if (data.containsKey("festival")) {
    String festivalName = data["festival"].as<String>();
    if (festivalName.length() > 0) {
      lunarInfo.festival.name = festivalName;
      lunarInfo.festival.type = "other";
    }
  }
  
  return lunarInfo;
}
