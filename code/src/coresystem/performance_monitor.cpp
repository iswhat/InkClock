#include "performance_monitor.h"

// 静态实例初始化
PerformanceMonitor* PerformanceMonitor::instance = nullptr;

// PerformanceMonitor 构造函数
PerformanceMonitor::PerformanceMonitor() {
  hardwareDetector = nullptr;
  initialized = false;
  lastMonitorTime = 0;
  collectionInterval = 10000; // 默认10秒
  maxAlertEvents = 100;
  maxHistoryDataPoints = 1000;
}

// PerformanceMonitor 单例获取
PerformanceMonitor* PerformanceMonitor::getInstance() {
  if (instance == nullptr) {
    instance = new PerformanceMonitor();
  }
  return instance;
}

// 初始化默认配置
void PerformanceMonitor::initDefaultConfig() {
  // 初始化告警阈值
  alertThresholds[METRIC_CPU_USAGE] = 80.0;
  alertThresholds[METRIC_MEMORY_USAGE] = 85.0;
  alertThresholds[METRIC_STORAGE_USAGE] = 90.0;
  alertThresholds[METRIC_NETWORK_USAGE] = 95.0;
  alertThresholds[METRIC_POWER_USAGE] = 90.0;
  alertThresholds[METRIC_DISPLAY_REFRESH] = 5.0; // 低于5fps告警
  alertThresholds[METRIC_TASK_EXECUTION] = 1000.0; // 超过1秒告警
  alertThresholds[METRIC_API_RESPONSE] = 3000.0; // 超过3秒告警
  alertThresholds[METRIC_SYSTEM_LOAD] = 8.0; // 系统负载超过8
  alertThresholds[METRIC_BATTERY_LEVEL] = 20.0; // 低于20%告警
  alertThresholds[METRIC_TEMPERATURE] = 80.0; // 超过80度告警
  alertThresholds[METRIC_NETWORK_SIGNAL] = -80.0; // 低于-80dBm告警
  alertThresholds[METRIC_GPU_USAGE] = 85.0;
  alertThresholds[METRIC_CUSTOM] = 90.0;
  
  // 初始化告警级别阈值
  for (int type = METRIC_CPU_USAGE; type <= METRIC_CUSTOM; type++) {
    PerformanceMetricType metricType = static_cast<PerformanceMetricType>(type);
    std::map<AlertLevel, float> levelThresholds;
    
    switch (metricType) {
      case METRIC_BATTERY_LEVEL:
      case METRIC_NETWORK_SIGNAL:
        // 这些指标值越低越严重
        levelThresholds[ALERT_LEVEL_INFO] = 50.0;
        levelThresholds[ALERT_LEVEL_WARNING] = 30.0;
        levelThresholds[ALERT_LEVEL_CRITICAL] = 15.0;
        levelThresholds[ALERT_LEVEL_EMERGENCY] = 5.0;
        break;
      default:
        // 这些指标值越高越严重
        levelThresholds[ALERT_LEVEL_INFO] = 60.0;
        levelThresholds[ALERT_LEVEL_WARNING] = 75.0;
        levelThresholds[ALERT_LEVEL_CRITICAL] = 90.0;
        levelThresholds[ALERT_LEVEL_EMERGENCY] = 95.0;
        break;
    }
    
    alertLevelThresholds[metricType] = levelThresholds;
  }
  
  // 初始化性能历史数据
  for (int type = METRIC_CPU_USAGE; type <= METRIC_CUSTOM; type++) {
    PerformanceMetricType metricType = static_cast<PerformanceMetricType>(type);
    PerformanceHistory history;
    history.type = metricType;
    history.name = getMetricName(metricType);
    history.maxDataPoints = maxHistoryDataPoints;
    history.lastUpdateTime = 0;
    history.minHistoricalValue = 0;
    history.maxHistoricalValue = 0;
    history.averageHistoricalValue = 0;
    performanceHistory[metricType] = history;
  }
}

// 获取指标名称
String PerformanceMonitor::getMetricName(PerformanceMetricType type) {
  switch (type) {
    case METRIC_CPU_USAGE:
      return "CPU使用率";
    case METRIC_MEMORY_USAGE:
      return "内存使用率";
    case METRIC_STORAGE_USAGE:
      return "存储使用率";
    case METRIC_NETWORK_USAGE:
      return "网络使用率";
    case METRIC_POWER_USAGE:
      return "电源使用率";
    case METRIC_DISPLAY_REFRESH:
      return "显示刷新率";
    case METRIC_TASK_EXECUTION:
      return "任务执行时间";
    case METRIC_API_RESPONSE:
      return "API响应时间";
    case METRIC_SYSTEM_LOAD:
      return "系统负载";
    case METRIC_BATTERY_LEVEL:
      return "电池电量";
    case METRIC_TEMPERATURE:
      return "温度";
    case METRIC_NETWORK_SIGNAL:
      return "网络信号强度";
    case METRIC_GPU_USAGE:
      return "GPU使用率";
    case METRIC_CUSTOM:
      return "自定义指标";
    default:
      return "未知指标";
  }
}

// 获取指标单位
String PerformanceMonitor::getMetricUnit(PerformanceMetricType type) {
  switch (type) {
    case METRIC_CPU_USAGE:
    case METRIC_MEMORY_USAGE:
    case METRIC_STORAGE_USAGE:
    case METRIC_NETWORK_USAGE:
    case METRIC_POWER_USAGE:
    case METRIC_BATTERY_LEVEL:
    case METRIC_GPU_USAGE:
      return "%";
    case METRIC_DISPLAY_REFRESH:
      return "fps";
    case METRIC_TASK_EXECUTION:
    case METRIC_API_RESPONSE:
      return "ms";
    case METRIC_SYSTEM_LOAD:
      return "";
    case METRIC_TEMPERATURE:
      return "°C";
    case METRIC_NETWORK_SIGNAL:
      return "dBm";
    case METRIC_CUSTOM:
      return "";
    default:
      return "";
  }
}

// 初始化
bool PerformanceMonitor::init() {
  if (initialized) {
    return true;
  }
  
  DEBUG_PRINTLN("初始化性能监控器...");
  
  // 初始化默认配置
  initDefaultConfig();
  
  // 获取硬件检测器实例
  hardwareDetector = HardwareDetector::getInstance();
  if (!hardwareDetector) {
    DEBUG_PRINTLN("硬件检测器未初始化，性能监控器将使用模拟数据");
  }
  
  // 启动监控
  startMonitoring();
  
  initialized = true;
  DEBUG_PRINTLN("性能监控器初始化完成");
  return true;
}

// 启动监控
bool PerformanceMonitor::startMonitoring() {
  lastMonitorTime = millis();
  DEBUG_PRINTLN("性能监控器启动");
  return true;
}

// 停止监控
bool PerformanceMonitor::stopMonitoring() {
  DEBUG_PRINTLN("性能监控器停止");
  return true;
}

// 执行监控周期
bool PerformanceMonitor::runMonitoringCycle() {
  unsigned long now = millis();
  if (now - lastMonitorTime < collectionInterval) {
    return true;
  }
  
  lastMonitorTime = now;
  
  // 收集性能数据
  collectPerformanceData();
  
  // 分析性能数据
  analyzePerformanceData();
  
  // 检测告警
  detectAlerts();
  
  // 清理过期数据
  cleanupExpiredData();
  
  return true;
}

// 收集性能数据
void PerformanceMonitor::collectPerformanceData() {
  // 收集CPU使用率
  PerformanceDataPoint cpuData;
  cpuData.type = METRIC_CPU_USAGE;
  cpuData.name = getMetricName(METRIC_CPU_USAGE);
  cpuData.unit = getMetricUnit(METRIC_CPU_USAGE);
  cpuData.timestamp = millis();
  
  if (hardwareDetector) {
    HardwareResourceInfo cpuInfo = hardwareDetector->getResourceInfo(RESOURCE_TYPE_CPU);
    cpuData.value = cpuInfo.usage;
  } else {
    // 模拟数据
    cpuData.value = random(10, 70);
  }
  
  cpuData.threshold = alertThresholds[METRIC_CPU_USAGE];
  updateHistoryData(cpuData);
  
  // 收集内存使用率
  PerformanceDataPoint memoryData;
  memoryData.type = METRIC_MEMORY_USAGE;
  memoryData.name = getMetricName(METRIC_MEMORY_USAGE);
  memoryData.unit = getMetricUnit(METRIC_MEMORY_USAGE);
  memoryData.timestamp = millis();
  
  if (hardwareDetector) {
    HardwareResourceInfo memoryInfo = hardwareDetector->getResourceInfo(RESOURCE_TYPE_MEMORY);
    memoryData.value = memoryInfo.usage;
  } else {
    // 模拟数据
    memoryData.value = random(20, 80);
  }
  
  memoryData.threshold = alertThresholds[METRIC_MEMORY_USAGE];
  updateHistoryData(memoryData);
  
  // 收集存储使用率
  PerformanceDataPoint storageData;
  storageData.type = METRIC_STORAGE_USAGE;
  storageData.name = getMetricName(METRIC_STORAGE_USAGE);
  storageData.unit = getMetricUnit(METRIC_STORAGE_USAGE);
  storageData.timestamp = millis();
  
  if (hardwareDetector) {
    HardwareResourceInfo storageInfo = hardwareDetector->getResourceInfo(RESOURCE_TYPE_STORAGE);
    storageData.value = storageInfo.usage;
  } else {
    // 模拟数据
    storageData.value = random(10, 60);
  }
  
  storageData.threshold = alertThresholds[METRIC_STORAGE_USAGE];
  updateHistoryData(storageData);
  
  // 收集网络使用率
  PerformanceDataPoint networkData;
  networkData.type = METRIC_NETWORK_USAGE;
  networkData.name = getMetricName(METRIC_NETWORK_USAGE);
  networkData.unit = getMetricUnit(METRIC_NETWORK_USAGE);
  networkData.timestamp = millis();
  
  if (hardwareDetector) {
    HardwareResourceInfo networkInfo = hardwareDetector->getResourceInfo(RESOURCE_TYPE_NETWORK);
    networkData.value = networkInfo.usage;
  } else {
    // 模拟数据
    networkData.value = random(5, 50);
  }
  
  networkData.threshold = alertThresholds[METRIC_NETWORK_USAGE];
  updateHistoryData(networkData);
  
  // 收集电源使用率
  PerformanceDataPoint powerData;
  powerData.type = METRIC_POWER_USAGE;
  powerData.name = getMetricName(METRIC_POWER_USAGE);
  powerData.unit = getMetricUnit(METRIC_POWER_USAGE);
  powerData.timestamp = millis();
  
  if (hardwareDetector) {
    HardwareResourceInfo powerInfo = hardwareDetector->getResourceInfo(RESOURCE_TYPE_POWER);
    powerData.value = powerInfo.usage;
  } else {
    // 模拟数据
    powerData.value = random(15, 75);
  }
  
  powerData.threshold = alertThresholds[METRIC_POWER_USAGE];
  updateHistoryData(powerData);
  
  // 收集其他指标（使用模拟数据）
  PerformanceDataPoint displayData;
  displayData.type = METRIC_DISPLAY_REFRESH;
  displayData.name = getMetricName(METRIC_DISPLAY_REFRESH);
  displayData.unit = getMetricUnit(METRIC_DISPLAY_REFRESH);
  displayData.timestamp = millis();
  displayData.value = random(1, 10);
  displayData.threshold = alertThresholds[METRIC_DISPLAY_REFRESH];
  updateHistoryData(displayData);
  
  PerformanceDataPoint taskData;
  taskData.type = METRIC_TASK_EXECUTION;
  taskData.name = getMetricName(METRIC_TASK_EXECUTION);
  taskData.unit = getMetricUnit(METRIC_TASK_EXECUTION);
  taskData.timestamp = millis();
  taskData.value = random(10, 500);
  taskData.threshold = alertThresholds[METRIC_TASK_EXECUTION];
  updateHistoryData(taskData);
  
  PerformanceDataPoint apiData;
  apiData.type = METRIC_API_RESPONSE;
  apiData.name = getMetricName(METRIC_API_RESPONSE);
  apiData.unit = getMetricUnit(METRIC_API_RESPONSE);
  apiData.timestamp = millis();
  apiData.value = random(100, 2000);
  apiData.threshold = alertThresholds[METRIC_API_RESPONSE];
  updateHistoryData(apiData);
  
  PerformanceDataPoint batteryData;
  batteryData.type = METRIC_BATTERY_LEVEL;
  batteryData.name = getMetricName(METRIC_BATTERY_LEVEL);
  batteryData.unit = getMetricUnit(METRIC_BATTERY_LEVEL);
  batteryData.timestamp = millis();
  batteryData.value = random(30, 100);
  batteryData.threshold = alertThresholds[METRIC_BATTERY_LEVEL];
  updateHistoryData(batteryData);
  
  PerformanceDataPoint tempData;
  tempData.type = METRIC_TEMPERATURE;
  tempData.name = getMetricName(METRIC_TEMPERATURE);
  tempData.unit = getMetricUnit(METRIC_TEMPERATURE);
  tempData.timestamp = millis();
  tempData.value = random(20, 60);
  tempData.threshold = alertThresholds[METRIC_TEMPERATURE];
  updateHistoryData(tempData);
  
  PerformanceDataPoint signalData;
  signalData.type = METRIC_NETWORK_SIGNAL;
  signalData.name = getMetricName(METRIC_NETWORK_SIGNAL);
  signalData.unit = getMetricUnit(METRIC_NETWORK_SIGNAL);
  signalData.timestamp = millis();
  signalData.value = random(-100, -50);
  signalData.threshold = alertThresholds[METRIC_NETWORK_SIGNAL];
  updateHistoryData(signalData);
}

// 更新历史数据
void PerformanceMonitor::updateHistoryData(PerformanceDataPoint dataPoint) {
  if (performanceHistory.find(dataPoint.type) == performanceHistory.end()) {
    return;
  }
  
  PerformanceHistory& history = performanceHistory[dataPoint.type];
  
  // 添加数据点到历史记录
  history.dataPoints.push_back(dataPoint);
  
  // 限制历史数据点数
  if (history.dataPoints.size() > history.maxDataPoints) {
    history.dataPoints.pop_front();
  }
  
  // 更新最后更新时间
  history.lastUpdateTime = millis();
  
  // 计算统计数据
  calculateStatistics(history);
  
  // 更新数据点的统计信息
  dataPoint.minValue = history.minHistoricalValue;
  dataPoint.maxValue = history.maxHistoricalValue;
  dataPoint.averageValue = history.averageHistoricalValue;
  
  // 发布性能数据事件
  publishPerformanceDataEvent(dataPoint);
}

// 计算统计数据
void PerformanceMonitor::calculateStatistics(PerformanceHistory& history) {
  if (history.dataPoints.empty()) {
    return;
  }
  
  float sum = 0;
  float min = history.dataPoints[0].value;
  float max = history.dataPoints[0].value;
  
  for (const auto& dataPoint : history.dataPoints) {
    sum += dataPoint.value;
    if (dataPoint.value < min) {
      min = dataPoint.value;
    }
    if (dataPoint.value > max) {
      max = dataPoint.value;
    }
  }
  
  history.minHistoricalValue = min;
  history.maxHistoricalValue = max;
  history.averageHistoricalValue = sum / history.dataPoints.size();
}

// 分析性能数据
void PerformanceMonitor::analyzePerformanceData() {
  // 这里可以添加更复杂的性能数据分析逻辑
  // 例如：趋势分析、异常检测等
}

// 检测告警
void PerformanceMonitor::detectAlerts() {
  for (auto& pair : performanceHistory) {
    PerformanceHistory& history = pair.second;
    if (history.dataPoints.empty()) {
      continue;
    }
    
    PerformanceDataPoint latestData = history.dataPoints.back();
    float value = latestData.value;
    float threshold = latestData.threshold;
    
    // 确定告警级别
    AlertLevel alertLevel = ALERT_LEVEL_INFO;
    std::map<AlertLevel, float> levelThresholds = alertLevelThresholds[latestData.type];
    
    switch (latestData.type) {
      case METRIC_BATTERY_LEVEL:
      case METRIC_NETWORK_SIGNAL:
        // 这些指标值越低越严重
        if (value <= levelThresholds[ALERT_LEVEL_EMERGENCY]) {
          alertLevel = ALERT_LEVEL_EMERGENCY;
        } else if (value <= levelThresholds[ALERT_LEVEL_CRITICAL]) {
          alertLevel = ALERT_LEVEL_CRITICAL;
        } else if (value <= levelThresholds[ALERT_LEVEL_WARNING]) {
          alertLevel = ALERT_LEVEL_WARNING;
        }
        break;
      default:
        // 这些指标值越高越严重
        if (value >= levelThresholds[ALERT_LEVEL_EMERGENCY]) {
          alertLevel = ALERT_LEVEL_EMERGENCY;
        } else if (value >= levelThresholds[ALERT_LEVEL_CRITICAL]) {
          alertLevel = ALERT_LEVEL_CRITICAL;
        } else if (value >= levelThresholds[ALERT_LEVEL_WARNING]) {
          alertLevel = ALERT_LEVEL_WARNING;
        }
        break;
    }
    
    // 检查是否需要告警
    bool isAlert = alertLevel >= ALERT_LEVEL_WARNING;
    if (isAlert) {
      generateAlertEvent(latestData.type, value, threshold, alertLevel);
    }
  }
}

// 生成告警事件
void PerformanceMonitor::generateAlertEvent(PerformanceMetricType type, float value, float threshold, AlertLevel level) {
  AlertEvent alert;
  
  // 生成告警ID
  char alertId[32];
  sprintf(alertId, "alert_%d_%lu", type, millis());
  alert.id = String(alertId);
  
  // 设置告警信息
  alert.title = getMetricName(type) + "告警";
  alert.description = String("指标值: ") + String(value) + String(getMetricUnit(type)) + ", 阈值: " + String(threshold) + String(getMetricUnit(type));
  alert.level = level;
  alert.metricType = type;
  alert.metricValue = value;
  alert.threshold = threshold;
  alert.timestamp = millis();
  alert.resolved = false;
  alert.resolveTime = 0;
  alert.resolution = "";
  
  // 添加到告警事件队列
  alertEvents.push_back(alert);
  
  // 限制告警事件数
  if (alertEvents.size() > maxAlertEvents) {
    alertEvents.pop_front();
  }
  
  // 发布告警事件
  publishAlertEvent(alert);
  
  // 打印告警信息
  switch (level) {
    case ALERT_LEVEL_INFO:
      DEBUG_PRINTF("[INFO] %s: %s\n", alert.title.c_str(), alert.description.c_str());
      break;
    case ALERT_LEVEL_WARNING:
      DEBUG_PRINTF("[WARNING] %s: %s\n", alert.title.c_str(), alert.description.c_str());
      break;
    case ALERT_LEVEL_CRITICAL:
      DEBUG_PRINTF("[CRITICAL] %s: %s\n", alert.title.c_str(), alert.description.c_str());
      break;
    case ALERT_LEVEL_EMERGENCY:
      DEBUG_PRINTF("[EMERGENCY] %s: %s\n", alert.title.c_str(), alert.description.c_str());
      break;
  }
}

// 清理过期数据
void PerformanceMonitor::cleanupExpiredData() {
  // 清理过期的告警事件（超过24小时）
  unsigned long now = millis();
  unsigned long expireTime = 24 * 60 * 60 * 1000; // 24小时
  
  std::deque<AlertEvent> validAlerts;
  for (const auto& alert : alertEvents) {
    if (now - alert.timestamp < expireTime || !alert.resolved) {
      validAlerts.push_back(alert);
    }
  }
  
  alertEvents = validAlerts;
}

// 发布性能数据事件
void PerformanceMonitor::publishPerformanceDataEvent(PerformanceDataPoint dataPoint) {
  // 这里可以添加事件发布逻辑
  // DEBUG_PRINTF("性能数据事件: %s = %.2f %s\n", dataPoint.name.c_str(), dataPoint.value, dataPoint.unit.c_str());
}

// 发布告警事件
void PerformanceMonitor::publishAlertEvent(AlertEvent alert) {
  // 这里可以添加事件发布逻辑
  // DEBUG_PRINTF("告警事件: %s - %s\n", alert.title.c_str(), alert.description.c_str());
}

// 获取性能数据
PerformanceDataPoint PerformanceMonitor::getPerformanceData(PerformanceMetricType type) {
  if (performanceHistory.find(type) == performanceHistory.end()) {
    PerformanceDataPoint defaultData;
    defaultData.type = type;
    defaultData.name = getMetricName(type);
    defaultData.value = 0;
    defaultData.minValue = 0;
    defaultData.maxValue = 0;
    defaultData.averageValue = 0;
    defaultData.threshold = alertThresholds[type];
    defaultData.alertLevel = ALERT_LEVEL_INFO;
    defaultData.isAlert = false;
    defaultData.timestamp = millis();
    defaultData.unit = getMetricUnit(type);
    return defaultData;
  }
  
  PerformanceHistory& history = performanceHistory[type];
  if (history.dataPoints.empty()) {
    PerformanceDataPoint defaultData;
    defaultData.type = type;
    defaultData.name = history.name;
    defaultData.value = 0;
    defaultData.minValue = history.minHistoricalValue;
    defaultData.maxValue = history.maxHistoricalValue;
    defaultData.averageValue = history.averageHistoricalValue;
    defaultData.threshold = alertThresholds[type];
    defaultData.alertLevel = ALERT_LEVEL_INFO;
    defaultData.isAlert = false;
    defaultData.timestamp = millis();
    defaultData.unit = getMetricUnit(type);
    return defaultData;
  }
  
  return history.dataPoints.back();
}

// 获取所有性能数据
std::vector<PerformanceDataPoint> PerformanceMonitor::getAllPerformanceData() {
  std::vector<PerformanceDataPoint> allData;
  
  for (auto& pair : performanceHistory) {
    PerformanceDataPoint data = getPerformanceData(pair.first);
    allData.push_back(data);
  }
  
  return allData;
}

// 获取性能历史数据
std::deque<PerformanceDataPoint> PerformanceMonitor::getPerformanceHistory(PerformanceMetricType type, unsigned int count) {
  if (performanceHistory.find(type) == performanceHistory.end()) {
    return std::deque<PerformanceDataPoint>();
  }
  
  PerformanceHistory& history = performanceHistory[type];
  if (history.dataPoints.size() <= count) {
    return history.dataPoints;
  }
  
  std::deque<PerformanceDataPoint> result;
  for (unsigned int i = history.dataPoints.size() - count; i < history.dataPoints.size(); i++) {
    result.push_back(history.dataPoints[i]);
  }
  
  return result;
}

// 获取告警事件
std::deque<AlertEvent> PerformanceMonitor::getAlertEvents(unsigned int count) {
  if (alertEvents.size() <= count) {
    return alertEvents;
  }
  
  std::deque<AlertEvent> result;
  for (unsigned int i = alertEvents.size() - count; i < alertEvents.size(); i++) {
    result.push_back(alertEvents[i]);
  }
  
  return result;
}

// 获取未解决的告警事件
std::deque<AlertEvent> PerformanceMonitor::getUnresolvedAlerts() {
  std::deque<AlertEvent> unresolvedAlerts;
  
  for (const auto& alert : alertEvents) {
    if (!alert.resolved) {
      unresolvedAlerts.push_back(alert);
    }
  }
  
  return unresolvedAlerts;
}

// 解决告警事件
bool PerformanceMonitor::resolveAlert(const String& alertId, const String& resolution) {
  for (auto& alert : alertEvents) {
    if (alert.id == alertId && !alert.resolved) {
      alert.resolved = true;
      alert.resolveTime = millis();
      alert.resolution = resolution;
      DEBUG_PRINTF("告警已解决: %s\n", alert.title.c_str());
      return true;
    }
  }
  
  return false;
}

// 设置告警阈值
bool PerformanceMonitor::setAlertThreshold(PerformanceMetricType type, float threshold) {
  if (alertThresholds.find(type) == alertThresholds.end()) {
    return false;
  }
  
  alertThresholds[type] = threshold;
  return true;
}

// 设置告警级别阈值
bool PerformanceMonitor::setAlertLevelThreshold(PerformanceMetricType type, AlertLevel level, float threshold) {
  if (alertLevelThresholds.find(type) == alertLevelThresholds.end()) {
    return false;
  }
  
  alertLevelThresholds[type][level] = threshold;
  return true;
}

// 获取告警阈值
float PerformanceMonitor::getAlertThreshold(PerformanceMetricType type) {
  if (alertThresholds.find(type) == alertThresholds.end()) {
    return 0;
  }
  
  return alertThresholds[type];
}

// 注册自定义指标
bool PerformanceMonitor::registerCustomMetric(const String& name, float threshold) {
  // 这里可以添加自定义指标注册逻辑
  return true;
}

// 更新自定义指标
bool PerformanceMonitor::updateCustomMetric(const String& name, float value) {
  // 这里可以添加自定义指标更新逻辑
  return true;
}

// 获取性能报告
String PerformanceMonitor::getPerformanceReport() {
  String report = "===== 性能监控报告 =====\n";
  
  // 添加系统信息
  report += "系统信息:\n";
  if (hardwareDetector) {
    HardwareEvaluationResult result = hardwareDetector->getEvaluationResult();
    report += String("硬件平台: ") + result.platform + "\n";
    report += String("总内存: ") + String(result.totalMemory) + " KB\n";
    report += String("总存储: ") + String(result.totalStorage) + " KB\n";
    report += String("硬件得分: ") + String(result.overallScore) + "\n";
  }
  
  // 添加性能指标
  report += "\n性能指标:\n";
  for (auto& pair : performanceHistory) {
    PerformanceDataPoint data = getPerformanceData(pair.first);
    report += String(data.name) + ": " + String(data.value) + data.unit + " (平均: " + String(data.averageValue) + data.unit + ")\n";
  }
  
  // 添加告警信息
  std::deque<AlertEvent> unresolvedAlerts = getUnresolvedAlerts();
  report += String("\n未解决告警: ") + String(unresolvedAlerts.size()) + "\n";
  for (const auto& alert : unresolvedAlerts) {
    report += String("- [") + String(alert.level) + "] " + alert.title + ": " + alert.description + "\n";
  }
  
  report += "=====================\n";
  return report;
}

// 获取系统健康状态
String PerformanceMonitor::getSystemHealthStatus() {
  // 计算系统健康得分
  float healthScore = 100.0;
  
  // 检查CPU使用率
  PerformanceDataPoint cpuData = getPerformanceData(METRIC_CPU_USAGE);
  if (cpuData.value > 80) {
    healthScore -= (cpuData.value - 80) * 0.5;
  }
  
  // 检查内存使用率
  PerformanceDataPoint memoryData = getPerformanceData(METRIC_MEMORY_USAGE);
  if (memoryData.value > 80) {
    healthScore -= (memoryData.value - 80) * 0.5;
  }
  
  // 检查存储使用率
  PerformanceDataPoint storageData = getPerformanceData(METRIC_STORAGE_USAGE);
  if (storageData.value > 80) {
    healthScore -= (storageData.value - 80) * 0.3;
  }
  
  // 检查电池电量
  PerformanceDataPoint batteryData = getPerformanceData(METRIC_BATTERY_LEVEL);
  if (batteryData.value < 30) {
    healthScore -= (30 - batteryData.value) * 0.2;
  }
  
  // 检查温度
  PerformanceDataPoint tempData = getPerformanceData(METRIC_TEMPERATURE);
  if (tempData.value > 60) {
    healthScore -= (tempData.value - 60) * 0.4;
  }
  
  // 确保得分在0-100之间
  if (healthScore < 0) {
    healthScore = 0;
  }
  
  // 确定健康状态
  String status;
  if (healthScore >= 90) {
    status = "优秀";
  } else if (healthScore >= 75) {
    status = "良好";
  } else if (healthScore >= 60) {
    status = "一般";
  } else if (healthScore >= 40) {
    status = "较差";
  } else {
    status = "危险";
  }
  
  return String("系统健康状态: ") + status + " (" + String(healthScore, 1) + "%)";
}

// 导出性能数据
String PerformanceMonitor::exportPerformanceData() {
  String data = "===== 性能数据导出 =====\n";
  
  // 导出性能历史数据
  for (auto& pair : performanceHistory) {
    PerformanceHistory& history = pair.second;
    data += String("\n") + history.name + ":\n";
    
    for (const auto& dataPoint : history.dataPoints) {
      data += String(dataPoint.timestamp) + "," + String(dataPoint.value) + "," + dataPoint.unit + "\n";
    }
  }
  
  // 导出告警事件
  data += "\n告警事件:\n";
  for (const auto& alert : alertEvents) {
    data += String(alert.timestamp) + "," + String(alert.level) + "," + alert.title + "," + alert.description + "\n";
  }
  
  return data;
}

// 导入性能数据
bool PerformanceMonitor::importPerformanceData(const String& data) {
  // 这里可以添加性能数据导入逻辑
  return true;
}

// 设置数据采集间隔
bool PerformanceMonitor::setCollectionInterval(unsigned long interval) {
  if (interval < 100) {
    return false;
  }
  
  collectionInterval = interval;
  return true;
}

// 获取数据采集间隔
unsigned long PerformanceMonitor::getCollectionInterval() {
  return collectionInterval;
}

// 设置最大历史数据点数
bool PerformanceMonitor::setMaxHistoryDataPoints(unsigned int count) {
  if (count < 10) {
    return false;
  }
  
  maxHistoryDataPoints = count;
  
  // 更新所有历史数据的最大点数
  for (auto& pair : performanceHistory) {
    PerformanceHistory& history = pair.second;
    history.maxDataPoints = count;
    
    // 截断历史数据
    while (history.dataPoints.size() > count) {
      history.dataPoints.pop_front();
    }
  }
  
  return true;
}

// 获取最大历史数据点数
unsigned int PerformanceMonitor::getMaxHistoryDataPoints() {
  return maxHistoryDataPoints;
}

// 重置监控器
bool PerformanceMonitor::reset() {
  // 清空性能历史数据
  for (auto& pair : performanceHistory) {
    PerformanceHistory& history = pair.second;
    history.dataPoints.clear();
    history.lastUpdateTime = 0;
    history.minHistoricalValue = 0;
    history.maxHistoricalValue = 0;
    history.averageHistoricalValue = 0;
  }
  
  // 清空告警事件
  alertEvents.clear();
  
  // 重置监控时间
  lastMonitorTime = millis();
  
  return true;
}

// 检查系统健康状态
bool PerformanceMonitor::checkSystemHealth() {
  String healthStatus = getSystemHealthStatus();
  DEBUG_PRINTLN(healthStatus);
  
  // 检查是否有严重告警
  std::deque<AlertEvent> unresolvedAlerts = getUnresolvedAlerts();
  for (const auto& alert : unresolvedAlerts) {
    if (alert.level >= ALERT_LEVEL_CRITICAL) {
      return false;
    }
  }
  
  return true;
}

// 执行性能基准测试
float PerformanceMonitor::runBenchmark() {
  unsigned long startTime = millis();
  
  // 执行一些计算密集型操作
  float result = 0;
  for (int i = 0; i < 1000000; i++) {
    result += sqrt(i) * sin(i) * cos(i);
  }
  
  unsigned long endTime = millis();
  float benchmarkTime = endTime - startTime;
  
  DEBUG_PRINTF("基准测试完成: %.2f ms\n", benchmarkTime);
  return benchmarkTime;
}
