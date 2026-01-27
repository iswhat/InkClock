#ifndef HARDWARE_DETECTOR_H
#define HARDWARE_DETECTOR_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "arduino_compat.h"
#endif

#include <string>
#include <vector>
#include <map>
#include <memory>
#include "event_bus.h"

// 硬件资源类型枚举
enum HardwareResourceType {
  RESOURCE_TYPE_CPU,        // CPU资源
  RESOURCE_TYPE_MEMORY,     // 内存资源
  RESOURCE_TYPE_STORAGE,    // 存储资源
  RESOURCE_TYPE_NETWORK,    // 网络资源
  RESOURCE_TYPE_DISPLAY,    // 显示资源
  RESOURCE_TYPE_SENSOR,     // 传感器资源
  RESOURCE_TYPE_POWER,      // 电源资源
  RESOURCE_TYPE_OTHER       // 其他资源
};

// 硬件能力级别枚举
enum HardwareCapabilityLevel {
  CAPABILITY_LEVEL_LOW,     // 低能力
  CAPABILITY_LEVEL_MEDIUM,  // 中等能力
  CAPABILITY_LEVEL_HIGH,    // 高能力
  CAPABILITY_LEVEL_EXCELLENT // 优秀能力
};

// 硬件资源信息结构
typedef struct {
  HardwareResourceType type;     // 资源类型
  String name;                  // 资源名称
  String description;           // 资源描述
  float total;                  // 总资源量
  float used;                   // 已使用资源量
  float usage;                  // 资源使用率（百分比）
  HardwareCapabilityLevel level; // 能力级别
  bool available;               // 资源是否可用
  unsigned long lastUpdateTime; // 最后更新时间
  std::map<String, String> properties; // 资源属性
} HardwareResourceInfo;

// 硬件能力信息结构
typedef struct {
  String name;                  // 能力名称
  String description;           // 能力描述
  HardwareCapabilityLevel level; // 能力级别
  float score;                  // 能力得分（0-100）
  bool supported;               // 是否支持
  unsigned long lastUpdateTime; // 最后更新时间
  std::map<String, String> properties; // 能力属性
} HardwareCapabilityInfo;

// 硬件评估结果结构
typedef struct {
  String hardwareId;            // 硬件ID
  String hardwareName;          // 硬件名称
  String hardwareType;          // 硬件类型
  String platform;              // 平台类型
  String cpuInfo;               // CPU信息
  float totalMemory;            // 总内存（KB）
  float totalStorage;           // 总存储（KB）
  HardwareCapabilityLevel overallLevel; // 整体能力级别
  float overallScore;           // 整体能力得分（0-100）
  std::vector<HardwareResourceInfo> resources; // 资源信息
  std::vector<HardwareCapabilityInfo> capabilities; // 能力信息
  unsigned long evaluationTime;        // 评估时间
  bool isValid;                 // 评估结果是否有效
} HardwareEvaluationResult;

// 硬件检测器接口
class IHardwareDetector {
public:
  virtual ~IHardwareDetector() {}
  
  // 初始化检测器
  virtual bool init() = 0;
  
  // 检测硬件资源
  virtual bool detectResources() = 0;
  
  // 评估硬件能力
  virtual bool evaluateCapabilities() = 0;
  
  // 获取硬件评估结果
  virtual HardwareEvaluationResult getEvaluationResult() = 0;
  
  // 获取资源信息
  virtual HardwareResourceInfo getResourceInfo(HardwareResourceType type) = 0;
  
  // 获取能力信息
  virtual HardwareCapabilityInfo getCapabilityInfo(const String& capabilityName) = 0;
  
  // 检查硬件兼容性
  virtual bool checkCompatibility() = 0;
  
  // 检查硬件限制
  virtual bool checkHardwareLimits() = 0;
  
  // 监控资源使用情况
  virtual bool monitorResources() = 0;
  
  // 重置检测器
  virtual void reset() = 0;
  
  // 获取检测器名称
  virtual String getName() const = 0;
};

// 硬件检测器类
class HardwareDetector {
private:
  static HardwareDetector* instance;
  
  // 硬件评估结果
  HardwareEvaluationResult evaluationResult;
  
  // 资源信息映射
  std::map<HardwareResourceType, HardwareResourceInfo> resources;
  
  // 能力信息映射
  std::map<String, HardwareCapabilityInfo> capabilities;
  
  // 检测器列表
  std::vector<std::shared_ptr<IHardwareDetector>> detectors;
  
  // 初始化状态
  bool initialized;
  
  // 评估时间
  unsigned long lastEvaluationTime;
  
  // 私有构造函数
  HardwareDetector();
  
  // 私有方法：初始化默认检测器
  void initDefaultDetectors();
  
  // 私有方法：更新评估结果
  void updateEvaluationResult();
  
  // 私有方法：计算整体能力得分
  float calculateOverallScore();
  
  // 私有方法：确定整体能力级别
  HardwareCapabilityLevel determineOverallLevel(float score);
  
  // 私有方法：发布硬件评估事件
  void publishHardwareEvaluationEvent();
  
  // 私有方法：发布资源告警事件
  void publishResourceAlertEvent(HardwareResourceType type, float usage, float threshold);

public:
  // 获取单例实例
  static HardwareDetector* getInstance();
  
  // 初始化
  bool init();
  
  // 注册检测器
  bool registerDetector(std::shared_ptr<IHardwareDetector> detector);
  
  // 检测硬件资源
  bool detectResources();
  
  // 评估硬件能力
  bool evaluateCapabilities();
  
  // 获取硬件评估结果
  HardwareEvaluationResult getEvaluationResult();
  
  // 获取资源信息
  HardwareResourceInfo getResourceInfo(HardwareResourceType type);
  
  // 获取所有资源信息
  std::vector<HardwareResourceInfo> getAllResourcesInfo();
  
  // 获取能力信息
  HardwareCapabilityInfo getCapabilityInfo(const String& capabilityName);
  
  // 获取所有能力信息
  std::vector<HardwareCapabilityInfo> getAllCapabilitiesInfo();
  
  // 检查硬件兼容性
  bool checkCompatibility();
  
  // 检查硬件限制
  bool checkHardwareLimits();
  
  // 监控资源使用情况
  bool monitorResources();
  
  // 重置检测器
  void reset();
  
  // 获取检测器名称
  String getName() const;
  
  // 获取平台信息
  String getPlatformInfo();
  
  // 获取硬件ID
  String getHardwareId();
  
  // 获取硬件名称
  String getHardwareName();
  
  // 获取硬件类型
  String getHardwareType();
};

// CPU检测器类
class CpuDetector : public IHardwareDetector {
private:
  HardwareResourceInfo cpuInfo;
  HardwareCapabilityInfo cpuCapability;
  
public:
  CpuDetector();
  ~CpuDetector();
  
  bool init() override;
  bool detectResources() override;
  bool evaluateCapabilities() override;
  HardwareEvaluationResult getEvaluationResult() override;
  HardwareResourceInfo getResourceInfo(HardwareResourceType type) override;
  HardwareCapabilityInfo getCapabilityInfo(const String& capabilityName) override;
  bool checkCompatibility() override;
  bool checkHardwareLimits() override;
  bool monitorResources() override;
  void reset() override;
  String getName() const override;
};

// 内存检测器类
class MemoryDetector : public IHardwareDetector {
private:
  HardwareResourceInfo memoryInfo;
  HardwareCapabilityInfo memoryCapability;
  
public:
  MemoryDetector();
  ~MemoryDetector();
  
  bool init() override;
  bool detectResources() override;
  bool evaluateCapabilities() override;
  HardwareEvaluationResult getEvaluationResult() override;
  HardwareResourceInfo getResourceInfo(HardwareResourceType type) override;
  HardwareCapabilityInfo getCapabilityInfo(const String& capabilityName) override;
  bool checkCompatibility() override;
  bool checkHardwareLimits() override;
  bool monitorResources() override;
  void reset() override;
  String getName() const override;
};

// 存储检测器类
class StorageDetector : public IHardwareDetector {
private:
  HardwareResourceInfo storageInfo;
  HardwareCapabilityInfo storageCapability;
  
public:
  StorageDetector();
  ~StorageDetector();
  
  bool init() override;
  bool detectResources() override;
  bool evaluateCapabilities() override;
  HardwareEvaluationResult getEvaluationResult() override;
  HardwareResourceInfo getResourceInfo(HardwareResourceType type) override;
  HardwareCapabilityInfo getCapabilityInfo(const String& capabilityName) override;
  bool checkCompatibility() override;
  bool checkHardwareLimits() override;
  bool monitorResources() override;
  void reset() override;
  String getName() const override;
};

// 网络检测器类
class NetworkDetector : public IHardwareDetector {
private:
  HardwareResourceInfo networkInfo;
  HardwareCapabilityInfo networkCapability;
  
public:
  NetworkDetector();
  ~NetworkDetector();
  
  bool init() override;
  bool detectResources() override;
  bool evaluateCapabilities() override;
  HardwareEvaluationResult getEvaluationResult() override;
  HardwareResourceInfo getResourceInfo(HardwareResourceType type) override;
  HardwareCapabilityInfo getCapabilityInfo(const String& capabilityName) override;
  bool checkCompatibility() override;
  bool checkHardwareLimits() override;
  bool monitorResources() override;
  void reset() override;
  String getName() const override;
};

// 电源检测器类
class PowerDetector : public IHardwareDetector {
private:
  HardwareResourceInfo powerInfo;
  HardwareCapabilityInfo powerCapability;
  
public:
  PowerDetector();
  ~PowerDetector();
  
  bool init() override;
  bool detectResources() override;
  bool evaluateCapabilities() override;
  HardwareEvaluationResult getEvaluationResult() override;
  HardwareResourceInfo getResourceInfo(HardwareResourceType type) override;
  HardwareCapabilityInfo getCapabilityInfo(const String& capabilityName) override;
  bool checkCompatibility() override;
  bool checkHardwareLimits() override;
  bool monitorResources() override;
  void reset() override;
  String getName() const override;
};

// 硬件检测器宏
#define HARDWARE_DETECTOR HardwareDetector::getInstance()
#define HARDWARE_EVALUATION HARDWARE_DETECTOR->getEvaluationResult()
#define HARDWARE_RESOURCE(type) HARDWARE_DETECTOR->getResourceInfo(type)
#define HARDWARE_CAPABILITY(name) HARDWARE_DETECTOR->getCapabilityInfo(name)

#endif // HARDWARE_DETECTOR_H
