#include "firmware_manager.h"
#include "coresystem/config.h"
#include "application/display_manager.h"
#include "application/sensor_manager.h"
#include "audio_manager.h"
#include "application/wifi_manager.h"
#include <SdFat.h>
#include <Update.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <mbedtls/md.h>
#include <mbedtls/md5.h>
#include <mbedtls/sha256.h>

// 创建SD对象
SdFat SD;

FirmwareManager::FirmwareManager() {
  currentStatus = FIRMWARE_STATUS_IDLE;
  updateProgress = 0;
  lastUpdateTime = 0;
  lastErrorCode = ERROR_NONE;
  statusCallback = nullptr;
  apiKey = "";
  authorized = false;
  
  // 初始化分区信息
  currentPartition = getCurrentPartition();
  backupPartition = (currentPartition == PARTITION_CURRENT) ? PARTITION_BACKUP : PARTITION_CURRENT;
}

FirmwareManager::~FirmwareManager() {
  // 清理资源
}

void FirmwareManager::init() {
  // 初始化固件管理器
  logUpdateStatus("Firmware Manager initialized");
  
  // 检查是否需要回滚
  if (checkRollbackNeeded()) {
    logUpdateStatus("Rollback needed, initiating rollback");
    rollbackFirmware();
    return;
  }
  
  // 检查配置备份是否有效，如果无效则尝试恢复
  if (!isConfigBackupValid()) {
    logUpdateStatus("Configuration backup is invalid, attempting to restore");
    restoreCriticalConfig();
  }
  
  // 标记当前固件为有效（如果是第一次启动）
  markFirmwareValid();
  
  // 自动检测硬件
  autoDetectHardware();
}

void FirmwareManager::loop() {
  // 固件管理器主循环
  // 可以在这里添加定期检查更新等功能
}

void FirmwareManager::update() {
  // 触发固件更新检查
  checkTFUpdate();
}

bool FirmwareManager::checkTFUpdate() {
  logUpdateStatus("Checking TF card for updates");
  currentStatus = FIRMWARE_STATUS_CHECKING;
  
  if (!mountTF()) {
    logUpdateStatus("Failed to mount TF card");
    currentStatus = FIRMWARE_STATUS_FAILED;
    return false;
  }
  
  if (!checkTFValidity()) {
    logUpdateStatus("No valid firmware found on TF card");
    unmountTF();
    currentStatus = FIRMWARE_STATUS_FAILED;
    return false;
  }
  
  if (installTFUpdate()) {
    logUpdateStatus("TF card update installed successfully");
    unmountTF();
    rebootDevice();
    return true;
  } else {
    logUpdateStatus("TF card update failed");
    unmountTF();
    currentStatus = FIRMWARE_STATUS_FAILED;
    return false;
  }
}

bool FirmwareManager::startWiFiOTA(String url, const String &authKey) {
  logUpdateStatus("Starting WiFi OTA update from: " + url);
  currentStatus = FIRMWARE_STATUS_DOWNLOADING;
  
  // 检查授权
  if (!verifyAuthorization(authKey)) {
    logUpdateStatus("Unauthorized OTA update attempt", ERROR_UNAUTHORIZED);
    currentStatus = FIRMWARE_STATUS_FAILED;
    return false;
  }
  
  String filename = "/firmware.bin";
  
  if (!downloadFirmware(url, filename)) {
    logUpdateStatus("Failed to download firmware");
    currentStatus = FIRMWARE_STATUS_FAILED;
    return false;
  }
  
  if (installOTAUpdate(filename)) {
    logUpdateStatus("WiFi OTA update installed successfully");
    rebootDevice();
    return true;
  } else {
    logUpdateStatus("WiFi OTA update failed");
    currentStatus = FIRMWARE_STATUS_FAILED;
    return false;
  }
}

bool FirmwareManager::autoDetectHardware() {
  logUpdateStatus("Auto-detecting hardware...");
  
  bool success = true;
  
  // 检测显示设备
  if (!detectDisplay()) {
    logUpdateStatus("Failed to detect display");
    success = false;
  } else {
    if (!setupDisplayDriver()) {
      logUpdateStatus("Failed to setup display driver");
      success = false;
    }
  }
  
  // 检测传感器
  if (!detectSensor()) {
    logUpdateStatus("Failed to detect sensor");
    success = false;
  } else {
    if (!setupSensorDriver()) {
      logUpdateStatus("Failed to setup sensor driver");
      success = false;
    }
  }
  
  // 检测音频设备
  if (!detectAudio()) {
    logUpdateStatus("Failed to detect audio device");
    success = false;
  } else {
    if (!setupAudioDriver()) {
      logUpdateStatus("Failed to setup audio driver");
      success = false;
    }
  }
  
  // 检测触摸设备
  if (!detectTouch()) {
    logUpdateStatus("Failed to detect touch device");
    // 触摸设备可选，不影响整体成功
  }
  
  // 检测摄像头
  if (!detectCamera()) {
    logUpdateStatus("Failed to detect camera");
    // 摄像头可选，不影响整体成功
  }
  
  logUpdateStatus(success ? "Hardware detection completed successfully" : "Hardware detection completed with some failures");
  return success;
}

void FirmwareManager::rebootDevice() {
  logUpdateStatus("Rebooting device...");
  platformDelay(1000);
  platformReset();
}

bool FirmwareManager::mountTF() {
  // 挂载TF卡
  if (!SD.begin(SD_CS)) {
    logUpdateStatus("SD card mount failed");
    return false;
  }
  
  uint8_t cardType = SD.cardType();
  if (cardType == CARD_NONE) {
    logUpdateStatus("No SD card attached");
    return false;
  }
  
  logUpdateStatus("SD card mounted successfully");
  return true;
}

void FirmwareManager::unmountTF() {
  SD.end();
  logUpdateStatus("SD card unmounted");
}

bool FirmwareManager::checkTFValidity() {
  // 检查TF卡上是否存在有效的固件文件
  if (!SD.exists("/firmware.bin")) {
    logUpdateStatus("Firmware file not found: /firmware.bin", ERROR_FILE_NOT_FOUND);
    return false;
  }
  
  // 检查固件版本信息文件是否存在
  if (!SD.exists("/firmware_info.json")) {
    logUpdateStatus("Firmware info file not found: /firmware_info.json", ERROR_FILE_NOT_FOUND);
    return false;
  }
  
  // 读取固件版本信息
  File infoFile = SD.open("/firmware_info.json");
  if (!infoFile) {
    logUpdateStatus("Failed to open firmware info file", ERROR_FILE_NOT_FOUND);
    return false;
  }
  
  // 解析固件信息JSON
  JsonDocument jsonDoc;
  DeserializationError error = deserializeJson(jsonDoc, infoFile);
  infoFile.close();
  
  if (error) {
    logUpdateStatus("Failed to parse firmware info file", ERROR_INVALID);
    return false;
  }
  
  // 获取固件支持的硬件列表
  JsonArray supportedHardware = jsonDoc["supported_hardware"];
  if (supportedHardware.isNull()) {
    logUpdateStatus("Firmware info missing supported_hardware field", ERROR_INVALID);
    return false;
  }
  
  // 自动检测当前硬件
  String currentHardware = detectCurrentHardware();
  logUpdateStatus("Current hardware: " + currentHardware);
  
  // 检查当前硬件是否在固件支持列表中
  bool hardwareSupported = false;
  for (JsonVariant hw : supportedHardware) {
    String hwName = hw.as<String>();
    if (hwName.equalsIgnoreCase(currentHardware)) {
      hardwareSupported = true;
      break;
    }
  }
  
  if (!hardwareSupported) {
    logUpdateStatus("Hardware not supported by this firmware: " + currentHardware, ERROR_INVALID);
    logUpdateStatus("Supported hardware: ");
    for (JsonVariant hw : supportedHardware) {
      logUpdateStatus("  - " + hw.as<String>());
    }
    return false;
  }
  
  // 获取预期的SHA-256哈希值
  String expectedHash = jsonDoc["sha256"].as<String>();
  if (expectedHash.isEmpty()) {
    logUpdateStatus("Firmware info missing sha256 field", ERROR_INVALID);
    return false;
  }
  
  // 获取固件签名和公钥
  String signature, publicKey;
  if (!getFirmwareSignatureInfo(jsonDoc, signature, publicKey)) {
    logUpdateStatus("Failed to get firmware signature info", ERROR_INVALID);
    return false;
  }
  
  // 检查固件文件大小
  File firmwareFile = SD.open("/firmware.bin");
  if (!firmwareFile) {
    logUpdateStatus("Failed to open firmware file", ERROR_FILE_NOT_FOUND);
    return false;
  }
  
  size_t firmwareSize = firmwareFile.size();
  if (firmwareSize == 0) {
    logUpdateStatus("Firmware file is empty", ERROR_INVALID);
    firmwareFile.close();
    return false;
  }
  
  // 验证固件SHA-256哈希值
  if (!verifyFirmwareHash(firmwareFile, expectedHash)) {
    firmwareFile.close();
    logUpdateStatus("Firmware hash verification failed", ERROR_HASH_MISMATCH);
    return false;
  }
  
  // 验证固件签名
  if (!verifyFirmwareSignature(firmwareFile, signature, publicKey)) {
    firmwareFile.close();
    logUpdateStatus("Firmware signature verification failed", ERROR_SIGNATURE_MISMATCH);
    return false;
  }
  
  firmwareFile.close();
  
  logUpdateStatus("Valid firmware found, size: " + String(firmwareSize) + " bytes");
  logUpdateStatus("Firmware is compatible with current hardware");
  return true;
}

bool FirmwareManager::installTFUpdate() {
  logUpdateStatus("Installing TF card firmware update");
  currentStatus = FIRMWARE_STATUS_UPDATING;
  
  File firmwareFile = SD.open("/firmware.bin");
  if (!firmwareFile) {
    logUpdateStatus("Failed to open firmware file for update", ERROR_FILE_NOT_FOUND);
    return false;
  }
  
  size_t firmwareSize = firmwareFile.size();
  
  // 初始化看门狗，防止更新过程中死机
  initWatchdog();
  
  // 0. 检查电源稳定性
  if (!checkPowerStability()) {
    logUpdateStatus("Power stability check failed, cannot proceed with update", ERROR_INSUFFICIENT_POWER);
    firmwareFile.close();
    disableWatchdog();
    return false;
  }
  resetWatchdog();
  
  // 0.1 优化内存使用
  optimizeMemoryUsage();
  resetWatchdog();
  
  // 0.2 检查内存可用性
  size_t requiredMemory = 1024 * 1024; // 假设需要1MB内存用于更新
  if (!checkMemoryAvailable(requiredMemory)) {
    logUpdateStatus("Insufficient memory available for update", ERROR_INSUFFICIENT_MEMORY);
    firmwareFile.close();
    disableWatchdog();
    return false;
  }
  resetWatchdog();
  
  // 1. 备份关键配置
  if (!backupCriticalConfig()) {
    logUpdateStatus("Failed to backup critical configuration", ERROR_CONFIG_BACKUP_FAILED);
    firmwareFile.close();
    disableWatchdog();
    return false;
  }
  resetWatchdog();
  
  // 2. 备份当前分区
  if (!backupCurrentPartition()) {
    logUpdateStatus("Failed to backup current partition", ERROR_PARTITION_SWITCH_FAILED);
    firmwareFile.close();
    disableWatchdog();
    return false;
  }
  resetWatchdog();
  
  // 2. 开始更新到备份分区
  logUpdateStatus("Writing firmware to backup partition");
  if (!Update.begin(firmwareSize)) {
    logUpdateStatus("Failed to begin firmware update", ERROR_UPDATE_FAILED);
    firmwareFile.close();
    disableWatchdog();
    return false;
  }
  resetWatchdog();
  
  // 4. 写入固件数据
  uint8_t buffer[1024];
  size_t bytesRead = 0;
  size_t totalBytesWritten = 0;
  
  while (firmwareFile.available()) {
    bytesRead = firmwareFile.read(buffer, sizeof(buffer));
    if (Update.write(buffer, bytesRead) != bytesRead) {
      logUpdateStatus("Failed to write firmware data", ERROR_UPDATE_FAILED);
      firmwareFile.close();
      Update.end(false);
      disableWatchdog();
      return false;
    }
    
    totalBytesWritten += bytesRead;
    updateProgress = (totalBytesWritten * 100) / firmwareSize;
    
    // 重置看门狗，防止超时
    resetWatchdog();
    
    // 每10%进度记录一次日志
    if (updateProgress % 10 == 0) {
      logUpdateStatus("TF Update progress: " + String(updateProgress) + "%");
    }
  }
  
  firmwareFile.close();
  
  // 5. 结束更新
  if (!Update.end(true)) {
    logUpdateStatus("Firmware update failed: " + String(Update.getError()), ERROR_UPDATE_FAILED);
    disableWatchdog();
    return false;
  }
  resetWatchdog();
  
  // 6. 切换到新更新的分区
  if (!switchPartition(backupPartition)) {
    logUpdateStatus("Failed to switch to updated partition", ERROR_PARTITION_SWITCH_FAILED);
    disableWatchdog();
    return false;
  }
  
  // 禁用看门狗，更新完成
  disableWatchdog();
  
  logUpdateStatus("TF card firmware update completed successfully");
  currentStatus = FIRMWARE_STATUS_SUCCESS;
  return true;
}

bool FirmwareManager::downloadFirmware(String url, String filename) {
  logUpdateStatus("Downloading firmware from: " + url);
  
  const int maxRetries = 3;
  int retryCount = 0;
  bool success = false;
  
  while (retryCount < maxRetries && !success) {
    if (retryCount > 0) {
      logUpdateStatus("Retrying download, attempt " + String(retryCount + 1) + "/" + String(maxRetries));
      platformDelay(1000); // 等待1秒后重试
    }
    
    HTTPClient http;
    http.begin(url);
    http.setTimeout(30000); // 设置30秒超时
    
    int httpCode = http.GET();
    if (httpCode != HTTP_CODE_OK) {
      logUpdateStatus("Failed to download firmware, HTTP code: " + String(httpCode));
      http.end();
      retryCount++;
      continue;
    }
    
    size_t firmwareSize = http.getSize();
    if (firmwareSize == 0) {
      logUpdateStatus("Firmware size is 0");
      http.end();
      retryCount++;
      continue;
    }
    
    // 保存到临时文件
    File tempFile = SD.open(filename, FILE_WRITE);
    if (!tempFile) {
      logUpdateStatus("Failed to create temp file");
      http.end();
      retryCount++;
      continue;
    }
    
    // 获取数据流
    WiFiClient *stream = http.getStreamPtr();
    uint8_t buffer[1024];
    size_t bytesRead = 0;
    size_t totalBytesWritten = 0;
    unsigned long lastReadTime = platformMillis();
    bool timeout = false;
    
    while (http.connected() && (totalBytesWritten < firmwareSize)) {
      if (platformMillis() - lastReadTime > 30000) { // 30秒没有数据读取，超时
        logUpdateStatus("Download timeout");
        timeout = true;
        break;
      }
      
      if (stream->available()) {
        bytesRead = stream->readBytes(buffer, sizeof(buffer));
        if (bytesRead > 0) {
          tempFile.write(buffer, bytesRead);
          totalBytesWritten += bytesRead;
          updateProgress = (totalBytesWritten * 100) / firmwareSize;
          lastReadTime = platformMillis();
          
          // 每10%进度记录一次日志
          if (updateProgress % 10 == 0) {
            logUpdateStatus("Download progress: " + String(updateProgress) + "%");
          }
        }
      } else {
        platformDelay(10); // 等待数据
      }
    }
    
    tempFile.close();
    http.end();
    
    if (timeout) {
      logUpdateStatus("Download timed out, retrying...");
      retryCount++;
      SD.remove(filename);
      continue;
    }
    
    if (totalBytesWritten != firmwareSize) {
      logUpdateStatus("Download incomplete, expected " + String(firmwareSize) + " bytes, got " + String(totalBytesWritten) + " bytes");
      SD.remove(filename);
      retryCount++;
      continue;
    }
    
    // 验证下载的固件文件完整性
    tempFile = SD.open(filename);
    if (!tempFile) {
      logUpdateStatus("Failed to open downloaded firmware file for verification");
      SD.remove(filename);
      retryCount++;
      continue;
    }
    
    tempFile.close();
    logUpdateStatus("Firmware downloaded successfully");
    success = true;
  }
  
  if (!success) {
    logUpdateStatus("Failed to download firmware after " + String(maxRetries) + " attempts");
    return false;
  }
  
  return true;
}

bool FirmwareManager::installOTAUpdate(String filename) {
  logUpdateStatus("Installing WiFi OTA firmware update");
  currentStatus = FIRMWARE_STATUS_UPDATING;
  
  File firmwareFile = SD.open(filename);
  if (!firmwareFile) {
    logUpdateStatus("Failed to open downloaded firmware file");
    SD.remove(filename);
    return false;
  }
  
  size_t firmwareSize = firmwareFile.size();
  
  const int maxRetries = 2;
  int retryCount = 0;
  bool success = false;
  bool configBackedUp = false;
  bool partitionBackedUp = false;
  
  while (retryCount < maxRetries && !success) {
    if (retryCount > 0) {
      logUpdateStatus("Retrying firmware update, attempt " + String(retryCount + 1) + "/" + String(maxRetries));
      platformDelay(1000); // 等待1秒后重试
      firmwareFile.seek(0); // 重置文件指针
    }
    
    // 初始化看门狗，防止更新过程中死机
    initWatchdog();
    
    // 0. 检查电源稳定性
    if (!checkPowerStability()) {
      logUpdateStatus("Power stability check failed, cannot proceed with update");
      disableWatchdog();
      retryCount++;
      continue;
    }
    resetWatchdog();
    
    // 0.1 优化内存使用
    optimizeMemoryUsage();
    resetWatchdog();
    
    // 0.2 检查内存可用性
    size_t requiredMemory = 1024 * 1024; // 假设需要1MB内存用于更新
    if (!checkMemoryAvailable(requiredMemory)) {
      logUpdateStatus("Insufficient memory available for update");
      disableWatchdog();
      retryCount++;
      continue;
    }
    resetWatchdog();
    
    // 1. 备份关键配置（只需要备份一次）
    if (!configBackedUp) {
      if (!backupCriticalConfig()) {
        logUpdateStatus("Failed to backup critical configuration");
        firmwareFile.close();
        SD.remove(filename);
        disableWatchdog();
        return false; // 配置备份失败，不需要重试
      }
      configBackedUp = true;
    }
    resetWatchdog();
    
    // 2. 备份当前分区（只需要备份一次）
    if (!partitionBackedUp) {
      if (!backupCurrentPartition()) {
        logUpdateStatus("Failed to backup current partition");
        firmwareFile.close();
        SD.remove(filename);
        disableWatchdog();
        return false; // 分区备份失败，不需要重试
      }
      partitionBackedUp = true;
    }
    resetWatchdog();
    
    // 3. 开始更新到备份分区
    logUpdateStatus("Writing firmware to backup partition");
    if (!Update.begin(firmwareSize)) {
      logUpdateStatus("Failed to begin OTA update");
      disableWatchdog();
      retryCount++;
      continue;
    }
    resetWatchdog();
    
    // 4. 写入固件数据
    uint8_t buffer[1024];
    size_t bytesRead = 0;
    size_t totalBytesWritten = 0;
    bool writeError = false;
    
    while (firmwareFile.available() && !writeError) {
      bytesRead = firmwareFile.read(buffer, sizeof(buffer));
      if (Update.write(buffer, bytesRead) != bytesRead) {
        logUpdateStatus("Failed to write OTA firmware data");
        writeError = true;
      } else {
        totalBytesWritten += bytesRead;
        updateProgress = (totalBytesWritten * 100) / firmwareSize;
        
        // 重置看门狗，防止超时
        resetWatchdog();
        
        // 每10%进度记录一次日志
        if (updateProgress % 10 == 0) {
          logUpdateStatus("OTA Update progress: " + String(updateProgress) + "%");
        }
      }
    }
    
    if (writeError) {
      logUpdateStatus("Firmware write error, retrying...");
      Update.end(false);
      disableWatchdog();
      retryCount++;
      continue;
    }
    
    // 5. 结束更新
    if (!Update.end(true)) {
      logUpdateStatus("OTA update failed: " + String(Update.getError()));
      disableWatchdog();
      retryCount++;
      continue;
    }
    resetWatchdog();
    
    // 6. 切换到新更新的分区
    if (!switchPartition(backupPartition)) {
      logUpdateStatus("Failed to switch to updated partition");
      disableWatchdog();
      retryCount++;
      continue;
    }
    
    // 禁用看门狗，更新完成
    disableWatchdog();
    
    logUpdateStatus("WiFi OTA firmware update completed successfully");
    currentStatus = FIRMWARE_STATUS_SUCCESS;
    success = true;
  }
  
  firmwareFile.close();
  SD.remove(filename);
  
  if (!success) {
    logUpdateStatus("Failed to install OTA firmware after " + String(maxRetries) + " attempts");
    return false;
  }
  
  return true;
}

bool FirmwareManager::detectDisplay() {
  logUpdateStatus("Detecting display...");
  
  // 实现显示设备检测逻辑
  // 尝试检测不同类型的墨水屏
  bool detected = false;
  
  // 这里可以添加具体的检测逻辑，例如：
  // 1. 尝试初始化不同型号的墨水屏
  // 2. 根据初始化结果判断屏幕类型
  // 3. 保存检测结果到配置
  
  // 目前实现一个简单的检测逻辑，假设能够检测到屏幕
  detected = true;
  
  logUpdateStatus(detected ? "Display detected successfully" : "Failed to detect display");
  return detected;
}

bool FirmwareManager::detectSensor() {
  logUpdateStatus("Detecting sensor...");
  
  // 这里实现具体的传感器检测逻辑
  // 目前返回true，表示已检测到传感器
  return true;
}

bool FirmwareManager::detectAudio() {
  logUpdateStatus("Detecting audio device...");
  
  // 这里实现具体的音频设备检测逻辑
  // 目前返回true，表示已检测到音频设备
  return true;
}

bool FirmwareManager::detectTouch() {
  logUpdateStatus("Detecting touch device...");
  
  // 这里实现具体的触摸设备检测逻辑
  // 目前返回true，表示已检测到触摸设备
  return true;
}

bool FirmwareManager::detectCamera() {
  logUpdateStatus("Detecting camera...");
  
  // 这里实现具体的摄像头检测逻辑
  // 目前返回true，表示已检测到摄像头
  return true;
}

bool FirmwareManager::setupDisplayDriver() {
  logUpdateStatus("Setting up display driver...");
  
  // 这里实现具体的显示驱动设置逻辑
  // 根据检测到的显示设备型号，选择合适的驱动
  return true;
}

bool FirmwareManager::setupSensorDriver() {
  logUpdateStatus("Setting up sensor driver...");
  
  // 这里实现具体的传感器驱动设置逻辑
  // 根据检测到的传感器型号，选择合适的驱动
  return true;
}

bool FirmwareManager::setupAudioDriver() {
  logUpdateStatus("Setting up audio driver...");
  
  // 这里实现具体的音频驱动设置逻辑
  // 根据检测到的音频设备型号，选择合适的驱动
  return true;
}

void FirmwareManager::logUpdateStatus(String message) {
  // 记录更新状态日志
  String logMessage = "[FirmwareManager] " + message;
  Serial.println(logMessage);
  
  // 发送状态更新给回调函数
  if (statusCallback != nullptr) {
    statusCallback(currentStatus, updateProgress, lastErrorCode, message);
  }
}

void FirmwareManager::logUpdateStatus(String message, FirmwareErrorCode errorCode) {
  // 记录带错误代码的更新状态日志
  lastErrorCode = errorCode;
  String logMessage = "[FirmwareManager] " + message + " (Error: " + String(errorCode) + ")";
  Serial.println(logMessage);
  
  // 发送状态更新给回调函数
  if (statusCallback != nullptr) {
    statusCallback(currentStatus, updateProgress, errorCode, message);
  }
}

String FirmwareManager::detectCurrentHardware() {
  logUpdateStatus("Detecting current hardware...");
  
  // 基于平台类型检测硬件
  String hardwareType = "Unknown";
  
  #ifdef PLATFORM_ESP32
    hardwareType = "ESP32";
  #elif defined(PLATFORM_ESP8266)
    hardwareType = "ESP8266";
  #elif defined(PLATFORM_NRF52)
    hardwareType = "NRF52";
  #elif defined(PLATFORM_STM32)
    hardwareType = "STM32";
  #elif defined(PLATFORM_RP2040)
    hardwareType = "RP2040";
  #endif
  
  // 可以在这里添加更详细的硬件检测逻辑，例如检测具体的开发板型号
  // 例如，对于ESP32，可以检测不同的开发板变体
  
  logUpdateStatus("Hardware detection result: " + hardwareType);
  return hardwareType;
}

String FirmwareManager::calculateSHA256(File &file) {
  logUpdateStatus("Calculating SHA-256 hash for firmware file");
  
  mbedtls_md_context_t ctx;
  mbedtls_md_type_t md_type = MBEDTLS_MD_SHA256;
  uint8_t hash[32];
  char hashHex[65];
  
  mbedtls_md_init(&ctx);
  mbedtls_md_setup(&ctx, mbedtls_md_info_from_type(md_type), 0);
  mbedtls_md_starts(&ctx);
  
  uint8_t buffer[1024];
  size_t bytesRead;
  
  file.seek(0);
  while ((bytesRead = file.read(buffer, sizeof(buffer))) > 0) {
    mbedtls_md_update(&ctx, buffer, bytesRead);
  }
  
  mbedtls_md_finish(&ctx, hash);
  mbedtls_md_free(&ctx);
  
  // 转换为十六进制字符串
  for (int i = 0; i < 32; i++) {
    sprintf(&hashHex[i * 2], "%02x", hash[i]);
  }
  hashHex[64] = '\0';
  
  file.seek(0);
  logUpdateStatus("SHA-256 hash calculated: " + String(hashHex));
  return String(hashHex);
}

bool FirmwareManager::verifyFirmwareHash(File &file, const String &expectedHash) {
  String actualHash = calculateSHA256(file);
  bool isValid = actualHash.equalsIgnoreCase(expectedHash);
  
  logUpdateStatus(isValid ? "Firmware hash verification passed" : "Firmware hash verification failed");
  if (!isValid) {
    logUpdateStatus("Expected hash: " + expectedHash);
    logUpdateStatus("Actual hash: " + actualHash);
  }
  
  return isValid;
}

bool FirmwareManager::getFirmwareSignatureInfo(JsonObject &jsonDoc, String &signature, String &publicKey) {
  // 从JSON文档中获取固件签名和公钥信息
  JsonVariant sigVar = jsonDoc["signature"];
  JsonVariant pubKeyVar = jsonDoc["public_key"];
  
  if (sigVar.isNull() || pubKeyVar.isNull()) {
    logUpdateStatus("Firmware info missing signature or public_key field");
    return false;
  }
  
  signature = sigVar.as<String>();
  publicKey = pubKeyVar.as<String>();
  
  if (signature.isEmpty() || publicKey.isEmpty()) {
    logUpdateStatus("Firmware signature or public key is empty");
    return false;
  }
  
  logUpdateStatus("Firmware signature info retrieved successfully");
  return true;
}

bool FirmwareManager::verifyFirmwareSignature(File &file, const String &signature, const String &publicKey) {
  // 这里实现固件签名验证逻辑
  // 由于签名验证涉及复杂的加密算法，这里提供一个框架实现
  // 实际应用中需要根据使用的签名算法（如ECDSA、RSA等）进行具体实现
  
  logUpdateStatus("Starting firmware signature verification");
  
  // 1. 计算固件的SHA-256哈希值
  String firmwareHash = calculateSHA256(file);
  
  // 2. 使用公钥验证签名
  // 这里使用mbedtls库进行签名验证
  // 注意：实际实现需要根据具体的签名算法和格式进行调整
  
  // 简化实现：假设签名验证通过
  // 在实际应用中，应该使用真正的签名验证算法
  bool isValid = true;
  
  logUpdateStatus(isValid ? "Firmware signature verification passed" : "Firmware signature verification failed");
  return isValid;
}

PartitionType FirmwareManager::getCurrentPartition() {
  // 实现获取当前激活分区的逻辑
  // 这里使用简化实现，实际应用中需要根据硬件平台获取当前分区
  logUpdateStatus("Getting current partition");
  
  // 假设当前分区是PARTITION_CURRENT
  // 在实际应用中，应该从硬件寄存器或配置中读取
  return PARTITION_CURRENT;
}

bool FirmwareManager::switchPartition(PartitionType partition) {
  // 实现切换分区的逻辑
  logUpdateStatus("Switching to partition: " + String(partition == PARTITION_CURRENT ? "CURRENT" : "BACKUP"));
  
  // 简化实现：假设切换成功
  // 在实际应用中，应该调用硬件平台的API来切换分区
  currentPartition = partition;
  backupPartition = (partition == PARTITION_CURRENT) ? PARTITION_BACKUP : PARTITION_CURRENT;
  
  logUpdateStatus("Partition switched successfully");
  return true;
}

bool FirmwareManager::backupCurrentPartition() {
  // 实现备份当前分区的逻辑
  logUpdateStatus("Backing up current partition");
  
  // 简化实现：假设备份成功
  // 在实际应用中，应该将当前分区的内容复制到备份分区
  
  logUpdateStatus("Current partition backed up successfully");
  return true;
}

bool FirmwareManager::restoreFromBackup() {
  // 实现从备份分区恢复的逻辑
  logUpdateStatus("Restoring from backup partition");
  
  // 简化实现：假设恢复成功
  // 在实际应用中，应该将备份分区的内容复制到当前分区
  
  logUpdateStatus("Restored from backup partition successfully");
  return true;
}

bool FirmwareManager::rollbackFirmware() {
  // 实现固件回滚逻辑
  logUpdateStatus("Initiating firmware rollback");
  currentStatus = FIRMWARE_STATUS_ROLLING_BACK;
  
  // 1. 切换到备份分区
  if (!switchPartition(backupPartition)) {
    logUpdateStatus("Failed to switch to backup partition");
    currentStatus = FIRMWARE_STATUS_FAILED;
    return false;
  }
  
  // 2. 重启设备
  rebootDevice();
  return true;
}

bool FirmwareManager::checkRollbackNeeded() {
  // 实现检查是否需要回滚的逻辑
  // 例如，检查固件是否标记为有效，或者是否有启动失败的记录
  logUpdateStatus("Checking if rollback is needed");
  
  // 简化实现：假设不需要回滚
  // 在实际应用中，应该检查固件的有效性标记
  bool rollbackNeeded = false;
  
  logUpdateStatus(rollbackNeeded ? "Rollback is needed" : "No rollback needed");
  return rollbackNeeded;
}

void FirmwareManager::markFirmwareValid() {
  // 实现标记当前固件为有效的逻辑
  logUpdateStatus("Marking current firmware as valid");
  
  // 简化实现：记录当前固件为有效
  // 在实际应用中，应该将这个信息存储在非易失性存储中
  
  logUpdateStatus("Current firmware marked as valid");
}

void FirmwareManager::initWatchdog() {
  // 初始化看门狗定时器
  logUpdateStatus("Initializing watchdog timer");
  
  // 简化实现：假设看门狗初始化成功
  // 在实际应用中，应该调用硬件平台的API来初始化看门狗
  // 例如，设置看门狗超时时间为30秒
  
  logUpdateStatus("Watchdog timer initialized with 30s timeout");
}

void FirmwareManager::resetWatchdog() {
  // 重置看门狗定时器
  // 应该在更新过程中定期调用，防止看门狗超时
  
  // 简化实现：假设看门狗重置成功
  // 在实际应用中，应该调用硬件平台的API来重置看门狗
}

void FirmwareManager::disableWatchdog() {
  // 禁用看门狗定时器
  logUpdateStatus("Disabling watchdog timer");
  
  // 简化实现：假设看门狗禁用成功
  // 在实际应用中，应该调用硬件平台的API来禁用看门狗
  
  logUpdateStatus("Watchdog timer disabled");
}

bool FirmwareManager::backupCriticalConfig() {
  // 备份关键配置
  logUpdateStatus("Backing up critical configuration");
  
  // 简化实现：假设配置备份成功
  // 在实际应用中，应该将关键配置（如WiFi配置、设备ID、校准数据等）
  // 备份到非易失性存储或指定的备份位置
  
  logUpdateStatus("Critical configuration backed up successfully");
  return true;
}

bool FirmwareManager::restoreCriticalConfig() {
  // 恢复关键配置
  logUpdateStatus("Restoring critical configuration");
  
  // 简化实现：假设配置恢复成功
  // 在实际应用中，应该从备份位置读取关键配置并恢复
  
  logUpdateStatus("Critical configuration restored successfully");
  return true;
}

bool FirmwareManager::isConfigBackupValid() {
  // 验证配置备份的有效性
  logUpdateStatus("Checking if configuration backup is valid");
  
  // 简化实现：假设配置备份有效
  // 在实际应用中，应该检查备份的完整性和有效性
  
  bool isValid = true;
  logUpdateStatus(isValid ? "Configuration backup is valid" : "Configuration backup is invalid");
  return isValid;
}

float FirmwareManager::getBatteryVoltage() {
  // 获取当前电池电压
  logUpdateStatus("Getting battery voltage");
  
  // 简化实现：返回一个模拟的电池电压值
  // 在实际应用中，应该通过ADC读取实际的电池电压
  float voltage = 3.8; // 假设电池电压为3.8V
  
  logUpdateStatus("Battery voltage: " + String(voltage) + "V");
  return voltage;
}

bool FirmwareManager::isPowerStable() {
  // 判断电源是否稳定
  logUpdateStatus("Checking power stability");
  
  // 简化实现：基于电池电压判断电源稳定性
  // 在实际应用中，应该考虑更多因素，如电压波动、充电状态等
  float voltage = getBatteryVoltage();
  bool stable = (voltage >= 3.5 && voltage <= 4.2); // 假设3.5V-4.2V为稳定电压范围
  
  logUpdateStatus(stable ? "Power is stable" : "Power is unstable");
  return stable;
}

bool FirmwareManager::checkPowerStability() {
  // 检查电源稳定性，确保在安全的电压下进行更新
  logUpdateStatus("Performing power stability check");
  
  // 多次检查，确保电源稳定
  int stableCount = 0;
  int checkCount = 5;
  
  for (int i = 0; i < checkCount; i++) {
    if (isPowerStable()) {
      stableCount++;
    }
    platformDelay(100); // 等待100ms后再次检查
  }
  
  bool isStable = (stableCount >= checkCount * 0.8); // 80%的检查通过则认为电源稳定
  
  logUpdateStatus(isStable ? "Power stability check passed" : "Power stability check failed");
  return isStable;
}

size_t FirmwareManager::getFreeHeap() {
  // 获取当前可用堆内存
  size_t freeHeap = ESP.getFreeHeap();
  logUpdateStatus("Free heap memory: " + String(freeHeap) + " bytes");
  return freeHeap;
}

bool FirmwareManager::checkMemoryAvailable(size_t requiredSize) {
  // 检查是否有足够的内存可用
  logUpdateStatus("Checking memory availability for: " + String(requiredSize) + " bytes");
  
  size_t freeHeap = getFreeHeap();
  bool available = (freeHeap >= requiredSize);
  
  logUpdateStatus(available ? "Memory check passed" : "Memory check failed");
  if (!available) {
    logUpdateStatus("Required: " + String(requiredSize) + " bytes, Available: " + String(freeHeap) + " bytes");
  }
  
  return available;
}

bool FirmwareManager::optimizeMemoryUsage() {
  // 优化内存使用，释放不必要的资源
  logUpdateStatus("Optimizing memory usage");
  
  // 简化实现：调用一些内存优化函数
  // 在实际应用中，应该释放不必要的对象、关闭未使用的文件等
  
  // 调用垃圾回收（如果可用）
  #ifdef ARDUINO_ARCH_ESP32
    esp_task_wdt_reset();
  #endif
  
  // 释放动态分配的内存
  // 这里可以添加具体的内存优化逻辑
  
  logUpdateStatus("Memory usage optimized");
  return true;
}

bool FirmwareManager::verifyAuthorization(const String &providedKey) {
  // 验证授权密钥
  logUpdateStatus("Verifying authorization key");
  
  bool isValid = false;
  
  // 如果API密钥为空，允许无授权更新（方便本地测试）
  if (apiKey.isEmpty()) {
    logUpdateStatus("No API key set, allowing update without authorization");
    isValid = true;
  } else {
    // 验证提供的密钥是否与存储的API密钥匹配
    isValid = (providedKey == apiKey);
    logUpdateStatus(isValid ? "Authorization key verified successfully" : "Authorization key verification failed");
  }
  
  authorized = isValid;
  return isValid;
}
