#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <Arduino.h>
#include "config.h"

// 插件状态枚举
enum PluginStatus {
  PLUGIN_DISABLED,
  PLUGIN_ENABLED,
  PLUGIN_RUNNING,
  PLUGIN_ERROR
};

// 插件函数指针类型
typedef bool (*PluginInitFunc)();
typedef void (*PluginUpdateFunc)();
typedef void (*PluginLoopFunc)();
typedef void (*PluginDeinitFunc)();

// 插件数据结构
typedef struct {
  String name;              // 插件名称
  String version;           // 插件版本
  String description;       // 插件描述
  PluginStatus status;      // 插件状态
  PluginInitFunc init;      // 初始化函数
  PluginUpdateFunc update;  // 更新函数
  PluginLoopFunc loop;      // 循环函数
  PluginDeinitFunc deinit;  // 反初始化函数
  bool valid;               // 数据是否有效
} PluginData;

class PluginManager {
public:
  PluginManager();
  ~PluginManager();
  
  void init();
  void update();
  void loop();
  
  // 插件管理功能
  bool registerPlugin(String name, String version, String description,
                     PluginInitFunc init, PluginUpdateFunc update,
                     PluginLoopFunc loop, PluginDeinitFunc deinit);
  bool unregisterPlugin(String name);
  bool enablePlugin(String name);
  bool disablePlugin(String name);
  bool isPluginEnabled(String name);
  
  // 获取插件数据
  PluginData getPlugin(String name);
  PluginData getPlugin(int index);
  int getPluginCount();
  
  // 插件存储功能
  bool savePlugins();
  bool loadPlugins();
  
private:
  // 插件数组
  PluginData plugins[MAX_PLUGINS];
  
  // 插件计数
  int pluginCount;
  
  // 更新标志
  unsigned long lastUpdate;
  bool dataUpdated;
  
  // 私有方法
  int findPluginIndex(String name);
  bool initPlugin(int index);
  bool deinitPlugin(int index);
};

#endif // PLUGIN_MANAGER_H