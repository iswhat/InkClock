#include "stock_manager.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "wifi_manager.h"
#include "time_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;
extern TimeManager timeManager;

// 股票API配置（示例，实际使用时需替换为可用的股票API）
#define STOCK_API_HOST "api.example.com"
#define STOCK_API_PATH "/stock/get"
#define STOCK_API_KEY "your_stock_api_key"

StockManager::StockManager() {
  // 初始化股票数组
  for (int i = 0; i < MAX_STOCKS; i++) {
    stocks[i].code = "";
    stocks[i].name = "";
    stocks[i].price = 0.0;
    stocks[i].change = 0.0;
    stocks[i].changePercent = 0.0;
    stocks[i].open = 0.0;
    stocks[i].high = 0.0;
    stocks[i].low = 0.0;
    stocks[i].close = 0.0;
    stocks[i].volume = 0;
    stocks[i].amount = 0;
    stocks[i].time = "";
    stocks[i].valid = false;
    
    // 初始化默认股票代码
    const char* defaultCodes[] = STOCK_CODES;
    if (i < sizeof(defaultCodes)/sizeof(defaultCodes[0])) {
      stockCodes[i] = String(defaultCodes[i]);
    } else {
      stockCodes[i] = "";
    }
  }
  
  // 初始化股票计数
  stockCount = sizeof(STOCK_CODES)/sizeof(STOCK_CODES[0]);
  if (stockCount > MAX_STOCKS) {
    stockCount = MAX_STOCKS;
  }
  
  // 初始化更新标志
  lastUpdate = 0;
  dataUpdated = false;
}

StockManager::~StockManager() {
  // 清理资源，保存股票列表
  saveStockList();
}

void StockManager::init() {
  DEBUG_PRINTLN("初始化股票管理器...");
  
  // 初始化SPIFFS文件系统（如果未初始化）
  if (!SPIFFS.begin(false)) {
    DEBUG_PRINTLN("SPIFFS初始化失败");
    return;
  }
  
  // 加载保存的股票列表
  if (!loadStockList()) {
    DEBUG_PRINTLN("加载股票列表失败，将使用默认股票列表");
    saveStockList();
  }
  
  DEBUG_PRINTLN("股票管理器初始化完成");
  DEBUG_PRINT("当前股票数: ");
  DEBUG_PRINTLN(stockCount);
  
  // 打印股票代码列表
  DEBUG_PRINT("股票代码列表: ");
  for (int i = 0; i < stockCount; i++) {
    DEBUG_PRINT(stockCodes[i]);
    if (i < stockCount - 1) {
      DEBUG_PRINT(", ");
    }
  }
  DEBUG_PRINTLN();
}

void StockManager::update() {
  // 只在WiFi连接时更新股票数据
  if (!wifiManager.isConnected()) {
    return;
  }
  
  DEBUG_PRINTLN("更新股票数据...");
  
  // 更新所有股票数据
  for (int i = 0; i < stockCount; i++) {
    if (stockCodes[i].length() > 0) {
      fetchStockData(stockCodes[i], stocks[i]);
    }
  }
  
  // 设置数据更新标志
  dataUpdated = true;
  lastUpdate = millis();
  
  DEBUG_PRINTLN("股票数据更新完成");
}

void StockManager::loop() {
  // 定期更新股票数据
  static unsigned long lastUpdateCheck = 0;
  if (millis() - lastUpdateCheck > STOCK_UPDATE_INTERVAL) {
    lastUpdateCheck = millis();
    update();
  }
}

bool StockManager::addStock(String code) {
  DEBUG_PRINT("添加股票: ");
  DEBUG_PRINTLN(code);
  
  // 检查股票数组是否已满
  if (stockCount >= MAX_STOCKS) {
    DEBUG_PRINTLN("股票列表已满");
    return false;
  }
  
  // 检查股票代码是否已存在
  for (int i = 0; i < stockCount; i++) {
    if (stockCodes[i] == code) {
      DEBUG_PRINTLN("股票已存在");
      return false;
    }
  }
  
  // 添加股票代码
  stockCodes[stockCount] = code;
  
  // 初始化股票数据
  stocks[stockCount].code = code;
  stocks[stockCount].name = "";
  stocks[stockCount].price = 0.0;
  stocks[stockCount].change = 0.0;
  stocks[stockCount].changePercent = 0.0;
  stocks[stockCount].open = 0.0;
  stocks[stockCount].high = 0.0;
  stocks[stockCount].low = 0.0;
  stocks[stockCount].close = 0.0;
  stocks[stockCount].volume = 0;
  stocks[stockCount].amount = 0;
  stocks[stockCount].time = "";
  stocks[stockCount].valid = false;
  
  // 更新股票计数
  stockCount++;
  
  // 保存股票列表
  saveStockList();
  
  // 更新股票数据
  fetchStockData(code, stocks[stockCount - 1]);
  
  DEBUG_PRINTLN("股票添加成功");
  return true;
}

bool StockManager::removeStock(int index) {
  DEBUG_PRINT("删除股票，索引: ");
  DEBUG_PRINTLN(index);
  
  // 检查索引是否有效
  if (index < 0 || index >= stockCount) {
    DEBUG_PRINTLN("无效的股票索引");
    return false;
  }
  
  // 删除股票
  for (int i = index; i < stockCount - 1; i++) {
    stockCodes[i] = stockCodes[i + 1];
    stocks[i] = stocks[i + 1];
  }
  
  // 清空最后一个股票
  stockCodes[stockCount - 1] = "";
  stocks[stockCount - 1].code = "";
  stocks[stockCount - 1].name = "";
  stocks[stockCount - 1].price = 0.0;
  stocks[stockCount - 1].change = 0.0;
  stocks[stockCount - 1].changePercent = 0.0;
  stocks[stockCount - 1].open = 0.0;
  stocks[stockCount - 1].high = 0.0;
  stocks[stockCount - 1].low = 0.0;
  stocks[stockCount - 1].close = 0.0;
  stocks[stockCount - 1].volume = 0;
  stocks[stockCount - 1].amount = 0;
  stocks[stockCount - 1].time = "";
  stocks[stockCount - 1].valid = false;
  
  // 更新股票计数
  stockCount--;
  
  // 保存股票列表
  saveStockList();
  
  DEBUG_PRINTLN("股票删除成功");
  return true;
}

bool StockManager::setStockList(String codes[]) {
  DEBUG_PRINTLN("设置股票列表...");
  
  // 清空现有股票列表
  stockCount = 0;
  
  // 添加新的股票代码
  for (int i = 0; i < MAX_STOCKS; i++) {
    if (codes[i].length() > 0) {
      stockCodes[stockCount] = codes[i];
      
      // 初始化股票数据
      stocks[stockCount].code = codes[i];
      stocks[stockCount].name = "";
      stocks[stockCount].price = 0.0;
      stocks[stockCount].change = 0.0;
      stocks[stockCount].changePercent = 0.0;
      stocks[stockCount].open = 0.0;
      stocks[stockCount].high = 0.0;
      stocks[stockCount].low = 0.0;
      stocks[stockCount].close = 0.0;
      stocks[stockCount].volume = 0;
      stocks[stockCount].amount = 0;
      stocks[stockCount].time = "";
      stocks[stockCount].valid = false;
      
      stockCount++;
    }
  }
  
  // 保存股票列表
  saveStockList();
  
  // 更新所有股票数据
  update();
  
  DEBUG_PRINTLN("股票列表设置成功");
  return true;
}

StockData StockManager::getStockData(int index) {
  if (index >= 0 && index < stockCount) {
    return stocks[index];
  }
  
  // 返回无效股票数据
  StockData invalidStock;
  invalidStock.code = "";
  invalidStock.name = "";
  invalidStock.price = 0.0;
  invalidStock.change = 0.0;
  invalidStock.changePercent = 0.0;
  invalidStock.open = 0.0;
  invalidStock.high = 0.0;
  invalidStock.low = 0.0;
  invalidStock.close = 0.0;
  invalidStock.volume = 0;
  invalidStock.amount = 0;
  invalidStock.time = "";
  invalidStock.valid = false;
  
  return invalidStock;
}

int StockManager::getStockCount() {
  return stockCount;
}

bool StockManager::saveStockList() {
  DEBUG_PRINTLN("保存股票列表到文件...");
  
  // 创建JSON文档
  DynamicJsonDocument doc(2048);
  
  // 添加股票代码数组
  JsonArray codeArray = doc.createNestedArray("stockCodes");
  
  for (int i = 0; i < stockCount; i++) {
    codeArray.add(stockCodes[i]);
  }
  
  // 添加元数据
  doc["stockCount"] = stockCount;
  
  // 打开文件
  File file = SPIFFS.open("/stocks.json", FILE_WRITE);
  if (!file) {
    DEBUG_PRINTLN("无法打开股票文件进行写入");
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
  
  DEBUG_PRINTLN("股票列表保存成功");
  return true;
}

bool StockManager::loadStockList() {
  DEBUG_PRINTLN("从文件加载股票列表...");
  
  // 检查文件是否存在
  if (!SPIFFS.exists("/stocks.json")) {
    DEBUG_PRINTLN("股票文件不存在");
    return false;
  }
  
  // 打开文件
  File file = SPIFFS.open("/stocks.json", FILE_READ);
  if (!file) {
    DEBUG_PRINTLN("无法打开股票文件进行读取");
    return false;
  }
  
  // 创建JSON文档
  DynamicJsonDocument doc(2048);
  
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
  
  // 加载股票代码数组
  JsonArray codeArray = doc["stockCodes"];
  
  // 清空现有股票列表
  stockCount = 0;
  
  for (String code : codeArray) {
    // 检查股票数组是否已满
    if (stockCount >= MAX_STOCKS) {
      break;
    }
    
    // 添加股票代码
    stockCodes[stockCount] = code;
    
    // 初始化股票数据
    stocks[stockCount].code = code;
    stocks[stockCount].name = "";
    stocks[stockCount].price = 0.0;
    stocks[stockCount].change = 0.0;
    stocks[stockCount].changePercent = 0.0;
    stocks[stockCount].open = 0.0;
    stocks[stockCount].high = 0.0;
    stocks[stockCount].low = 0.0;
    stocks[stockCount].close = 0.0;
    stocks[stockCount].volume = 0;
    stocks[stockCount].amount = 0;
    stocks[stockCount].time = "";
    stocks[stockCount].valid = false;
    
    stockCount++;
  }
  
  DEBUG_PRINT("股票列表加载成功，共加载 ");
  DEBUG_PRINT(stockCount);
  DEBUG_PRINTLN(" 只股票");
  
  return true;
}

bool StockManager::fetchStockData(String code, StockData &data) {
  DEBUG_PRINT("获取股票数据: ");
  DEBUG_PRINTLN(code);
  
  // 检查WiFi连接
  if (!wifiManager.isConnected()) {
    DEBUG_PRINTLN("WiFi未连接，无法获取股票数据");
    return false;
  }
  
  // 构建API请求URL
  String url = getStockApiUrl(code);
  
  // 连接到API服务器
  WiFiClientSecure client;
  client.setInsecure(); // 禁用证书验证，简化开发
  
  if (!client.connect(STOCK_API_HOST, 443)) {
    DEBUG_PRINTLN("无法连接到股票API服务器");
    return false;
  }
  
  // 发送HTTP请求
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + String(STOCK_API_HOST) + "\r\n" +
               "Connection: close\r\n\r\n");
  
  // 等待响应
  delay(2000);
  
  // 读取响应
  String response = "";
  while (client.available()) {
    String line = client.readStringUntil('\r');
    response += line;
  }
  
  // 关闭连接
  client.stop();
  
  // 解析响应
  int jsonIndex = response.indexOf('{');
  if (jsonIndex == -1) {
    DEBUG_PRINTLN("无法找到JSON数据");
    return false;
  }
  
  String json = response.substring(jsonIndex);
  
  // 解析股票数据
  if (!parseStockData(json, data)) {
    DEBUG_PRINTLN("解析股票数据失败");
    return false;
  }
  
  // 更新时间
  data.time = timeManager.getDateTimeString();
  
  DEBUG_PRINT("股票数据获取成功: " + data.name + " (" + data.code + ") ");
  DEBUG_PRINT(data.price);
  DEBUG_PRINT(" " + String(data.change, 2));
  DEBUG_PRINTLN(" (" + String(data.changePercent, 2) + "%)");
  
  return true;
}

bool StockManager::parseStockData(String json, StockData &data) {
  // 解析JSON数据
  DynamicJsonDocument doc(2048);
  DeserializationError error = deserializeJson(doc, json);
  
  if (error) {
    DEBUG_PRINT("JSON解析错误: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // 示例解析，实际解析需根据API返回格式调整
  // 这里假设API返回的JSON格式如下：
  // {
  //   "code": "600000",
  //   "name": "浦发银行",
  //   "price": 8.50,
  //   "change": 0.10,
  //   "changePercent": 1.19,
  //   "open": 8.40,
  //   "high": 8.55,
  //   "low": 8.38,
  //   "close": 8.40,
  //   "volume": 123456789,
  //   "amount": 1049382606
  // }
  
  // 解析股票数据
  data.code = doc["code"].as<String>();
  data.name = doc["name"].as<String>();
  data.price = doc["price"].as<float>();
  data.change = doc["change"].as<float>();
  data.changePercent = doc["changePercent"].as<float>();
  data.open = doc["open"].as<float>();
  data.high = doc["high"].as<float>();
  data.low = doc["low"].as<float>();
  data.close = doc["close"].as<float>();
  data.volume = doc["volume"].as<long>();
  data.amount = doc["amount"].as<long>();
  data.valid = true;
  
  return true;
}

String StockManager::getStockApiUrl(String code) {
  // 构建股票API请求URL
  String url = String(STOCK_API_PATH) + 
               "?code=" + code + 
               "&apikey=" + String(STOCK_API_KEY);
  
  return url;
}