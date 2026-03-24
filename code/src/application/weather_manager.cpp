#include "weather_manager.h"
#include "application/wifi_manager.h"
#include "application/geo_manager.h"

// 包含依赖注入容器
#include "coresystem/dependency_injection.h"

WeatherManager::WeatherManager() {
  // 初始化天气数据
  currentWeather.temp = 0.0;
  currentWeather.feelsLike = 0.0;
  currentWeather.humidity = 0;
  currentWeather.pressure = 0;
  currentWeather.windSpeed = 0;
  currentWeather.condition = "未知";
  currentWeather.weatherIcon = "";
  currentWeather.uvIndex = 0;
  currentWeather.uvIndexLevel = "未知";
  currentWeather.visibility = 0;
  currentWeather.airQuality = 0.0;
  currentWeather.airQualityLevel = "未知";
  currentWeather.aqi = 0.0;
  currentWeather.sunrise = 0;
  currentWeather.sunset = 0;
  
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
  dataRequested = false;
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
  auto wifiManager = DependencyInjectionContainer::getInstance()->getWiFiManager();
  if (wifiManager && wifiManager->isConnected()) {
    if (!fetchWeatherData()) {
      // 如果获取天气数据失败，尝试使用缓存数据
      if (!hasValidData()) {
        // 如果没有有效的缓存数据，设置默认值
        setDefaultWeatherData();
      }
    }
  } else {
    DEBUG_PRINTLN("WiFi未连接，无法更新天气数据");
  }
}

void WeatherManager::loop() {
  // 惰性计算：只在数据被请求且需要更新时才更新
  if (dataRequested && isDataStale()) {
    update();
  }
  
  // 定期更新天气数据（即使没有被请求，也按设定间隔更新一次，确保数据不会太旧）
  static unsigned long lastUpdateCheck = 0;
  if (millis() - lastUpdateCheck > WEATHER_UPDATE_INTERVAL) {
    lastUpdateCheck = millis();
    update();
  }
}



bool WeatherManager::fetchWeatherData() {
  DEBUG_PRINTLN("获取天气数据...");
  
  // 获取城市信息
  auto geoManager = DependencyInjectionContainer::getInstance()->getGeoManager();
  String cityName = "北京"; // 默认城市
  String cityId = "101010100"; // 默认城市ID
  float latitude = 39.9042; // 默认纬度
  float longitude = 116.4074; // 默认经度
  
  if (geoManager) {
    cityName = geoManager->getCityName();
    cityId = geoManager->getCityId();
    latitude = geoManager->getLatitude();
    longitude = geoManager->getLongitude();
  }
  
  // 构建主API请求URL (wttr.in - 公共免密钥)
  String url = String(WEATHER_API_URL) + cityName + "?format=j1"; // 使用JSON格式
  
  // 使用API管理器发送HTTP请求
  auto apiManager = DependencyInjectionContainer::getInstance()->getAPIManager();
  if (!apiManager) {
    DEBUG_PRINTLN("无法获取API管理器");
    return false;
  }
  ApiResponse apiResponse = apiManager->get(url, API_TYPE_WEATHER, 1800000); // 缓存30分钟
  
  // 检查请求结果
  if (apiResponse.status == API_STATUS_SUCCESS || apiResponse.status == API_STATUS_CACHED) {
    String response = apiResponse.response;
    if (!response.isEmpty()) {
      // 解析响应
      int jsonIndex = response.indexOf('{');
      if (jsonIndex != -1) {
        String json = response.substring(jsonIndex);
        parseWeatherData(json);
        dataUpdated = true;
        lastUpdate = millis();
        DEBUG_PRINTLN("使用主API获取天气数据成功");
        return true;
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
  
  ApiResponse backupApiResponse = apiManager->get(backupUrl, API_TYPE_WEATHER, 1800000);
  
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
  
  // 次备用API和第四次备用API暂时禁用，因为缺少必要的方法声明
  DEBUG_PRINTLN("次备用API和第四次备用API暂时禁用");
  
  DEBUG_PRINTLN("所有API获取天气数据失败");
  return false;
}

void WeatherManager::parseWeatherData(String json) {
  // 解析 JSON 数据 (wttr.in 格式)
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    DEBUG_PRINT("JSON 解析错误：");
    DEBUG_PRINTLN(error.c_str());
    // 设置默认值，避免后续使用空数据
    currentWeather.city = "未知";
    currentWeather.temp = 0.0f;
    currentWeather.humidity = 0;
    currentWeather.condition = "未知";
    currentWeather.feelsLike = 0.0f;
    currentWeather.pressure = 0;
    currentWeather.visibility = 0;
    currentWeather.airQuality = 0;
    currentWeather.airQualityLevel = "未知";
    return;
  }
  
  // 解析当前天气数据
  JsonArray currentCondition = doc["current_condition"];
  if (currentCondition.size() == 0) {
    DEBUG_PRINTLN("未找到当前天气数据");
    return;
  }
  
  JsonObject current = currentCondition[0];
  currentWeather.city = doc["nearest_area"][0]["areaName"][0]["value"].as<String>();
  currentWeather.temp = current["temp_C"].as<float>();
  currentWeather.humidity = current["humidity"].as<int>();
  currentWeather.condition = current["weatherDesc"][0]["value"].as<String>();
  currentWeather.temp = current["temp_C"].as<float>();
  currentWeather.feelsLike = current["feelslike_C"].as<float>();
  currentWeather.pressure = current["pressure"].as<int>();
  currentWeather.visibility = current["visibility"].as<int>() * 1000; // 转换为米
  
  // 解析空气质量数据
  if (current.containsKey("air_quality")) {
    JsonObject airQuality = current["air_quality"];
    int aqi = airQuality["us-epa-index"].as<int>();
    currentWeather.airQuality = aqi;
    if (aqi == 1) {
      currentWeather.airQualityLevel = "优";
    } else if (aqi == 2) {
      currentWeather.airQualityLevel = "良";
    } else if (aqi == 3) {
      currentWeather.airQualityLevel = "轻度污染";
    } else if (aqi == 4) {
      currentWeather.airQualityLevel = "中度污染";
    } else if (aqi == 5) {
      currentWeather.airQualityLevel = "重度污染";
    } else if (aqi == 6) {
      currentWeather.airQualityLevel = "严重污染";
    } else {
      currentWeather.airQualityLevel = "未知";
    }
  } else {
    currentWeather.airQuality = 0;
    currentWeather.airQualityLevel = "未知";
  }
  
  // 解析紫外线指数数据
  if (current.containsKey("uvIndex")) {
    currentWeather.uvIndex = current["uvIndex"].as<float>();
    if (currentWeather.uvIndex <= 2) {
      currentWeather.uvIndexLevel = "低"; 
    } else if (currentWeather.uvIndex <= 5) {
      currentWeather.uvIndexLevel = "中等"; 
    } else if (currentWeather.uvIndex <= 7) {
      currentWeather.uvIndexLevel = "高"; 
    } else if (currentWeather.uvIndex <= 10) {
      currentWeather.uvIndexLevel = "很高"; 
    } else {
      currentWeather.uvIndexLevel = "极高"; 
    }
  } else {
    currentWeather.uvIndex = 0;
    currentWeather.uvIndexLevel = "未知";
  }
  
  // 转换风力风向
  float windSpeed = current["windspeedKmph"].as<float>() / 3.6; // 转换为m/s
  float windDeg = current["winddirDegree"].as<float>();
  currentWeather.windSpeed = windSpeed;
  
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
    
    if (i == 0) {
      currentWeather.tempHigh = forecastData[i].tempDay;
      currentWeather.tempLow = forecastData[i].tempNight;
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

bool WeatherManager::hasValidData() {
  // 检查是否有有效的天气数据
  return currentWeather.temp != 0.0 || 
         currentWeather.humidity != 0 || 
         !currentWeather.condition.equals("未知");
}

void WeatherManager::setDefaultWeatherData() {
  // 设置默认天气数据
  currentWeather.temp = 25.0;
  currentWeather.feelsLike = 25.0;
  currentWeather.humidity = 60;
  currentWeather.pressure = 1013;
  currentWeather.windSpeed = 3.0;
  currentWeather.condition = "晴";
  currentWeather.weatherIcon = "☀️";
  currentWeather.uvIndex = 5;
  currentWeather.uvIndexLevel = "中等";
  currentWeather.visibility = 10000;
  currentWeather.airQuality = 50;
  currentWeather.airQualityLevel = "良";
  currentWeather.aqi = 50;
  
  DEBUG_PRINTLN("使用默认天气数据");
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

// 惰性计算：获取天气数据
WeatherData WeatherManager::getWeatherData() {
  // 如果数据需要更新，先更新数据
  if (isDataStale()) {
    update();
  }
  
  // 标记数据已被请求
  dataRequested = true;
  
  return currentWeather;
}

// 惰性计算：获取天气预报数据
ForecastData WeatherManager::getForecastData(int index) {
  // 如果数据需要更新，先更新数据
  if (isDataStale()) {
    update();
  }
  
  // 标记数据已被请求
  dataRequested = true;
  
  // 检查索引是否有效
  if (index >= 0 && index < 5) {
    return forecastData[index];
  }
  
  // 返回默认数据
  ForecastData defaultData;
  defaultData.date = "";
  defaultData.tempDay = 0.0;
  defaultData.tempNight = 0.0;
  defaultData.condition = "未知";
  defaultData.wind = "未知";
  defaultData.humidity = 0;
  return defaultData;
}

// 强制更新数据
void WeatherManager::forceUpdate() {
  update();
}

// 检查数据是否需要更新
bool WeatherManager::isDataStale() {
  // 数据从未更新过
  if (lastUpdate == 0) {
    return true;
  }
  
  // 数据超过30分钟未更新
  unsigned long currentTime = millis();
  if (currentTime - lastUpdate > 30 * 60 * 1000) {
    return true;
  }
  

  
  return false;
}

bool WeatherManager::parseWeatherDataBackup(String json) {
  // 解析备用天气API的数据 (open-meteo.com格式)
  JsonDocument doc;
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
  
  // 使用已知的城市名称
  auto geoManager = DependencyInjectionContainer::getInstance()->getGeoManager();
  if (geoManager) {
    currentWeather.city = geoManager->getCityName();
  } else {
    currentWeather.city = "北京";
  }
  currentWeather.temp = current["temperature"].as<float>();
  currentWeather.humidity = 0; // open-meteo当前天气不提供湿度
  currentWeather.condition = "未知";

  currentWeather.pressure = 0;
  currentWeather.visibility = 0;
  currentWeather.airQuality = 0;
  currentWeather.airQualityLevel = "未知";
  currentWeather.uvIndex = 0;
  currentWeather.uvIndexLevel = "未知";
  
  // 转换风力风向
  float windSpeed = current["windspeed"].as<float>();
  float windDeg = current["winddirection"].as<float>();
  currentWeather.windSpeed = windSpeed;
  
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
    
    if (i == 0) {
      currentWeather.tempHigh = forecastData[i].tempDay;
      currentWeather.tempLow = forecastData[i].tempNight;
    }
  }
  

  return true;
}