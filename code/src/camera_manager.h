#ifndef CAMERA_MANAGER_H
#define CAMERA_MANAGER_H

#include <Arduino.h>
#include "core/config.h"

// 摄像头状态
enum CameraStatus {
  CAMERA_STATUS_IDLE,
  CAMERA_STATUS_INITIALIZING,
  CAMERA_STATUS_READY,
  CAMERA_STATUS_RECORDING,
  CAMERA_STATUS_ERROR
};

// 视频质量设置
enum VideoQuality {
  VIDEO_QUALITY_LOW,
  VIDEO_QUALITY_MEDIUM,
  VIDEO_QUALITY_HIGH
};

class CameraManager {
private:
  CameraStatus currentStatus;
  VideoQuality currentQuality;
  unsigned long recordingStartTime;
  unsigned long recordingDuration;
  bool isRecording;
  
  // 私有方法
  bool initCamera();
  void deinitCamera();
  void setVideoQuality(VideoQuality quality);
  
public:
  CameraManager();
  ~CameraManager();
  
  void init();
  void update();
  void loop();
  
  // 获取摄像头状态
  CameraStatus getStatus() { return currentStatus; }
  
  // 开始录制视频
  bool startRecording(unsigned long duration = 10000, VideoQuality quality = VIDEO_QUALITY_MEDIUM);
  
  // 停止录制视频
  void stopRecording();
  
  // 拍摄照片
  bool takePhoto();
  
  // 检查是否正在录制
  bool isRecordingVideo() { return isRecording; }
  
  // 获取录制时长
  unsigned long getRecordingDuration() { return recordingDuration; }
  
  // 解码视频文件
  bool decodeVideo(const char* filename);
  
  // 播放视频留言
  bool playVideoMessage(const char* filename);
};

#endif // CAMERA_MANAGER_H