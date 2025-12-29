#include "plugin_manager.h"
#include <SPIFFS.h>
#include <ArduinoJson.h>
#include "../app/web_client.h"
#include "../services/wifi_manager.h"

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

/**
 * @brief 更新URL插件数据
 * @param name 插件名称
 * @return bool 更新是否成功
 * @note 根据插件类型从URL获取数据并解析
 */
bool PluginManager::updateURLPlugin(String name) {
  DEBUG_PRINT("更新URL插件: ");
  DEBUG_PRINTLN(name);
  
  // 查找插件
  int index = findPluginIndex(name);
  if (index == -1) {
    DEBUG_PRINTLN("插件不存在");
    return false;
  }
  
  PluginData &plugin = plugins[index];
  
  // 检查插件是否为URL插件
  if (plugin.type < PLUGIN_TYPE_URL_XML || plugin.type > PLUGIN_TYPE_URL_JS) {
    DEBUG_PRINTLN("不是URL插件");
    return false;
  }
  
  // 检查WiFi连接状态
  if (!WiFi.isConnected()) {
    DEBUG_PRINTLN("WiFi未连接");
    return false;
  }
  
  // 检查URL是否有效
  if (plugin.urlData.url.isEmpty()) {
    DEBUG_PRINTLN("URL无效");
    return false;
  }
  
  // 从URL获取数据
  WebClient webClient;
  String response = webClient.sendRequest(plugin.urlData.url, "GET");
  
  if (response.isEmpty()) {
    DEBUG_PRINTLN("获取数据失败");
    return false;
  }
  
  String extractedData = "";
  
  // 根据插件类型解析数据
    switch (plugin.type) {
        case PLUGIN_TYPE_URL_XML:
            // 解析XML数据
            // 轻量级XML解析，支持简单XPath如 /root/node 或 /root/node/text()
            { 
                String path = plugin.urlData.dataXPath;
                bool extractText = path.endsWith("/text()");
                if (extractText) {
                    path = path.substring(0, path.length() - 7);
                }
                
                // 移除首尾的/，并分割路径
                if (path.startsWith("/")) path = path.substring(1);
                if (path.endsWith("/")) path = path.substring(0, path.length() - 1);
                
                // 简单的XML标签提取
                String startTag, endTag;
                if (path.indexOf('/') == -1) {
                    // 直接子节点
                    startTag = "<" + path + ">";
                    endTag = "</" + path + ">";
                } else {
                    // 多级路径，只处理最后一级
                    int lastSlash = path.lastIndexOf('/');
                    String tagName = path.substring(lastSlash + 1);
                    startTag = "<" + tagName + ">";
                    endTag = "</" + tagName + ">";
                }
                
                int startPos = response.indexOf(startTag);
                int endPos = response.indexOf(endTag, startPos + startTag.length());
                
                if (startPos != -1 && endPos != -1) {
                    startPos += startTag.length();
                    extractedData = response.substring(startPos, endPos);
                    // 去除首尾空格
                    extractedData.trim();
                } else {
                    // 无法找到标签，使用原始数据的前100个字符
                    extractedData = response.substring(0, min(response.length(), 100));
                }
            }
            DEBUG_PRINTLN("XML解析完成");
            break;
            
        case PLUGIN_TYPE_URL_JSON:
            // 解析JSON数据
            try {
                DynamicJsonDocument doc(2048);
                DeserializationError error = deserializeJson(doc, response);
                if (error) {
                    DEBUG_PRINT("JSON解析失败: ");
                    DEBUG_PRINTLN(error.c_str());
                    return false;
                }
                
                // 根据JSON路径提取数据
                JsonVariant data = doc;
                if (!plugin.urlData.dataXPath.isEmpty()) {
                    // 支持多级JSON路径，如 "data.temperature" 或 "weather[0].main"
                    String path = plugin.urlData.dataXPath;
                    
                    // 分割路径为多个部分
                    int start = 0;
                    while (start < path.length()) {
                        int dotPos = path.indexOf('.', start);
                        int bracketOpenPos = path.indexOf('[', start);
                        
                        int nextSeparatorPos;
                        if (dotPos != -1 && (bracketOpenPos == -1 || dotPos < bracketOpenPos)) {
                            nextSeparatorPos = dotPos;
                        } else if (bracketOpenPos != -1) {
                            nextSeparatorPos = bracketOpenPos;
                        } else {
                            nextSeparatorPos = path.length();
                        }
                        
                        // 提取当前路径部分
                        String key = path.substring(start, nextSeparatorPos);
                        
                        // 处理对象访问
                        if (nextSeparatorPos == dotPos || nextSeparatorPos == path.length()) {
                            if (data.is<JsonObject>() && data.as<JsonObject>().containsKey(key)) {
                                data = data.as<JsonObject>()[key];
                                start = nextSeparatorPos + 1;
                            } else {
                                // 路径不存在，使用原始数据
                                break;
                            }
                        }
                        // 处理数组访问
                        else if (nextSeparatorPos == bracketOpenPos) {
                            if (data.is<JsonArray>()) {
                                JsonArray arr = data.as<JsonArray>();
                                // 提取数组索引
                                int bracketClosePos = path.indexOf(']', bracketOpenPos);
                                if (bracketClosePos != -1) {
                                    String indexStr = path.substring(bracketOpenPos + 1, bracketClosePos);
                                    int index = indexStr.toInt();
                                    if (index >= 0 && index < arr.size()) {
                                        data = arr[index];
                                        start = bracketClosePos + 1;
                                        // 如果下一个字符是点，跳过它
                                        if (start < path.length() && path[start] == '.') {
                                            start++;
                                        }
                                    } else {
                                        // 索引无效，使用原始数据
                                        break;
                                    }
                                } else {
                                    // 无效的数组语法，使用原始数据
                                    break;
                                }
                            } else {
                                // 不是数组，使用原始数据
                                break;
                            }
                        }
                    }
                }
                
                // 转换为字符串
                StringWriter writer;
                serializeJson(data, writer);
                extractedData = writer.str();
                
                // 如果是简单类型，去除引号
                if (extractedData.startsWith("\"") && extractedData.endsWith("\"")) {
                    extractedData = extractedData.substring(1, extractedData.length() - 1);
                }
            } catch (const std::exception& e) {
                DEBUG_PRINT("JSON解析异常: ");
                DEBUG_PRINTLN(e.what());
                return false;
            }
            DEBUG_PRINTLN("JSON解析完成");
            break;
            
        case PLUGIN_TYPE_URL_JS:
            // 解析JS数据
            // 轻量级JS解析，支持简单的变量访问和表达式
            { 
                // 查找变量声明或返回语句
                String jsCode = response;
                String targetVar = plugin.urlData.dataXPath;
                
                // 简单的JS变量提取
                // 支持：var name = value; 或 const name = value; 或 let name = value;
                String patterns[] = {
                    "var " + targetVar + " = ",
                    "const " + targetVar + " = ",
                    "let " + targetVar + " = ",
                    targetVar + " = "
                };
                
                extractedData = "";
                for (const String& pattern : patterns) {
                    int pos = jsCode.indexOf(pattern);
                    if (pos != -1) {
                        pos += pattern.length();
                        int endPos = jsCode.indexOf(';', pos);
                        if (endPos != -1) {
                            extractedData = jsCode.substring(pos, endPos);
                            break;
                        }
                    }
                }
                
                // 如果没有找到变量，尝试查找返回语句
                if (extractedData.isEmpty()) {
                    int returnPos = jsCode.indexOf("return ");
                    if (returnPos != -1) {
                        returnPos += 7;
                        int endPos = jsCode.indexOf(';', returnPos);
                        if (endPos != -1) {
                            extractedData = jsCode.substring(returnPos, endPos);
                        } else {
                            // 没有分号，取到行尾
                            endPos = jsCode.indexOf('\n', returnPos);
                            if (endPos != -1) {
                                extractedData = jsCode.substring(returnPos, endPos);
                            } else {
                                extractedData = jsCode.substring(returnPos);
                            }
                        }
                    }
                }
                
                // 去除首尾空格和引号
                extractedData.trim();
                if (extractedData.startsWith("\"") && extractedData.endsWith("\"")) {
                    extractedData = extractedData.substring(1, extractedData.length() - 1);
                }
                
                // 如果还是空，使用原始数据的前100个字符
                if (extractedData.isEmpty()) {
                    extractedData = response.substring(0, min(response.length(), 100));
                }
            }
            DEBUG_PRINTLN("JS解析完成");
            break;
            
        default:
            DEBUG_PRINTLN("未知的URL插件类型");
            return false;
    }
  
  // 更新插件数据
  plugin.urlData.lastData = extractedData;
  plugin.urlData.lastUpdateTime = millis();
  
  DEBUG_PRINTF("URL插件更新成功，获取数据: %s\n", extractedData.c_str());
  return true;
}

/**
 * @brief 设置URL插件的更新间隔
 * @param name 插件名称
 * @param interval 更新间隔（毫秒）
 * @return bool 设置是否成功
 */
bool PluginManager::setURLPluginInterval(String name, unsigned long interval) {
  int index = findPluginIndex(name);
  if (index == -1) {
    return false;
  }
  
  plugins[index].urlData.updateInterval = interval;
  dataUpdated = true;
  return true;
}

/**
 * @brief 设置URL插件的数据提取路径
 * @param name 插件名称
 * @param path 数据提取路径（XPath或JSON路径）
 * @return bool 设置是否成功
 */
bool PluginManager::setURLPluginPath(String name, String path) {
  int index = findPluginIndex(name);
  if (index == -1) {
    return false;
  }
  
  plugins[index].urlData.dataXPath = path;
  dataUpdated = true;
  return true;
}

/**
 * @brief 设置URL插件的显示格式
 * @param name 插件名称
 * @param format 显示格式模板
 * @return bool 设置是否成功
 */
bool PluginManager::setURLPluginFormat(String name, String format) {
  int index = findPluginIndex(name);
  if (index == -1) {
    return false;
  }
  
  plugins[index].urlData.displayFormat = format;
  dataUpdated = true;
  return true;
}

/**
 * @brief 获取URL插件的最新数据
 * @param name 插件名称
 * @return String 插件的最新数据
 */
String PluginManager::getURLPluginData(String name) {
  int index = findPluginIndex(name);
  if (index == -1) {
    return "";
  }
  
  return plugins[index].urlData.lastData;
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