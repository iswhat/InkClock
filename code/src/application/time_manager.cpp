#include "time_manager.h"
#include "wifi_manager.h"
#include "../coresystem/event_bus.h"

// 外部全局对象
extern WiFiManager wifiManager;

// NTP配置
const int NTP_PACKET_SIZE = 48;
byte packetBuffer[NTP_PACKET_SIZE];
const unsigned int NTP_UPDATE_INTERVAL = 3600000; // 1小时
const int NTP_PORT = 123;

TimeManager::TimeManager() {
  // 初始化当前时间
  currentTime.year = 2023;
  currentTime.month = 1;
  currentTime.day = 1;
  currentTime.hour = 0;
  currentTime.minute = 0;
  currentTime.second = 0;
  currentTime.weekday = 0;
  currentTime.isLeapYear = false;
  currentTime.lunarDate = "正月初一";
  currentTime.solarTerm = "";
  
  timeUpdated = false;
  lastUpdate = 0;
  calculationPrecision = 3; // 默认最高精度
  
  // 订阅农历数据更新事件
  EVENT_SUBSCRIBE(EVENT_LUNAR_DATA_UPDATED, [this](EventType type, std::shared_ptr<EventData> data) {
    auto lunarData = std::dynamic_pointer_cast<LunarDataEventData>(data);
    if (lunarData) {
      if (lunarData->year == currentTime.year && 
          lunarData->month == currentTime.month && 
          lunarData->day == currentTime.day) {
        currentTime.lunarDate = lunarData->lunarDate;
        currentTime.solarTerm = lunarData->solarTerm;
      }
    }
  }, "TimeManager");
}

TimeManager::~TimeManager() {
  // 清理资源
}

void TimeManager::init() {
  DEBUG_PRINTLN("初始化时间管理器...");
  
  // 初始化UDP客户端
  ntpUDP.begin(NTP_PORT);
  
  DEBUG_PRINTLN("时间管理器初始化完成");
}

void TimeManager::update() {
  // 只在WiFi连接时更新时间
  if (wifiManager.isConnected()) {
    updateNTPTime();
  }
  
  // 更新本地时间
  if (timeUpdated) {
    // 使用millis()更新本地时间，减少NTP请求
    unsigned long now = millis();
    unsigned long elapsed = now - lastUpdate;
    
    if (elapsed >= 1000) {
      lastUpdate = now;
      
      // 更新秒数
      currentTime.second++;
      bool dayChanged = false;
      
      // 处理时间进位
      if (currentTime.second >= 60) {
        currentTime.second = 0;
        currentTime.minute++;
        
        if (currentTime.minute >= 60) {
          currentTime.minute = 0;
          currentTime.hour++;
          
          if (currentTime.hour >= 24) {
            currentTime.hour = 0;
            currentTime.day++;
            dayChanged = true;
            
            // 更新星期
            currentTime.weekday = (currentTime.weekday + 1) % 7;
            
            // 检查月份天数
            int daysInMonth = getDaysInMonth(currentTime.year, currentTime.month);
            if (currentTime.day > daysInMonth) {
              currentTime.day = 1;
              currentTime.month++;
              
              if (currentTime.month > 12) {
                currentTime.month = 1;
                currentTime.year++;
                currentTime.isLeapYear = isLeapYear(currentTime.year);
              }
            }
          }
        }
      }
      
      // 发布时间更新事件
      auto timeData = std::make_shared<TimeDataEventData>(currentTime);
      EVENT_PUBLISH(EVENT_TIME_UPDATED, timeData);
      
      // 如果日期改变，发布日期更新事件
      if (dayChanged) {
        EVENT_PUBLISH(EVENT_DATE_CHANGED, timeData);
      }
    }
  }
}

void TimeManager::loop() {
  // 定期更新NTP时间
  static unsigned long lastNTPUpdate = 0;
  if (millis() - lastNTPUpdate > NTP_UPDATE_INTERVAL) {
    lastNTPUpdate = millis();
    update();
  }
  
  // 每10秒更新一次计算精度
  static unsigned long lastPrecisionUpdate = 0;
  if (millis() - lastPrecisionUpdate > 10000) {
    lastPrecisionUpdate = millis();
    updateCalculationPrecision();
  }
}

String TimeManager::getTimeString() {
  // 格式化时间字符串，如"12:34:56"
  char buffer[9];
  snprintf(buffer, sizeof(buffer), "%02d:%02d:%02d", 
           currentTime.hour, currentTime.minute, currentTime.second);
  return String(buffer);
}

String TimeManager::getDateString() {
  // 格式化日期字符串，如"2023-01-01 星期日"
  char buffer[20];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %s", 
           currentTime.year, currentTime.month, currentTime.day, 
           getWeekdayName(currentTime.weekday).c_str());
  return String(buffer);
}

String TimeManager::getLunarDateString() {
  // 返回农历日期字符串
  return currentTime.lunarDate;
}

void TimeManager::setTime(int hour, int minute, int second) {
  currentTime.hour = hour;
  currentTime.minute = minute;
  currentTime.second = second;
  
  timeUpdated = true;
  lastUpdate = millis();
}

void TimeManager::setDate(int year, int month, int day) {
  currentTime.year = year;
  currentTime.month = month;
  currentTime.day = day;
  
  // 更新闰年标志
  currentTime.isLeapYear = isLeapYear(year);
  
  // 更新农历日期和节气
  currentTime.lunarDate = getLunarDate(year, month, day);
  currentTime.solarTerm = getSolarTerm(year, month, day);
  
  timeUpdated = true;
  lastUpdate = millis();
}

bool TimeManager::sendNTPRequest(const char* serverName) {
  DEBUG_PRINT("尝试连接NTP服务器: ");
  DEBUG_PRINTLN(serverName);
  
  // 发送NTP请求
  IPAddress ntpServerIP;
  if (!WiFi.hostByName(serverName, ntpServerIP)) {
    DEBUG_PRINTLN("无法解析NTP服务器域名");
    return false;
  }
  
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;             // Stratum, or type of clock
  packetBuffer[2] = 6;             // Polling Interval
  packetBuffer[3] = 0xEC;          // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;
  
  ntpUDP.beginPacket(ntpServerIP, NTP_PORT);
  ntpUDP.write(packetBuffer, NTP_PACKET_SIZE);
  ntpUDP.endPacket();
  
  // 等待响应
  delay(1000);
  
  if (ntpUDP.parsePacket()) {
    ntpUDP.read(packetBuffer, NTP_PACKET_SIZE);
    
    // 解析NTP时间
    uint32_t secsSince1900 = (uint32_t)packetBuffer[40] << 24 |
                              (uint32_t)packetBuffer[41] << 16 |
                              (uint32_t)packetBuffer[42] << 8  |
                              (uint32_t)packetBuffer[43];
    
    // NTP时间是从1900年开始，转换为Unix时间（从1970年开始）
    const uint32_t seventyYears = 2208988800UL;
    uint32_t unixTime = secsSince1900 - seventyYears;
    
    // 应用时区偏移
    unixTime += TIME_ZONE_OFFSET * 3600;
    
    // 解析时间
    parseNTPTime(unixTime);
    
    return true;
  } else {
    return false;
  }
}

void TimeManager::updateNTPTime() {
  DEBUG_PRINTLN("更新NTP时间...");
  
  // 先尝试主NTP服务器
  if (sendNTPRequest(NTP_SERVER)) {
    timeUpdated = true;
    lastUpdate = millis();
    DEBUG_PRINT("NTP时间更新成功: ");
    DEBUG_PRINTLN(getDateTimeString());
    return;
  }
  
  // 主NTP服务器失败，尝试备用NTP服务器
  DEBUG_PRINTLN("主NTP服务器失败，尝试备用NTP服务器");
  if (sendNTPRequest(NTP_SERVER_BACKUP)) {
    timeUpdated = true;
    lastUpdate = millis();
    DEBUG_PRINT("备用NTP服务器更新成功: ");
    DEBUG_PRINTLN(getDateTimeString());
    return;
  }
  
  // 所有NTP服务器都失败
  DEBUG_PRINTLN("所有NTP服务器都失败");
}

void TimeManager::parseNTPTime(uint32_t unixTime) {
  // 解析Unix时间为年月日时分秒
  // 参考：https://github.com/arduino-libraries/NTPClient/blob/master/NTPClient.cpp
  
  uint32_t seconds = unixTime % 60;
  uint32_t minutes = (unixTime / 60) % 60;
  uint32_t hours = (unixTime / 3600) % 24;
  
  uint32_t days = unixTime / 86400;
  uint32_t years = 1970;
  
  // 计算年份
  while (true) {
    int daysInYear = isLeapYear(years) ? 366 : 365;
    if (days < daysInYear) {
      break;
    }
    days -= daysInYear;
    years++;
  }
  
  // 计算月份
  int months[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  if (isLeapYear(years)) {
    months[1] = 29;
  }
  
  int month = 0;
  while (days >= months[month]) {
    days -= months[month];
    month++;
  }
  month++;
  
  // 计算星期几（1970-01-01是星期四，即4）
  int weekday = (days + 4) % 7;
  
  // 更新时间数据
  currentTime.year = years;
  currentTime.month = month;
  currentTime.day = days + 1;
  currentTime.hour = hours;
  currentTime.minute = minutes;
  currentTime.second = seconds;
  currentTime.weekday = weekday;
  currentTime.isLeapYear = isLeapYear(years);
  
  // 请求农历数据
  auto requestData = std::make_shared<LunarRequestEventData>(years, month, days + 1);
  EVENT_PUBLISH(EVENT_LUNAR_DATA_REQUESTED, requestData);
  
  // 发布时间更新事件
  auto timeData = std::make_shared<TimeDataEventData>(currentTime);
  EVENT_PUBLISH(EVENT_TIME_UPDATED, timeData);
  EVENT_PUBLISH(EVENT_DATE_CHANGED, timeData);
}

String TimeManager::getWeekdayName(int weekday) {
  // 获取星期名称
  const char* weekdays[] = {"星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六"};
  if (weekday >= 0 && weekday < 7) {
    return String(weekdays[weekday]);
  }
  return "未知";
}



bool TimeManager::isLeapYear(int year) {
  // 判断是否是闰年
  return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
}

int TimeManager::getDaysInMonth(int year, int month) {
  // 获取月份天数
  int days[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
  
  if (month == 2 && isLeapYear(year)) {
    return 29;
  }
  
  if (month >= 1 && month <= 12) {
    return days[month - 1];
  }
  
  return 30; // 默认值
}

// 辅助函数：获取完整的日期时间字符串
String TimeManager::getDateTimeString() {
  char buffer[30];
  snprintf(buffer, sizeof(buffer), "%04d-%02d-%02d %02d:%02d:%02d", 
           currentTime.year, currentTime.month, currentTime.day, 
           currentTime.hour, currentTime.minute, currentTime.second);
  return String(buffer);
}

// 获取系统负载等级
int TimeManager::getSystemLoadLevel() {
  // 获取核心系统实例
  CoreSystem* coreSystem = CoreSystem::getInstance();
  
  // 获取内存信息
  size_t freeHeap, minFreeHeap;
  coreSystem->getMemoryInfo(freeHeap, minFreeHeap);
  
  // 计算内存使用率
  size_t totalHeap = minFreeHeap * 10; // 估算总堆内存
  size_t usedHeap = totalHeap - freeHeap;
  int memoryUsage = (usedHeap * 100) / totalHeap;
  
  // 获取CPU频率
  uint32_t cpuFreq = coreSystem->getCpuFrequencyMhz();
  
  // 获取电池电量
  int batteryLevel = coreSystem->getBatteryPercentage();
  
  // 综合计算负载等级
  int loadLevel = 0;
  
  if (memoryUsage > 80) {
    loadLevel += 3;
  } else if (memoryUsage > 60) {
    loadLevel += 2;
  } else if (memoryUsage > 40) {
    loadLevel += 1;
  }
  
  if (cpuFreq < 120) {
    loadLevel += 2;
  } else if (cpuFreq < 180) {
    loadLevel += 1;
  }
  
  if (batteryLevel < 20) {
    loadLevel += 2;
  } else if (batteryLevel < 50) {
    loadLevel += 1;
  }
  
  // 限制负载等级在0-5之间
  if (loadLevel > 5) loadLevel = 5;
  
  return loadLevel;
}

// 更新计算精度
void TimeManager::updateCalculationPrecision() {
  int loadLevel = getSystemLoadLevel();
  
  // 根据负载等级调整计算精度
  switch (loadLevel) {
    case 0: // 低负载
    case 1:
      calculationPrecision = 3; // 最高精度
      break;
    case 2: // 中低负载
      calculationPrecision = 2; // 中等精度
      break;
    case 3: // 中等负载
      calculationPrecision = 2; // 中等精度
      break;
    case 4: // 中高负载
    case 5: // 高负载
      calculationPrecision = 1; // 低精度
      break;
  }
}