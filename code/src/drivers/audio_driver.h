#ifndef AUDIO_DRIVER_H
#define AUDIO_DRIVER_H

#include <Arduino.h>
#include "coresystem/config.h"

// 使用config.h中定义的AudioDriverType枚举

// 音频驱动抽象类
class AudioDriver {
public:
  virtual ~AudioDriver() = default;
  
  // 初始化音频驱动
  virtual bool init() = 0;
  
  // 反初始化音频驱动
  virtual void deinit() = 0;
  
  // 设置音量
  virtual void setVolume(uint8_t volume) = 0;
  
  // 开始播放
  virtual bool startPlayback(const char* filename) = 0;
  
  // 停止播放
  virtual void stopPlayback() = 0;
  
  // 暂停播放
  virtual void pausePlayback() = 0;
  
  // 恢复播放
  virtual void resumePlayback() = 0;
  
  // 开始录音
  virtual bool startRecording(const char* filename) = 0;
  
  // 停止录音
  virtual void stopRecording() = 0;
  
  // 获取当前播放状态
  virtual bool isPlaying() = 0;
  
  // 获取当前录音状态
  virtual bool isRecording() = 0;
  
  // 获取当前音量
  virtual uint8_t getVolume() = 0;
  
  // 获取当前播放进度（毫秒）
  virtual unsigned long getPlayPosition() = 0;
  
  // 获取总播放时长（毫秒）
  virtual unsigned long getTotalDuration() = 0;
  
  // 音频驱动轮询函数
  virtual void loop() = 0;
  
  // 检测驱动与硬件是否匹配
  virtual bool matchHardware() = 0;
  
  // 获取驱动类型
  virtual AudioDriverType getType() = 0;
};

// 根据配置创建音频驱动实例
AudioDriver* createAudioDriver();

#endif // AUDIO_DRIVER_H