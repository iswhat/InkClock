#ifndef FIRMWARE_MANAGER_H
#define FIRMWARE_MANAGER_H

#include <Arduino.h>
#include "coresystem/config.h"

// 固件更新状态枚举
enum FirmwareUpdateStatus {
  FIRMWARE_STATUS_IDLE,
  FIRMWARE_STATUS_CHECKING,
  FIRMWARE_STATUS_DOWNLOADING,
  FIRMWARE_STATUS_UPDATING,
  FIRMWARE_STATUS_SUCCESS,
  FIRMWARE_STATUS_FAILED,
  FIRMWARE_STATUS_INVALID,
  FIRMWARE_STATUS_ROLLING_BACK
};

// 错误代码枚举
enum FirmwareErrorCode {
  ERROR_NONE,
  ERROR_FILE_NOT_FOUND,
  ERROR_INSUFFICIENT_MEMORY,
  ERROR_INSUFFICIENT_POWER,
  ERROR_HASH_MISMATCH,
  ERROR_SIGNATURE_MISMATCH,
  ERROR_UPDATE_FAILED,
  ERROR_PARTITION_SWITCH_FAILED,
  ERROR_CONFIG_BACKUP_FAILED,
  ERROR_CONFIG_RESTORE_FAILED,
  ERROR_WATCHDOG_TIMEOUT,
  ERROR_UNAUTHORIZED,
  ERROR_UNKNOWN
};

// 分区类型枚举
enum PartitionType {
  PARTITION_CURRENT,
  PARTITION_BACKUP
};

// 状态回调函数类型
typedef void (*FirmwareStatusCallback)(FirmwareUpdateStatus status, int progress, FirmwareErrorCode errorCode, const String &message);

class FirmwareManager {
public:
  FirmwareManager();
  ~FirmwareManager();
  
  void init();
  void loop();
  void update();
  
  // 开始检查TF卡更新
  bool checkTFUpdate();
  
  // 开始WiFi OTA更新
  bool startWiFiOTA(String url, const String &authKey = "");
  
  // 获取更新状态
  FirmwareUpdateStatus getStatus() { return currentStatus; }
  
  // 获取更新进度（0-100）
  int getProgress() { return updateProgress; }
  
  // 获取最后错误代码
  FirmwareErrorCode getLastError() { return lastErrorCode; }
  
  // 设置状态回调函数
  void setStatusCallback(FirmwareStatusCallback callback) { statusCallback = callback; }
  
  // 检查硬件并自动适配驱动
  bool autoDetectHardware();
  
  // 重启设备
  void rebootDevice();
  
  // 回滚到上一个固件版本
  bool rollbackFirmware();
  
  // 检查是否需要回滚
  bool checkRollbackNeeded();
  
  // 标记当前固件为有效
  void markFirmwareValid();
  
private:
  // 更新状态
  FirmwareUpdateStatus currentStatus;
  int updateProgress;
  unsigned long lastUpdateTime;
  FirmwareErrorCode lastErrorCode;
  FirmwareStatusCallback statusCallback;
  
  // 授权相关
  String apiKey;
  bool authorized;
  
  // 分区相关
  PartitionType currentPartition;
  PartitionType backupPartition;
  
  // 获取当前激活的分区
  PartitionType getCurrentPartition();
  
  // 切换到指定分区
  bool switchPartition(PartitionType partition);
  
  // 备份当前分区到备份分区
  bool backupCurrentPartition();
  
  // 从备份分区恢复
  bool restoreFromBackup();
  
  // 看门狗相关
  void initWatchdog();
  void resetWatchdog();
  void disableWatchdog();
  
  // 配置备份和恢复相关
  bool backupCriticalConfig();
  bool restoreCriticalConfig();
  bool isConfigBackupValid();
  
  // 电源稳定性检查相关
  bool checkPowerStability();
  float getBatteryVoltage();
  bool isPowerStable();
  
  // 内存管理相关
  size_t getFreeHeap();
  bool checkMemoryAvailable(size_t requiredSize);
  bool optimizeMemoryUsage();
  
  // 授权相关
  void setApiKey(const String &key) { apiKey = key; }
  String getApiKey() { return apiKey; }
  bool verifyAuthorization(const String &providedKey);
  bool isAuthorized() { return authorized; }
  void setAuthorized(bool auth) { authorized = auth; }
  
  // TF卡更新相关
  bool mountTF();
  void unmountTF();
  bool checkTFValidity();
  bool installTFUpdate();
  
  // WiFi OTA更新相关
  bool downloadFirmware(String url, String filename);
  bool installOTAUpdate(String filename);
  
  // 硬件检测相关
  bool detectDisplay();
  bool detectSensor();
  bool detectAudio();
  bool detectTouch();
  bool detectCamera();
  
  // 驱动适配相关
  bool setupDisplayDriver();
  bool setupSensorDriver();
  bool setupAudioDriver();
  
  // 日志记录
  void logUpdateStatus(String message);
  void logUpdateStatus(String message, FirmwareErrorCode errorCode);
  
  // 检测当前硬件型号
  String detectCurrentHardware();
  
  // 计算文件SHA-256哈希值
  String calculateSHA256(File &file);
  
  // 验证固件哈希值
  bool verifyFirmwareHash(File &file, const String &expectedHash);
  
  // 验证固件签名
  bool verifyFirmwareSignature(File &file, const String &signature, const String &publicKey);
  
  // 从固件信息中获取签名和公钥
  bool getFirmwareSignatureInfo(JsonObject &jsonDoc, String &signature, String &publicKey);
};

#endif // FIRMWARE_MANAGER_H