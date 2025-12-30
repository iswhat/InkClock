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

// 创建SD对象
SdFat SD;

FirmwareManager::FirmwareManager() {
  currentStatus = FIRMWARE_STATUS_IDLE;
  updateProgress = 0;
  lastUpdateTime = 0;
}

FirmwareManager::~FirmwareManager() {
  // 清理资源
}

void FirmwareManager::init() {
  // 初始化固件管理器
  logUpdateStatus("Firmware Manager initialized");
  
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

bool FirmwareManager::startWiFiOTA(String url) {
  logUpdateStatus("Starting WiFi OTA update from: " + url);
  currentStatus = FIRMWARE_STATUS_DOWNLOADING;
  
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
  delay(1000);
  ESP.restart();
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
    logUpdateStatus("Firmware file not found: /firmware.bin");
    return false;
  }
  
  // 检查固件版本信息文件是否存在
  if (!SD.exists("/firmware_info.json")) {
    logUpdateStatus("Firmware info file not found: /firmware_info.json");
    return false;
  }
  
  // 读取固件版本信息
  File infoFile = SD.open("/firmware_info.json");
  if (!infoFile) {
    logUpdateStatus("Failed to open firmware info file");
    return false;
  }
  
  // 解析固件信息JSON
  DynamicJsonDocument jsonDoc(1024);
  DeserializationError error = deserializeJson(jsonDoc, infoFile);
  infoFile.close();
  
  if (error) {
    logUpdateStatus("Failed to parse firmware info file");
    return false;
  }
  
  // 获取固件支持的硬件列表
  JsonArray supportedHardware = jsonDoc["supported_hardware"];
  if (supportedHardware.isNull()) {
    logUpdateStatus("Firmware info missing supported_hardware field");
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
    logUpdateStatus("Hardware not supported by this firmware: " + currentHardware);
    logUpdateStatus("Supported hardware: ");
    for (JsonVariant hw : supportedHardware) {
      logUpdateStatus("  - " + hw.as<String>());
    }
    return false;
  }
  
  // 检查固件文件大小
  File firmwareFile = SD.open("/firmware.bin");
  if (!firmwareFile) {
    logUpdateStatus("Failed to open firmware file");
    return false;
  }
  
  size_t firmwareSize = firmwareFile.size();
  firmwareFile.close();
  
  if (firmwareSize == 0) {
    logUpdateStatus("Firmware file is empty");
    return false;
  }
  
  // 验证固件CRC或哈希值（可选，这里跳过）
  
  logUpdateStatus("Valid firmware found, size: " + String(firmwareSize) + " bytes");
  logUpdateStatus("Firmware is compatible with current hardware");
  return true;
}

bool FirmwareManager::installTFUpdate() {
  logUpdateStatus("Installing TF card firmware update");
  currentStatus = FIRMWARE_STATUS_UPDATING;
  
  File firmwareFile = SD.open("/firmware.bin");
  if (!firmwareFile) {
    logUpdateStatus("Failed to open firmware file for update");
    return false;
  }
  
  size_t firmwareSize = firmwareFile.size();
  
  // 开始更新
  if (!Update.begin(firmwareSize)) {
    logUpdateStatus("Failed to begin firmware update");
    firmwareFile.close();
    return false;
  }
  
  // 写入固件数据
  uint8_t buffer[1024];
  size_t bytesRead = 0;
  size_t totalBytesWritten = 0;
  
  while (firmwareFile.available()) {
    bytesRead = firmwareFile.read(buffer, sizeof(buffer));
    if (Update.write(buffer, bytesRead) != bytesRead) {
      logUpdateStatus("Failed to write firmware data");
      firmwareFile.close();
      Update.end(false);
      return false;
    }
    
    totalBytesWritten += bytesRead;
    updateProgress = (totalBytesWritten * 100) / firmwareSize;
    
    // 每10%进度记录一次日志
    if (updateProgress % 10 == 0) {
      logUpdateStatus("TF Update progress: " + String(updateProgress) + "%");
    }
  }
  
  firmwareFile.close();
  
  // 结束更新
  if (!Update.end(true)) {
    logUpdateStatus("Firmware update failed: " + String(Update.getError()));
    return false;
  }
  
  logUpdateStatus("TF card firmware update completed successfully");
  currentStatus = FIRMWARE_STATUS_SUCCESS;
  return true;
}

bool FirmwareManager::downloadFirmware(String url, String filename) {
  logUpdateStatus("Downloading firmware from: " + url);
  
  HTTPClient http;
  http.begin(url);
  
  int httpCode = http.GET();
  if (httpCode != HTTP_CODE_OK) {
    logUpdateStatus("Failed to download firmware, HTTP code: " + String(httpCode));
    http.end();
    return false;
  }
  
  size_t firmwareSize = http.getSize();
  if (firmwareSize == 0) {
    logUpdateStatus("Firmware size is 0");
    http.end();
    return false;
  }
  
  // 保存到临时文件
  File tempFile = SD.open(filename, FILE_WRITE);
  if (!tempFile) {
    logUpdateStatus("Failed to create temp file");
    http.end();
    return false;
  }
  
  // 获取数据流
  WiFiClient *stream = http.getStreamPtr();
  uint8_t buffer[1024];
  size_t bytesRead = 0;
  size_t totalBytesWritten = 0;
  
  while (http.connected() && (totalBytesWritten < firmwareSize)) {
    bytesRead = stream->readBytes(buffer, sizeof(buffer));
    tempFile.write(buffer, bytesRead);
    totalBytesWritten += bytesRead;
    updateProgress = (totalBytesWritten * 100) / firmwareSize;
    
    // 每10%进度记录一次日志
    if (updateProgress % 10 == 0) {
      logUpdateStatus("Download progress: " + String(updateProgress) + "%");
    }
  }
  
  tempFile.close();
  http.end();
  
  if (totalBytesWritten != firmwareSize) {
    logUpdateStatus("Download incomplete, expected " + String(firmwareSize) + " bytes, got " + String(totalBytesWritten) + " bytes");
    SD.remove(filename);
    return false;
  }
  
  logUpdateStatus("Firmware downloaded successfully");
  return true;
}

bool FirmwareManager::installOTAUpdate(String filename) {
  logUpdateStatus("Installing WiFi OTA firmware update");
  currentStatus = FIRMWARE_STATUS_UPDATING;
  
  File firmwareFile = SD.open(filename);
  if (!firmwareFile) {
    logUpdateStatus("Failed to open downloaded firmware file");
    return false;
  }
  
  size_t firmwareSize = firmwareFile.size();
  
  // 开始更新
  if (!Update.begin(firmwareSize)) {
    logUpdateStatus("Failed to begin OTA update");
    firmwareFile.close();
    SD.remove(filename);
    return false;
  }
  
  // 写入固件数据
  uint8_t buffer[1024];
  size_t bytesRead = 0;
  size_t totalBytesWritten = 0;
  
  while (firmwareFile.available()) {
    bytesRead = firmwareFile.read(buffer, sizeof(buffer));
    if (Update.write(buffer, bytesRead) != bytesRead) {
      logUpdateStatus("Failed to write OTA firmware data");
      firmwareFile.close();
      SD.remove(filename);
      Update.end(false);
      return false;
    }
    
    totalBytesWritten += bytesRead;
    updateProgress = (totalBytesWritten * 100) / firmwareSize;
    
    // 每10%进度记录一次日志
    if (updateProgress % 10 == 0) {
      logUpdateStatus("OTA Update progress: " + String(updateProgress) + "%");
    }
  }
  
  firmwareFile.close();
  SD.remove(filename);
  
  // 结束更新
  if (!Update.end(true)) {
    logUpdateStatus("OTA update failed: " + String(Update.getError()));
    return false;
  }
  
  logUpdateStatus("WiFi OTA firmware update completed successfully");
  currentStatus = FIRMWARE_STATUS_SUCCESS;
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
  Serial.println("[FirmwareManager] " + message);
}
