#include "scene_manager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "../coresystem/module_registry.h"

// 前向声明模块包装器类
class WiFiModuleWrapper;
class BluetoothModuleWrapper;
class DisplayModuleWrapper;
class SensorModuleWrapper;
class PluginModuleWrapper;

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
}

SceneManager::~SceneManager() {
  // 清理资源
}

void SceneManager::init() {
  DEBUG_PRINTLN("初始化场景管理器...");
  
  // 通过ModuleRegistry获取模块引用
  #if ENABLE_WIFI
    WiFiModuleWrapper* wifiWrapper = getModule<WiFiModuleWrapper>();
    if (wifiWrapper) {
      wifiManager = &wifiWrapper->getWiFiManager();
    }
  #endif
  
  #if ENABLE_BLUETOOTH
    BluetoothModuleWrapper* bluetoothWrapper = getModule<BluetoothModuleWrapper>();
    if (bluetoothWrapper) {
      bluetoothManager = &bluetoothWrapper->getBluetoothManager();
    }
  #endif
  
  DisplayModuleWrapper* displayWrapper = getModule<DisplayModuleWrapper>();
  if (displayWrapper) {
    displayManager = &displayWrapper->getDisplayManager();
  }
  
  SensorModuleWrapper* sensorWrapper = getModule<SensorModuleWrapper>();
  if (sensorWrapper) {
    sensorManager = &sensorWrapper->getSensorManager();
  }
  
  #if ENABLE_PLUGIN
    PluginModuleWrapper* pluginWrapper = getModule<PluginModuleWrapper>();
    if (pluginWrapper) {
      pluginManager = &pluginWrapper->getPluginManager();
    }
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
  if (config.enableDisplay) {
    // 设置显示亮度
    if (displayManager) {
      // displayManager->setBrightness(config.displayBrightness);
    }
  } else {
    // 关闭显示
    if (displayManager) {
      // displayManager->turnOff();
    }
  }
  
  // 应用WiFi设置
  if (config.enableWiFi) {
    // 启用WiFi
    if (wifiManager) {
      // wifiManager->enable();
    }
  } else {
    // 禁用WiFi
    if (wifiManager) {
      // wifiManager->disable();
    }
  }
  
  // 应用蓝牙设置
  if (config.enableBluetooth) {
    // 启用蓝牙
    if (bluetoothManager) {
      // bluetoothManager->enable();
    }
  } else {
    // 禁用蓝牙
    if (bluetoothManager) {
      // bluetoothManager->disable();
    }
  }
  
  // 应用传感器设置
  if (config.enableSensors) {
    // 启用传感器
    if (sensorManager) {
      // sensorManager->enable();
    }
  } else {
    // 禁用传感器
    if (sensorManager) {
      // sensorManager->disable();
    }
  }
  
  // 应用插件设置
  if (config.enablePlugins) {
    // 启用插件
    if (pluginManager) {
      // pluginManager->enable();
    }
  } else {
    // 禁用插件
    if (pluginManager) {
      // pluginManager->disable();
    }
  }
  
  // 设置刷新间隔
  if (displayManager) {
    // displayManager->setRefreshInterval(config.refreshInterval * 1000);
  }
  
  DEBUG_PRINTLN("场景配置应用完成");
}

bool SceneManager::setCurrentScene(SceneMode mode) {
  if (mode < SCENE_NORMAL || mode > SCENE_SLEEP) {
    return false;
  }
  
  currentScene = mode;
  applySceneConfig(sceneConfigs[mode]);
  
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
  if (mode < SCENE_NORMAL || mode > SCENE_CUSTOM_3) {
    SceneConfig invalid = {SCENE_NORMAL, "", "", false, false, false, false, false, 0, 0, false};
    return invalid;
  }
  
  return sceneConfigs[mode];
}

bool SceneManager::switchToScene(SceneMode mode) {
  return setCurrentScene(mode);
}

bool SceneManager::switchToNextScene() {
  SceneMode nextScene = (SceneMode)((currentScene + 1) % 10);
  return setCurrentScene(nextScene);
}

bool SceneManager::switchToPreviousScene() {
  SceneMode prevScene = (SceneMode)((currentScene - 1 + 10) % 10);
  return setCurrentScene(prevScene);
}

bool SceneManager::saveScenes() {
  DEBUG_PRINTLN("保存场景配置到文件");
  
  // 这里可以实现将场景配置保存到SPIFFS或其他存储介质
  // 暂时返回true，表示保存成功
  return true;
}

bool SceneManager::loadScenes() {
  DEBUG_PRINTLN("从文件加载场景配置");
  
  // 这里可以实现从SPIFFS或其他存储介质加载场景配置
  // 暂时返回false，表示使用默认配置
  return false;
}

void SceneManager::initDefaultQuickActions() {
  DEBUG_PRINTLN("初始化默认快捷功能");
  
  // 预设快捷功能映射
  quickActions[0] = SCENE_NORMAL;     // 快捷1：正常模式
  quickActions[1] = SCENE_OFFICE;     // 快捷2：办公模式
  quickActions[2] = SCENE_HOME;       // 快捷3：家庭模式
  quickActions[3] = SCENE_SLEEP;      // 快捷4：睡眠模式
  quickActions[4] = SCENE_RELAX;      // 快捷5：放松模式
  quickActions[5] = SCENE_PARTY;      // 快捷6：派对模式
  quickActions[6] = SCENE_OUTDOOR;    // 快捷7：户外模式
  quickActions[7] = SCENE_CUSTOM_1;   // 快捷8：自定义模式1
  quickActions[8] = SCENE_CUSTOM_2;   // 快捷9：自定义模式2
  quickActions[9] = SCENE_CUSTOM_3;   // 快捷10：自定义模式3
  
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
  if (actionId < 0 || actionId >= 10 || scene < SCENE_NORMAL || scene > SCENE_CUSTOM_3) {
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
