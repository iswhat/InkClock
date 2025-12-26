#include "weather_manager.h"
#include "wifi_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;
extern APIManager apiManager;

WeatherManager::WeatherManager() {
  // 初始化天气数据
  currentWeather.city = "未知城市";
  currentWeather.temp = 0.0;
  currentWeather.humidity = 0;
  currentWeather.condition = "未知";
  currentWeather.wind = "未知";
  currentWeather.tempMin = 0.0;
  currentWeather.tempMax = 0.0;
  currentWeather.pressure = 0;
  currentWeather.visibility = 0;
  currentWeather.sunrise = 0;
  currentWeather.sunset = 0;
  currentWeather.valid = false;
  
  // 初始化天气预报数据
  for (int i = 0; i < 5; i++) {
    forecastData[i].date = "";
    forecastData[i].tempDay = 0.0;
    forecastData[i].tempNight = 0.0;
    forecastData[i].condition = "未知";
    forecastData[i].wind = "未知";
    forecastData[i].humidity = 0;
  }
  
  lastUpdate = 0;
  dataUpdated = false;
}

WeatherManager::~WeatherManager() {
  // 清理资源
  client.stop();
}

void WeatherManager::init() {
  DEBUG_PRINTLN("初始化天气管理器...");
  
  // 初始化HTTPS客户端
  client.setInsecure(); // 禁用证书验证，简化开发
  
  DEBUG_PRINTLN("天气管理器初始化完成");
}

void WeatherManager::update() {
  // 只在WiFi连接时更新天气数据
  if (wifiManager.isConnected()) {
    fetchWeatherData();
  }
}

void WeatherManager::loop() {
  // 定期更新天气数据
  static unsigned long lastUpdateCheck = 0;
  if (millis() - lastUpdateCheck > WEATHER_UPDATE_INTERVAL) {
    lastUpdateCheck = millis();
    update();
  }
}

ForecastData WeatherManager::getForecastData(int index) {
  if (index >= 0 && index < 5) {
    return forecastData[index];
  }
  
  // 返回默认值
  ForecastData defaultForecast;
  defaultForecast.date = "";
  defaultForecast.tempDay = 0.0;
  defaultForecast.tempNight = 0.0;
  defaultForecast.condition = "未知";
  defaultForecast.wind = "未知";
  defaultForecast.humidity = 0;
  
  return defaultForecast;
}

bool WeatherManager::fetchWeatherData() {
  DEBUG_PRINTLN("获取天气数据...");
  
  // 构建API请求URL
  String url = String(WEATHER_API_URL) + 
               "?id=" + String(WEATHER_CITY_ID) + 
               "&appid=" + String(WEATHER_API_KEY) + 
               "&units=metric" + // 使用摄氏度
               "&lang=zh_cn";   // 使用中文
  
  // 使用API管理器发送HTTP请求
  ApiResponse apiResponse = apiManager.get(url, API_TYPE_WEATHER, 1800000); // 缓存30分钟
  
  // 检查请求结果
  if (apiResponse.status != API_STATUS_SUCCESS && apiResponse.status != API_STATUS_CACHED) {
    DEBUG_PRINTLN("获取天气数据失败: " + apiResponse.error);
    return false;
  }
  
  String response = apiResponse.response;
  if (response.isEmpty()) {
    DEBUG_PRINTLN("获取天气数据失败，响应为空");
    return false;
  }
  
  // 解析响应
  int jsonIndex = response.indexOf('{');
  if (jsonIndex == -1) {
    DEBUG_PRINTLN("无法找到JSON数据");
    return false;
  }
  
  String json = response.substring(jsonIndex);
  parseWeatherData(json);
  
  dataUpdated = true;
  lastUpdate = millis();
  
  DEBUG_PRINTLN("天气数据获取成功");
  return true;
}

void WeatherManager::parseWeatherData(String json) {
  // 解析JSON数据
  DynamicJsonDocument doc(16384); // 足够大的缓冲区
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    DEBUG_PRINT("JSON解析错误: ");
    DEBUG_PRINTLN(error.c_str());
    return;
  }
  
  // 解析当前天气数据
  JsonObject current = doc["list"][0];
  JsonObject main = current["main"];
  JsonObject wind = current["wind"];
  JsonArray weatherArray = current["weather"];
  JsonObject weather = weatherArray[0];
  
  currentWeather.city = doc["city"]["name"].as<String>();
  currentWeather.temp = main["temp"].as<float>();
  currentWeather.humidity = main["humidity"].as<int>();
  currentWeather.condition = weather["description"].as<String>();
  currentWeather.tempMin = main["temp_min"].as<float>();
  currentWeather.tempMax = main["temp_max"].as<float>();
  currentWeather.pressure = main["pressure"].as<int>();
  currentWeather.visibility = current["visibility"].as<int>();
  currentWeather.sunrise = doc["city"]["sunrise"].as<long>();
  currentWeather.sunset = doc["city"]["sunset"].as<long>();
  
  // 转换风力风向
  float windSpeed = wind["speed"].as<float>();
  float windDeg = wind["deg"].as<float>();
  currentWeather.wind = convertWindSpeed(windSpeed) + " " + convertWindDirection(windDeg);
  
  currentWeather.valid = true;
  
  // 解析未来5天天气预报
  for (int i = 0; i < 5; i++) {
    // 每8小时一个数据点，取每天的第0个数据点（当天）和每8小时的数据点
    int index = i * 8;
    if (index < doc["list"].size()) {
      JsonObject forecast = doc["list"][index];
      JsonObject forecastMain = forecast["main"];
      JsonObject forecastWind = forecast["wind"];
      JsonArray forecastWeatherArray = forecast["weather"];
      JsonObject forecastWeather = forecastWeatherArray[0];
      
      // 解析日期
      String dt_txt = forecast["dt_txt"].as<String>();
      forecastData[i].date = dt_txt.substring(0, 10); // YYYY-MM-DD
      
      forecastData[i].tempDay = forecastMain["temp"].as<float>();
      forecastData[i].tempNight = forecastMain["temp"].as<float>(); // 简化处理，实际应取夜间温度
      forecastData[i].condition = forecastWeather["description"].as<String>();
      
      // 转换风力风向
      float forecastWindSpeed = forecastWind["speed"].as<float>();
      float forecastWindDeg = forecastWind["deg"].as<float>();
      forecastData[i].wind = convertWindSpeed(forecastWindSpeed) + " " + convertWindDirection(forecastWindDeg);
      
      forecastData[i].humidity = forecastMain["humidity"].as<int>();
    }
  }
}

String WeatherManager::getWeatherIcon(String condition) {
  // 根据天气状况返回对应的图标文本
  if (condition.indexOf("晴") != -1) {
    return "☀️";
  } else if (condition.indexOf("云") != -1) {
    return "☁️";
  } else if (condition.indexOf("雨") != -1) {
    return "🌧️";
  } else if (condition.indexOf("雪") != -1) {
    return "❄️";
  } else if (condition.indexOf("雾") != -1 || condition.indexOf("霾") != -1) {
    return "🌫️";
  } else {
    return "🌈";
  }
}

String WeatherManager::convertWindSpeed(float speed) {
  // 将风速从m/s转换为级
  // 参考：https://baike.baidu.com/item/%E9%A3%8E%E7%BA%A7/439846
  if (speed < 0.3) {
    return "0级";
  } else if (speed < 1.6) {
    return "1级";
  } else if (speed < 3.4) {
    return "2级";
  } else if (speed < 5.5) {
    return "3级";
  } else if (speed < 8.0) {
    return "4级";
  } else if (speed < 10.8) {
    return "5级";
  } else if (speed < 13.9) {
    return "6级";
  } else if (speed < 17.2) {
    return "7级";
  } else if (speed < 20.8) {
    return "8级";
  } else if (speed < 24.5) {
    return "9级";
  } else if (speed < 28.5) {
    return "10级";
  } else if (speed < 32.7) {
    return "11级";
  } else {
    return "12级";
  }
}

String WeatherManager::convertWindDirection(float deg) {
  // 将风向角度转换为方向
  if (deg >= 337.5 || deg < 22.5) {
    return "北风";
  } else if (deg >= 22.5 && deg < 67.5) {
    return "东北风";
  } else if (deg >= 67.5 && deg < 112.5) {
    return "东风";
  } else if (deg >= 112.5 && deg < 157.5) {
    return "东南风";
  } else if (deg >= 157.5 && deg < 202.5) {
    return "南风";
  } else if (deg >= 202.5 && deg < 247.5) {
    return "西南风";
  } else if (deg >= 247.5 && deg < 292.5) {
    return "西风";
  } else if (deg >= 292.5 && deg < 337.5) {
    return "西北风";
  } else {
    return "未知";
  }
}