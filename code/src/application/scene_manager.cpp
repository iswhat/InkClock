#include "scene_manager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "../coresystem/module_registry.h"
#include "wifi_manager.h"
#include "display_manager.h"
#include "sensor_manager.h"
#include "../coresystem/plugin_manager.h"
#include "../bluetooth_manager.h"



// 模块获取辅助函数
template <typename T>
T* getModule() {
  return ModuleRegistry::getInstance()->getModule<T>();
}

// 全局模块引用
WiFiManager* wifiManager = nullptr;
BluetoothManager* bluetoothManager = nullptr;
DisplayManager* displayManager = nullptr;
SensorManager* sensorManager = nullptr;
PluginManager* pluginManager = nullptr;

SceneManager::SceneManager() {
  currentScene = SCENE_NORMAL;
  
  // 初始化快捷功能数组
  for (int i = 0; i < 10; i++) {
    quickActions[i] = SCENE_NORMAL;
  }
  
  // 初始化智能切换相关变量
  lastUserActivityTime = millis();
  lastSceneSwitchTime = millis();
  userActivityCount = 0;
}

SceneManager::~SceneManager() {
  // 清理资源
}

void SceneManager::init() {
  DEBUG_PRINTLN("初始化场景管理器...");
  
  // 直接从ModuleRegistry获取模块引用
  ModuleRegistry* registry = ModuleRegistry::getInstance();
  
  #if ENABLE_WIFI
    wifiManager = static_cast<WiFiManager*>(registry->getModuleByType(MODULE_TYPE_WIFI));
  #endif
  
  #if ENABLE_BLUETOOTH
    bluetoothManager = static_cast<BluetoothManager*>(registry->getModuleByType(MODULE_TYPE_BLUETOOTH));
  #endif
  
  displayManager = static_cast<DisplayManager*>(registry->getModuleByType(MODULE_TYPE_DISPLAY));
  sensorManager = static_cast<SensorManager*>(registry->getModuleByType(MODULE_TYPE_SENSOR));
  
  #if ENABLE_PLUGIN
    pluginManager = static_cast<PluginManager*>(registry->getModuleByType(MODULE_TYPE_PLUGIN));
  #endif
  
  // 初始化默认场景配置
  initDefaultScenes();
  
  // 初始化默认快捷功能
  initDefaultQuickActions();
  
  // 加载场景配置
  loadScenes();
  
  // 应用当前场景配置
  applySceneConfig(sceneConfigs[currentScene]);
  
  DEBUG_PRINTLN("场景管理器初始化完成");
}

void SceneManager::update() {
  // 定期保存场景配置
  static unsigned long lastSave = 0;
  if (millis() - lastSave > 60000) { // 每分钟保存一次
    lastSave = millis();
    saveScenes();
  }
  
  // 定期检查是否需要智能切换场景
  static unsigned long lastAutoSwitchCheck = 0;
  if (millis() - lastAutoSwitchCheck > 30000) { // 每30秒检查一次
    lastAutoSwitchCheck = millis();
    autoSwitchScene();
  }
  
  // 检查用户活动，超时进入睡眠模式
  static unsigned long lastActivityCheck = 0;
  if (millis() - lastActivityCheck > 60000) { // 每分钟检查一次
    lastActivityCheck = millis();
    switchBasedOnUserActivity();
  }
  
  // 基于时间自动切换场景
  static unsigned long lastTimeCheck = 0;
  if (millis() - lastTimeCheck > 300000) { // 每5分钟检查一次
    lastTimeCheck = millis();
    switchBasedOnTime();
  }
}

void SceneManager::loop() {
  update();
}

void SceneManager::initDefaultScenes() {
  DEBUG_PRINTLN("初始化默认场景配置");
  
  // 正常模式
  sceneConfigs[SCENE_NORMAL] = {
    SCENE_NORMAL,
    "正常模式",
    "聚焦当前界面内容，其他功能服务休眠，低功耗",
    true,
    true,
    true,
    true,
    false,
    80,
    120,
    true
  };
  
  // 互动模式
  sceneConfigs[SCENE_INTERACTIVE] = {
    SCENE_INTERACTIVE,
    "互动模式",
    "高性能模式，预加载功能和网络数据，交互更流畅",
    true,
    true,
    true,
    true,
    true,
    100,
    30,
    true
  };
  
  // 睡眠模式
  sceneConfigs[SCENE_SLEEP] = {
    SCENE_SLEEP,
    "睡眠模式",
    "凌晨到早晨时段，无互动情况下进入，只保留最低供电需求",
    false,
    false,
    false,
    false,
    false,
    0,
    600,
    true
  };
}

void SceneManager::applySceneConfig(SceneConfig config) {
  DEBUG_PRINTF("应用场景配置: %s\n", config.name.c_str());
  
  // 应用显示设置
  /*
  if (config.enableDisplay) {
    // 设置显示亮度
    if (displayManager) {
      displayManager->setBrightness(config.displayBrightness);
    }
  } else {
    // 关闭显示
    if (displayManager) {
      displayManager->turnOff();
    }
  }
  
  // 应用WiFi设置
  if (config.enableWiFi) {
    // 启用WiFi
    if (wifiManager) {
      wifiManager->enable();
    }
  } else {
    // 禁用WiFi
    if (wifiManager) {
      wifiManager->disable();
    }
  }
  
  // 应用蓝牙设置
  if (config.enableBluetooth) {
    // 启用蓝牙
    if (bluetoothManager) {
      bluetoothManager->enable();
    }
  } else {
    // 禁用蓝牙
    if (bluetoothManager) {
      bluetoothManager->disable();
    }
  }
  
  // 应用传感器设置
  if (config.enableSensors) {
    // 启用传感器
    if (sensorManager) {
      sensorManager->enable();
    }
  } else {
    // 禁用传感器
    if (sensorManager) {
      sensorManager->disable();
    }
  }
  
  // 应用插件设置
  if (config.enablePlugins) {
    // 启用插件
    if (pluginManager) {
      pluginManager->enable();
    }
  } else {
    // 禁用插件
    if (pluginManager) {
      pluginManager->disable();
    }
  }
  */
  
  // 设置刷新间隔
  if (displayManager) {
    displayManager->setRefreshInterval(config.refreshInterval * 1000);
  }
  
  DEBUG_PRINTLN("场景配置应用完成");
}

bool SceneManager::setCurrentScene(SceneMode mode) {
  if (mode < SCENE_NORMAL || mode > SCENE_SLEEP) {
    return false;
  }
  
  currentScene = mode;
  applySceneConfig(sceneConfigs[mode]);
  
  // 更新场景切换时间
  lastSceneSwitchTime = millis();
  
  DEBUG_PRINTF("设置当前场景为: %s\n", sceneConfigs[mode].name.c_str());
  return true;
}

SceneMode SceneManager::getCurrentScene() {
  return currentScene;
}

bool SceneManager::setSceneConfig(SceneMode mode, SceneConfig config) {
  if (mode < SCENE_NORMAL || mode > SCENE_SLEEP) {
    return false;
  }
  
  sceneConfigs[mode] = config;
  
  // 如果是当前场景，立即应用配置
  if (mode == currentScene) {
    applySceneConfig(config);
  }
  
  DEBUG_PRINTF("更新场景配置: %s\n", config.name.c_str());
  return true;
}

SceneConfig SceneManager::getSceneConfig(SceneMode mode) {
  if (mode < SCENE_NORMAL || mode > SCENE_SLEEP) {
    SceneConfig invalid = {SCENE_NORMAL, "", "", false, false, false, false, false, 0, 0, false};
    return invalid;
  }
  
  return sceneConfigs[mode];
}

bool SceneManager::switchToScene(SceneMode mode) {
  return setCurrentScene(mode);
}

bool SceneManager::switchToNextScene() {
  SceneMode nextScene = (SceneMode)((currentScene + 1) % 3);
  return setCurrentScene(nextScene);
}

bool SceneManager::switchToPreviousScene() {
  SceneMode prevScene = (SceneMode)((currentScene - 1 + 3) % 3);
  return setCurrentScene(prevScene);
}

bool SceneManager::saveScenes() {
  DEBUG_PRINTLN("保存场景配置到文件");
  
  // 初始化SPIFFS
  if (!SPIFFS.begin()) {
    DEBUG_PRINTLN("SPIFFS初始化失败，无法保存场景配置");
    return false;
  }
  
  // 创建JSON文档
  JsonDocument doc;
  
  // 保存场景配置
  JsonArray scenes = doc.createNestedArray("scenes");
  
  for (int i = 0; i < 3; i++) {
    SceneMode mode = static_cast<SceneMode>(i);
    SceneConfig config = sceneConfigs[mode];
    
    JsonObject sceneObj = scenes.createNestedObject();
    sceneObj["mode"] = config.mode;
    sceneObj["name"] = config.name;
    sceneObj["description"] = config.description;
    sceneObj["enableDisplay"] = config.enableDisplay;
    sceneObj["enableWiFi"] = config.enableWiFi;
    sceneObj["enableBluetooth"] = config.enableBluetooth;
    sceneObj["enableSensors"] = config.enableSensors;
    sceneObj["enablePlugins"] = config.enablePlugins;
    sceneObj["displayBrightness"] = config.displayBrightness;
    sceneObj["refreshInterval"] = config.refreshInterval;
    sceneObj["valid"] = config.valid;
  }
  
  // 保存快捷功能
  JsonArray quickActionsArray = doc.createNestedArray("quickActions");
  for (int i = 0; i < 10; i++) {
    quickActionsArray.add(quickActions[i]);
  }
  
  // 打开文件
  File file = SPIFFS.open("/scenes.json", "w");
  if (!file) {
    DEBUG_PRINTLN("打开场景配置文件失败");
    SPIFFS.end();
    return false;
  }
  
  // 序列化JSON到文件
  if (serializeJson(doc, file) == 0) {
    DEBUG_PRINTLN("写入场景配置失败");
    file.close();
    SPIFFS.end();
    return false;
  }
  
  // 关闭文件
  file.close();
  SPIFFS.end();
  
  DEBUG_PRINTLN("场景配置保存成功");
  return true;
}

bool SceneManager::loadScenes() {
  DEBUG_PRINTLN("从文件加载场景配置");
  
  // 初始化SPIFFS
  if (!SPIFFS.begin()) {
    DEBUG_PRINTLN("SPIFFS初始化失败，使用默认场景配置");
    return false;
  }
  
  // 检查文件是否存在
  if (!SPIFFS.exists("/scenes.json")) {
    DEBUG_PRINTLN("场景配置文件不存在，使用默认配置");
    SPIFFS.end();
    return false;
  }
  
  // 打开文件
  File file = SPIFFS.open("/scenes.json", "r");
  if (!file) {
    DEBUG_PRINTLN("打开场景配置文件失败，使用默认配置");
    SPIFFS.end();
    return false;
  }
  
  // 读取文件内容
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    DEBUG_PRINTF("解析场景配置文件失败: %s\n", error.c_str());
    file.close();
    SPIFFS.end();
    return false;
  }
  
  // 加载场景配置
  if (doc.containsKey("scenes")) {
    JsonArray scenes = doc["scenes"];
    for (JsonVariant scene : scenes) {
      int mode = scene["mode"];
      if (mode >= 0 && mode < 3) {
        SceneConfig config;
        config.mode = static_cast<SceneMode>(mode);
        config.name = scene["name"].as<String>();
        config.description = scene["description"].as<String>();
        config.enableDisplay = scene["enableDisplay"];
        config.enableWiFi = scene["enableWiFi"];
        config.enableBluetooth = scene["enableBluetooth"];
        config.enableSensors = scene["enableSensors"];
        config.enablePlugins = scene["enablePlugins"];
        config.displayBrightness = scene["displayBrightness"];
        config.refreshInterval = scene["refreshInterval"];
        config.valid = scene["valid"];
        
        sceneConfigs[mode] = config;
      }
    }
  }
  
  // 加载快捷功能
  if (doc.containsKey("quickActions")) {
    JsonArray quickActionsArray = doc["quickActions"];
    for (int i = 0; i < quickActionsArray.size() && i < 10; i++) {
      quickActions[i] = static_cast<SceneMode>(quickActionsArray[i].as<int>());
    }
  }
  
  // 关闭文件
  file.close();
  SPIFFS.end();
  
  DEBUG_PRINTLN("场景配置加载成功");
  return true;
}

void SceneManager::initDefaultQuickActions() {
  DEBUG_PRINTLN("初始化默认快捷功能");
  
  // 预设快捷功能映射
  quickActions[0] = SCENE_NORMAL;     // 快捷1：正常模式
  quickActions[1] = SCENE_INTERACTIVE;     // 快捷2：互动模式
  quickActions[2] = SCENE_SLEEP;      // 快捷3：睡眠模式
  quickActions[3] = SCENE_NORMAL;     // 快捷4：正常模式
  quickActions[4] = SCENE_INTERACTIVE;      // 快捷5：互动模式
  quickActions[5] = SCENE_SLEEP;      // 快捷6：睡眠模式
  quickActions[6] = SCENE_NORMAL;    // 快捷7：正常模式
  quickActions[7] = SCENE_INTERACTIVE;   // 快捷8：互动模式
  quickActions[8] = SCENE_SLEEP;   // 快捷9：睡眠模式
  quickActions[9] = SCENE_NORMAL;   // 快捷10：正常模式
  
  DEBUG_PRINTLN("默认快捷功能初始化完成");
}

bool SceneManager::triggerQuickAction(int actionId) {
  if (actionId < 0 || actionId >= 10) {
    return false;
  }
  
  SceneMode scene = quickActions[actionId];
  bool result = switchToScene(scene);
  
  if (result) {
    DEBUG_PRINTF("触发快捷功能 %d: %s\n", actionId, sceneConfigs[scene].name.c_str());
  }
  
  return result;
}

bool SceneManager::registerQuickAction(int actionId, SceneMode scene) {
  if (actionId < 0 || actionId >= 10 || scene < SCENE_NORMAL || scene > SCENE_SLEEP) {
    return false;
  }
  
  quickActions[actionId] = scene;
  DEBUG_PRINTF("注册快捷功能 %d -> %s\n", actionId, sceneConfigs[scene].name.c_str());
  return true;
}

SceneMode SceneManager::getQuickActionScene(int actionId) {
  if (actionId < 0 || actionId >= 10) {
    return SCENE_NORMAL;
  }
  
  return quickActions[actionId];
}

void SceneManager::resetScenes() {
  DEBUG_PRINTLN("重置场景配置为默认值");
  initDefaultScenes();
  initDefaultQuickActions();
  applySceneConfig(sceneConfigs[currentScene]);
}

// 智能场景切换
bool SceneManager::autoSwitchScene() {
  if (!shouldSwitchScene()) {
    return false;
  }
  
  SceneMode recommendedScene = getRecommendedScene();
  if (recommendedScene != currentScene) {
    return setCurrentScene(recommendedScene);
  }
  
  return false;
}

// 基于时间切换场景
bool SceneManager::switchBasedOnTime() {
  // 获取当前时间
  time_t now = time(nullptr);
  struct tm* timeInfo = localtime(&now);
  int hour = timeInfo->tm_hour;
  int minute = timeInfo->tm_min;
  
  // 凌晨12点到早上5点：睡眠模式
  if (hour >= 0 && hour < 5) {
    if (currentScene != SCENE_SLEEP) {
      DEBUG_PRINTLN("基于时间切换到睡眠模式");
      return setCurrentScene(SCENE_SLEEP);
    }
  }
  // 早上5点到7点：正常模式（刚起床，不需要高性能）
  else if (hour >= 5 && hour < 7) {
    if (currentScene == SCENE_SLEEP) {
      DEBUG_PRINTLN("基于时间切换到正常模式");
      return setCurrentScene(SCENE_NORMAL);
    }
  }
  // 晚上7点到10点：互动模式（用户可能频繁使用）
  else if (hour >= 19 && hour < 22) {
    if (currentScene != SCENE_INTERACTIVE && currentScene != SCENE_SLEEP) {
      DEBUG_PRINTLN("基于时间切换到互动模式");
      return setCurrentScene(SCENE_INTERACTIVE);
    }
  }
  
  return false;
}

// 基于用户活动切换场景
bool SceneManager::switchBasedOnUserActivity() {
  unsigned long currentTime = millis();
  unsigned long inactivityTime = currentTime - lastUserActivityTime;
  
  // 用户无活动30分钟，且当前不是睡眠模式，切换到睡眠模式
  if (inactivityTime > 1800000 && currentScene != SCENE_SLEEP) {
    DEBUG_PRINTLN("用户无活动超时，切换到睡眠模式");
    return setCurrentScene(SCENE_SLEEP);
  }
  // 用户有活动，且当前是睡眠模式，切换到正常模式
  else if (inactivityTime < 600000 && currentScene == SCENE_SLEEP) {
    DEBUG_PRINTLN("检测到用户活动，从睡眠模式切换到正常模式");
    return setCurrentScene(SCENE_NORMAL);
  }
  
  return false;
}

// 记录用户活动
void SceneManager::recordUserActivity() {
  unsigned long currentTime = millis();
  lastUserActivityTime = currentTime;
  userActivityCount++;
  
  // 用户活动时，如果当前是睡眠模式，切换到正常模式
  if (currentScene == SCENE_SLEEP) {
    DEBUG_PRINTLN("检测到用户活动，从睡眠模式切换到正常模式");
    setCurrentScene(SCENE_NORMAL);
  }
  // 用户频繁活动（1分钟内超过5次操作），切换到互动模式
  else {
    static unsigned long lastActivityReset = 0;
    // 每分钟重置计数
    if (currentTime - lastActivityReset > 60000) {
      lastActivityReset = currentTime;
      userActivityCount = 1; // 重置为1，因为当前操作也算一次
    } 
    // 如果1分钟内活动次数超过5次，切换到互动模式
    else if (userActivityCount > 5 && currentScene != SCENE_INTERACTIVE) {
      DEBUG_PRINTLN("检测到用户频繁活动，切换到互动模式");
      setCurrentScene(SCENE_INTERACTIVE);
    }
  }
  
  // 打印调试信息
  DEBUG_PRINTF("用户活动记录：计数=%d, 上次活动时间=%lu\n", userActivityCount, lastUserActivityTime);
}

// 检查是否需要场景切换
bool SceneManager::shouldSwitchScene() {
  // 避免频繁切换场景，至少间隔30秒
  if (millis() - lastSceneSwitchTime < 30000) {
    return false;
  }
  
  return true;
}

// 获取推荐场景
SceneMode SceneManager::getRecommendedScene() {
  // 首先检查时间因素
  time_t now = time(nullptr);
  struct tm* timeInfo = localtime(&now);
  int hour = timeInfo->tm_hour;
  
  // 凌晨12点到早上5点：睡眠模式
  if (hour >= 0 && hour < 5) {
    return SCENE_SLEEP;
  }
  
  // 检查用户活动
  unsigned long inactivityTime = millis() - lastUserActivityTime;
  if (inactivityTime > 1800000) { // 30分钟无活动
    return SCENE_SLEEP;
  }
  
  // 检查用户活动频率
  if (userActivityCount > 3) { // 近期有活动
    // 晚上7点到10点：互动模式
    if (hour >= 19 && hour < 22) {
      return SCENE_INTERACTIVE;
    }
  }
  
  // 默认：正常模式
  return SCENE_NORMAL;
}
