#include "weather_manager.h"
#include "wifi_manager.h"
#include "geo_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;
extern APIManager apiManager;
extern GeoManager geoManager;

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
  
  // 初始化完成，现在使用API管理器处理HTTP请求
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
  
  // 获取城市信息
  String cityName = geoManager.getCityName();
  String cityId = geoManager.getCityId();
  float latitude = geoManager.getLatitude();
  float longitude = geoManager.getLongitude();
  
  // 构建主API请求URL (wttr.in - 公共免密钥)
  String url = String(WEATHER_API_URL) + cityName + "?format=j1"; // 使用JSON格式
  
  // 使用API管理器发送HTTP请求
  ApiResponse apiResponse = apiManager.get(url, API_TYPE_WEATHER, 1800000); // 缓存30分钟
  
  // 检查请求结果
  if (apiResponse.status == API_STATUS_SUCCESS || apiResponse.status == API_STATUS_CACHED) {
    String response = apiResponse.response;
    if (!response.isEmpty()) {
      // 解析响应
      int jsonIndex = response.indexOf('{');
      if (jsonIndex != -1) {
        String json = response.substring(jsonIndex);
        if (parseWeatherData(json)) {
          dataUpdated = true;
          lastUpdate = millis();
          DEBUG_PRINTLN("使用主API获取天气数据成功");
          return true;
        }
      }
    }
    DEBUG_PRINTLN("主API获取天气数据失败，尝试使用备用API");
  } else {
    DEBUG_PRINTLN("主API获取天气数据失败: " + apiResponse.error + "，尝试使用备用API");
  }
  
  // 尝试使用备用API (open-meteo.com - 公共免密钥)
  DEBUG_PRINTLN("尝试使用备用天气API");
  String backupUrl = String(WEATHER_API_URL_BACKUP) + 
                     "?latitude=" + String(latitude) + 
                     "&longitude=" + String(longitude) + 
                     "&current_weather=true" + 
                     "&daily=temperature_2m_max,temperature_2m_min,relative_humidity_2m_max,wind_speed_10m_max" + 
                     "&timezone=Asia/Shanghai" + 
                     "&forecast_days=5";
  
  ApiResponse backupApiResponse = apiManager.get(backupUrl, API_TYPE_WEATHER, 1800000);
  
  if (backupApiResponse.status == API_STATUS_SUCCESS || backupApiResponse.status == API_STATUS_CACHED) {
    String backupResponse = backupApiResponse.response;
    if (!backupResponse.isEmpty()) {
      if (parseWeatherDataBackup(backupResponse)) {
        dataUpdated = true;
        lastUpdate = millis();
        DEBUG_PRINTLN("使用备用API获取天气数据成功");
        return true;
      }
    }
    DEBUG_PRINTLN("备用API获取天气数据失败，尝试使用次备用API");
  } else {
    DEBUG_PRINTLN("备用API获取天气数据失败: " + backupApiResponse.error + "，尝试使用次备用API");
  }
  
  // 尝试使用次备用API (OpenWeatherMap - 需要密钥)
  DEBUG_PRINTLN("尝试使用次备用天气API (OpenWeatherMap)");
  String secondaryBackupUrl = String(WEATHER_API_URL_SECONDARY_BACKUP) + 
                             "?id=" + cityId + 
                             "&appid=" + String(WEATHER_API_KEY) + 
                             "&units=metric" + // 使用摄氏度
                             "&lang=zh_cn";   // 使用中文
  
  ApiResponse secondaryBackupApiResponse = apiManager.get(secondaryBackupUrl, API_TYPE_WEATHER, 1800000);
  
  if (secondaryBackupApiResponse.status == API_STATUS_SUCCESS || secondaryBackupApiResponse.status == API_STATUS_CACHED) {
    String secondaryBackupResponse = secondaryBackupApiResponse.response;
    if (!secondaryBackupResponse.isEmpty()) {
      int jsonIndex = secondaryBackupResponse.indexOf('{');
      if (jsonIndex != -1) {
        String json = secondaryBackupResponse.substring(jsonIndex);
        if (parseWeatherDataSecondaryBackup(json)) {
          dataUpdated = true;
          lastUpdate = millis();
          DEBUG_PRINTLN("使用次备用API获取天气数据成功");
          return true;
        }
      }
    }
    DEBUG_PRINTLN("次备用API获取天气数据失败，尝试使用第四次备用API");
  } else {
    DEBUG_PRINTLN("次备用API获取天气数据失败: " + secondaryBackupApiResponse.error + "，尝试使用第四次备用API");
  }
  
  // 尝试使用第四次备用API (WeatherAPI - 需要密钥)
  DEBUG_PRINTLN("尝试使用第四次备用天气API (WeatherAPI)");
  String tertiaryBackupUrl = String(WEATHER_API_URL_TERTIARY_BACKUP) + 
                             "?q=" + cityId + 
                             "&key=" + String(WEATHER_API_KEY_BACKUP) + 
                             "&days=5" + // 获取5天天气预报
                             "&aqi=no" + // 不包含空气质量
                             "&alerts=no" + // 不包含预警信息
                             "&lang=zh"; // 使用中文
  
  ApiResponse tertiaryBackupApiResponse = apiManager.get(tertiaryBackupUrl, API_TYPE_WEATHER, 1800000);
  
  if (tertiaryBackupApiResponse.status == API_STATUS_SUCCESS || tertiaryBackupApiResponse.status == API_STATUS_CACHED) {
    String tertiaryBackupResponse = tertiaryBackupApiResponse.response;
    if (!tertiaryBackupResponse.isEmpty()) {
      int jsonIndex = tertiaryBackupResponse.indexOf('{');
      if (jsonIndex != -1) {
        String json = tertiaryBackupResponse.substring(jsonIndex);
        if (parseWeatherDataTertiaryBackup(json)) {
          dataUpdated = true;
          lastUpdate = millis();
          DEBUG_PRINTLN("使用第四次备用API获取天气数据成功");
          return true;
        }
      }
    }
  } else {
    DEBUG_PRINTLN("第四次备用API获取天气数据失败: " + tertiaryBackupApiResponse.error);
  }
  
  DEBUG_PRINTLN("所有API获取天气数据失败");
  return false;
}

bool WeatherManager::parseWeatherData(String json) {
  // 解析JSON数据 (wttr.in格式)
  DynamicJsonDocument doc(16384); // 足够大的缓冲区
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    DEBUG_PRINT("JSON解析错误: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // 解析当前天气数据
  JsonArray currentCondition = doc["current_condition"];
  if (currentCondition.size() == 0) {
    DEBUG_PRINTLN("未找到当前天气数据");
    return false;
  }
  
  JsonObject current = currentCondition[0];
  currentWeather.city = doc["nearest_area"][0]["areaName"][0]["value"].as<String>();
  currentWeather.temp = current["temp_C"].as<float>();
  currentWeather.humidity = current["humidity"].as<int>();
  currentWeather.condition = current["weatherDesc"][0]["value"].as<String>();
  currentWeather.tempMin = current["temp_C"].as<float>(); // wttr.in当前没有直接提供最低/最高温度
  currentWeather.tempMax = current["temp_C"].as<float>();
  currentWeather.pressure = current["pressure"].as<int>();
  currentWeather.visibility = current["visibility"].as<int>() * 1000; // 转换为米
  
  // 转换风力风向
  float windSpeed = current["windspeedKmph"].as<float>() / 3.6; // 转换为m/s
  float windDeg = current["winddirDegree"].as<float>();
  currentWeather.wind = convertWindSpeed(windSpeed) + " " + convertWindDirection(windDeg);
  
  // 解析5天天气预报
  JsonArray weatherArray = doc["weather"];
  for (int i = 0; i < weatherArray.size() && i < 5; i++) {
    JsonObject day = weatherArray[i];
    forecastData[i].date = day["date"].as<String>();
    forecastData[i].tempDay = day["maxtempC"].as<float>();
    forecastData[i].tempNight = day["mintempC"].as<float>();
    forecastData[i].condition = day["hourly"][0]["weatherDesc"][0]["value"].as<String>();
    
    // 转换风力风向
    float forecastWindSpeed = day["hourly"][0]["windspeedKmph"].as<float>() / 3.6;
    float forecastWindDeg = day["hourly"][0]["winddirDegree"].as<float>();
    forecastData[i].wind = convertWindSpeed(forecastWindSpeed) + " " + convertWindDirection(forecastWindDeg);
    forecastData[i].humidity = day["hourly"][0]["humidity"].as<int>();
  }
  
  currentWeather.valid = true;
  return true;
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

bool WeatherManager::parseWeatherDataBackup(String json) {
  // 解析备用天气API的数据 (open-meteo.com格式)
  DynamicJsonDocument doc(16384); // 足够大的缓冲区
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    DEBUG_PRINT("备用天气API JSON解析错误: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // 解析当前天气数据
  JsonObject current = doc["current_weather"];
  if (!current) {
    DEBUG_PRINTLN("未找到当前天气数据");
    return false;
  }
  
  currentWeather.city = geoManager.getCityName(); // 使用已知的城市名称
  currentWeather.temp = current["temperature"].as<float>();
  currentWeather.humidity = 0; // open-meteo当前天气不提供湿度
  currentWeather.condition = "未知";
  currentWeather.tempMin = 0.0; // 初始值
  currentWeather.tempMax = 0.0;
  currentWeather.pressure = 0;
  currentWeather.visibility = 0;
  
  // 转换风力风向
  float windSpeed = current["windspeed"].as<float>();
  float windDeg = current["winddirection"].as<float>();
  currentWeather.wind = convertWindSpeed(windSpeed) + " " + convertWindDirection(windDeg);
  
  // 解析5天天气预报
  JsonObject daily = doc["daily"];
  JsonArray time = daily["time"];
  JsonArray tempMax = daily["temperature_2m_max"];
  JsonArray tempMin = daily["temperature_2m_min"];
  JsonArray humidityMax = daily["relative_humidity_2m_max"];
  JsonArray windSpeedMax = daily["wind_speed_10m_max"];
  
  for (int i = 0; i < time.size() && i < 5; i++) {
    forecastData[i].date = time[i].as<String>();
    forecastData[i].tempDay = tempMax[i].as<float>();
    forecastData[i].tempNight = tempMin[i].as<float>();
    forecastData[i].condition = "未知";
    forecastData[i].wind = convertWindSpeed(windSpeedMax[i].as<float>()) + " 未知风向";
    forecastData[i].humidity = humidityMax[i].as<int>();
    
    // 保存最高和最低温度到当前天气
    if (i == 0) {
      currentWeather.tempMin = tempMin[i].as<float>();
      currentWeather.tempMax = tempMax[i].as<float>();
    }
  }
  
  currentWeather.valid = true;
  return true;
}

bool WeatherManager::parseWeatherDataSecondaryBackup(String json) {
  // 解析次备用天气API的数据 (OpenWeatherMap格式)
  DynamicJsonDocument doc(16384); // 足够大的缓冲区
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    DEBUG_PRINT("次备用天气API JSON解析错误: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // 解析当前天气数据
  JsonObject current = doc["list"][0];
  JsonObject main = current["main"];
  JsonObject wind = current["wind"];
  JsonArray weatherArray = current["weather"];
  if (weatherArray.size() == 0) {
    DEBUG_PRINTLN("未找到天气状况数据");
    return false;
  }
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
  
  // 解析未来5天天气预报
  for (int i = 0; i < 5; i++) {
    // 每8小时一个数据点，取每天的第0个数据点（当天）和每8小时的数据点
    int index = i * 8;
    if (index < doc["list"].size()) {
      JsonObject forecast = doc["list"][index];
      JsonObject forecastMain = forecast["main"];
      JsonObject forecastWind = forecast["wind"];
      JsonArray forecastWeatherArray = forecast["weather"];
      if (forecastWeatherArray.size() == 0) continue;
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
  
  currentWeather.valid = true;
  return true;
}

bool WeatherManager::parseWeatherDataTertiaryBackup(String json) {
  // 解析第四次备用天气API的数据 (WeatherAPI格式)
  DynamicJsonDocument doc(16384); // 足够大的缓冲区
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    DEBUG_PRINT("第四次备用天气API JSON解析错误: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // 解析当前天气数据
  JsonObject location = doc["location"];
  JsonObject current = doc["current"];
  JsonObject condition = current["condition"];
  
  currentWeather.city = location["name"].as<String>();
  currentWeather.temp = current["temp_c"].as<float>();
  currentWeather.humidity = current["humidity"].as<int>();
  currentWeather.condition = condition["text"].as<String>();
  
  // 解析风向
  float windDeg = 0.0;
  if (current.containsKey("wind_degree")) {
    windDeg = current["wind_degree"].as<float>();
  }
  currentWeather.wind = convertWindSpeed(current["wind_kph"].as<float>()) + " " + convertWindDirection(windDeg);
  
  // 解析5天天气预报
  JsonArray forecastDays = doc["forecast"]["forecastday"];
  for (int i = 0; i < forecastDays.size() && i < 5; i++) {
    JsonObject forecastDay = forecastDays[i];
    JsonObject day = forecastDay["day"];
    JsonObject dayCondition = day["condition"];
    
    forecastData[i].date = forecastDay["date"].as<String>();
    forecastData[i].tempDay = day["maxtemp_c"].as<float>();
    forecastData[i].tempNight = day["mintemp_c"].as<float>();
    forecastData[i].condition = dayCondition["text"].as<String>();
    
    // 解析风向
    float forecastWindDeg = 0.0;
    forecastData[i].wind = convertWindSpeed(day["maxwind_kph"].as<float>()) + " " + convertWindDirection(forecastWindDeg);
    forecastData[i].humidity = day["avghumidity"].as<int>();
  }
  
  currentWeather.valid = true;
  return true;
}