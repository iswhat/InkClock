#include "stock_manager.h"
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <SPIFFS.h>
#include "../services/wifi_manager.h"
#include "../services/time_manager.h"

// 外部全局对象
extern WiFiManager wifiManager;
extern TimeManager timeManager;
extern APIManager apiManager;

// 股票API配置 - 使用公共免密钥API
#define STOCK_API_HOST_PRIMARY "api.money.126.net" // 主股票API（网易财经，公共免密钥）
#define STOCK_API_HOST_BACKUP "hq.sinajs.cn" // 备用股票API（新浪财经，公共免密钥）
#define STOCK_API_HOST_SECONDARY_BACKUP "push2.eastmoney.com" // 次备用股票API（东方财富，公共免密钥）

StockManager::StockManager() {
  // 初始化股票数组
  for (int i = 0; i < MAX_STOCKS; i++) {
    stocks[i].code = "";
    stocks[i].name = "";
    stocks[i].market = "";
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
    stocks[i].chartDataCount = 0;
    // 初始化大盘曲线数据点
    for (int j = 0; j < 50; j++) {
      stocks[i].chartData[j].price = 0.0;
      stocks[i].chartData[j].time = "";
    }
    
    // 初始化默认股票代码
    const char* defaultCodes[] = STOCK_CODES;
    if (i < sizeof(defaultCodes)/sizeof(defaultCodes[0])) {
      String code = String(defaultCodes[i]);
      stockCodes[i] = code;
      // 解析股票市场（默认处理）
      if (code.startsWith("6") || code.startsWith("sh")) {
        stocks[i].market = "sh";
      } else if (code.startsWith("0") || code.startsWith("3") || code.startsWith("sz")) {
        stocks[i].market = "sz";
      } else {
        stocks[i].market = "sh"; // 默认上海市场
      }
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

bool StockManager::addStock(String code, String market) {
  DEBUG_PRINT("添加股票: ");
  DEBUG_PRINT(code);
  DEBUG_PRINT(" (市场: ");
  DEBUG_PRINT(market);
  DEBUG_PRINTLN(")");
  
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
  stocks[stockCount].market = market;
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
  stocks[stockCount].chartDataCount = 0;
  // 初始化大盘曲线数据点
  for (int j = 0; j < 50; j++) {
    stocks[stockCount].chartData[j].price = 0.0;
    stocks[stockCount].chartData[j].time = "";
  }
  
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
  stocks[stockCount - 1].market = "";
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
  stocks[stockCount - 1].chartDataCount = 0;
  // 清空大盘曲线数据点
  for (int j = 0; j < 50; j++) {
    stocks[stockCount - 1].chartData[j].price = 0.0;
    stocks[stockCount - 1].chartData[j].time = "";
  }
  
  // 更新股票计数
  stockCount--;
  
  // 保存股票列表
  saveStockList();
  
  DEBUG_PRINTLN("股票删除成功");
  return true;
}

bool StockManager::fetchStockChartData(String code, String market, StockData &data) {
  DEBUG_PRINT("获取股票曲线数据: ");
  DEBUG_PRINTLN(code);
  
  // 检查WiFi连接
  if (!wifiManager.isConnected()) {
    DEBUG_PRINTLN("WiFi未连接，无法获取股票曲线数据");
    return false;
  }
  
  // 尝试使用东方财富API获取曲线数据
  String chartUrl = "https://" + String(STOCK_API_HOST_SECONDARY_BACKUP) + "/api/qt/stock/kline/get?secid=" + (market == "sh" ? "1." + code : "0." + code) + "&fields1=f1,f2,f3,f4,f5,f6&fields2=f51,f52,f53,f54,f55,f56,f57,f58&klt=1&fqt=0&end=20500101&lmt=50";
  
  DEBUG_PRINT("尝试获取曲线数据: ");
  DEBUG_PRINTLN(chartUrl);
  
  ApiResponse chartResponse = apiManager.get(chartUrl, API_TYPE_STOCK, 600000); // 缓存10分钟
  if (chartResponse.status != API_STATUS_SUCCESS && chartResponse.status != API_STATUS_CACHED) {
    DEBUG_PRINTLN("获取股票曲线数据失败: " + chartResponse.error);
    return false;
  }
  
  String response = chartResponse.response;
  if (response.isEmpty()) {
    DEBUG_PRINTLN("获取股票曲线数据失败，响应为空");
    return false;
  }
  
  // 解析曲线数据
  DynamicJsonDocument doc(4096);
  DeserializationError error = deserializeJson(doc, response);
  
  if (error) {
    DEBUG_PRINT("股票曲线数据解析失败: ");
    DEBUG_PRINTLN(error.c_str());
    return false;
  }
  
  // 检查响应状态
  if (doc.containsKey("errorcode") && doc["errorcode"].as<int>() != 0) {
    DEBUG_PRINT("股票曲线API请求失败: ");
    DEBUG_PRINTLN(doc["errmsg"].as<String>());
    return false;
  }
  
  // 解析曲线数据
  JsonObject dataObj = doc["data"];
  String klines = dataObj["klines"];
  
  // 清空现有曲线数据
  data.chartDataCount = 0;
  
  // 解析k线数据
  int startIndex = 0;
  int endIndex = klines.indexOf('","');
  
  while (endIndex != -1 && data.chartDataCount < 50) {
    String kline = klines.substring(startIndex + 2, endIndex); // 去掉引号
    
    // 解析k线数据（时间,开盘价,收盘价,最高价,最低价,成交量,成交额）
    int commaIndex = kline.indexOf(',');
    if (commaIndex != -1) {
      String time = kline.substring(0, commaIndex);
      
      // 继续查找收盘价
      int priceIndex = kline.indexOf(',', commaIndex + 1);
      if (priceIndex != -1) {
        priceIndex = kline.indexOf(',', priceIndex + 1); // 跳过开盘价，找到收盘价
        if (priceIndex != -1) {
          float price = kline.substring(priceIndex + 1, kline.indexOf(',', priceIndex + 1)).toFloat();
          
          // 添加到曲线数据
          data.chartData[data.chartDataCount].time = time;
          data.chartData[data.chartDataCount].price = price;
          data.chartDataCount++;
        }
      }
    }
    
    startIndex = endIndex + 3; // 移动到下一个k线数据
    endIndex = klines.indexOf('","', startIndex);
  }
  
  DEBUG_PRINT("股票曲线数据获取成功，共 ");
  DEBUG_PRINT(data.chartDataCount);
  DEBUG_PRINTLN(" 个数据点");
  
  return true;
}

bool StockManager::setStockList(String codes[], String markets[]) {
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
      stocks[stockCount].market = markets[i].length() > 0 ? markets[i] : "sh"; // 默认上海市场
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
      stocks[stockCount].chartDataCount = 0;
      // 初始化大盘曲线数据点
      for (int j = 0; j < 50; j++) {
        stocks[stockCount].chartData[j].price = 0.0;
        stocks[stockCount].chartData[j].time = "";
      }
      
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
  invalidStock.market = "";
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
  DEBUG_PRINTLN("获取股票数据: " + code);
  
  // 检查WiFi连接
  if (!wifiManager.isConnected()) {
    DEBUG_PRINTLN("WiFi未连接，无法获取股票数据");
    return false;
  }
  
  // 尝试使用主API（网易财经）
  String primaryUrl = "https://" + String(STOCK_API_HOST_PRIMARY) + getStockApiUrl(code, 1);
  DEBUG_PRINT("尝试使用主API: ");
  DEBUG_PRINTLN(primaryUrl);
  
  ApiResponse primaryResponse = apiManager.get(primaryUrl, API_TYPE_STOCK, 600000); // 缓存10分钟
  if (primaryResponse.status == API_STATUS_SUCCESS || primaryResponse.status == API_STATUS_CACHED) {
    if (!primaryResponse.response.isEmpty()) {
      if (parseStockData(primaryResponse.response, data, 1)) {
        // 更新时间
        data.time = timeManager.getDateTimeString();
        
        DEBUG_PRINT("股票数据获取成功: " + data.name + " (" + data.code + ") ");
        DEBUG_PRINT(data.price);
        DEBUG_PRINT(" " + String(data.change, 2));
        DEBUG_PRINTLN(" (" + String(data.changePercent, 2) + "%)");
        
        // 获取股票曲线数据
        fetchStockChartData(data.code, data.market, data);
        
        return true;
      }
    }
  }
  
  DEBUG_PRINTLN("主API获取股票数据失败: " + primaryResponse.error);
  
  // 尝试使用备用API（新浪财经）
  String backupUrl = "https://" + String(STOCK_API_HOST_BACKUP) + getStockApiUrl(code, 2);
  DEBUG_PRINT("尝试使用备用API: ");
  DEBUG_PRINTLN(backupUrl);
  
  ApiResponse backupResponse = apiManager.get(backupUrl, API_TYPE_STOCK, 600000);
  if (backupResponse.status == API_STATUS_SUCCESS || backupResponse.status == API_STATUS_CACHED) {
    if (!backupResponse.response.isEmpty()) {
      if (parseStockData(backupResponse.response, data, 2)) {
        // 更新时间
        data.time = timeManager.getDateTimeString();
        
        DEBUG_PRINT("股票数据获取成功: " + data.name + " (" + data.code + ") ");
        DEBUG_PRINT(data.price);
        DEBUG_PRINT(" " + String(data.change, 2));
        DEBUG_PRINTLN(" (" + String(data.changePercent, 2) + "%)");
        
        // 获取股票曲线数据
        fetchStockChartData(data.code, data.market, data);
        
        return true;
      }
    }
  }
  
  DEBUG_PRINTLN("备用API获取股票数据失败: " + backupResponse.error);
  
  // 尝试使用次备用API（东方财富）
  String secondaryBackupUrl = "https://" + String(STOCK_API_HOST_SECONDARY_BACKUP) + getStockApiUrl(code, 3);
  DEBUG_PRINT("尝试使用次备用API: ");
  DEBUG_PRINTLN(secondaryBackupUrl);
  
  ApiResponse secondaryBackupResponse = apiManager.get(secondaryBackupUrl, API_TYPE_STOCK, 600000);
  if (secondaryBackupResponse.status == API_STATUS_SUCCESS || secondaryBackupResponse.status == API_STATUS_CACHED) {
    if (!secondaryBackupResponse.response.isEmpty()) {
      if (parseStockData(secondaryBackupResponse.response, data, 3)) {
        // 更新时间
        data.time = timeManager.getDateTimeString();
        
        DEBUG_PRINT("股票数据获取成功: " + data.name + " (" + data.code + ") ");
        DEBUG_PRINT(data.price);
        DEBUG_PRINT(" " + String(data.change, 2));
        DEBUG_PRINTLN(" (" + String(data.changePercent, 2) + "%)");
        
        // 获取股票曲线数据
        fetchStockChartData(data.code, data.market, data);
        
        return true;
      }
    }
  }
  
  DEBUG_PRINTLN("次备用API获取股票数据失败: " + secondaryBackupResponse.error);
  
  return false;
}

bool StockManager::parseStockData(String response, StockData &data, int apiType) {
  // 根据不同API类型解析数据
  switch(apiType) {
    case 1: // 网易财经
      {
        // 解析JSONP响应
        int jsonIndex = response.indexOf('{');
        int endIndex = response.lastIndexOf('}');
        if (jsonIndex == -1 || endIndex == -1) {
          DEBUG_PRINTLN("无法找到JSON数据");
          return false;
        }
        
        // 优化：直接使用响应的子字符串，避免创建新的String对象
        // 优化：减少JSON文档大小，只使用必要的大小
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, response.substring(jsonIndex, endIndex + 1));
        
        if (error) {
          DEBUG_PRINT("网易财经API JSON解析失败: ");
          DEBUG_PRINTLN(error.c_str());
          return false;
        }
        
        JsonObject stock = doc[data.code];
        if (stock.isNull()) {
          DEBUG_PRINTLN("找不到股票数据");
          return false;
        }
        
        // 只解析必要的字段，减少CPU占用
        data.name = stock["name"].as<String>();
        data.price = stock["price"].as<float>();
        data.change = stock["pricechange"].as<float>();
        data.changePercent = stock["percent"].as<float>();
        data.open = stock["open"].as<float>();
        data.high = stock["high"].as<float>();
        data.low = stock["low"].as<float>();
        data.close = stock["close"].as<float>();
        data.volume = stock["volume"].as<long>();
        data.amount = stock["amount"].as<long>();
        data.valid = true;
        
        return true;
      }
    case 2: // 新浪财经
      {
        // 解析CSV格式响应
        int startIndex = response.indexOf('"');
        int endIndex = response.lastIndexOf('"');
        if (startIndex == -1 || endIndex == -1) {
          DEBUG_PRINTLN("无法找到股票数据");
          return false;
        }
        
        String csv = response.substring(startIndex + 1, endIndex);
        int commaIndex = csv.indexOf(',');
        if (commaIndex == -1) {
          DEBUG_PRINTLN("无法解析股票数据");
          return false;
        }
        
        // 新浪财经格式：name,open,high,low,close,volume,...
        int index = 0;
        String fields[8];
        for (int i = 0; i < 8; i++) {
          int nextComma = csv.indexOf(',', index);
          if (nextComma == -1 && i < 7) {
            DEBUG_PRINTLN("无法解析所有股票字段");
            return false;
          }
          fields[i] = csv.substring(index, i < 7 ? nextComma : csv.length());
          index = nextComma + 1;
        }
        
        data.name = fields[0];
        data.open = fields[1].toFloat();
        data.high = fields[2].toFloat();
        data.low = fields[3].toFloat();
        data.price = fields[4].toFloat();
        data.close = fields[4].toFloat(); // 新浪财经返回的是当前价，没有单独的收盘价
        data.volume = fields[5].toLong();
        data.change = data.price - data.close;
        data.changePercent = data.close > 0 ? (data.change / data.close) * 100 : 0;
        data.amount = 0; // 新浪财经API没有直接提供成交额
        data.valid = true;
        
        return true;
      }
    case 3: // 东方财富
      {
        // 解析JSON响应
        // 优化：减少JSON文档大小，只使用必要的大小
        DynamicJsonDocument doc(1024);
        DeserializationError error = deserializeJson(doc, response);
        
        if (error) {
          DEBUG_PRINT("东方财富API JSON解析失败: ");
          DEBUG_PRINTLN(error.c_str());
          return false;
        }
        
        if (doc.containsKey("errorcode") && doc["errorcode"].as<int>() != 0) {
          DEBUG_PRINT("东方财富API请求失败: ");
          DEBUG_PRINTLN(doc["errmsg"].as<String>());
          return false;
        }
        
        JsonObject dataObj = doc["data"];
        JsonObject fieldData = dataObj["data"];
        
        // 只解析必要的字段，减少CPU占用
        data.name = fieldData["name"].as<String>();
        data.price = fieldData["f43"].as<float>(); // 最新价
        data.high = fieldData["f44"].as<float>(); // 最高价
        data.low = fieldData["f45"].as<float>(); // 最低价
        data.open = fieldData["f46"].as<float>(); // 开盘价
        data.close = fieldData["f47"].as<float>(); // 昨收价
        data.change = data.price - data.close;
        data.changePercent = fieldData["f3"].as<float>(); // 涨跌幅
        data.volume = fieldData["f2"].as<long>(); // 成交量
        data.amount = 0; // 东方财富API没有直接提供成交额
        data.valid = true;
        
        return true;
      }
    default:
      DEBUG_PRINTLN("不支持的API类型");
      return false;
  }
}

String StockManager::getStockApiUrl(String code, int apiType) {
  // 根据不同API类型构建不同的URL格式
  switch(apiType) {
    case 1: // 网易财经
      return "/data/feed/" + code + ",money.api?callback=?";
    case 2: // 新浪财经
      return "/list/" + code;
    case 3: // 东方财富
      return "/api/qt/stock/get?fields=f43,f44,f45,f46,f47,f2,f3,f14&secid=" + (code.startsWith("6") ? "1." + code : "0." + code);
    default:
      return "";
  }
}