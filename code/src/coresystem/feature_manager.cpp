#include "feature_manager.h"

// 静态实例初始化
FeatureManager* FeatureManager::instance = nullptr;

// FeatureManager 构造函数
FeatureManager::FeatureManager() {
  initialized = false;
  lastEvaluationTime = 0;
}

// FeatureManager 单例获取
FeatureManager* FeatureManager::getInstance() {
  if (instance == nullptr) {
    instance = new FeatureManager();
  }
  return instance;
}

// 初始化默认功能配置
void FeatureManager::initDefaultFeatures() {
  // 核心功能
  FeatureConfig timeFeature = {
    "Time",
    "时间管理和同步",
    FEATURE_LEVEL_FULL,
    FEATURE_LEVEL_FULL,
    true,
    5,
    1,
    0,
    {},
    0
  };
  timeFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"syncInterval", "3600000"}, {"ntpServers", "1"}};
  timeFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"syncInterval", "1800000"}, {"ntpServers", "2"}};
  timeFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"syncInterval", "600000"}, {"ntpServers", "3"}};
  timeFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"syncInterval", "300000"}, {"ntpServers", "3"}};
  timeFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"syncInterval", "60000"}, {"ntpServers", "4"}};
  featureConfigs["Time"] = timeFeature;

  FeatureConfig displayFeature = {
    "Display",
    "显示管理",
    FEATURE_LEVEL_FULL,
    FEATURE_LEVEL_FULL,
    true,
    10,
    2,
    0,
    {},
    0
  };
  displayFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"refreshInterval", "60000"}, {"animationEnabled", "false"}, {"complexEffects", "false"}};
  displayFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"refreshInterval", "30000"}, {"animationEnabled", "false"}, {"complexEffects", "false"}};
  displayFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"refreshInterval", "15000"}, {"animationEnabled", "true"}, {"complexEffects", "false"}};
  displayFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"refreshInterval", "10000"}, {"animationEnabled", "true"}, {"complexEffects", "true"}};
  displayFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"refreshInterval", "5000"}, {"animationEnabled", "true"}, {"complexEffects", "true"}};
  featureConfigs["Display"] = displayFeature;

  FeatureConfig powerFeature = {
    "Power",
    "电源管理",
    FEATURE_LEVEL_FULL,
    FEATURE_LEVEL_FULL,
    true,
    3,
    1,
    0,
    {},
    0
  };
  powerFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"sleepEnabled", "true"}, {"sleepTimeout", "600000"}, {"deepSleepEnabled", "true"}};
  powerFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"sleepEnabled", "true"}, {"sleepTimeout", "300000"}, {"deepSleepEnabled", "true"}};
  powerFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"sleepEnabled", "true"}, {"sleepTimeout", "180000"}, {"deepSleepEnabled", "true"}};
  powerFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"sleepEnabled", "true"}, {"sleepTimeout", "120000"}, {"deepSleepEnabled", "true"}};
  powerFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"sleepEnabled", "true"}, {"sleepTimeout", "60000"}, {"deepSleepEnabled", "true"}};
  featureConfigs["Power"] = powerFeature;

  // 重要功能
  FeatureConfig wifiFeature = {
    "WiFi",
    "WiFi连接和管理",
    FEATURE_LEVEL_STANDARD,
    FEATURE_LEVEL_STANDARD,
    false,
    15,
    5,
    30,
    {},
    0
  };
  wifiFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"autoReconnect", "true"}, {"scanInterval", "300000"}, {"powerSaveMode", "true"}};
  wifiFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"autoReconnect", "true"}, {"scanInterval", "180000"}, {"powerSaveMode", "true"}};
  wifiFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"autoReconnect", "true"}, {"scanInterval", "60000"}, {"powerSaveMode", "false"}};
  wifiFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"autoReconnect", "true"}, {"scanInterval", "30000"}, {"powerSaveMode", "false"}, {"fastConnect", "true"}};
  wifiFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"autoReconnect", "true"}, {"scanInterval", "15000"}, {"powerSaveMode", "false"}, {"fastConnect", "true"}, {"roamingEnabled", "true"}};
  featureConfigs["WiFi"] = wifiFeature;

  FeatureConfig weatherFeature = {
    "Weather",
    "天气数据获取和显示",
    FEATURE_LEVEL_STANDARD,
    FEATURE_LEVEL_STANDARD,
    false,
    10,
    5,
    40,
    {},
    0
  };
  weatherFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"updateInterval", "3600000"}, {"forecastDays", "1"}, {"detailedInfo", "false"}};
  weatherFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"updateInterval", "1800000"}, {"forecastDays", "2"}, {"detailedInfo", "false"}};
  weatherFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"updateInterval", "600000"}, {"forecastDays", "3"}, {"detailedInfo", "true"}};
  weatherFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"updateInterval", "300000"}, {"forecastDays", "5"}, {"detailedInfo", "true"}};
  weatherFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"updateInterval", "180000"}, {"forecastDays", "7"}, {"detailedInfo", "true"}, {"hourlyForecast", "true"}};
  featureConfigs["Weather"] = weatherFeature;

  // 可选功能
  FeatureConfig sensorFeature = {
    "Sensor",
    "传感器数据采集",
    FEATURE_LEVEL_STANDARD,
    FEATURE_LEVEL_STANDARD,
    false,
    8,
    3,
    35,
    {},
    0
  };
  sensorFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"updateInterval", "60000"}, {"sensorCount", "1"}, {"dataLogging", "false"}};
  sensorFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"updateInterval", "30000"}, {"sensorCount", "2"}, {"dataLogging", "false"}};
  sensorFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"updateInterval", "15000"}, {"sensorCount", "4"}, {"dataLogging", "true"}};
  sensorFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"updateInterval", "10000"}, {"sensorCount", "6"}, {"dataLogging", "true"}};
  sensorFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"updateInterval", "5000"}, {"sensorCount", "8"}, {"dataLogging", "true"}, {"advancedProcessing", "true"}};
  featureConfigs["Sensor"] = sensorFeature;

  FeatureConfig stockFeature = {
    "Stock",
    "股票数据获取和显示",
    FEATURE_LEVEL_BASIC,
    FEATURE_LEVEL_BASIC,
    false,
    12,
    5,
    45,
    {},
    0
  };
  stockFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"updateInterval", "3600000"}, {"stockCount", "1"}, {"detailedInfo", "false"}};
  stockFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"updateInterval", "1800000"}, {"stockCount", "2"}, {"detailedInfo", "false"}};
  stockFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"updateInterval", "600000"}, {"stockCount", "3"}, {"detailedInfo", "true"}};
  stockFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"updateInterval", "300000"}, {"stockCount", "5"}, {"detailedInfo", "true"}};
  stockFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"updateInterval", "180000"}, {"stockCount", "10"}, {"detailedInfo", "true"}, {"realTimeUpdates", "true"}};
  featureConfigs["Stock"] = stockFeature;

  FeatureConfig webServerFeature = {
    "WebServer",
    "Web服务器和配置界面",
    FEATURE_LEVEL_STANDARD,
    FEATURE_LEVEL_STANDARD,
    false,
    20,
    10,
    50,
    {},
    0
  };
  webServerFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"enabled", "true"}, {"maxConnections", "1"}, {"complexUI", "false"}};
  webServerFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"enabled", "true"}, {"maxConnections", "2"}, {"complexUI", "false"}};
  webServerFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"enabled", "true"}, {"maxConnections", "3"}, {"complexUI", "true"}};
  webServerFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"enabled", "true"}, {"maxConnections", "4"}, {"complexUI", "true"}, {"sslEnabled", "false"}};
  webServerFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"enabled", "true"}, {"maxConnections", "5"}, {"complexUI", "true"}, {"sslEnabled", "true"}, {"apiEnabled", "true"}};
  featureConfigs["WebServer"] = webServerFeature;

  FeatureConfig bluetoothFeature = {
    "Bluetooth",
    "蓝牙配置和通信",
    FEATURE_LEVEL_BASIC,
    FEATURE_LEVEL_BASIC,
    false,
    15,
    5,
    40,
    {},
    0
  };
  bluetoothFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"enabled", "true"}, {"advertisingInterval", "1000"}, {"advancedFeatures", "false"}};
  bluetoothFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"enabled", "true"}, {"advertisingInterval", "500"}, {"advancedFeatures", "false"}};
  bluetoothFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"enabled", "true"}, {"advertisingInterval", "300"}, {"advancedFeatures", "true"}};
  bluetoothFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"enabled", "true"}, {"advertisingInterval", "200"}, {"advancedFeatures", "true"}, {"continuousAdvertising", "true"}};
  bluetoothFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"enabled", "true"}, {"advertisingInterval", "100"}, {"advancedFeatures", "true"}, {"continuousAdvertising", "true"}, {"dataTransfer", "true"}};
  featureConfigs["Bluetooth"] = bluetoothFeature;

  FeatureConfig pluginFeature = {
    "Plugin",
    "插件系统",
    FEATURE_LEVEL_BASIC,
    FEATURE_LEVEL_BASIC,
    false,
    25,
    15,
    60,
    {},
    0
  };
  pluginFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"enabled", "false"}, {"maxPlugins", "0"}};
  pluginFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"enabled", "true"}, {"maxPlugins", "1"}, {"complexPlugins", "false"}};
  pluginFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"enabled", "true"}, {"maxPlugins", "2"}, {"complexPlugins", "false"}};
  pluginFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"enabled", "true"}, {"maxPlugins", "3"}, {"complexPlugins", "true"}};
  pluginFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"enabled", "true"}, {"maxPlugins", "5"}, {"complexPlugins", "true"}, {"autoUpdate", "true"}};
  featureConfigs["Plugin"] = pluginFeature;

  FeatureConfig audioFeature = {
    "Audio",
    "音频播放和录制",
    FEATURE_LEVEL_DISABLED,
    FEATURE_LEVEL_DISABLED,
    false,
    30,
    20,
    65,
    {},
    0
  };
  audioFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"enabled", "false"}};
  audioFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"enabled", "true"}, {"playbackOnly", "true"}, {"quality", "low"}};
  audioFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"enabled", "true"}, {"playbackOnly", "true"}, {"quality", "medium"}};
  audioFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"enabled", "true"}, {"playbackOnly", "false"}, {"quality", "medium"}};
  audioFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"enabled", "true"}, {"playbackOnly", "false"}, {"quality", "high"}, {"effectsEnabled", "true"}};
  featureConfigs["Audio"] = audioFeature;

  FeatureConfig cameraFeature = {
    "Camera",
    "摄像头和图像识别",
    FEATURE_LEVEL_DISABLED,
    FEATURE_LEVEL_DISABLED,
    false,
    40,
    30,
    75,
    {},
    0
  };
  cameraFeature.levelConfigs[FEATURE_LEVEL_MINIMAL] = {{"enabled", "false"}};
  cameraFeature.levelConfigs[FEATURE_LEVEL_BASIC] = {{"enabled", "false"}};
  cameraFeature.levelConfigs[FEATURE_LEVEL_STANDARD] = {{"enabled", "true"}, {"resolution", "low"}, {"recognition", "false"}};
  cameraFeature.levelConfigs[FEATURE_LEVEL_ADVANCED] = {{"enabled", "true"}, {"resolution", "medium"}, {"recognition", "true"}, {"basicRecognition", "true"}};
  cameraFeature.levelConfigs[FEATURE_LEVEL_FULL] = {{"enabled", "true"}, {"resolution", "high"}, {"recognition", "true"}, {"advancedRecognition", "true"}, {"faceDetection", "true"}};
  featureConfigs["Camera"] = cameraFeature;
}

// 初始化
bool FeatureManager::init() {
  if (initialized) {
    return true;
  }
  
  DEBUG_PRINTLN("初始化功能管理器...");
  
  // 初始化默认功能配置
  initDefaultFeatures();
  
  // 更新硬件评估结果
  if (!updateHardwareEvaluation()) {
    DEBUG_PRINTLN("硬件评估失败，使用默认配置");
  }
  
  // 评估功能级别
  evaluateFeatures();
  
  initialized = true;
  DEBUG_PRINTLN("功能管理器初始化完成");
  return true;
}

// 更新硬件评估结果
bool FeatureManager::updateHardwareEvaluation() {
  HardwareDetector* hardwareDetector = HardwareDetector::getInstance();
  if (!hardwareDetector) {
    return false;
  }
  
  // 执行硬件资源检测
  hardwareDetector->detectResources();
  hardwareDetector->evaluateCapabilities();
  
  // 获取硬件评估结果
  hardwareResult = hardwareDetector->getEvaluationResult();
  lastEvaluationTime = millis();
  
  return hardwareResult.isValid;
}

// 根据硬件能力计算功能级别
FeatureLevel FeatureManager::calculateFeatureLevel(const FeatureConfig& config) {
  if (!hardwareResult.isValid) {
    return config.defaultLevel;
  }
  
  // 检查核心功能
  if (config.essential) {
    return FEATURE_LEVEL_FULL;
  }
  
  // 检查硬件要求
  if (hardwareResult.totalMemory < config.minMemoryRequired ||
      hardwareResult.totalStorage < config.minStorageRequired ||
      hardwareResult.overallScore < config.minScoreRequired) {
    return FEATURE_LEVEL_MINIMAL;
  }
  
  // 根据硬件能力级别计算功能级别
  return getLevelForHardwareCapability(hardwareResult.overallLevel);
}

// 获取硬件能力级别对应的功能级别映射
FeatureLevel FeatureManager::getLevelForHardwareCapability(HardwareCapabilityLevel capabilityLevel) {
  switch (capabilityLevel) {
    case CAPABILITY_LEVEL_EXCELLENT:
      return FEATURE_LEVEL_FULL;
    case CAPABILITY_LEVEL_HIGH:
      return FEATURE_LEVEL_ADVANCED;
    case CAPABILITY_LEVEL_MEDIUM:
      return FEATURE_LEVEL_STANDARD;
    case CAPABILITY_LEVEL_LOW:
      return FEATURE_LEVEL_BASIC;
    default:
      return FEATURE_LEVEL_MINIMAL;
  }
}

// 评估并调整所有功能级别
bool FeatureManager::evaluateFeatures() {
  DEBUG_PRINTLN("评估功能级别...");
  
  bool success = true;
  
  for (auto& pair : featureConfigs) {
    FeatureConfig& config = pair.second;
    FeatureLevel oldLevel = config.currentLevel;
    FeatureLevel newLevel = calculateFeatureLevel(config);
    
    if (oldLevel != newLevel) {
      config.currentLevel = newLevel;
      config.lastUpdateTime = millis();
      applyFeatureLevel(config.name, newLevel);
      publishFeatureStatusEvent(config.name, oldLevel, newLevel);
      DEBUG_PRINTF("功能 %s 级别调整: %d -> %d\n", config.name.c_str(), oldLevel, newLevel);
    }
  }
  
  DEBUG_PRINTLN("功能级别评估完成");
  return success;
}

// 应用功能级别配置
void FeatureManager::applyFeatureLevel(const String& featureName, FeatureLevel level) {
  // 这里可以添加具体的功能级别应用逻辑
  // 例如：通知相关模块功能级别变化
  DEBUG_PRINTF("应用功能 %s 级别: %d\n", featureName.c_str(), level);
}

// 发布功能状态变化事件
void FeatureManager::publishFeatureStatusEvent(const String& featureName, FeatureLevel oldLevel, FeatureLevel newLevel) {
  // 这里可以添加事件发布逻辑
  DEBUG_PRINTF("功能状态变化事件: %s %d -> %d\n", featureName.c_str(), oldLevel, newLevel);
}

// 获取功能配置
FeatureConfig FeatureManager::getFeatureConfig(const String& featureName) {
  if (featureConfigs.find(featureName) != featureConfigs.end()) {
    return featureConfigs[featureName];
  }
  
  // 返回默认配置
  FeatureConfig defaultConfig = {
    featureName,
    "未知功能",
    FEATURE_LEVEL_DISABLED,
    FEATURE_LEVEL_DISABLED,
    false,
    0,
    0,
    0,
    {},
    0
  };
  return defaultConfig;
}

// 获取所有功能配置
std::vector<FeatureConfig> FeatureManager::getAllFeatureConfigs() {
  std::vector<FeatureConfig> configs;
  for (auto& pair : featureConfigs) {
    configs.push_back(pair.second);
  }
  return configs;
}

// 检查功能是否启用
bool FeatureManager::isFeatureEnabled(const String& featureName) {
  if (featureConfigs.find(featureName) != featureConfigs.end()) {
    return featureConfigs[featureName].currentLevel > FEATURE_LEVEL_DISABLED;
  }
  return false;
}

// 获取功能级别
FeatureLevel FeatureManager::getFeatureLevel(const String& featureName) {
  if (featureConfigs.find(featureName) != featureConfigs.end()) {
    return featureConfigs[featureName].currentLevel;
  }
  return FEATURE_LEVEL_DISABLED;
}

// 设置功能级别（手动覆盖）
bool FeatureManager::setFeatureLevel(const String& featureName, FeatureLevel level) {
  if (featureConfigs.find(featureName) == featureConfigs.end()) {
    return false;
  }
  
  FeatureConfig& config = featureConfigs[featureName];
  FeatureLevel oldLevel = config.currentLevel;
  
  // 核心功能不能禁用
  if (config.essential && level == FEATURE_LEVEL_DISABLED) {
    return false;
  }
  
  config.currentLevel = level;
  config.lastUpdateTime = millis();
  applyFeatureLevel(featureName, level);
  publishFeatureStatusEvent(featureName, oldLevel, level);
  
  return true;
}

// 重置所有功能到默认级别
bool FeatureManager::resetAllFeatures() {
  for (auto& pair : featureConfigs) {
    FeatureConfig& config = pair.second;
    FeatureLevel oldLevel = config.currentLevel;
    config.currentLevel = config.defaultLevel;
    config.lastUpdateTime = millis();
    applyFeatureLevel(config.name, config.defaultLevel);
    publishFeatureStatusEvent(config.name, oldLevel, config.defaultLevel);
  }
  return true;
}

// 获取功能配置参数
String FeatureManager::getFeatureConfigParam(const String& featureName, const String& paramName) {
  if (featureConfigs.find(featureName) == featureConfigs.end()) {
    return "";
  }
  
  FeatureConfig& config = featureConfigs[featureName];
  FeatureLevel level = config.currentLevel;
  
  if (config.levelConfigs.find(level) != config.levelConfigs.end()) {
    auto& levelConfig = config.levelConfigs[level];
    if (levelConfig.find(paramName) != levelConfig.end()) {
      return levelConfig[paramName];
    }
  }
  
  return "";
}

// 监控功能状态
bool FeatureManager::monitorFeatures() {
  // 定期更新硬件评估结果
  if (millis() - lastEvaluationTime > 300000) { // 5分钟
    updateHardwareEvaluation();
    evaluateFeatures();
  }
  
  return true;
}

// 获取核心功能列表
std::vector<String> FeatureManager::getEssentialFeatures() {
  std::vector<String> essentialFeatures;
  for (auto& pair : featureConfigs) {
    if (pair.second.essential) {
      essentialFeatures.push_back(pair.first);
    }
  }
  return essentialFeatures;
}

// 获取可选功能列表
std::vector<String> FeatureManager::getOptionalFeatures() {
  std::vector<String> optionalFeatures;
  for (auto& pair : featureConfigs) {
    if (!pair.second.essential) {
      optionalFeatures.push_back(pair.first);
    }
  }
  return optionalFeatures;
}
