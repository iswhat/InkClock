#ifndef STORAGE_MANAGER_H
#define STORAGE_MANAGER_H

#include <Arduino.h>
#include <string>
#include <map>
#include <vector>
#include <memory>

// 存储介质类型枚举
enum StorageMediumType {
  STORAGE_RAM,           // RAM内存（最快，易失性）
  STORAGE_SPIFFS,         // SPIFFS文件系统（中等速度，非易失性）
  STORAGE_TFCARD,         // TF卡（较慢，大容量，非易失性）
  STORAGE_EEPROM,         // EEPROM（慢，小容量，非易失性）
  STORAGE_CLOUD,          // 云存储（网络依赖，大容量）
  STORAGE_CUSTOM          // 自定义存储介质
};

// 数据重要性级别枚举
enum DataImportanceLevel {
  DATA_LEVEL_TRANSIENT,   // 临时数据（丢失无影响）
  DATA_LEVEL_LOW,         // 低重要性（丢失影响小）
  DATA_LEVEL_MEDIUM,      // 中等重要性（丢失有一定影响）
  DATA_LEVEL_HIGH,        // 高重要性（丢失影响较大）
  DATA_LEVEL_CRITICAL     // 关键数据（绝对不能丢失）
};

// 数据访问频率枚举
enum DataAccessFrequency {
  ACCESS_FREQUENCY_RARE,  // 很少访问（每月少于1次）
  ACCESS_FREQUENCY_LOW,    // 低频率（每周1-3次）
  ACCESS_FREQUENCY_MEDIUM, // 中等频率（每天1-10次）
  ACCESS_FREQUENCY_HIGH,   // 高频率（每小时多次）
  ACCESS_FREQUENCY_REAL_TIME // 实时访问（持续访问）
};

// 存储介质信息结构
typedef struct {
  StorageMediumType type;      // 存储介质类型
  String name;                // 存储介质名称
  String description;         // 存储介质描述
  unsigned long totalSize;    // 总容量（字节）
  unsigned long availableSize; // 可用容量（字节）
  unsigned long usedSize;     // 已用容量（字节）
  float readSpeed;            // 读取速度（KB/s）
  float writeSpeed;           // 写入速度（KB/s）
  bool available;             // 是否可用
  bool volatileStorage;       // 是否为易失性存储
  bool writable;              // 是否可写
  unsigned long lastAccessTime; // 最后访问时间
  std::map<String, String> properties; // 存储介质属性
} StorageMediumInfo;

// 数据存储配置结构
typedef struct {
  String dataId;              // 数据ID
  String dataName;            // 数据名称
  String description;         // 数据描述
  DataImportanceLevel importance; // 数据重要性
  DataAccessFrequency frequency; // 访问频率
  unsigned long dataSize;     // 数据大小（字节）
  unsigned long maxSize;      // 最大数据大小（字节）
  unsigned long lifespan;     // 数据生命周期（毫秒）
  bool compressible;          // 是否可压缩
  bool encrypted;             // 是否加密
  bool backupEnabled;         // 是否启用备份
  unsigned long backupInterval; // 备份间隔（毫秒）
  StorageMediumType preferredMedium; // 首选存储介质
  std::vector<StorageMediumType> fallbackMedia; //  fallback存储介质
  unsigned long lastModifiedTime; // 最后修改时间
  unsigned long lastAccessTime; // 最后访问时间
  std::map<String, String> metadata; // 数据元数据
} DataStorageConfig;

// 存储操作接口
class IStorageOperation {
public:
  virtual ~IStorageOperation() {}
  
  // 读取数据
  virtual bool read(const String& key, String& value) = 0;
  virtual bool read(const String& key, std::vector<byte>& value) = 0;
  
  // 写入数据
  virtual bool write(const String& key, const String& value) = 0;
  virtual bool write(const String& key, const std::vector<byte>& value) = 0;
  
  // 删除数据
  virtual bool remove(const String& key) = 0;
  
  // 检查数据是否存在
  virtual bool exists(const String& key) = 0;
  
  // 获取数据大小
  virtual unsigned long getSize(const String& key) = 0;
  
  // 列出所有键
  virtual std::vector<String> listKeys(const String& prefix = "") = 0;
  
  // 清空所有数据
  virtual bool clear() = 0;
  
  // 获取存储介质信息
  virtual StorageMediumInfo getMediumInfo() = 0;
  
  // 格式化存储介质
  virtual bool format() = 0;
  
  // 同步数据到物理存储
  virtual bool sync() = 0;
};

// RAM存储实现
class RAMStorage : public IStorageOperation {
private:
  std::map<String, std::vector<byte>> dataMap;
  StorageMediumInfo mediumInfo;
  unsigned long maxSize;
  unsigned long currentSize;
  
public:
  RAMStorage(unsigned long maxSizeBytes = 1024 * 1024); // 默认1MB
  ~RAMStorage();
  
  bool read(const String& key, String& value) override;
  bool read(const String& key, std::vector<byte>& value) override;
  bool write(const String& key, const String& value) override;
  bool write(const String& key, const std::vector<byte>& value) override;
  bool remove(const String& key) override;
  bool exists(const String& key) override;
  unsigned long getSize(const String& key) override;
  std::vector<String> listKeys(const String& prefix = "") override;
  bool clear() override;
  StorageMediumInfo getMediumInfo() override;
  bool format() override;
  bool sync() override;
};

// SPIFFS存储实现
class SPIFFSStorage : public IStorageOperation {
private:
  StorageMediumInfo mediumInfo;
  String basePath;
  
public:
  SPIFFSStorage(const String& basePath = "/spiffs");
  ~SPIFFSStorage();
  
  bool init();
  bool read(const String& key, String& value) override;
  bool read(const String& key, std::vector<byte>& value) override;
  bool write(const String& key, const String& value) override;
  bool write(const String& key, const std::vector<byte>& value) override;
  bool remove(const String& key) override;
  bool exists(const String& key) override;
  unsigned long getSize(const String& key) override;
  std::vector<String> listKeys(const String& prefix = "") override;
  bool clear() override;
  StorageMediumInfo getMediumInfo() override;
  bool format() override;
  bool sync() override;
};

// TF卡存储实现
class TFCardStorage : public IStorageOperation {
private:
  StorageMediumInfo mediumInfo;
  String basePath;
  bool initialized;
  
public:
  TFCardStorage(const String& basePath = "/sd");
  ~TFCardStorage();
  
  bool init(int chipSelectPin = 4);
  bool read(const String& key, String& value) override;
  bool read(const String& key, std::vector<byte>& value) override;
  bool write(const String& key, const String& value) override;
  bool write(const String& key, const std::vector<byte>& value) override;
  bool remove(const String& key) override;
  bool exists(const String& key) override;
  unsigned long getSize(const String& key) override;
  std::vector<String> listKeys(const String& prefix = "") override;
  bool clear() override;
  StorageMediumInfo getMediumInfo() override;
  bool format() override;
  bool sync() override;
};

// 存储管理器类
class StorageManager {
private:
  static StorageManager* instance;
  
  // 存储介质映射
  std::map<StorageMediumType, std::shared_ptr<IStorageOperation>> storageMedia;
  
  // 数据存储配置映射
  std::map<String, DataStorageConfig> dataConfigs;
  
  // 存储使用统计
  std::map<StorageMediumType, unsigned long> storageUsage;
  
  // 初始化状态
  bool initialized;
  
  // 最后清理时间
  unsigned long lastCleanupTime;
  
  // 私有构造函数
  StorageManager();
  
  // 私有方法：初始化默认存储介质
  void initDefaultStorageMedia();
  
  // 私有方法：根据数据特性选择存储介质
  StorageMediumType selectStorageMedium(const DataStorageConfig& config);
  
  // 私有方法：获取存储介质
  std::shared_ptr<IStorageOperation> getStorageMedium(StorageMediumType type);
  
  // 私有方法：验证存储介质可用性
  bool isStorageMediumAvailable(StorageMediumType type);
  
  // 私有方法：计算数据存储成本
  float calculateStorageCost(StorageMediumType type, const DataStorageConfig& config);
  
  // 私有方法：清理过期数据
  void cleanupExpiredData();
  
  // 私有方法：执行数据备份
  bool backupData(const String& dataId);
  
  // 私有方法：优化存储布局
  void optimizeStorageLayout();
  
  // 私有方法：压缩数据
  bool compressData(const String& dataId);
  
  // 私有方法：解压缩数据
  bool decompressData(const String& dataId);
  
public:
  // 获取单例实例
  static StorageManager* getInstance();
  
  // 初始化
  bool init();
  
  // 注册存储介质
  bool registerStorageMedium(StorageMediumType type, std::shared_ptr<IStorageOperation> storage);
  
  // 注册数据存储配置
  bool registerDataConfig(const DataStorageConfig& config);
  
  // 读取数据
  bool read(const String& dataId, String& value);
  bool read(const String& dataId, std::vector<byte>& value);
  
  // 写入数据
  bool write(const String& dataId, const String& value);
  bool write(const String& dataId, const std::vector<byte>& value);
  
  // 删除数据
  bool remove(const String& dataId);
  
  // 检查数据是否存在
  bool exists(const String& dataId);
  
  // 获取数据大小
  unsigned long getSize(const String& dataId);
  
  // 列出所有数据ID
  std::vector<String> listDataIds(const String& prefix = "");
  
  // 获取数据存储配置
  DataStorageConfig getDataConfig(const String& dataId);
  
  // 更新数据存储配置
  bool updateDataConfig(const String& dataId, const DataStorageConfig& config);
  
  // 获取存储介质信息
  StorageMediumInfo getStorageMediumInfo(StorageMediumType type);
  
  // 获取所有存储介质信息
  std::vector<StorageMediumInfo> getAllStorageMediumInfo();
  
  // 获取存储使用统计
  std::map<StorageMediumType, unsigned long> getStorageUsage();
  
  // 执行存储清理
  bool cleanup();
  
  // 执行存储优化
  bool optimize();
  
  // 导出所有数据
  bool exportAllData(const String& exportPath);
  
  // 导入数据
  bool importData(const String& importPath);
  
  // 设置存储介质优先级
  bool setStorageMediumPriority(const std::vector<StorageMediumType>& priorityList);
  
  // 获取推荐的存储介质
  StorageMediumType getRecommendedStorageMedium(const DataImportanceLevel& importance, const DataAccessFrequency& frequency);
  
  // 验证存储系统健康状态
  bool checkHealth();
  
  // 重置存储管理器
  bool reset();
};

// 存储管理器宏
#define STORAGE_MANAGER StorageManager::getInstance()
#define STORAGE_READ(id, value) STORAGE_MANAGER->read(id, value)
#define STORAGE_WRITE(id, value) STORAGE_MANAGER->write(id, value)
#define STORAGE_EXISTS(id) STORAGE_MANAGER->exists(id)
#define STORAGE_REMOVE(id) STORAGE_MANAGER->remove(id)

#endif // STORAGE_MANAGER_H
