#ifndef STOCK_MANAGER_H
#define STOCK_MANAGER_H

#include <Arduino.h>
#include "../coresystem/config.h"
#include "../services/api_manager.h"

// 大盘曲线数据点结构
typedef struct {
  float price;    // 价格
  String time;    // 时间
} StockChartPoint;

// 股票数据结构
typedef struct {
  String code;            // 股票代码
  String name;            // 股票名称
  String market;          // 股票市场（如：sh, sz, hk, us等）
  float price;            // 当前价格
  float change;           // 涨跌额
  float changePercent;    // 涨跌幅（%）
  float open;             // 开盘价
  float high;             // 最高价
  float low;              // 最低价
  float close;            // 收盘价
  long volume;            // 成交量
  long amount;            // 成交额
  String time;            // 更新时间
  bool valid;             // 数据是否有效
  StockChartPoint chartData[50]; // 大盘曲线数据，最多50个点
  int chartDataCount;     // 大盘曲线数据点数量
} StockData;

class StockManager {
public:
  StockManager();
  ~StockManager();
  
  void init();
  void update();
  void loop();
  
  // 股票管理功能
  bool addStock(String code, String market = "sh");
  bool removeStock(int index);
  bool setStockList(String codes[], String markets[]);
  
  // 获取股票数据
  StockData getStockData(int index);
  int getStockCount();
  
  // 股票存储功能
  bool saveStockList();
  bool loadStockList();
  
private:
  // 股票数组
  StockData stocks[MAX_STOCKS];
  
  // 股票代码数组
  String stockCodes[MAX_STOCKS];
  
  // 股票计数
  int stockCount;
  
  // 更新标志
  unsigned long lastUpdate;
  bool dataUpdated;
  
  // 私有方法
  bool fetchStockData(String code, StockData &data);
  bool fetchStockChartData(String code, String market, StockData &data);
  bool parseStockData(String json, StockData &data, int apiType = 1);
  String getStockApiUrl(String code, int apiType = 1);
};

#endif // STOCK_MANAGER_H