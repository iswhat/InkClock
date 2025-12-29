#ifndef AUDIO_MANAGER_H
#define AUDIO_MANAGER_H

#include <Arduino.h>
#include "core/config.h"
#include <ESP32-audioI2S.h>
#include "core/event_bus.h"

// 音频状态枚举
enum AudioState {
  AUDIO_IDLE,
  AUDIO_RECORDING,
  AUDIO_PLAYING,
  AUDIO_PAUSED
};

class AudioManager {
public:
  AudioManager();
  ~AudioManager();
  
  void init();
  void update();
  void loop();
  
  // 录音功能
  bool startRecording(String filename = "");
  bool stopRecording();
  bool isRecording() { return state == AUDIO_RECORDING; }
  
  // 播放功能
  bool startPlaying(String filename);
  bool stopPlaying();
  bool pausePlaying();
  bool resumePlaying();
  bool isPlaying() { return state == AUDIO_PLAYING; }
  bool isPaused() { return state == AUDIO_PAUSED; }
  
  // 音量控制
  void setVolume(uint8_t volume);
  uint8_t getVolume() { return volume; }
  
  // 获取音频状态
  AudioState getState() { return state; }
  
  // 获取当前播放的文件名
  String getCurrentFilename() { return currentFilename; }
  
  // 获取当前播放进度
  unsigned long getPlayPosition() { return playPosition; }
  
  // 获取总播放时长
  unsigned long getTotalDuration() { return totalDuration; }
  
  // 播放报警声音
  bool playAlarmSound();
  
private:
  // 音频状态
  AudioState state;
  
  // 音量设置
  uint8_t volume;
  
  // 当前文件名
  String currentFilename;
  
  // 录音开始时间
  unsigned long recordStartTime;
  
  // 播放进度
  unsigned long playPosition;
  unsigned long totalDuration;
  
  // ESP32-audioI2S对象
  Audio audio;
  
  // 私有方法
  void initI2S();
  void deinitI2S();
  bool saveRecording(String filename);
  bool loadAudioFile(String filename);
  void updatePlayback();
  void updateRecording();
  
  // 音频回调函数
  static void audioInfoCallback(const char *info);
  static void audioErrorCallback(const char *info);
  static void audioStatusCallback(void* arg, int code, const char* status);
};

#endif // AUDIO_MANAGER_H