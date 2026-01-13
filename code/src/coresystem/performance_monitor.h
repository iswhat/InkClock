#ifndef PERFORMANCE_MONITOR_H
#define PERFORMANCE_MONITOR_H

#include <Arduino.h>
#include <string>
#include <map>
#include <vector>
#include <deque>
#include "hardware_detector.h"

// 性能指标类型枚举
enum PerformanceMetricType {
  METRIC_CPU_USAGE,         // CPU使用率
  METRIC_MEMORY_USAGE,      // 内存使用率
  METRIC_STORAGE_USAGE,     // 存储使用率
  METRIC_NETWORK_USAGE,     // 网络使用率
  METRIC_POWER_USAGE,       // 电源使用率
  METRIC_DISPLAY_REFRESH,   // 显示刷新率
  METRIC_TASK_EXECUTION,    // 任务执行时间
  METRIC_API_RESPONSE,      // API响应时间
  METRIC_SYSTEM_LOAD,       // 系统负载
  METRIC_BATTERY_LEVEL,     // 电池电量
  METRIC_TEMPERATURE,       // 温度
  METRIC_NETWORK_SIGNAL,    // 网络信号强度
  METRIC_GPU_USAGE,         // GPU使用率（如果适用）
  METRIC_CUSTOM             // 自定义指标
};

// 告警级别枚举
enum AlertLevel {
  ALERT_LEVEL_INFO,         // 信息
  ALERT_LEVEL_WARNING,      // 警告
  ALERT_LEVEL_CRITICAL,     // 严重
  ALERT_LEVEL_EMERGENCY     // 紧急
};

// 性能数据点结构
typedef struct {
  PerformanceMetricType type;  // 指标类型
  String name;                // 指标名称
  float value;                // 指标值
  float minValue;             // 最小值
  float maxValue;             // 最大值
  float averageValue;         // 平均值
  float threshold;            // 阈值
  AlertLevel alertLevel;      // 告警级别
  bool isAlert;               // 是否告警
  unsigned long timestamp;    // 时间戳
  String unit;                // 单位
  std::map<String, String> metadata; // 元数据
} PerformanceDataPoint;

// 告警事件结构
typedef struct {
  String id;                 // 告警ID
  String title;              // 告警标题
  String description;        // 告警描述
  AlertLevel level;          // 告警级别
  PerformanceMetricType metricType; // 关联的指标类型
  float metricValue;         // 指标值
  float threshold;           // 触发阈值
  unsigned long timestamp;   // 时间戳
  bool resolved;             // 是否已解决
  unsigned long resolveTime; // 解决时间
  String resolution;         // 解决方法
} AlertEvent;

// 性能历史数据结构
typedef struct {
  PerformanceMetricType type;  // 指标类型
  String name;                // 指标名称
  std::deque<PerformanceDataPoint> dataPoints; // 数据点队列
  unsigned long lastUpdateTime; // 最后更新时间
  unsigned int maxDataPoints;   // 最大数据点数
  float minHistoricalValue;    // 历史最小值
  float maxHistoricalValue;    // 历史最大值
  float averageHistoricalValue; // 历史平均值
} PerformanceHistory;

// 性能监控器类
class PerformanceMonitor {
private:
  static PerformanceMonitor* instance;
  
  // 性能历史数据
  std::map<PerformanceMetricType, PerformanceHistory> performanceHistory;
  
  // 告警事件队列
  std::deque<AlertEvent> alertEvents;
  
  // 硬件检测器
  HardwareDetector* hardwareDetector;
  
  // 初始化状态
  bool initialized;
  
  // 最后监控时间
  unsigned long lastMonitorTime;
  
  // 告警阈值配置
  std::map<PerformanceMetricType, float> alertThresholds;
  
  // 告警级别配置
  std::map<PerformanceMetricType, std::map<AlertLevel, float>> alertLevelThresholds;
  
  // 数据采集间隔
  unsigned long collectionInterval;
  
  // 最大告警事件数
  unsigned int maxAlertEvents;
  
  // 最大历史数据点数
  unsigned int maxHistoryDataPoints;
  
  // 私有构造函数
  PerformanceMonitor();
  
  // 私有方法：初始化默认配置
  void initDefaultConfig();
  
  // 私有方法：收集性能数据
  void collectPerformanceData();
  
  // 私有方法：分析性能数据
  void analyzePerformanceData();
  
  // 私有方法：检测告警
  void detectAlerts();
  
  // 私有方法：生成告警事件
  void generateAlertEvent(PerformanceMetricType type, float value, float threshold, AlertLevel level);
  
  // 私有方法：更新历史数据
  void updateHistoryData(PerformanceDataPoint dataPoint);
  
  // 私有方法：清理过期数据
  void cleanupExpiredData();
  
  // 私有方法：计算统计数据
  void calculateStatistics(PerformanceHistory& history);
  
  // 私有方法：发布性能数据事件
  void publishPerformanceDataEvent(PerformanceDataPoint dataPoint);
  
  // 私有方法：发布告警事件
  void publishAlertEvent(AlertEvent alert);
  
  // 私有方法：获取指标名称
  String getMetricName(PerformanceMetricType type);
  
  // 私有方法：获取指标单位
  String getMetricUnit(PerformanceMetricType type);
  
public:
  // 获取单例实例
  static PerformanceMonitor* getInstance();
  
  // 初始化
  bool init();
  
  // 启动监控
  bool startMonitoring();
  
  // 停止监控
  bool stopMonitoring();
  
  // 执行监控周期
  bool runMonitoringCycle();
  
  // 获取性能数据
  PerformanceDataPoint getPerformanceData(PerformanceMetricType type);
  
  // 获取所有性能数据
  std::vector<PerformanceDataPoint> getAllPerformanceData();
  
  // 获取性能历史数据
  std::deque<PerformanceDataPoint> getPerformanceHistory(PerformanceMetricType type, unsigned int count = 100);
  
  // 获取告警事件
  std::deque<AlertEvent> getAlertEvents(unsigned int count = 50);
  
  // 获取未解决的告警事件
  std::deque<AlertEvent> getUnresolvedAlerts();
  
  // 解决告警事件
  bool resolveAlert(const String& alertId, const String& resolution = "");
  
  // 设置告警阈值
  bool setAlertThreshold(PerformanceMetricType type, float threshold);
  
  // 设置告警级别阈值
  bool setAlertLevelThreshold(PerformanceMetricType type, AlertLevel level, float threshold);
  
  // 获取告警阈值
  float getAlertThreshold(PerformanceMetricType type);
  
  // 注册自定义指标
  bool registerCustomMetric(const String& name, float threshold = 90.0);
  
  // 更新自定义指标
  bool updateCustomMetric(const String& name, float value);
  
  // 获取性能报告
  String getPerformanceReport();
  
  // 获取系统健康状态
  String getSystemHealthStatus();
  
  // 导出性能数据
  String exportPerformanceData();
  
  // 导入性能数据
  bool importPerformanceData(const String& data);
  
  // 设置数据采集间隔
  bool setCollectionInterval(unsigned long interval);
  
  // 获取数据采集间隔
  unsigned long getCollectionInterval();
  
  // 设置最大历史数据点数
  bool setMaxHistoryDataPoints(unsigned int count);
  
  // 获取最大历史数据点数
  unsigned int getMaxHistoryDataPoints();
  
  // 重置监控器
  bool reset();
  
  // 检查系统健康状态
  bool checkSystemHealth();
  
  // 执行性能基准测试
  float runBenchmark();
};

// 性能监控器宏
#define PERFORMANCE_MONITOR PerformanceMonitor::getInstance()
#define PERFORMANCE_DATA(type) PERFORMANCE_MONITOR->getPerformanceData(type)
#define PERFORMANCE_HISTORY(type, count) PERFORMANCE_MONITOR->getPerformanceHistory(type, count)
#define PERFORMANCE_REPORT() PERFORMANCE_MONITOR->getPerformanceReport()
#define SYSTEM_HEALTH() PERFORMANCE_MONITOR->getSystemHealthStatus()

#endif // PERFORMANCE_MONITOR_H
