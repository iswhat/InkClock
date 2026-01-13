#ifndef FEATURE_MANAGER_H
#define FEATURE_MANAGER_H

#include <Arduino.h>
#include <string>
#include <map>
#include <vector>
#include "hardware_detector.h"

// 功能级别枚举
enum FeatureLevel {
  FEATURE_LEVEL_DISABLED,  // 禁用
  FEATURE_LEVEL_MINIMAL,    // 最小功能
  FEATURE_LEVEL_BASIC,      // 基本功能
  FEATURE_LEVEL_STANDARD,   // 标准功能
  FEATURE_LEVEL_ADVANCED,   // 高级功能
  FEATURE_LEVEL_FULL        // 完整功能
};

// 功能配置结构
typedef struct {
  String name;              // 功能名称
  String description;       // 功能描述
  FeatureLevel defaultLevel; // 默认级别
  FeatureLevel currentLevel; // 当前级别
  bool essential;           // 是否为核心功能
  int minMemoryRequired;    // 最小内存要求（KB）
  int minStorageRequired;   // 最小存储要求（KB）
  float minScoreRequired;   // 最小硬件得分要求
  std::map<FeatureLevel, std::map<String, String>> levelConfigs; // 不同级别的配置参数
  unsigned long lastUpdateTime; // 最后更新时间
} FeatureConfig;

// 功能管理器类
class FeatureManager {
private:
  static FeatureManager* instance;
  
  // 功能配置映射
  std::map<String, FeatureConfig> featureConfigs;
  
  // 硬件评估结果
  HardwareEvaluationResult hardwareResult;
  
  // 初始化状态
  bool initialized;
  
  // 最后评估时间
  unsigned long lastEvaluationTime;
  
  // 私有构造函数
  FeatureManager();
  
  // 私有方法：初始化默认功能配置
  void initDefaultFeatures();
  
  // 私有方法：根据硬件能力计算功能级别
  FeatureLevel calculateFeatureLevel(const FeatureConfig& config);
  
  // 私有方法：应用功能级别配置
  void applyFeatureLevel(const String& featureName, FeatureLevel level);
  
  // 私有方法：发布功能状态变化事件
  void publishFeatureStatusEvent(const String& featureName, FeatureLevel oldLevel, FeatureLevel newLevel);
  
public:
  // 获取单例实例
  static FeatureManager* getInstance();
  
  // 初始化
  bool init();
  
  // 更新硬件评估结果
  bool updateHardwareEvaluation();
  
  // 评估并调整所有功能级别
  bool evaluateFeatures();
  
  // 获取功能配置
  FeatureConfig getFeatureConfig(const String& featureName);
  
  // 获取所有功能配置
  std::vector<FeatureConfig> getAllFeatureConfigs();
  
  // 检查功能是否启用
  bool isFeatureEnabled(const String& featureName);
  
  // 获取功能级别
  FeatureLevel getFeatureLevel(const String& featureName);
  
  // 设置功能级别（手动覆盖）
  bool setFeatureLevel(const String& featureName, FeatureLevel level);
  
  // 重置所有功能到默认级别
  bool resetAllFeatures();
  
  // 获取功能配置参数
  String getFeatureConfigParam(const String& featureName, const String& paramName);
  
  // 获取硬件能力级别对应的功能级别映射
  FeatureLevel getLevelForHardwareCapability(HardwareCapabilityLevel capabilityLevel);
  
  // 监控功能状态
  bool monitorFeatures();
  
  // 获取核心功能列表
  std::vector<String> getEssentialFeatures();
  
  // 获取可选功能列表
  std::vector<String> getOptionalFeatures();
};

// 功能管理器宏
#define FEATURE_MANAGER FeatureManager::getInstance()
#define FEATURE_ENABLED(name) FEATURE_MANAGER->isFeatureEnabled(name)
#define FEATURE_LEVEL(name) FEATURE_MANAGER->getFeatureLevel(name)
#define FEATURE_CONFIG(name, param) FEATURE_MANAGER->getFeatureConfigParam(name, param)

#endif // FEATURE_MANAGER_H
