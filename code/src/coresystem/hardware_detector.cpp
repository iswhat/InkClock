#include "hardware_detector.h"

#if defined(ESP32)
#include <SPIFFS.h>
#include <WiFi.h>
#elif defined(ESP8266)
#include <FS.h>
#include <ESP8266WiFi.h>
#endif

// 静态实例初始化
HardwareDetector* HardwareDetector::instance = nullptr;

// HardwareDetector 构造函数
HardwareDetector::HardwareDetector() {
  initialized = false;
  lastEvaluationTime = 0;
  
  // 初始化评估结果
  evaluationResult.hardwareId = "";
  evaluationResult.hardwareName = "";
  evaluationResult.hardwareType = "";
  evaluationResult.platform = "";
  evaluationResult.cpuInfo = "";
  evaluationResult.totalMemory = 0;
  evaluationResult.totalStorage = 0;
  evaluationResult.overallLevel = CAPABILITY_LEVEL_LOW;
  evaluationResult.overallScore = 0;
  evaluationResult.evaluationTime = 0;
  evaluationResult.isValid = false;
}

// HardwareDetector 单例获取
HardwareDetector* HardwareDetector::getInstance() {
  if (instance == nullptr) {
    instance = new HardwareDetector();
  }
  return instance;
}

// 初始化默认检测器
void HardwareDetector::initDefaultDetectors() {
  // 注册默认检测器
  registerDetector(std::make_shared<CpuDetector>());
  registerDetector(std::make_shared<MemoryDetector>());
  registerDetector(std::make_shared<StorageDetector>());
  registerDetector(std::make_shared<NetworkDetector>());
  registerDetector(std::make_shared<PowerDetector>());
}

// 初始化
bool HardwareDetector::init() {
  if (initialized) {
    return true;
  }
  
  DEBUG_PRINTLN("初始化硬件检测器...");
  
  // 初始化默认检测器
  initDefaultDetectors();
  
  // 初始化所有检测器
  for (auto& detector : detectors) {
    if (!detector->init()) {
      DEBUG_PRINTF("检测器初始化失败: %s\n", detector->getName().c_str());
      // 继续初始化其他检测器，不返回错误
    }
  }
  
  // 执行初始硬件检测
  detectResources();
  evaluateCapabilities();
  
  initialized = true;
  DEBUG_PRINTLN("硬件检测器初始化完成");
  return true;
}

// 注册检测器
bool HardwareDetector::registerDetector(std::shared_ptr<IHardwareDetector> detector) {
  if (!detector) {
    return false;
  }
  
  detectors.push_back(detector);
  DEBUG_PRINTF("注册检测器: %s\n", detector->getName().c_str());
  return true;
}

// 检测硬件资源
bool HardwareDetector::detectResources() {
  DEBUG_PRINTLN("检测硬件资源...");
  
  bool success = true;
  
  // 清空资源信息
  resources.clear();
  
  // 执行所有检测器的资源检测
  for (auto& detector : detectors) {
    if (!detector->detectResources()) {
      DEBUG_PRINTF("资源检测失败: %s\n", detector->getName().c_str());
      success = false;
    } else {
      // 收集检测器的资源信息
      for (int type = RESOURCE_TYPE_CPU; type <= RESOURCE_TYPE_OTHER; type++) {
        HardwareResourceType resourceType = static_cast<HardwareResourceType>(type);
        HardwareResourceInfo info = detector->getResourceInfo(resourceType);
        if (info.available) {
          resources[resourceType] = info;
        }
      }
    }
  }
  
  // 更新评估结果
  updateEvaluationResult();
  
  // 发布硬件评估事件
  publishHardwareEvaluationEvent();
  
  DEBUG_PRINTLN("硬件资源检测完成");
  return success;
}

// 评估硬件能力
bool HardwareDetector::evaluateCapabilities() {
  DEBUG_PRINTLN("评估硬件能力...");
  
  bool success = true;
  
  // 清空能力信息
  capabilities.clear();
  
  // 执行所有检测器的能力评估
  for (auto& detector : detectors) {
    if (!detector->evaluateCapabilities()) {
      DEBUG_PRINTF("能力评估失败: %s\n", detector->getName().c_str());
      success = false;
    } else {
      // 收集检测器的能力信息
      std::vector<String> capabilityNames = {"CPU", "Memory", "Storage", "Network", "Power"};
      for (const auto& name : capabilityNames) {
        HardwareCapabilityInfo info = detector->getCapabilityInfo(name);
        if (info.supported) {
          capabilities[name] = info;
        }
      }
    }
  }
  
  // 更新评估结果
  updateEvaluationResult();
  
  // 发布硬件评估事件
  publishHardwareEvaluationEvent();
  
  DEBUG_PRINTLN("硬件能力评估完成");
  return success;
}

// 获取硬件评估结果
HardwareEvaluationResult HardwareDetector::getEvaluationResult() {
  return evaluationResult;
}

// 获取资源信息
HardwareResourceInfo HardwareDetector::getResourceInfo(HardwareResourceType type) {
  if (resources.find(type) != resources.end()) {
    return resources[type];
  }
  
  // 返回默认资源信息
  HardwareResourceInfo defaultInfo;
  defaultInfo.type = type;
  defaultInfo.name = "未知资源";
  defaultInfo.description = "资源未检测";
  defaultInfo.total = 0;
  defaultInfo.used = 0;
  defaultInfo.usage = 0;
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.available = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 获取所有资源信息
std::vector<HardwareResourceInfo> HardwareDetector::getAllResourcesInfo() {
  std::vector<HardwareResourceInfo> resourceList;
  for (auto& pair : resources) {
    resourceList.push_back(pair.second);
  }
  return resourceList;
}

// 获取能力信息
HardwareCapabilityInfo HardwareDetector::getCapabilityInfo(const String& capabilityName) {
  if (capabilities.find(capabilityName) != capabilities.end()) {
    return capabilities[capabilityName];
  }
  
  // 返回默认能力信息
  HardwareCapabilityInfo defaultInfo;
  defaultInfo.name = capabilityName;
  defaultInfo.description = "能力未评估";
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.score = 0;
  defaultInfo.supported = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 获取所有能力信息
std::vector<HardwareCapabilityInfo> HardwareDetector::getAllCapabilitiesInfo() {
  std::vector<HardwareCapabilityInfo> capabilityList;
  for (auto& pair : capabilities) {
    capabilityList.push_back(pair.second);
  }
  return capabilityList;
}

// 检查硬件兼容性
bool HardwareDetector::checkCompatibility() {
  DEBUG_PRINTLN("检查硬件兼容性...");
  
  bool compatible = true;
  
  // 执行所有检测器的兼容性检查
  for (auto& detector : detectors) {
    if (!detector->checkCompatibility()) {
      DEBUG_PRINTF("兼容性检查失败: %s\n", detector->getName().c_str());
      compatible = false;
    }
  }
  
  DEBUG_PRINTF("硬件兼容性检查结果: %s\n", compatible ? "兼容" : "不兼容");
  return compatible;
}

// 检查硬件限制
bool HardwareDetector::checkHardwareLimits() {
  DEBUG_PRINTLN("检查硬件限制...");
  
  bool withinLimits = true;
  
  // 执行所有检测器的硬件限制检查
  for (auto& detector : detectors) {
    if (!detector->checkHardwareLimits()) {
      DEBUG_PRINTF("硬件限制检查失败: %s\n", detector->getName().c_str());
      withinLimits = false;
    }
  }
  
  DEBUG_PRINTF("硬件限制检查结果: %s\n", withinLimits ? "在限制范围内" : "超出限制");
  return withinLimits;
}

// 监控资源使用情况
bool HardwareDetector::monitorResources() {
  DEBUG_PRINTLN("监控资源使用情况...");
  
  bool success = true;
  
  // 执行所有检测器的资源监控
  for (auto& detector : detectors) {
    if (!detector->monitorResources()) {
      DEBUG_PRINTF("资源监控失败: %s\n", detector->getName().c_str());
      success = false;
    }
  }
  
  // 更新评估结果
  updateEvaluationResult();
  
  // 检查资源使用情况，发布告警事件
  for (auto& pair : resources) {
    HardwareResourceInfo info = pair.second;
    if (info.usage > 90) {
      publishResourceAlertEvent(info.type, info.usage, 90);
    } else if (info.usage > 75) {
      publishResourceAlertEvent(info.type, info.usage, 75);
    }
  }
  
  DEBUG_PRINTLN("资源监控完成");
  return success;
}

// 重置检测器
void HardwareDetector::reset() {
  DEBUG_PRINTLN("重置硬件检测器...");
  
  // 重置所有检测器
  for (auto& detector : detectors) {
    detector->reset();
  }
  
  // 重置评估结果
  evaluationResult.hardwareId = "";
  evaluationResult.hardwareName = "";
  evaluationResult.hardwareType = "";
  evaluationResult.platform = "";
  evaluationResult.cpuInfo = "";
  evaluationResult.totalMemory = 0;
  evaluationResult.totalStorage = 0;
  evaluationResult.overallLevel = CAPABILITY_LEVEL_LOW;
  evaluationResult.overallScore = 0;
  evaluationResult.evaluationTime = 0;
  evaluationResult.isValid = false;
  evaluationResult.resources.clear();
  evaluationResult.capabilities.clear();
  
  // 清空资源和能力信息
  resources.clear();
  capabilities.clear();
  
  // 重新执行检测和评估
  detectResources();
  evaluateCapabilities();
  
  DEBUG_PRINTLN("硬件检测器重置完成");
}

// 获取检测器名称
String HardwareDetector::getName() const {
  return "HardwareDetector";
}

// 获取平台信息
String HardwareDetector::getPlatformInfo() {
  #ifdef ESP32
    return "ESP32";
  #elif defined(ESP8266)
    return "ESP8266";
  #elif defined(ARDUINO)
    return "Arduino";
  #else
    return "Unknown";
  #endif
}

// 获取硬件ID
String HardwareDetector::getHardwareId() {
  #ifdef ESP32
    uint64_t chipId = ESP.getEfuseMac();
    char chipIdStr[17];
    sprintf(chipIdStr, "%016llX", chipId);
    return String(chipIdStr);
  #elif defined(ESP8266)
    uint32_t chipId = ESP.getChipId();
    char chipIdStr[11];
    sprintf(chipIdStr, "%08X", chipId);
    return String(chipIdStr);
  #else
    return "Unknown";
  #endif
}

// 获取硬件名称
String HardwareDetector::getHardwareName() {
  #ifdef ESP32
    return "ESP32 Development Board";
  #elif defined(ESP8266)
    return "ESP8266 Development Board";
  #elif defined(ARDUINO)
    return "Arduino Board";
  #else
    return "Unknown Hardware";
  #endif
}

// 获取硬件类型
String HardwareDetector::getHardwareType() {
  return "Development Board";
}

// 更新评估结果
void HardwareDetector::updateEvaluationResult() {
  // 更新基本信息
  evaluationResult.hardwareId = getHardwareId();
  evaluationResult.hardwareName = getHardwareName();
  evaluationResult.hardwareType = getHardwareType();
  evaluationResult.platform = getPlatformInfo();
  
  // 收集资源信息
  evaluationResult.resources.clear();
  for (auto& pair : resources) {
    evaluationResult.resources.push_back(pair.second);
  }
  
  // 收集能力信息
  evaluationResult.capabilities.clear();
  for (auto& pair : capabilities) {
    evaluationResult.capabilities.push_back(pair.second);
  }
  
  // 计算整体能力得分和级别
  evaluationResult.overallScore = calculateOverallScore();
  evaluationResult.overallLevel = determineOverallLevel(evaluationResult.overallScore);
  
  // 更新评估时间
  evaluationResult.evaluationTime = millis();
  evaluationResult.isValid = true;
  
  // 更新内存和存储信息
  HardwareResourceInfo memoryInfo = getResourceInfo(RESOURCE_TYPE_MEMORY);
  HardwareResourceInfo storageInfo = getResourceInfo(RESOURCE_TYPE_STORAGE);
  evaluationResult.totalMemory = memoryInfo.total;
  evaluationResult.totalStorage = storageInfo.total;
  
  // 更新CPU信息
  HardwareCapabilityInfo cpuCapability = getCapabilityInfo("CPU");
  evaluationResult.cpuInfo = cpuCapability.description;
}

// 计算整体能力得分
float HardwareDetector::calculateOverallScore() {
  if (capabilities.empty()) {
    return 0;
  }
  
  float totalScore = 0;
  int count = 0;
  
  for (auto& pair : capabilities) {
    HardwareCapabilityInfo info = pair.second;
    if (info.supported) {
      totalScore += info.score;
      count++;
    }
  }
  
  return count > 0 ? totalScore / count : 0;
}

// 确定整体能力级别
HardwareCapabilityLevel HardwareDetector::determineOverallLevel(float score) {
  if (score >= 90) {
    return CAPABILITY_LEVEL_EXCELLENT;
  } else if (score >= 70) {
    return CAPABILITY_LEVEL_HIGH;
  } else if (score >= 40) {
    return CAPABILITY_LEVEL_MEDIUM;
  } else {
    return CAPABILITY_LEVEL_LOW;
  }
}

// 发布硬件评估事件
void HardwareDetector::publishHardwareEvaluationEvent() {
  // 这里可以添加事件发布逻辑
  DEBUG_PRINTF("硬件评估完成，得分: %.2f, 级别: %d\n", evaluationResult.overallScore, evaluationResult.overallLevel);
}

// 发布资源告警事件
void HardwareDetector::publishResourceAlertEvent(HardwareResourceType type, float usage, float threshold) {
  // 这里可以添加事件发布逻辑
  DEBUG_PRINTF("资源告警: 类型=%d, 使用率=%.2f%%, 阈值=%.2f%%\n", type, usage, threshold);
}

// CPU检测器实现

// CpuDetector 构造函数
CpuDetector::CpuDetector() {
  // 初始化CPU资源信息
  cpuInfo.type = RESOURCE_TYPE_CPU;
  cpuInfo.name = "CPU";
  cpuInfo.description = "中央处理器";
  cpuInfo.total = 0;
  cpuInfo.used = 0;
  cpuInfo.usage = 0;
  cpuInfo.level = CAPABILITY_LEVEL_LOW;
  cpuInfo.available = false;
  cpuInfo.lastUpdateTime = 0;
  
  // 初始化CPU能力信息
  cpuCapability.name = "CPU";
  cpuCapability.description = "中央处理器能力";
  cpuCapability.level = CAPABILITY_LEVEL_LOW;
  cpuCapability.score = 0;
  cpuCapability.supported = false;
  cpuCapability.lastUpdateTime = 0;
}

// CpuDetector 析构函数
CpuDetector::~CpuDetector() {
}

// 初始化
bool CpuDetector::init() {
  DEBUG_PRINTLN("初始化CPU检测器...");
  
  // 设置CPU信息
  #ifdef ESP32
    cpuInfo.description = "ESP32 CPU";
    cpuInfo.total = 240; // 默认主频240MHz
    cpuInfo.available = true;
    cpuCapability.supported = true;
  #elif defined(ESP8266)
    cpuInfo.description = "ESP8266 CPU";
    cpuInfo.total = 80; // 默认主频80MHz
    cpuInfo.available = true;
    cpuCapability.supported = true;
  #else
    cpuInfo.description = "Unknown CPU";
    cpuInfo.total = 16; // 假设16MHz
    cpuInfo.available = true;
    cpuCapability.supported = true;
  #endif
  
  DEBUG_PRINTLN("CPU检测器初始化完成");
  return true;
}

// 检测硬件资源
bool CpuDetector::detectResources() {
  // 更新CPU使用情况
  #ifdef ESP32
    // 在ESP32上使用esp_cpu_utilization_get()函数
    static unsigned long lastCheckTime = 0;
    static unsigned long lastIdleTime = 0;
    static unsigned long lastTotalTime = 0;
    
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime > 1000) { // 每秒检查一次
      lastCheckTime = currentTime;
      
      // 获取CPU使用率（使用ESP32可用的方法）
      // 使用简单的CPU使用率估算
      // 注意：这是一个简化的实现，实际CPU使用率可能需要更复杂的计算
      float usage = 0.0; // 默认值
      cpuInfo.used = usage;
      cpuInfo.usage = usage;
    }
  #elif defined(ESP8266)
    // 在ESP8266上使用简单的时间采样方法
    static unsigned long lastCheckTime = 0;
    static unsigned long lastIdleTime = 0;
    
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime > 1000) { // 每秒检查一次
      lastCheckTime = currentTime;
      
      // 简单估算：假设系统在空闲时大部分时间都在yield()
      float usage = random(5, 30); // 模拟值，但范围更合理
      cpuInfo.used = usage;
      cpuInfo.usage = usage;
    }
  #else
    // 在其他平台上使用时间采样方法
    static unsigned long lastCheckTime = 0;
    static unsigned long lastIdleTime = 0;
    
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime > 1000) { // 每秒检查一次
      lastCheckTime = currentTime;
      
      // 简单估算
      float usage = random(5, 40); // 模拟值，但范围更合理
      cpuInfo.used = usage;
      cpuInfo.usage = usage;
    }
  #endif
  
  cpuInfo.lastUpdateTime = millis();
  return true;
}

// 评估硬件能力
bool CpuDetector::evaluateCapabilities() {
  // 根据CPU主频评估能力
  float score = 0;
  
  if (cpuInfo.total >= 240) {
    score = 95;
    cpuCapability.level = CAPABILITY_LEVEL_EXCELLENT;
  } else if (cpuInfo.total >= 160) {
    score = 85;
    cpuCapability.level = CAPABILITY_LEVEL_HIGH;
  } else if (cpuInfo.total >= 80) {
    score = 70;
    cpuCapability.level = CAPABILITY_LEVEL_MEDIUM;
  } else {
    score = 40;
    cpuCapability.level = CAPABILITY_LEVEL_LOW;
  }
  
  cpuCapability.score = score;
  cpuCapability.lastUpdateTime = millis();
  
  // 更新CPU资源级别
  cpuInfo.level = cpuCapability.level;
  
  return true;
}

// 获取硬件评估结果
HardwareEvaluationResult CpuDetector::getEvaluationResult() {
  HardwareEvaluationResult result;
  result.isValid = false;
  return result;
}

// 获取资源信息
HardwareResourceInfo CpuDetector::getResourceInfo(HardwareResourceType type) {
  if (type == RESOURCE_TYPE_CPU) {
    return cpuInfo;
  }
  
  // 返回默认资源信息
  HardwareResourceInfo defaultInfo;
  defaultInfo.type = type;
  defaultInfo.name = "未知资源";
  defaultInfo.description = "资源未检测";
  defaultInfo.total = 0;
  defaultInfo.used = 0;
  defaultInfo.usage = 0;
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.available = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 获取能力信息
HardwareCapabilityInfo CpuDetector::getCapabilityInfo(const String& capabilityName) {
  if (capabilityName == "CPU") {
    return cpuCapability;
  }
  
  // 返回默认能力信息
  HardwareCapabilityInfo defaultInfo;
  defaultInfo.name = capabilityName;
  defaultInfo.description = "能力未评估";
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.score = 0;
  defaultInfo.supported = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 检查硬件兼容性
bool CpuDetector::checkCompatibility() {
  // 所有CPU都兼容
  return true;
}

// 检查硬件限制
bool CpuDetector::checkHardwareLimits() {
  // CPU没有特别的限制
  return true;
}

// 监控资源使用情况
bool CpuDetector::monitorResources() {
  // 重新检测CPU资源
  return detectResources();
}

// 重置检测器
void CpuDetector::reset() {
  // 重置CPU信息
  cpuInfo.used = 0;
  cpuInfo.usage = 0;
  cpuInfo.lastUpdateTime = 0;
  
  // 重置CPU能力信息
  cpuCapability.score = 0;
  cpuCapability.lastUpdateTime = 0;
}

// 获取检测器名称
String CpuDetector::getName() const {
  return "CpuDetector";
}

// 内存检测器实现

// MemoryDetector 构造函数
MemoryDetector::MemoryDetector() {
  // 初始化内存资源信息
  memoryInfo.type = RESOURCE_TYPE_MEMORY;
  memoryInfo.name = "Memory";
  memoryInfo.description = "内存";
  memoryInfo.total = 0;
  memoryInfo.used = 0;
  memoryInfo.usage = 0;
  memoryInfo.level = CAPABILITY_LEVEL_LOW;
  memoryInfo.available = false;
  memoryInfo.lastUpdateTime = 0;
  
  // 初始化内存能力信息
  memoryCapability.name = "Memory";
  memoryCapability.description = "内存能力";
  memoryCapability.level = CAPABILITY_LEVEL_LOW;
  memoryCapability.score = 0;
  memoryCapability.supported = false;
  memoryCapability.lastUpdateTime = 0;
}

// MemoryDetector 析构函数
MemoryDetector::~MemoryDetector() {
}

// 初始化
bool MemoryDetector::init() {
  DEBUG_PRINTLN("初始化内存检测器...");
  
  // 设置内存信息
  #ifdef ESP32
    memoryInfo.description = "ESP32 Memory";
    memoryInfo.total = 320; // 假设320KB可用内存
    memoryInfo.available = true;
    memoryCapability.supported = true;
  #elif defined(ESP8266)
    memoryInfo.description = "ESP8266 Memory";
    memoryInfo.total = 80; // 假设80KB可用内存
    memoryInfo.available = true;
    memoryCapability.supported = true;
  #else
    memoryInfo.description = "Unknown Memory";
    memoryInfo.total = 2; // 假设2KB可用内存
    memoryInfo.available = true;
    memoryCapability.supported = true;
  #endif
  
  DEBUG_PRINTLN("内存检测器初始化完成");
  return true;
}

// 检测硬件资源
bool MemoryDetector::detectResources() {
  // 更新内存使用情况
  #ifdef ESP32
    memoryInfo.used = ESP.getHeapSize() - ESP.getFreeHeap();
    memoryInfo.total = ESP.getHeapSize();
  #elif defined(ESP8266)
    memoryInfo.used = ESP.getHeapSize() - ESP.getFreeHeap();
    memoryInfo.total = ESP.getHeapSize();
  #else
    // 模拟内存使用情况
    memoryInfo.used = random(0, (int)memoryInfo.total * 0.8);
  #endif
  
  memoryInfo.usage = (memoryInfo.used / memoryInfo.total) * 100;
  memoryInfo.lastUpdateTime = millis();
  
  return true;
}

// 评估硬件能力
bool MemoryDetector::evaluateCapabilities() {
  // 根据内存大小评估能力
  float score = 0;
  
  if (memoryInfo.total >= 300) {
    score = 95;
    memoryCapability.level = CAPABILITY_LEVEL_EXCELLENT;
  } else if (memoryInfo.total >= 150) {
    score = 85;
    memoryCapability.level = CAPABILITY_LEVEL_HIGH;
  } else if (memoryInfo.total >= 50) {
    score = 70;
    memoryCapability.level = CAPABILITY_LEVEL_MEDIUM;
  } else {
    score = 40;
    memoryCapability.level = CAPABILITY_LEVEL_LOW;
  }
  
  memoryCapability.score = score;
  memoryCapability.lastUpdateTime = millis();
  
  // 更新内存资源级别
  memoryInfo.level = memoryCapability.level;
  
  return true;
}

// 获取硬件评估结果
HardwareEvaluationResult MemoryDetector::getEvaluationResult() {
  HardwareEvaluationResult result;
  result.isValid = false;
  return result;
}

// 获取资源信息
HardwareResourceInfo MemoryDetector::getResourceInfo(HardwareResourceType type) {
  if (type == RESOURCE_TYPE_MEMORY) {
    return memoryInfo;
  }
  
  // 返回默认资源信息
  HardwareResourceInfo defaultInfo;
  defaultInfo.type = type;
  defaultInfo.name = "未知资源";
  defaultInfo.description = "资源未检测";
  defaultInfo.total = 0;
  defaultInfo.used = 0;
  defaultInfo.usage = 0;
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.available = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 获取能力信息
HardwareCapabilityInfo MemoryDetector::getCapabilityInfo(const String& capabilityName) {
  if (capabilityName == "Memory") {
    return memoryCapability;
  }
  
  // 返回默认能力信息
  HardwareCapabilityInfo defaultInfo;
  defaultInfo.name = capabilityName;
  defaultInfo.description = "能力未评估";
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.score = 0;
  defaultInfo.supported = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 检查硬件兼容性
bool MemoryDetector::checkCompatibility() {
  // 所有内存都兼容
  return true;
}

// 检查硬件限制
bool MemoryDetector::checkHardwareLimits() {
  // 检查内存是否足够
  return memoryInfo.total >= 10; // 至少需要10KB内存
}

// 监控资源使用情况
bool MemoryDetector::monitorResources() {
  // 重新检测内存资源
  return detectResources();
}

// 重置检测器
void MemoryDetector::reset() {
  // 重置内存信息
  memoryInfo.used = 0;
  memoryInfo.usage = 0;
  memoryInfo.lastUpdateTime = 0;
  
  // 重置内存能力信息
  memoryCapability.score = 0;
  memoryCapability.lastUpdateTime = 0;
}

// 获取检测器名称
String MemoryDetector::getName() const {
  return "MemoryDetector";
}

// 存储检测器实现

// StorageDetector 构造函数
StorageDetector::StorageDetector() {
  // 初始化存储资源信息
  storageInfo.type = RESOURCE_TYPE_STORAGE;
  storageInfo.name = "Storage";
  storageInfo.description = "存储";
  storageInfo.total = 0;
  storageInfo.used = 0;
  storageInfo.usage = 0;
  storageInfo.level = CAPABILITY_LEVEL_LOW;
  storageInfo.available = false;
  storageInfo.lastUpdateTime = 0;
  
  // 初始化存储能力信息
  storageCapability.name = "Storage";
  storageCapability.description = "存储能力";
  storageCapability.level = CAPABILITY_LEVEL_LOW;
  storageCapability.score = 0;
  storageCapability.supported = false;
  storageCapability.lastUpdateTime = 0;
}

// StorageDetector 析构函数
StorageDetector::~StorageDetector() {
}

// 初始化
bool StorageDetector::init() {
  DEBUG_PRINTLN("初始化存储检测器...");
  
  // 设置存储信息
  #ifdef ESP32
    storageInfo.description = "ESP32 Flash";
    storageInfo.total = 4096; // 假设4MB闪存
    storageInfo.available = true;
    storageCapability.supported = true;
  #elif defined(ESP8266)
    storageInfo.description = "ESP8266 Flash";
    storageInfo.total = 1024; // 假设1MB闪存
    storageInfo.available = true;
    storageCapability.supported = true;
  #else
    storageInfo.description = "Unknown Storage";
    storageInfo.total = 64; // 假设64KB存储
    storageInfo.available = true;
    storageCapability.supported = true;
  #endif
  
  DEBUG_PRINTLN("存储检测器初始化完成");
  return true;
}

// 检测硬件资源
bool StorageDetector::detectResources() {
  // 更新存储使用情况
  #ifdef ESP32
    // 在ESP32上使用SPIFFS
    if (SPIFFS.begin()) {
      // 使用简化的存储检测方法
      storageInfo.total = 4096; // 假设4MB SPIFFS
      storageInfo.used = 0;
      storageInfo.usage = 0;
      SPIFFS.end();
    }
  #elif defined(ESP8266)
    // 在ESP8266上使用SPIFFS
    if (SPIFFS.begin()) {
      // 使用简化的存储检测方法
      storageInfo.total = 1024; // 假设1MB SPIFFS
      storageInfo.used = 0;
      storageInfo.usage = 0;
      SPIFFS.end();
    }
  #else
    // 在其他平台上使用模拟值
    storageInfo.used = random(0, (int)storageInfo.total * 0.8);
    storageInfo.usage = (storageInfo.used / storageInfo.total) * 100;
  #endif
  
  storageInfo.lastUpdateTime = millis();
  return true;
}

// 评估硬件能力
bool StorageDetector::evaluateCapabilities() {
  // 根据存储大小评估能力
  float score = 0;
  
  if (storageInfo.total >= 4096) {
    score = 95;
    storageCapability.level = CAPABILITY_LEVEL_EXCELLENT;
  } else if (storageInfo.total >= 2048) {
    score = 85;
    storageCapability.level = CAPABILITY_LEVEL_HIGH;
  } else if (storageInfo.total >= 512) {
    score = 70;
    storageCapability.level = CAPABILITY_LEVEL_MEDIUM;
  } else {
    score = 40;
    storageCapability.level = CAPABILITY_LEVEL_LOW;
  }
  
  storageCapability.score = score;
  storageCapability.lastUpdateTime = millis();
  
  // 更新存储资源级别
  storageInfo.level = storageCapability.level;
  
  return true;
}

// 获取硬件评估结果
HardwareEvaluationResult StorageDetector::getEvaluationResult() {
  HardwareEvaluationResult result;
  result.isValid = false;
  return result;
}

// 获取资源信息
HardwareResourceInfo StorageDetector::getResourceInfo(HardwareResourceType type) {
  if (type == RESOURCE_TYPE_STORAGE) {
    return storageInfo;
  }
  
  // 返回默认资源信息
  HardwareResourceInfo defaultInfo;
  defaultInfo.type = type;
  defaultInfo.name = "未知资源";
  defaultInfo.description = "资源未检测";
  defaultInfo.total = 0;
  defaultInfo.used = 0;
  defaultInfo.usage = 0;
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.available = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 获取能力信息
HardwareCapabilityInfo StorageDetector::getCapabilityInfo(const String& capabilityName) {
  if (capabilityName == "Storage") {
    return storageCapability;
  }
  
  // 返回默认能力信息
  HardwareCapabilityInfo defaultInfo;
  defaultInfo.name = capabilityName;
  defaultInfo.description = "能力未评估";
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.score = 0;
  defaultInfo.supported = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 检查硬件兼容性
bool StorageDetector::checkCompatibility() {
  // 所有存储都兼容
  return true;
}

// 检查硬件限制
bool StorageDetector::checkHardwareLimits() {
  // 检查存储是否足够
  return storageInfo.total >= 128; // 至少需要128KB存储
}

// 监控资源使用情况
bool StorageDetector::monitorResources() {
  // 重新检测存储资源
  return detectResources();
}

// 重置检测器
void StorageDetector::reset() {
  // 重置存储信息
  storageInfo.used = 0;
  storageInfo.usage = 0;
  storageInfo.lastUpdateTime = 0;
  
  // 重置存储能力信息
  storageCapability.score = 0;
  storageCapability.lastUpdateTime = 0;
}

// 获取检测器名称
String StorageDetector::getName() const {
  return "StorageDetector";
}

// 网络检测器实现

// NetworkDetector 构造函数
NetworkDetector::NetworkDetector() {
  // 初始化网络资源信息
  networkInfo.type = RESOURCE_TYPE_NETWORK;
  networkInfo.name = "Network";
  networkInfo.description = "网络";
  networkInfo.total = 100;
  networkInfo.used = 0;
  networkInfo.usage = 0;
  networkInfo.level = CAPABILITY_LEVEL_LOW;
  networkInfo.available = false;
  networkInfo.lastUpdateTime = 0;
  
  // 初始化网络能力信息
  networkCapability.name = "Network";
  networkCapability.description = "网络能力";
  networkCapability.level = CAPABILITY_LEVEL_LOW;
  networkCapability.score = 0;
  networkCapability.supported = false;
  networkCapability.lastUpdateTime = 0;
}

// NetworkDetector 析构函数
NetworkDetector::~NetworkDetector() {
}

// 初始化
bool NetworkDetector::init() {
  DEBUG_PRINTLN("初始化网络检测器...");
  
  // 设置网络信息
  #ifdef ESP32
    networkInfo.description = "ESP32 WiFi";
    networkInfo.available = true;
    networkCapability.supported = true;
  #elif defined(ESP8266)
    networkInfo.description = "ESP8266 WiFi";
    networkInfo.available = true;
    networkCapability.supported = true;
  #else
    networkInfo.description = "Unknown Network";
    networkInfo.available = false;
    networkCapability.supported = false;
  #endif
  
  DEBUG_PRINTLN("网络检测器初始化完成");
  return true;
}

// 检测硬件资源
bool NetworkDetector::detectResources() {
  // 更新网络使用情况
  #ifdef ESP32
    // 在ESP32上使用WiFi相关函数
    if (WiFi.status() == WL_CONNECTED) {
      // 获取RSSI
      int rssi = WiFi.RSSI();
      networkInfo.used = abs(rssi); // 使用信号强度的绝对值作为使用量
      networkInfo.usage = 100 - (abs(rssi) - 30) * 2; // 将RSSI转换为0-100的使用率
      if (networkInfo.usage < 0) networkInfo.usage = 0;
      if (networkInfo.usage > 100) networkInfo.usage = 100;
    } else {
      networkInfo.used = 100;
      networkInfo.usage = 100;
    }
  #elif defined(ESP8266)
    // 在ESP8266上使用WiFi相关函数
    if (WiFi.status() == WL_CONNECTED) {
      // 获取RSSI
      int rssi = WiFi.RSSI();
      networkInfo.used = abs(rssi); // 使用信号强度的绝对值作为使用量
      networkInfo.usage = 100 - (abs(rssi) - 30) * 2; // 将RSSI转换为0-100的使用率
      if (networkInfo.usage < 0) networkInfo.usage = 0;
      if (networkInfo.usage > 100) networkInfo.usage = 100;
    } else {
      networkInfo.used = 100;
      networkInfo.usage = 100;
    }
  #else
    // 在其他平台上使用模拟值
    networkInfo.used = random(5, 70);
    networkInfo.usage = networkInfo.used;
  #endif
  
  networkInfo.lastUpdateTime = millis();
  return true;
}

// 评估硬件能力
bool NetworkDetector::evaluateCapabilities() {
  // 根据网络类型评估能力
  float score = 0;
  
  #ifdef ESP32
    score = 90;
    networkCapability.level = CAPABILITY_LEVEL_HIGH;
  #elif defined(ESP8266)
    score = 80;
    networkCapability.level = CAPABILITY_LEVEL_MEDIUM;
  #else
    score = 30;
    networkCapability.level = CAPABILITY_LEVEL_LOW;
  #endif
  
  networkCapability.score = score;
  networkCapability.lastUpdateTime = millis();
  
  // 更新网络资源级别
  networkInfo.level = networkCapability.level;
  
  return true;
}

// 获取硬件评估结果
HardwareEvaluationResult NetworkDetector::getEvaluationResult() {
  HardwareEvaluationResult result;
  result.isValid = false;
  return result;
}

// 获取资源信息
HardwareResourceInfo NetworkDetector::getResourceInfo(HardwareResourceType type) {
  if (type == RESOURCE_TYPE_NETWORK) {
    return networkInfo;
  }
  
  // 返回默认资源信息
  HardwareResourceInfo defaultInfo;
  defaultInfo.type = type;
  defaultInfo.name = "未知资源";
  defaultInfo.description = "资源未检测";
  defaultInfo.total = 0;
  defaultInfo.used = 0;
  defaultInfo.usage = 0;
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.available = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 获取能力信息
HardwareCapabilityInfo NetworkDetector::getCapabilityInfo(const String& capabilityName) {
  if (capabilityName == "Network") {
    return networkCapability;
  }
  
  // 返回默认能力信息
  HardwareCapabilityInfo defaultInfo;
  defaultInfo.name = capabilityName;
  defaultInfo.description = "能力未评估";
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.score = 0;
  defaultInfo.supported = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 检查硬件兼容性
bool NetworkDetector::checkCompatibility() {
  // 所有网络都兼容
  return true;
}

// 检查硬件限制
bool NetworkDetector::checkHardwareLimits() {
  // 网络没有特别的限制
  return true;
}

// 监控资源使用情况
bool NetworkDetector::monitorResources() {
  // 重新检测网络资源
  return detectResources();
}

// 重置检测器
void NetworkDetector::reset() {
  // 重置网络信息
  networkInfo.used = 0;
  networkInfo.usage = 0;
  networkInfo.lastUpdateTime = 0;
  
  // 重置网络能力信息
  networkCapability.score = 0;
  networkCapability.lastUpdateTime = 0;
}

// 获取检测器名称
String NetworkDetector::getName() const {
  return "NetworkDetector";
}

// 电源检测器实现

// PowerDetector 构造函数
PowerDetector::PowerDetector() {
  // 初始化电源资源信息
  powerInfo.type = RESOURCE_TYPE_POWER;
  powerInfo.name = "Power";
  powerInfo.description = "电源";
  powerInfo.total = 100;
  powerInfo.used = 0;
  powerInfo.usage = 0;
  powerInfo.level = CAPABILITY_LEVEL_LOW;
  powerInfo.available = false;
  powerInfo.lastUpdateTime = 0;
  
  // 初始化电源能力信息
  powerCapability.name = "Power";
  powerCapability.description = "电源能力";
  powerCapability.level = CAPABILITY_LEVEL_LOW;
  powerCapability.score = 0;
  powerCapability.supported = false;
  powerCapability.lastUpdateTime = 0;
}

// PowerDetector 析构函数
PowerDetector::~PowerDetector() {
}

// 初始化
bool PowerDetector::init() {
  DEBUG_PRINTLN("初始化电源检测器...");
  
  // 设置电源信息
  powerInfo.description = "System Power";
  powerInfo.available = true;
  powerCapability.supported = true;
  
  DEBUG_PRINTLN("电源检测器初始化完成");
  return true;
}

// 检测硬件资源
bool PowerDetector::detectResources() {
  // 更新电源使用情况
  #ifdef ESP32
    // 在ESP32上，可以通过读取ADC值来检测电池电压
    // 假设使用GPIO34作为电池电压检测引脚
    int adcValue = analogRead(34);
    float voltage = adcValue * (3.3 / 4096.0);
    // 假设电池满电为4.2V，低电为3.0V
    float batteryPercentage = ((voltage - 3.0) / (4.2 - 3.0)) * 100;
    if (batteryPercentage < 0) batteryPercentage = 0;
    if (batteryPercentage > 100) batteryPercentage = 100;
    
    powerInfo.used = 100 - batteryPercentage;
    powerInfo.usage = powerInfo.used;
  #elif defined(ESP8266)
    // 在ESP8266上，使用模拟值，但基于系统负载
    static unsigned long lastCheckTime = 0;
    static unsigned long lastIdleTime = 0;
    
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime > 1000) { // 每秒检查一次
      lastCheckTime = currentTime;
      
      // 基于系统负载生成更合理的电源使用值
      float usage = random(30, 70); // 模拟值，但范围更合理
      powerInfo.used = usage;
      powerInfo.usage = usage;
    }
  #else
    // 在其他平台上使用模拟值
    static unsigned long lastCheckTime = 0;
    
    unsigned long currentTime = millis();
    if (currentTime - lastCheckTime > 1000) { // 每秒检查一次
      lastCheckTime = currentTime;
      
      // 生成更合理的电源使用值
      float usage = random(25, 75); // 模拟值，但范围更合理
      powerInfo.used = usage;
      powerInfo.usage = usage;
    }
  #endif
  
  powerInfo.lastUpdateTime = millis();
  return true;
}

// 评估硬件能力
bool PowerDetector::evaluateCapabilities() {
  // 评估电源能力
  float score = 75;
  powerCapability.level = CAPABILITY_LEVEL_MEDIUM;
  
  powerCapability.score = score;
  powerCapability.lastUpdateTime = millis();
  
  // 更新电源资源级别
  powerInfo.level = powerCapability.level;
  
  return true;
}

// 获取硬件评估结果
HardwareEvaluationResult PowerDetector::getEvaluationResult() {
  HardwareEvaluationResult result;
  result.isValid = false;
  return result;
}

// 获取资源信息
HardwareResourceInfo PowerDetector::getResourceInfo(HardwareResourceType type) {
  if (type == RESOURCE_TYPE_POWER) {
    return powerInfo;
  }
  
  // 返回默认资源信息
  HardwareResourceInfo defaultInfo;
  defaultInfo.type = type;
  defaultInfo.name = "未知资源";
  defaultInfo.description = "资源未检测";
  defaultInfo.total = 0;
  defaultInfo.used = 0;
  defaultInfo.usage = 0;
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.available = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 获取能力信息
HardwareCapabilityInfo PowerDetector::getCapabilityInfo(const String& capabilityName) {
  if (capabilityName == "Power") {
    return powerCapability;
  }
  
  // 返回默认能力信息
  HardwareCapabilityInfo defaultInfo;
  defaultInfo.name = capabilityName;
  defaultInfo.description = "能力未评估";
  defaultInfo.level = CAPABILITY_LEVEL_LOW;
  defaultInfo.score = 0;
  defaultInfo.supported = false;
  defaultInfo.lastUpdateTime = millis();
  return defaultInfo;
}

// 检查硬件兼容性
bool PowerDetector::checkCompatibility() {
  // 所有电源都兼容
  return true;
}

// 检查硬件限制
bool PowerDetector::checkHardwareLimits() {
  // 电源没有特别的限制
  return true;
}

// 监控资源使用情况
bool PowerDetector::monitorResources() {
  // 重新检测电源资源
  return detectResources();
}

// 重置检测器
void PowerDetector::reset() {
  // 重置电源信息
  powerInfo.used = 0;
  powerInfo.usage = 0;
  powerInfo.lastUpdateTime = 0;
  
  // 重置电源能力信息
  powerCapability.score = 0;
  powerCapability.lastUpdateTime = 0;
}

// 获取检测器名称
String PowerDetector::getName() const {
  return "PowerDetector";
}
