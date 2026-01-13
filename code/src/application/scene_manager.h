#ifndef SCENE_MANAGER_H
#define SCENE_MANAGER_H

#include <Arduino.h>
#include "../coresystem/config.h"
#include "../coresystem/event_bus.h"

// 场景模式枚举
enum SceneMode {
  SCENE_NORMAL,        // 正常模式
  SCENE_INTERACTIVE,   // 互动模式
  SCENE_SLEEP          // 睡眠模式
};

// 场景配置结构体
typedef struct {
  SceneMode mode;              // 场景模式
  String name;                 // 场景名称
  String description;          // 场景描述
  bool enableDisplay;          // 是否启用显示
  bool enableWiFi;             // 是否启用WiFi
  bool enableBluetooth;        // 是否启用蓝牙
  bool enableSensors;          // 是否启用传感器
  bool enablePlugins;          // 是否启用插件
  int displayBrightness;       // 显示亮度
  int refreshInterval;         // 刷新间隔（秒）
  bool valid;                  // 数据是否有效
} SceneConfig;

class SceneManager {
public:
  SceneManager();
  ~SceneManager();
  
  void init();
  void update();
  void loop();
  
  // 场景管理功能
  bool setCurrentScene(SceneMode mode);
  SceneMode getCurrentScene();
  
  // 场景配置功能
  bool setSceneConfig(SceneMode mode, SceneConfig config);
  SceneConfig getSceneConfig(SceneMode mode);
  
  // 场景切换功能
  bool switchToScene(SceneMode mode);
  bool switchToNextScene();
  bool switchToPreviousScene();
  
  // 快捷功能集
  bool triggerQuickAction(int actionId);
  bool registerQuickAction(int actionId, SceneMode scene);
  SceneMode getQuickActionScene(int actionId);
  
  // 场景保存和加载
  bool saveScenes();
  bool loadScenes();
  void resetScenes();
  
private:
  // 当前场景
  SceneMode currentScene;
  
  // 场景配置数组
  SceneConfig sceneConfigs[3]; // 支持3种场景模式
  
  // 快捷功能映射
  SceneMode quickActions[10]; // 支持10个快捷操作
  
  // 初始化默认场景配置
  void initDefaultScenes();
  
  // 初始化默认快捷功能
  void initDefaultQuickActions();
  
  // 应用场景配置
  void applySceneConfig(SceneConfig config);
};

#endif // SCENE_MANAGER_H