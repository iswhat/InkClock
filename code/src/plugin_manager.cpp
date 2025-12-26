#include "plugin_manager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "web_client.h"
#include "wifi_manager.h"

PluginManager::PluginManager() {
  // 初始化插件数组
  for (int i = 0; i < MAX_PLUGINS; i++) {
    plugins[i].name = "";
    plugins[i].version = "";
    plugins[i].description = "";
    plugins[i].type = PLUGIN_TYPE_NATIVE;
    plugins[i].status = PLUGIN_DISABLED;
    plugins[i].init = NULL;
    plugins[i].update = NULL;
    plugins[i].loop = NULL;
    plugins[i].deinit = NULL;
    plugins[i].urlData.url = "";
    plugins[i].urlData.updateInterval = PLUGIN_UPDATE_INTERVAL;
    plugins[i].urlData.dataXPath = "";
    plugins[i].urlData.displayFormat = "%s";
    plugins[i].urlData.lastData = "";
    plugins[i].urlData.lastUpdateTime = 0;
    plugins[i].valid = false;
  }
  
  // 初始化插件计数
  pluginCount = 0;
  
  // 初始化更新标志
  lastUpdate = 0;
  dataUpdated = false;
}

PluginManager::~PluginManager() {
  // 清理资源，反初始化所有插件
  for (int i = 0; i < pluginCount; i++) {
    if (plugins[i].status == PLUGIN_ENABLED || plugins[i].status == PLUGIN_RUNNING) {
      deinitPlugin(i);
    }
  }
}

void PluginManager::init() {
  DEBUG_PRINTLN("初始化插件管理器...");
  
  // 初始化SPIFFS文件系统（如果未初始化）
  if (!SPIFFS.begin(false)) {
    DEBUG_PRINTLN("SPIFFS初始化失败");
    return;
  }
  
  // 加载保存的插件配置
  if (!loadPlugins()) {
    DEBUG_PRINTLN("加载插件配置失败，将使用默认配置");
    savePlugins();
  }
  
  DEBUG_PRINTLN("插件管理器初始化完成");
  DEBUG_PRINT("当前插件数: ");
  DEBUG_PRINTLN(pluginCount);
  
  // 初始化所有启用的插件
  for (int i = 0; i < pluginCount; i++) {
    if (plugins[i].status == PLUGIN_ENABLED) {
      initPlugin(i);
    }
  }
}

void PluginManager::update() {
  // 更新所有启用的插件
  for (int i = 0; i < pluginCount; i++) {
    PluginData &plugin = plugins[i];
    
    if (plugin.status == PLUGIN_RUNNING) {
      // 处理原生插件
      if (plugin.type == PLUGIN_TYPE_NATIVE && plugin.update != NULL) {
        plugin.update();
      }
      // 处理URL插件
      else if (plugin.type >= PLUGIN_TYPE_URL_XML && plugin.type <= PLUGIN_TYPE_URL_JS) {
        // 检查是否需要更新
        unsigned long currentTime = millis();
        if (currentTime - plugin.urlData.lastUpdateTime >= plugin.urlData.updateInterval) {
          updateURLPlugin(plugin.name);
        }
      }
    }
  }
  
  // 定期保存插件配置
  if (dataUpdated) {
    savePlugins();
    dataUpdated = false;
  }
}

void PluginManager::loop() {
  // 循环调用所有启用的插件
  for (int i = 0; i < pluginCount; i++) {
    if (plugins[i].status == PLUGIN_RUNNING && plugins[i].loop != NULL) {
      plugins[i].loop();
    }
  }
  
  // 定期更新插件
  static unsigned long lastUpdateCheck = 0;
  if (millis() - lastUpdateCheck > PLUGIN_UPDATE_INTERVAL) {
    lastUpdateCheck = millis();
    update();
  }
}

bool PluginManager::registerPlugin(String name, String version, String description,
                                 PluginInitFunc init, PluginUpdateFunc update,
                                 PluginLoopFunc loop, PluginDeinitFunc deinit) {
  DEBUG_PRINT("注册原生插件: ");
  DEBUG_PRINTLN(name);
  
  // 检查插件数组是否已满
  if (pluginCount >= MAX_PLUGINS) {
    DEBUG_PRINTLN("插件数组已满");
    return false;
  }
  
  // 检查插件是否已存在
  if (findPluginIndex(name) != -1) {
    DEBUG_PRINTLN("插件已存在");
    return false;
  }
  
  // 注册插件
  PluginData &plugin = plugins[pluginCount];
  plugin.name = name;
  plugin.version = version;
  plugin.description = description;
  plugin.type = PLUGIN_TYPE_NATIVE;
  plugin.status = PLUGIN_DISABLED;
  plugin.init = init;
  plugin.update = update;
  plugin.loop = loop;
  plugin.deinit = deinit;
  plugin.urlData.url = "";
  plugin.urlData.updateInterval = PLUGIN_UPDATE_INTERVAL;
  plugin.urlData.dataXPath = "";
  plugin.urlData.displayFormat = "%s";
  plugin.urlData.lastData = "";
  plugin.urlData.lastUpdateTime = 0;
  plugin.valid = true;
  
  // 更新插件计数
  pluginCount++;
  
  // 设置数据更新标志
  dataUpdated = true;
  
  DEBUG_PRINTLN("原生插件注册成功");
  return true;
}

/**
 * @brief 注册URL插件
 * @param name 插件名称
 * @param version 插件版本
 * @param description 插件描述
 * @param type 插件类型（XML/JSON/JS）
 * @param url 插件URL地址
 * @param updateInterval 更新间隔（毫秒）
 * @param dataPath 数据提取路径（XPath或JSON路径）
 * @param displayFormat 显示格式模板
 * @return bool 注册是否成功
 */
bool PluginManager::registerURLPlugin(String name, String version, String description,
                                    PluginType type, String url, unsigned long updateInterval,
                                    String dataPath, String displayFormat) {
  DEBUG_PRINT("注册URL插件: ");
  DEBUG_PRINTLN(name);
  
  // 检查插件数组是否已满
  if (pluginCount >= MAX_PLUGINS) {
    DEBUG_PRINTLN("插件数组已满");
    return false;
  }
  
  // 检查插件是否已存在
  if (findPluginIndex(name) != -1) {
    DEBUG_PRINTLN("插件已存在");
    return false;
  }
  
  // 注册URL插件
  PluginData &plugin = plugins[pluginCount];
  plugin.name = name;
  plugin.version = version;
  plugin.description = description;
  plugin.type = type;
  plugin.status = PLUGIN_ENABLED; // URL插件默认启用
  plugin.init = NULL;
  plugin.update = NULL;
  plugin.loop = NULL;
  plugin.deinit = NULL;
  plugin.urlData.url = url;
  plugin.urlData.updateInterval = updateInterval;
  plugin.urlData.dataXPath = dataPath;
  plugin.urlData.displayFormat = displayFormat;
  plugin.urlData.lastData = "";
  plugin.urlData.lastUpdateTime = 0;
  plugin.valid = true;
  
  // 更新插件计数
  pluginCount++;
  
  // 设置数据更新标志
  dataUpdated = true;
  
  DEBUG_PRINTLN("URL插件注册成功");
  return true;
}

bool PluginManager::unregisterPlugin(String name) {
  DEBUG_PRINT("注销插件: ");
  DEBUG_PRINTLN(name);
  
  // 查找插件索引
  int index = findPluginIndex(name);
  if (index == -1) {
    DEBUG_PRINTLN("插件不存在");
    return false;
  }
  
  // 反初始化插件
  deinitPlugin(index);
  
  // 注销插件
  for (int i = index; i < pluginCount - 1; i++) {
    plugins[i] = plugins[i + 1];
  }
  
  // 清空最后一个插件
  PluginData &lastPlugin = plugins[pluginCount - 1];
  lastPlugin.name = "";
  lastPlugin.version = "";
  lastPlugin.description = "";
  lastPlugin.status = PLUGIN_DISABLED;
  lastPlugin.init = NULL;
  lastPlugin.update = NULL;
  lastPlugin.loop = NULL;
  lastPlugin.deinit = NULL;
  lastPlugin.valid = false;
  
  // 更新插件计数
  pluginCount--;
  
  // 设置数据更新标志
  dataUpdated = true;
  
  DEBUG_PRINTLN("插件注销成功");
  return true;
}

bool PluginManager::enablePlugin(String name) {
  DEBUG_PRINT("启用插件: ");
  DEBUG_PRINTLN(name);
  
  // 查找插件索引
  int index = findPluginIndex(name);
  if (index == -1) {
    DEBUG_PRINTLN("插件不存在");
    return false;
  }
  
  // 初始化插件
  if (initPlugin(index)) {
    // 设置数据更新标志
    dataUpdated = true;
    DEBUG_PRINTLN("插件启用成功");
    return true;
  }
  
  DEBUG_PRINTLN("插件启用失败");
  return false;
}

bool PluginManager::disablePlugin(String name) {
  DEBUG_PRINT("禁用插件: ");
  DEBUG_PRINTLN(name);
  
  // 查找插件索引
  int index = findPluginIndex(name);
  if (index == -1) {
    DEBUG_PRINTLN("插件不存在");
    return false;
  }
  
  // 反初始化插件
  if (deinitPlugin(index)) {
    // 设置数据更新标志
    dataUpdated = true;
    DEBUG_PRINTLN("插件禁用成功");
    return true;
  }
  
  DEBUG_PRINTLN("插件禁用失败");
  return false;
}

bool PluginManager::isPluginEnabled(String name) {
  // 查找插件索引
  int index = findPluginIndex(name);
  if (index == -1) {
    return false;
  }
  
  return plugins[index].status == PLUGIN_RUNNING;
}

PluginData PluginManager::getPlugin(String name) {
  // 查找插件
  int index = findPluginIndex(name);
  if (index != -1) {
    return plugins[index];
  }
  
  // 返回无效插件
  PluginData invalidPlugin;
  invalidPlugin.name = "";
  invalidPlugin.version = "";
  invalidPlugin.description = "";
  invalidPlugin.status = PLUGIN_DISABLED;
  invalidPlugin.init = NULL;
  invalidPlugin.update = NULL;
  invalidPlugin.loop = NULL;
  invalidPlugin.deinit = NULL;
  invalidPlugin.valid = false;
  
  return invalidPlugin;
}

PluginData PluginManager::getPlugin(int index) {
  if (index >= 0 && index < pluginCount) {
    return plugins[index];
  }
  
  // 返回无效插件
  PluginData invalidPlugin;
  invalidPlugin.name = "";
  invalidPlugin.version = "";
  invalidPlugin.description = "";
  invalidPlugin.status = PLUGIN_DISABLED;
  invalidPlugin.init = NULL;
  invalidPlugin.update = NULL;
  invalidPlugin.loop = NULL;
  invalidPlugin.deinit = NULL;
  invalidPlugin.valid = false;
  
  return invalidPlugin;
}

int PluginManager::getPluginCount() {
  return pluginCount;
}

bool PluginManager::savePlugins() {
  DEBUG_PRINTLN("保存插件配置到文件...");
  
  // 创建JSON文档
  DynamicJsonDocument doc(4096);
  
  // 添加插件数组
  JsonArray pluginArray = doc.createNestedArray("plugins");
  
  for (int i = 0; i < pluginCount; i++) {
    JsonObject pluginObj = pluginArray.createNestedObject();
    pluginObj["name"] = plugins[i].name;
    pluginObj["version"] = plugins[i].version;
    pluginObj["description"] = plugins[i].description;
    pluginObj["status"] = plugins[i].status;
  }
  
  // 添加元数据
  doc["pluginCount"] = pluginCount;
  
  // 打开文件
  File file = SPIFFS.open("/plugins.json", FILE_WRITE);
  if (!file) {
    DEBUG_PRINTLN("无法打开插件文件进行写入");
    return false;
  }
  
  // 序列化JSON到文件
  if (serializeJson(doc, file) == 0) {
    DEBUG_PRINTLN("JSON序列化失败");
    file.close();
    return false;
  }
  
  // 关闭文件
  file.close();
  
  DEBUG_PRINTLN("插件配置保存成功");
  return true;
}

bool PluginManager::loadPlugins() {
  DEBUG_PRINTLN("从文件加载插件配置...");
  
  // 检查文件是否存在
  if (!SPIFFS.exists("/plugins.json")) {
    DEBUG_PRINTLN("插件配置文件不存在");
    return false;
  }
  
  // 打开文件
  File file = SPIFFS.open("/plugins.json", FILE_READ);
  if (!file) {
    DEBUG_PRINTLN("无法打开插件配置文件进行读取");
    return false;
  }
  
  // 创建JSON文档
  DynamicJsonDocument doc(4096);
  
  // 从文件反序列化JSON
  DeserializationError error = deserializeJson(doc, file);
  if (error) {
    DEBUG_PRINT("JSON反序列化失败: ");
    DEBUG_PRINTLN(error.c_str());
    file.close();
    return false;
  }
  
  // 关闭文件
  file.close();
  
  // 注意：这里只加载插件配置，不加载插件的函数指针
  // 函数指针需要在运行时通过registerPlugin注册
  
  DEBUG_PRINTLN("插件配置加载成功");
  return true;
}

int PluginManager::findPluginIndex(String name) {
  for (int i = 0; i < pluginCount; i++) {
    if (plugins[i].name == name) {
      return i;
    }
  }
  return -1;
}

bool PluginManager::initPlugin(int index) {
  if (index < 0 || index >= pluginCount) {
    return false;
  }
  
  PluginData &plugin = plugins[index];
  
  // 检查插件是否已经在运行
  if (plugin.status == PLUGIN_RUNNING) {
    return true;
  }
  
  // 检查插件是否有初始化函数
  if (plugin.init == NULL) {
    DEBUG_PRINT("插件 ");
    DEBUG_PRINT(plugin.name);
    DEBUG_PRINTLN(" 没有初始化函数");
    return false;
  }
  
  // 调用初始化函数
  if (plugin.init()) {
    plugin.status = PLUGIN_RUNNING;
    return true;
  }
  
  plugin.status = PLUGIN_ERROR;
  return false;
}

bool PluginManager::deinitPlugin(int index) {
  if (index < 0 || index >= pluginCount) {
    return false;
  }
  
  PluginData &plugin = plugins[index];
  
  // 检查插件是否已经禁用
  if (plugin.status == PLUGIN_DISABLED) {
    return true;
  }
  
  // 检查插件是否有反初始化函数
  if (plugin.deinit != NULL) {
    // 调用反初始化函数
    plugin.deinit();
  }
  
  // 更新插件状态
  plugin.status = PLUGIN_DISABLED;
  return true;
}