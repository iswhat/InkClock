#include "camera_manager.h"
#include "coresystem/tf_card_manager.h"

// 相机库包含
#if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
#include "esp_camera.h"
#endif

// 摄像头管理类实现
CameraManager::CameraManager() {
  currentStatus = CAMERA_STATUS_IDLE;
  currentQuality = VIDEO_QUALITY_MEDIUM;
  recordingStartTime = 0;
  recordingDuration = 0;
  isRecording = false;
}

CameraManager::~CameraManager() {
  // 清理资源
  deinitCamera();
}

void CameraManager::init() {
  DEBUG_PRINTLN("初始化摄像头管理器...");
  
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    DEBUG_PRINTLN("当前硬件支持摄像头功能");
    
    currentStatus = CAMERA_STATUS_INITIALIZING;
    
    if (initCamera()) {
      currentStatus = CAMERA_STATUS_READY;
      DEBUG_PRINTLN("摄像头管理器初始化完成");
    } else {
      currentStatus = CAMERA_STATUS_ERROR;
      DEBUG_PRINTLN("摄像头初始化失败");
    }
  #else
    DEBUG_PRINTLN("当前硬件不支持摄像头功能");
    currentStatus = CAMERA_STATUS_ERROR;
  #endif
}

void CameraManager::update() {
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    if (currentStatus == CAMERA_STATUS_RECORDING) {
      // 检查录制时长
      if (millis() - recordingStartTime >= recordingDuration) {
        // 录制时间结束，停止录制
        stopRecording();
      }
    }
  #endif
}

void CameraManager::loop() {
  // 定期更新摄像头状态
  if (millis() % 1000 < 10) {
    update();
  }
}

bool CameraManager::initCamera() {
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    DEBUG_PRINTLN("初始化摄像头...");
    
    // 摄像头配置
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_pwdn = -1;
    config.pin_reset = -1;
    config.pin_xclk = 21;
    config.pin_sscb_sda = 26;
    config.pin_sscb_scl = 27;
    
    config.pin_d7 = 35;
    config.pin_d6 = 34;
    config.pin_d5 = 39;
    config.pin_d4 = 36;
    config.pin_d3 = 19;
    config.pin_d2 = 18;
    config.pin_d1 = 5;
    config.pin_d0 = 4;
    config.pin_vsync = 25;
    config.pin_href = 23;
    config.pin_pclk = 22;
    
    // 设置像素格式
    config.pixel_format = PIXFORMAT_JPEG;
    
    // 根据质量设置分辨率
    setVideoQuality(currentQuality);
    
    // 初始化摄像头
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
      DEBUG_PRINTF("摄像头初始化失败: 0x%x\n", err);
      return false;
    }
    
    // 获取摄像头传感器
    sensor_t *s = esp_camera_sensor_get();
    if (s == NULL) {
      DEBUG_PRINTLN("获取摄像头传感器失败");
      return false;
    }
    
    // 设置自动增益和白平衡
    s->set_gain_ctrl(s, 1); // 自动增益开启
    s->set_exposure_ctrl(s, 1); // 自动曝光开启
    s->set_whitebal(s, 1); // 自动白平衡开启
    
    DEBUG_PRINTLN("摄像头初始化成功");
    return true;
  #else
    return false;
  #endif
}

void CameraManager::deinitCamera() {
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    if (currentStatus != CAMERA_STATUS_IDLE) {
      esp_camera_deinit();
      currentStatus = CAMERA_STATUS_IDLE;
      DEBUG_PRINTLN("摄像头已关闭");
    }
  #endif
}

void CameraManager::setVideoQuality(VideoQuality quality) {
  currentQuality = quality;
  
  // 根据质量设置分辨率
  sensor_t *s = esp_camera_sensor_get();
  if (s == NULL) {
    return;
  }
  
  switch (quality) {
    case VIDEO_QUALITY_LOW:
      s->set_framesize(s, FRAMESIZE_QVGA); // 320x240
      break;
    case VIDEO_QUALITY_MEDIUM:
      s->set_framesize(s, FRAMESIZE_VGA); // 640x480
      break;
    case VIDEO_QUALITY_HIGH:
      s->set_framesize(s, FRAMESIZE_SVGA); // 800x600
      break;
    default:
      s->set_framesize(s, FRAMESIZE_VGA); // 默认640x480
      break;
  }
}

bool CameraManager::startRecording(unsigned long duration, VideoQuality quality) {
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    if (currentStatus != CAMERA_STATUS_READY) {
      DEBUG_PRINTLN("摄像头未准备好");
      return false;
    }
    
    if (isRecording) {
      DEBUG_PRINTLN("已经在录制中");
      return false;
    }
    
    // 检查TF卡是否已挂载
    if (!isTFCardMounted()) {
      DEBUG_PRINTLN("TF卡未挂载，无法录制视频");
      return false;
    }
    
    // 设置视频质量
    setVideoQuality(quality);
    
    // 开始录制
    recordingStartTime = millis();
    recordingDuration = duration;
    isRecording = true;
    currentStatus = CAMERA_STATUS_RECORDING;
    
    DEBUG_PRINTF("开始录制视频，时长: %lu ms\n", duration);
    return true;
  #else
    return false;
  #endif
}

void CameraManager::stopRecording() {
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    if (isRecording) {
      isRecording = false;
      currentStatus = CAMERA_STATUS_READY;
      
      DEBUG_PRINTLN("停止录制视频");
    }
  #endif
}

bool CameraManager::takePhoto() {
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    if (currentStatus != CAMERA_STATUS_READY) {
      DEBUG_PRINTLN("摄像头未准备好");
      return false;
    }
    
    // 检查TF卡是否已挂载
    if (!isTFCardMounted()) {
      DEBUG_PRINTLN("TF卡未挂载，无法保存照片");
      return false;
    }
    
    // 拍摄照片
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      DEBUG_PRINTLN("拍摄照片失败");
      return false;
    }
    
    // 生成照片文件名
    char filename[30];
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    strftime(filename, sizeof(filename), "/photo_%Y%m%d_%H%M%S.jpg", &timeinfo);
    
    // 保存照片到TF卡
    File file = getTFCard().open(filename, FILE_WRITE);
    if (!file) {
      DEBUG_PRINTF("创建照片文件失败: %s\n", filename);
      esp_camera_fb_return(fb);
      return false;
    }
    
    // 写入照片数据
    if (file.write(fb->buf, fb->len) != fb->len) {
      DEBUG_PRINTF("写入照片数据失败: %s\n", filename);
      file.close();
      esp_camera_fb_return(fb);
      return false;
    }
    
    file.close();
    
    // 释放帧缓冲区
    esp_camera_fb_return(fb);
    
    DEBUG_PRINTF("拍摄照片成功，文件: %s\n", filename);
    return true;
  #else
    return false;
  #endif
}

bool CameraManager::decodeVideo(const char* filename) {
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    DEBUG_PRINTF("解码视频文件: %s\n", filename);
    
    // 这里需要实现视频解码逻辑
    // 由于ESP32的处理能力有限，实际应用中可能需要使用硬件加速或简化的解码算法
    
    DEBUG_PRINTLN("视频解码完成");
    return true;
  #else
    return false;
  #endif
}

bool CameraManager::playVideoMessage(const char* filename) {
  // 检查当前硬件是否支持摄像头
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    DEBUG_PRINTF("播放视频留言: %s\n", filename);
    
    // 首先解码视频文件
    if (!decodeVideo(filename)) {
      return false;
    }
    
    // 这里需要实现视频播放逻辑
    // 由于墨水屏的刷新率限制，实际应用中可能需要将视频转换为帧序列，然后逐帧显示
    
    DEBUG_PRINTLN("视频留言播放完成");
    return true;
  #else
    return false;
  #endif
}