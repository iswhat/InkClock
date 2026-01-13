#include "storage_manager.h"
#include <Arduino.h>
#include <SPIFFS.h>
#include <SD.h>
#include <vector>
#include <map>
#include <algorithm>

// 存储管理器单例实例
StorageManager* StorageManager::instance = nullptr;

// 构造函数
StorageManager::StorageManager() : initialized(false), lastCleanupTime(0) {
  // 初始化存储介质
  initDefaultStorageMedia();
}

// 获取单例实例
StorageManager* StorageManager::getInstance() {
  if (instance == nullptr) {
    instance = new StorageManager();
  }
  return instance;
}

// 初始化默认存储介质
void StorageManager::initDefaultStorageMedia() {
  // 注册RAM存储
  storageMedia[STORAGE_RAM] = std::make_shared<RAMStorage>();
  
  // 注册SPIFFS存储
  storageMedia[STORAGE_SPIFFS] = std::make_shared<SPIFFSStorage>();
  
  // 注册TF卡存储
  storageMedia[STORAGE_TFCARD] = std::make_shared<TFCardStorage>();
}

// 初始化
bool StorageManager::init() {
  if (initialized) {
    return true;
  }
  
  // 初始化SPIFFS存储
  auto spiffsStorage = std::dynamic_pointer_cast<SPIFFSStorage>(storageMedia[STORAGE_SPIFFS]);
  if (spiffsStorage) {
    spiffsStorage->init();
  }
  
  // 初始化TF卡存储
  auto tfCardStorage = std::dynamic_pointer_cast<TFCardStorage>(storageMedia[STORAGE_TFCARD]);
  if (tfCardStorage) {
    tfCardStorage->init();
  }
  
  // 初始化存储使用统计
  for (const auto& pair : storageMedia) {
    storageUsage[pair.first] = 0;
  }
  
  initialized = true;
  return true;
}

// 注册存储介质
bool StorageManager::registerStorageMedium(StorageMediumType type, std::shared_ptr<IStorageOperation> storage) {
  storageMedia[type] = storage;
  storageUsage[type] = 0;
  return true;
}

// 注册数据存储配置
bool StorageManager::registerDataConfig(const DataStorageConfig& config) {
  dataConfigs[config.dataId] = config;
  return true;
}

// 读取数据
bool StorageManager::read(const String& dataId, String& value) {
  if (!dataConfigs.count(dataId)) {
    return false;
  }
  
  const DataStorageConfig& config = dataConfigs[dataId];
  StorageMediumType medium = selectStorageMedium(config);
  
  auto storage = getStorageMedium(medium);
  if (!storage) {
    return false;
  }
  
  bool success = storage->read(dataId, value);
  if (success) {
    // 更新最后访问时间
    DataStorageConfig& mutableConfig = dataConfigs[dataId];
    mutableConfig.lastAccessTime = millis();
  }
  
  return success;
}

// 读取数据（二进制）
bool StorageManager::read(const String& dataId, std::vector<byte>& value) {
  if (!dataConfigs.count(dataId)) {
    return false;
  }
  
  const DataStorageConfig& config = dataConfigs[dataId];
  StorageMediumType medium = selectStorageMedium(config);
  
  auto storage = getStorageMedium(medium);
  if (!storage) {
    return false;
  }
  
  bool success = storage->read(dataId, value);
  if (success) {
    // 更新最后访问时间
    DataStorageConfig& mutableConfig = dataConfigs[dataId];
    mutableConfig.lastAccessTime = millis();
  }
  
  return success;
}

// 写入数据
bool StorageManager::write(const String& dataId, const String& value) {
  if (!dataConfigs.count(dataId)) {
    // 如果数据配置不存在，创建默认配置
    DataStorageConfig defaultConfig;
    defaultConfig.dataId = dataId;
    defaultConfig.importance = DATA_LEVEL_MEDIUM;
    defaultConfig.frequency = ACCESS_FREQUENCY_MEDIUM;
    defaultConfig.compressible = true;
    defaultConfig.preferredMedium = STORAGE_RAM;
    dataConfigs[dataId] = defaultConfig;
  }
  
  DataStorageConfig& config = dataConfigs[dataId];
  StorageMediumType medium = selectStorageMedium(config);
  
  auto storage = getStorageMedium(medium);
  if (!storage) {
    return false;
  }
  
  bool success = storage->write(dataId, value);
  if (success) {
    // 更新最后修改时间和访问时间
    config.lastModifiedTime = millis();
    config.lastAccessTime = millis();
    
    // 更新存储使用统计
    storageUsage[medium] += value.length();
    
    // 如果启用了备份，执行备份
    if (config.backupEnabled) {
      backupData(dataId);
    }
  }
  
  return success;
}

// 写入数据（二进制）
bool StorageManager::write(const String& dataId, const std::vector<byte>& value) {
  if (!dataConfigs.count(dataId)) {
    // 如果数据配置不存在，创建默认配置
    DataStorageConfig defaultConfig;
    defaultConfig.dataId = dataId;
    defaultConfig.importance = DATA_LEVEL_MEDIUM;
    defaultConfig.frequency = ACCESS_FREQUENCY_MEDIUM;
    defaultConfig.compressible = true;
    defaultConfig.preferredMedium = STORAGE_RAM;
    dataConfigs[dataId] = defaultConfig;
  }
  
  DataStorageConfig& config = dataConfigs[dataId];
  StorageMediumType medium = selectStorageMedium(config);
  
  auto storage = getStorageMedium(medium);
  if (!storage) {
    return false;
  }
  
  bool success = storage->write(dataId, value);
  if (success) {
    // 更新最后修改时间和访问时间
    config.lastModifiedTime = millis();
    config.lastAccessTime = millis();
    
    // 更新存储使用统计
    storageUsage[medium] += value.size();
    
    // 如果启用了备份，执行备份
    if (config.backupEnabled) {
      backupData(dataId);
    }
  }
  
  return success;
}

// 删除数据
bool StorageManager::remove(const String& dataId) {
  if (!dataConfigs.count(dataId)) {
    return false;
  }
  
  const DataStorageConfig& config = dataConfigs[dataId];
  StorageMediumType medium = selectStorageMedium(config);
  
  auto storage = getStorageMedium(medium);
  if (!storage) {
    return false;
  }
  
  bool success = storage->remove(dataId);
  if (success) {
    // 从配置映射中移除
    dataConfigs.erase(dataId);
  }
  
  return success;
}

// 检查数据是否存在
bool StorageManager::exists(const String& dataId) {
  if (!dataConfigs.count(dataId)) {
    return false;
  }
  
  const DataStorageConfig& config = dataConfigs[dataId];
  StorageMediumType medium = selectStorageMedium(config);
  
  auto storage = getStorageMedium(medium);
  if (!storage) {
    return false;
  }
  
  return storage->exists(dataId);
}

// 获取数据大小
unsigned long StorageManager::getSize(const String& dataId) {
  if (!dataConfigs.count(dataId)) {
    return 0;
  }
  
  const DataStorageConfig& config = dataConfigs[dataId];
  StorageMediumType medium = selectStorageMedium(config);
  
  auto storage = getStorageMedium(medium);
  if (!storage) {
    return 0;
  }
  
  return storage->getSize(dataId);
}

// 列出所有数据ID
std::vector<String> StorageManager::listDataIds(const String& prefix) {
  std::vector<String> dataIds;
  
  for (const auto& pair : dataConfigs) {
    const String& dataId = pair.first;
    if (prefix.isEmpty() || dataId.startsWith(prefix)) {
      dataIds.push_back(dataId);
    }
  }
  
  return dataIds;
}

// 获取数据存储配置
DataStorageConfig StorageManager::getDataConfig(const String& dataId) {
  if (dataConfigs.count(dataId)) {
    return dataConfigs[dataId];
  }
  
  // 返回默认配置
  DataStorageConfig defaultConfig;
  defaultConfig.dataId = dataId;
  return defaultConfig;
}

// 更新数据存储配置
bool StorageManager::updateDataConfig(const String& dataId, const DataStorageConfig& config) {
  if (!dataConfigs.count(dataId)) {
    return false;
  }
  
  dataConfigs[dataId] = config;
  return true;
}

// 获取存储介质信息
StorageMediumInfo StorageManager::getStorageMediumInfo(StorageMediumType type) {
  auto storage = getStorageMedium(type);
  if (storage) {
    return storage->getMediumInfo();
  }
  
  // 返回默认信息
  StorageMediumInfo info;
  info.type = type;
  info.available = false;
  return info;
}

// 获取所有存储介质信息
std::vector<StorageMediumInfo> StorageManager::getAllStorageMediumInfo() {
  std::vector<StorageMediumInfo> infos;
  
  for (const auto& pair : storageMedia) {
    auto storage = pair.second;
    infos.push_back(storage->getMediumInfo());
  }
  
  return infos;
}

// 获取存储使用统计
std::map<StorageMediumType, unsigned long> StorageManager::getStorageUsage() {
  return storageUsage;
}

// 执行存储清理
bool StorageManager::cleanup() {
  // 清理过期数据
  cleanupExpiredData();
  
  // 优化存储布局
  optimizeStorageLayout();
  
  return true;
}

// 执行存储优化
bool StorageManager::optimize() {
  // 压缩可压缩数据
  for (auto& pair : dataConfigs) {
    const String& dataId = pair.first;
    const DataStorageConfig& config = pair.second;
    
    if (config.compressible) {
      compressData(dataId);
    }
  }
  
  // 清理过期数据
  cleanupExpiredData();
  
  // 优化存储布局
  optimizeStorageLayout();
  
  return true;
}

// 导出所有数据
bool StorageManager::exportAllData(const String& exportPath) {
  // 实现导出功能
  return true;
}

// 导入数据
bool StorageManager::importData(const String& importPath) {
  // 实现导入功能
  return true;
}

// 设置存储介质优先级
bool StorageManager::setStorageMediumPriority(const std::vector<StorageMediumType>& priorityList) {
  // 实现优先级设置
  return true;
}

// 获取推荐的存储介质
StorageMediumType StorageManager::getRecommendedStorageMedium(const DataImportanceLevel& importance, const DataAccessFrequency& frequency) {
  // 根据数据重要性和访问频率推荐存储介质
  if (frequency == ACCESS_FREQUENCY_REAL_TIME || frequency == ACCESS_FREQUENCY_HIGH) {
    return STORAGE_RAM;
  } else if (importance == DATA_LEVEL_CRITICAL) {
    return STORAGE_SPIFFS;
  } else if (importance == DATA_LEVEL_HIGH) {
    return STORAGE_SPIFFS;
  } else {
    return STORAGE_TFCARD;
  }
}

// 验证存储系统健康状态
bool StorageManager::checkHealth() {
  // 检查所有存储介质的健康状态
  bool allHealthy = true;
  
  for (const auto& pair : storageMedia) {
    auto storage = pair.second;
    auto info = storage->getMediumInfo();
    if (!info.available) {
      allHealthy = false;
    }
  }
  
  return allHealthy;
}

// 重置存储管理器
bool StorageManager::reset() {
  // 清空所有存储介质
  for (const auto& pair : storageMedia) {
    auto storage = pair.second;
    storage->clear();
  }
  
  // 清空配置
  dataConfigs.clear();
  storageUsage.clear();
  
  // 重新初始化
  initialized = false;
  return init();
}

// 根据数据特性选择存储介质
StorageMediumType StorageManager::selectStorageMedium(const DataStorageConfig& config) {
  // 首先检查首选存储介质
  if (isStorageMediumAvailable(config.preferredMedium)) {
    return config.preferredMedium;
  }
  
  // 检查fallback存储介质
  for (const auto& medium : config.fallbackMedia) {
    if (isStorageMediumAvailable(medium)) {
      return medium;
    }
  }
  
  // 根据重要性和频率推荐
  return getRecommendedStorageMedium(config.importance, config.frequency);
}

// 获取存储介质
std::shared_ptr<IStorageOperation> StorageManager::getStorageMedium(StorageMediumType type) {
  if (storageMedia.count(type)) {
    return storageMedia[type];
  }
  return nullptr;
}

// 验证存储介质可用性
bool StorageManager::isStorageMediumAvailable(StorageMediumType type) {
  auto storage = getStorageMedium(type);
  if (!storage) {
    return false;
  }
  
  auto info = storage->getMediumInfo();
  return info.available;
}

// 计算数据存储成本
float StorageManager::calculateStorageCost(StorageMediumType type, const DataStorageConfig& config) {
  // 简化的成本计算
  float cost = 0.0;
  
  // 基于存储介质类型的基础成本
  switch (type) {
    case STORAGE_RAM:
      cost = 1.0;  // 最高成本
      break;
    case STORAGE_SPIFFS:
      cost = 0.5;
      break;
    case STORAGE_TFCARD:
      cost = 0.1;  // 最低成本
      break;
    default:
      cost = 0.5;
  }
  
  // 基于访问频率的成本调整
  switch (config.frequency) {
    case ACCESS_FREQUENCY_REAL_TIME:
      cost *= 0.8;  // 高频访问降低RAM成本
      break;
    case ACCESS_FREQUENCY_HIGH:
      cost *= 0.9;
      break;
    case ACCESS_FREQUENCY_RARE:
      cost *= 1.2;  // 低频访问增加RAM成本
      break;
  }
  
  // 基于重要性的成本调整
  switch (config.importance) {
    case DATA_LEVEL_CRITICAL:
      cost *= 0.7;  // 关键数据降低非易失性存储成本
      break;
    case DATA_LEVEL_HIGH:
      cost *= 0.8;
      break;
  }
  
  return cost;
}

// 清理过期数据
void StorageManager::cleanupExpiredData() {
  unsigned long currentTime = millis();
  
  // 找出过期的数据
  std::vector<String> expiredDataIds;
  
  for (const auto& pair : dataConfigs) {
    const String& dataId = pair.first;
    const DataStorageConfig& config = pair.second;
    
    if (config.lifespan > 0 && currentTime - config.lastModifiedTime > config.lifespan) {
      expiredDataIds.push_back(dataId);
    }
  }
  
  // 删除过期数据
  for (const auto& dataId : expiredDataIds) {
    remove(dataId);
  }
  
  lastCleanupTime = currentTime;
}

// 执行数据备份
bool StorageManager::backupData(const String& dataId) {
  if (!dataConfigs.count(dataId)) {
    return false;
  }
  
  const DataStorageConfig& config = dataConfigs[dataId];
  
  // 读取数据
  String data;
  if (!read(dataId, data)) {
    return false;
  }
  
  // 备份到其他存储介质
  DEBUG_PRINTF("开始备份数据: %s\n", dataId.c_str());
  
  // 确定源存储介质
  StorageMediumType sourceMedium = selectStorageMedium(config);
  
  // 确定备份目标存储介质
  std::vector<StorageMediumType> backupTargets;
  
  // 为不同类型的源介质选择合适的备份目标
  switch (sourceMedium) {
    case STORAGE_RAM:
      // 从RAM备份到SPIFFS和TF卡
      backupTargets.push_back(STORAGE_SPIFFS);
      backupTargets.push_back(STORAGE_TFCARD);
      break;
    case STORAGE_SPIFFS:
      // 从SPIFFS备份到TF卡和RAM
      backupTargets.push_back(STORAGE_TFCARD);
      backupTargets.push_back(STORAGE_RAM);
      break;
    case STORAGE_TFCARD:
      // 从TF卡备份到SPIFFS
      backupTargets.push_back(STORAGE_SPIFFS);
      break;
    default:
      // 其他介质备份到SPIFFS
      backupTargets.push_back(STORAGE_SPIFFS);
      break;
  }
  
  // 执行备份到每个目标介质
  bool backupSuccess = false;
  for (StorageMediumType targetMedium : backupTargets) {
    // 跳过源介质
    if (targetMedium == sourceMedium) {
      continue;
    }
    
    // 检查目标介质是否可用
    if (!isStorageMediumAvailable(targetMedium)) {
      DEBUG_PRINTF("备份目标介质不可用: %d\n", targetMedium);
      continue;
    }
    
    // 获取目标存储介质
    auto targetStorage = getStorageMedium(targetMedium);
    if (!targetStorage) {
      DEBUG_PRINTF("无法获取备份目标存储介质: %d\n", targetMedium);
      continue;
    }
    
    // 构建备份文件路径
    String backupKey = "backup_" + dataId;
    
    // 写入备份数据
    if (targetStorage->write(backupKey, data)) {
      DEBUG_PRINTF("数据备份成功到介质 %d: %s\n", targetMedium, backupKey.c_str());
      backupSuccess = true;
    } else {
      DEBUG_PRINTF("数据备份失败到介质 %d: %s\n", targetMedium, backupKey.c_str());
    }
  }
  
  // 如果配置了定期备份，更新最后备份时间
  if (backupSuccess && config.backupEnabled) {
    DataStorageConfig& mutableConfig = dataConfigs[dataId];
    mutableConfig.lastModifiedTime = millis();
  }
  
  return backupSuccess;
}

// 优化存储布局
void StorageManager::optimizeStorageLayout() {
  // 基于访问频率和重要性重新分配数据
  // 这里简化实现
}

// 简单的Run-Length Encoding (RLE)压缩函数
String compressRLE(const String& input) {
  if (input.length() == 0) {
    return "";
  }
  
  String output;
  char currentChar = input[0];
  int count = 1;
  
  for (size_t i = 1; i < input.length(); i++) {
    if (input[i] == currentChar && count < 255) {
      count++;
    } else {
      output += currentChar;
      output += (char)count;
      currentChar = input[i];
      count = 1;
    }
  }
  
  // 添加最后一个字符和计数
  output += currentChar;
  output += (char)count;
  
  return output;
}

// 简单的Run-Length Encoding (RLE)解压缩函数
String decompressRLE(const String& input) {
  if (input.length() == 0) {
    return "";
  }
  
  String output;
  
  for (size_t i = 0; i < input.length(); i += 2) {
    if (i + 1 < input.length()) {
      char currentChar = input[i];
      int count = (unsigned char)input[i + 1];
      
      for (int j = 0; j < count; j++) {
        output += currentChar;
      }
    }
  }
  
  return output;
}

// 压缩数据
bool StorageManager::compressData(const String& dataId) {
  if (!dataConfigs.count(dataId)) {
    return false;
  }
  
  DataStorageConfig& config = dataConfigs[dataId];
  
  // 检查是否可压缩
  if (!config.compressible) {
    return false;
  }
  
  // 读取原始数据
  String originalData;
  if (!read(dataId, originalData)) {
    return false;
  }
  
  // 执行压缩
  DEBUG_PRINTF("开始压缩数据: %s, 原始大小: %u\n", dataId.c_str(), originalData.length());
  
  String compressedData = compressRLE(originalData);
  
  // 检查压缩效果
  if (compressedData.length() >= originalData.length()) {
    DEBUG_PRINTF("压缩效果不明显，跳过压缩: %s\n", dataId.c_str());
    return false;
  }
  
  // 保存压缩数据
  String compressedKey = dataId + "_compressed";
  if (!write(compressedKey, compressedData)) {
    DEBUG_PRINTF("保存压缩数据失败: %s\n", dataId.c_str());
    return false;
  }
  
  // 更新配置，标记为已压缩
  config.metadata["compressed"] = "true";
  config.metadata["originalSize"] = String(originalData.length());
  config.metadata["compressedSize"] = String(compressedData.length());
  config.lastModifiedTime = millis();
  
  DEBUG_PRINTF("数据压缩成功: %s, 压缩大小: %u, 压缩率: %.2f%%\n", 
               dataId.c_str(), compressedData.length(), 
               (1.0 - (float)compressedData.length() / originalData.length()) * 100);
  
  return true;
}

// 解压缩数据
bool StorageManager::decompressData(const String& dataId) {
  if (!dataConfigs.count(dataId)) {
    return false;
  }
  
  const DataStorageConfig& config = dataConfigs[dataId];
  
  // 检查是否已压缩
  if (config.metadata.find("compressed") == config.metadata.end() || 
      config.metadata["compressed"] != "true") {
    return false;
  }
  
  // 读取压缩数据
  String compressedKey = dataId + "_compressed";
  String compressedData;
  if (!read(compressedKey, compressedData)) {
    return false;
  }
  
  // 执行解压缩
  DEBUG_PRINTF("开始解压缩数据: %s\n", dataId.c_str());
  
  String decompressedData = decompressRLE(compressedData);
  
  // 保存解压缩数据
  if (!write(dataId, decompressedData)) {
    DEBUG_PRINTF("保存解压缩数据失败: %s\n", dataId.c_str());
    return false;
  }
  
  DEBUG_PRINTF("数据解压缩成功: %s, 解压缩大小: %u\n", 
               dataId.c_str(), decompressedData.length());
  
  return true;
}

// RAM存储实现
RAMStorage::RAMStorage(unsigned long maxSizeBytes) : maxSize(maxSizeBytes), currentSize(0) {
  // 初始化介质信息
  mediumInfo.type = STORAGE_RAM;
  mediumInfo.name = "RAM Storage";
  mediumInfo.description = "内存存储，速度快但易失";
  mediumInfo.totalSize = maxSize;
  mediumInfo.availableSize = maxSize;
  mediumInfo.usedSize = 0;
  mediumInfo.readSpeed = 10000.0; // 假设10MB/s
  mediumInfo.writeSpeed = 10000.0;
  mediumInfo.available = true;
  mediumInfo.volatileStorage = true;
  mediumInfo.writable = true;
  mediumInfo.lastAccessTime = millis();
}

RAMStorage::~RAMStorage() {
  // 清理数据
  dataMap.clear();
}

bool RAMStorage::read(const String& key, String& value) {
  if (dataMap.count(key)) {
    const std::vector<byte>& bytes = dataMap[key];
    value = String((const char*)bytes.data(), bytes.size());
    mediumInfo.lastAccessTime = millis();
    return true;
  }
  return false;
}

bool RAMStorage::read(const String& key, std::vector<byte>& value) {
  if (dataMap.count(key)) {
    value = dataMap[key];
    mediumInfo.lastAccessTime = millis();
    return true;
  }
  return false;
}

bool RAMStorage::write(const String& key, const String& value) {
  std::vector<byte> bytes(value.begin(), value.end());
  return write(key, bytes);
}

bool RAMStorage::write(const String& key, const std::vector<byte>& value) {
  // 检查内存是否足够
  unsigned long newSize = currentSize;
  if (dataMap.count(key)) {
    newSize -= dataMap[key].size();
  }
  newSize += value.size();
  
  if (newSize > maxSize) {
    return false;
  }
  
  dataMap[key] = value;
  currentSize = newSize;
  mediumInfo.usedSize = currentSize;
  mediumInfo.availableSize = maxSize - currentSize;
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool RAMStorage::remove(const String& key) {
  if (dataMap.count(key)) {
    currentSize -= dataMap[key].size();
    dataMap.erase(key);
    mediumInfo.usedSize = currentSize;
    mediumInfo.availableSize = maxSize - currentSize;
    return true;
  }
  return false;
}

bool RAMStorage::exists(const String& key) {
  return dataMap.count(key) > 0;
}

unsigned long RAMStorage::getSize(const String& key) {
  if (dataMap.count(key)) {
    return dataMap[key].size();
  }
  return 0;
}

std::vector<String> RAMStorage::listKeys(const String& prefix) {
  std::vector<String> keys;
  
  for (const auto& pair : dataMap) {
    const String& key = pair.first;
    if (prefix.isEmpty() || key.startsWith(prefix)) {
      keys.push_back(key);
    }
  }
  
  return keys;
}

bool RAMStorage::clear() {
  dataMap.clear();
  currentSize = 0;
  mediumInfo.usedSize = 0;
  mediumInfo.availableSize = maxSize;
  return true;
}

StorageMediumInfo RAMStorage::getMediumInfo() {
  return mediumInfo;
}

bool RAMStorage::format() {
  return clear();
}

bool RAMStorage::sync() {
  // RAM存储不需要同步
  return true;
}

// SPIFFS存储实现
SPIFFSStorage::SPIFFSStorage(const String& basePath) : basePath(basePath) {
  // 初始化介质信息
  mediumInfo.type = STORAGE_SPIFFS;
  mediumInfo.name = "SPIFFS Storage";
  mediumInfo.description = "SPIFFS文件系统存储";
  mediumInfo.available = false;
  mediumInfo.volatileStorage = false;
  mediumInfo.writable = true;
}

SPIFFSStorage::~SPIFFSStorage() {
  // 析构函数
}

bool SPIFFSStorage::init() {
  if (!SPIFFS.begin()) {
    return false;
  }
  
  // 更新介质信息
  FSInfo info;
  SPIFFS.info(info);
  mediumInfo.totalSize = info.totalBytes;
  mediumInfo.usedSize = info.usedBytes;
  mediumInfo.availableSize = info.totalBytes - info.usedBytes;
  mediumInfo.readSpeed = 500.0; // 假设500KB/s
  mediumInfo.writeSpeed = 200.0; // 假设200KB/s
  mediumInfo.available = true;
  mediumInfo.lastAccessTime = millis();
  
  return true;
}

bool SPIFFSStorage::read(const String& key, String& value) {
  String path = basePath + "/" + key;
  File file = SPIFFS.open(path, "r");
  if (!file) {
    return false;
  }
  
  value = file.readString();
  file.close();
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool SPIFFSStorage::read(const String& key, std::vector<byte>& value) {
  String path = basePath + "/" + key;
  File file = SPIFFS.open(path, "r");
  if (!file) {
    return false;
  }
  
  value.resize(file.size());
  file.read(value.data(), value.size());
  file.close();
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool SPIFFSStorage::write(const String& key, const String& value) {
  String path = basePath + "/" + key;
  File file = SPIFFS.open(path, "w");
  if (!file) {
    return false;
  }
  
  file.print(value);
  file.close();
  
  // 更新介质信息
  FSInfo info;
  SPIFFS.info(info);
  mediumInfo.usedSize = info.usedBytes;
  mediumInfo.availableSize = info.totalBytes - info.usedBytes;
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool SPIFFSStorage::write(const String& key, const std::vector<byte>& value) {
  String path = basePath + "/" + key;
  File file = SPIFFS.open(path, "w");
  if (!file) {
    return false;
  }
  
  file.write(value.data(), value.size());
  file.close();
  
  // 更新介质信息
  FSInfo info;
  SPIFFS.info(info);
  mediumInfo.usedSize = info.usedBytes;
  mediumInfo.availableSize = info.totalBytes - info.usedBytes;
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool SPIFFSStorage::remove(const String& key) {
  String path = basePath + "/" + key;
  bool success = SPIFFS.remove(path);
  
  if (success) {
    // 更新介质信息
    FSInfo info;
    SPIFFS.info(info);
    mediumInfo.usedSize = info.usedBytes;
    mediumInfo.availableSize = info.totalBytes - info.usedBytes;
  }
  
  return success;
}

bool SPIFFSStorage::exists(const String& key) {
  String path = basePath + "/" + key;
  return SPIFFS.exists(path);
}

unsigned long SPIFFSStorage::getSize(const String& key) {
  String path = basePath + "/" + key;
  File file = SPIFFS.open(path, "r");
  if (!file) {
    return 0;
  }
  
  unsigned long size = file.size();
  file.close();
  return size;
}

std::vector<String> SPIFFSStorage::listKeys(const String& prefix) {
  std::vector<String> keys;
  String searchPath = basePath + "/" + prefix;
  
  File root = SPIFFS.open(searchPath);
  if (!root) {
    return keys;
  }
  
  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    // 移除basePath前缀
    if (fileName.startsWith(basePath + "/")) {
      fileName = fileName.substring((basePath + "/").length());
    }
    keys.push_back(fileName);
    file = root.openNextFile();
  }
  
  root.close();
  return keys;
}

bool SPIFFSStorage::clear() {
  // 清空SPIFFS目录
  String path = basePath;
  File root = SPIFFS.open(path);
  if (!root) {
    return false;
  }
  
  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    SPIFFS.remove(fileName);
    file = root.openNextFile();
  }
  
  root.close();
  
  // 更新介质信息
  FSInfo info;
  SPIFFS.info(info);
  mediumInfo.usedSize = info.usedBytes;
  mediumInfo.availableSize = info.totalBytes - info.usedBytes;
  return true;
}

StorageMediumInfo SPIFFSStorage::getMediumInfo() {
  return mediumInfo;
}

bool SPIFFSStorage::format() {
  if (!SPIFFS.format()) {
    return false;
  }
  
  // 重新初始化
  return init();
}

bool SPIFFSStorage::sync() {
  // SPIFFS自动同步
  return true;
}

// TF卡存储实现
TFCardStorage::TFCardStorage(const String& basePath) : basePath(basePath), initialized(false) {
  // 初始化介质信息
  mediumInfo.type = STORAGE_TFCARD;
  mediumInfo.name = "TF Card Storage";
  mediumInfo.description = "TF卡存储";
  mediumInfo.available = false;
  mediumInfo.volatileStorage = false;
  mediumInfo.writable = true;
}

TFCardStorage::~TFCardStorage() {
  // 析构函数
}

bool TFCardStorage::init(int chipSelectPin) {
  if (!SD.begin(chipSelectPin)) {
    return false;
  }
  
  // 更新介质信息
  uint64_t cardSize = SD.cardSize();
  uint64_t usedSize = SD.usedBytes();
  mediumInfo.totalSize = cardSize;
  mediumInfo.usedSize = usedSize;
  mediumInfo.availableSize = cardSize - usedSize;
  mediumInfo.readSpeed = 10000.0; // 假设10MB/s
  mediumInfo.writeSpeed = 5000.0; // 假设5MB/s
  mediumInfo.available = true;
  mediumInfo.lastAccessTime = millis();
  initialized = true;
  
  return true;
}

bool TFCardStorage::read(const String& key, String& value) {
  if (!initialized) {
    return false;
  }
  
  String path = basePath + "/" + key;
  File file = SD.open(path, FILE_READ);
  if (!file) {
    return false;
  }
  
  value = file.readString();
  file.close();
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool TFCardStorage::read(const String& key, std::vector<byte>& value) {
  if (!initialized) {
    return false;
  }
  
  String path = basePath + "/" + key;
  File file = SD.open(path, FILE_READ);
  if (!file) {
    return false;
  }
  
  value.resize(file.size());
  file.read(value.data(), value.size());
  file.close();
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool TFCardStorage::write(const String& key, const String& value) {
  if (!initialized) {
    return false;
  }
  
  String path = basePath + "/" + key;
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }
  
  file.print(value);
  file.close();
  
  // 更新介质信息
  uint64_t usedSize = SD.usedBytes();
  mediumInfo.usedSize = usedSize;
  mediumInfo.availableSize = mediumInfo.totalSize - usedSize;
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool TFCardStorage::write(const String& key, const std::vector<byte>& value) {
  if (!initialized) {
    return false;
  }
  
  String path = basePath + "/" + key;
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    return false;
  }
  
  file.write(value.data(), value.size());
  file.close();
  
  // 更新介质信息
  uint64_t usedSize = SD.usedBytes();
  mediumInfo.usedSize = usedSize;
  mediumInfo.availableSize = mediumInfo.totalSize - usedSize;
  mediumInfo.lastAccessTime = millis();
  return true;
}

bool TFCardStorage::remove(const String& key) {
  if (!initialized) {
    return false;
  }
  
  String path = basePath + "/" + key;
  bool success = SD.remove(path);
  
  if (success) {
    // 更新介质信息
    uint64_t usedSize = SD.usedBytes();
    mediumInfo.usedSize = usedSize;
    mediumInfo.availableSize = mediumInfo.totalSize - usedSize;
  }
  
  return success;
}

bool TFCardStorage::exists(const String& key) {
  if (!initialized) {
    return false;
  }
  
  String path = basePath + "/" + key;
  return SD.exists(path);
}

unsigned long TFCardStorage::getSize(const String& key) {
  if (!initialized) {
    return 0;
  }
  
  String path = basePath + "/" + key;
  File file = SD.open(path, FILE_READ);
  if (!file) {
    return 0;
  }
  
  unsigned long size = file.size();
  file.close();
  return size;
}

std::vector<String> TFCardStorage::listKeys(const String& prefix) {
  std::vector<String> keys;
  if (!initialized) {
    return keys;
  }
  
  String searchPath = basePath + "/" + prefix;
  File root = SD.open(searchPath);
  if (!root) {
    return keys;
  }
  
  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    // 移除basePath前缀
    if (fileName.startsWith(basePath + "/")) {
      fileName = fileName.substring((basePath + "/").length());
    }
    keys.push_back(fileName);
    file = root.openNextFile();
  }
  
  root.close();
  return keys;
}

bool TFCardStorage::clear() {
  if (!initialized) {
    return false;
  }
  
  // 清空TF卡目录
  String path = basePath;
  File root = SD.open(path);
  if (!root) {
    return false;
  }
  
  File file = root.openNextFile();
  while (file) {
    String fileName = file.name();
    SD.remove(fileName);
    file = root.openNextFile();
  }
  
  root.close();
  
  // 更新介质信息
  uint64_t usedSize = SD.usedBytes();
  mediumInfo.usedSize = usedSize;
  mediumInfo.availableSize = mediumInfo.totalSize - usedSize;
  return true;
}

StorageMediumInfo TFCardStorage::getMediumInfo() {
  return mediumInfo;
}

bool TFCardStorage::format() {
  if (!initialized) {
    return false;
  }
  
  // TF卡格式化
  // 注意：实际实现中需要小心使用格式化功能
  return false;
}

bool TFCardStorage::sync() {
  // TF卡自动同步
  return true;
}
