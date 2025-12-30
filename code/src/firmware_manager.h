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
  FIRMWARE_STATUS_INVALID
};

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
  bool startWiFiOTA(String url);
  
  // 获取更新状态
  FirmwareUpdateStatus getStatus() { return currentStatus; }
  
  // 获取更新进度（0-100）
  int getProgress() { return updateProgress; }
  
  // 检查硬件并自动适配驱动
  bool autoDetectHardware();
  
  // 重启设备
  void rebootDevice();
  
private:
  // 更新状态
  FirmwareUpdateStatus currentStatus;
  int updateProgress;
  unsigned long lastUpdateTime;
  
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
  
  // 检测当前硬件型号
  String detectCurrentHardware();
};

#endif // FIRMWARE_MANAGER_H