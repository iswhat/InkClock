#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <Arduino.h>
#include "../core/config.h"

// 插件状态枚举
enum PluginStatus {
  PLUGIN_DISABLED,
  PLUGIN_ENABLED,
  PLUGIN_RUNNING,
  PLUGIN_ERROR
};

// 插件类型枚举
enum PluginType {
  PLUGIN_TYPE_NATIVE,       // 原生插件（使用函数指针）
  PLUGIN_TYPE_URL_XML,      // URL XML插件
  PLUGIN_TYPE_URL_JSON,     // URL JSON插件
  PLUGIN_TYPE_URL_JS        // URL JS插件
};

// 插件函数指针类型
typedef bool (*PluginInitFunc)();
typedef void (*PluginUpdateFunc)();
typedef void (*PluginLoopFunc)();
typedef void (*PluginDeinitFunc)();

// URL插件数据结构
typedef struct {
  String url;               // 插件URL地址
  unsigned long updateInterval; // 更新间隔（毫秒）
  String dataXPath;         // 数据提取XPath（XML）或JSON路径（JSON）
  String displayFormat;     // 显示格式模板
  String lastData;          // 最后获取的数据
  unsigned long lastUpdateTime; // 最后更新时间
} URLPluginData;

// 插件数据结构
typedef struct {
  String name;              // 插件名称
  String version;           // 插件版本
  String description;       // 插件描述
  PluginType type;          // 插件类型
  PluginStatus status;      // 插件状态
  PluginInitFunc init;      // 初始化函数（原生插件）
  PluginUpdateFunc update;  // 更新函数（原生插件）
  PluginLoopFunc loop;      // 循环函数（原生插件）
  PluginDeinitFunc deinit;  // 反初始化函数（原生插件）
  URLPluginData urlData;    // URL插件数据
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
  // 注册原生插件
  bool registerPlugin(String name, String version, String description,
                     PluginInitFunc init, PluginUpdateFunc update,
                     PluginLoopFunc loop, PluginDeinitFunc deinit);
  // 注册URL插件
  bool registerURLPlugin(String name, String version, String description,
                        PluginType type, String url, unsigned long updateInterval,
                        String dataPath, String displayFormat);
  bool unregisterPlugin(String name);
  bool enablePlugin(String name);
  bool disablePlugin(String name);
  bool isPluginEnabled(String name);
  
  // URL插件管理功能
  bool updateURLPlugin(String name);
  bool setURLPluginInterval(String name, unsigned long interval);
  bool setURLPluginPath(String name, String path);
  bool setURLPluginFormat(String name, String format);
  String getURLPluginData(String name);
  
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