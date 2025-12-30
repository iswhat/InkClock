#include "audio_manager.h"
#include <SPIFFS.h>
#include "coresystem/spiffs_manager.h"

AudioManager::AudioManager() {
  // 初始化音频状态
  state = AUDIO_IDLE;
  
  // 初始化音量
  volume = AUDIO_VOLUME;
  
  // 初始化文件名
  currentFilename = "";
  
  // 初始化录音时间
  recordStartTime = 0;
  
  // 初始化播放进度
  playPosition = 0;
  totalDuration = 0;
  
  // 初始化音频驱动指针
  audioDriver = nullptr;
}

AudioManager::~AudioManager() {
  // 清理资源
  if (audioDriver != nullptr) {
    delete audioDriver;
    audioDriver = nullptr;
  }
}

void AudioManager::init() {
  DEBUG_PRINTLN("初始化音频管理器...");
  
  // 初始化SPIFFS文件系统
  if (!isSPIFFSMounted()) {
    DEBUG_PRINTLN("SPIFFS未挂载，尝试初始化...");
    if (!initSPIFFS()) {
      DEBUG_PRINTLN("SPIFFS初始化失败");
      return;
    }
  }
  DEBUG_PRINTLN("SPIFFS初始化完成");
  
  // 创建音频驱动实例
  audioDriver = createAudioDriver();
  
  // 初始化音频驱动
  if (audioDriver != nullptr) {
    if (audioDriver->init()) {
      DEBUG_PRINTLN("音频驱动初始化成功");
      
      // 设置初始音量
      setVolume(volume);
    } else {
      DEBUG_PRINTLN("音频驱动初始化失败");
      return;
    }
  } else {
    DEBUG_PRINTLN("创建音频驱动失败");
    return;
  }
  
  // 订阅报警事件
  EVENT_SUBSCRIBE(EVENT_ALARM_TRIGGERED, [this](EventType type, std::shared_ptr<EventData> data) {
    if (type == EVENT_ALARM_TRIGGERED) {
      this->playAlarmSound();
    }
  }, "AudioManager");
  
  DEBUG_PRINTLN("音频管理器初始化完成");
}

void AudioManager::update() {
  // 运行音频驱动的loop函数
  if (audioDriver != nullptr) {
    audioDriver->loop();
    
    // 更新播放状态和进度
    if (audioDriver->isPlaying()) {
      if (state != AUDIO_PLAYING) {
        state = AUDIO_PLAYING;
      }
      playPosition = audioDriver->getPlayPosition();
      totalDuration = audioDriver->getTotalDuration();
    } else if (audioDriver->isRecording()) {
      if (state != AUDIO_RECORDING) {
        state = AUDIO_RECORDING;
        recordStartTime = millis();
      }
      // 检查录音时长
      if (millis() - recordStartTime > AUDIO_RECORD_DURATION * 1000) {
        // 录音时长已到，自动停止
        stopRecording();
      }
    } else {
      if (state != AUDIO_IDLE) {
        state = AUDIO_IDLE;
      }
    }
  }
}

void AudioManager::loop() {
  // 定期更新音频处理
  update();
}

bool AudioManager::startRecording(String filename) {
  DEBUG_PRINTLN("开始录音...");
  
  // 检查当前状态
  if (state != AUDIO_IDLE) {
    DEBUG_PRINTLN("当前状态不允许录音");
    return false;
  }
  
  // 检查音频驱动
  if (audioDriver == nullptr) {
    DEBUG_PRINTLN("音频驱动未初始化");
    return false;
  }
  
  // 如果没有提供文件名，生成一个带时间戳的文件名
  if (filename.isEmpty()) {
    time_t now;
    struct tm timeinfo;
    time(&now);
    localtime_r(&now, &timeinfo);
    char timeStr[20];
    strftime(timeStr, sizeof(timeStr), "rec_%Y%m%d_%H%M%S", &timeinfo);
    filename = String(timeStr) + ".wav";
  } else if (!filename.endsWith(".wav")) {
    filename += ".wav";
  }
  
  // 保存文件名
  currentFilename = filename;
  
  // 开始录音
  if (audioDriver->startRecording(filename.c_str())) {
    // 更新状态
    state = AUDIO_RECORDING;
    recordStartTime = millis();
    
    DEBUG_PRINTF("录音开始，文件: %s\n", filename.c_str());
    return true;
  }
  
  DEBUG_PRINTLN("开始录音失败");
  return false;
}

bool AudioManager::stopRecording() {
  DEBUG_PRINTLN("停止录音...");
  
  // 检查当前状态
  if (state != AUDIO_RECORDING) {
    DEBUG_PRINTLN("当前状态不是录音状态");
    return false;
  }
  
  // 检查音频驱动
  if (audioDriver == nullptr) {
    DEBUG_PRINTLN("音频驱动未初始化");
    return false;
  }
  
  // 停止录音
  audioDriver->stopRecording();
  
  // 更新状态
  state = AUDIO_IDLE;
  
  DEBUG_PRINTF("录音停止，文件: %s\n", currentFilename.c_str());
  return true;
}

bool AudioManager::startPlaying(String filename) {
  DEBUG_PRINT("开始播放音频: ");
  DEBUG_PRINTLN(filename);
  
  // 检查当前状态
  if (state != AUDIO_IDLE && state != AUDIO_PAUSED) {
    DEBUG_PRINTLN("当前状态不允许播放");
    return false;
  }
  
  // 检查音频驱动
  if (audioDriver == nullptr) {
    DEBUG_PRINTLN("音频驱动未初始化");
    return false;
  }
  
  // 保存文件名
  currentFilename = filename;
  
  // 开始播放
  if (audioDriver->startPlayback(filename.c_str())) {
    // 更新状态
    state = AUDIO_PLAYING;
    playPosition = 0;
    
    DEBUG_PRINTLN("音频播放开始");
    return true;
  }
  
  DEBUG_PRINTLN("无法播放音频文件");
  return false;
}

bool AudioManager::stopPlaying() {
  DEBUG_PRINTLN("停止播放音频...");
  
  // 检查音频驱动
  if (audioDriver == nullptr) {
    DEBUG_PRINTLN("音频驱动未初始化");
    return false;
  }
  
  // 停止音频播放
  audioDriver->stopPlayback();
  
  // 更新状态
  state = AUDIO_IDLE;
  playPosition = 0;
  totalDuration = 0;
  
  DEBUG_PRINTLN("音频播放停止");
  return true;
}

bool AudioManager::pausePlaying() {
  DEBUG_PRINTLN("暂停播放音频...");
  
  // 检查当前状态
  if (state != AUDIO_PLAYING) {
    DEBUG_PRINTLN("当前状态不是播放状态");
    return false;
  }
  
  // 检查音频驱动
  if (audioDriver == nullptr) {
    DEBUG_PRINTLN("音频驱动未初始化");
    return false;
  }
  
  // 暂停音频播放
  audioDriver->pausePlayback();
  
  // 更新状态
  state = AUDIO_PAUSED;
  
  DEBUG_PRINTLN("音频播放暂停");
  return true;
}

bool AudioManager::resumePlaying() {
  DEBUG_PRINTLN("恢复播放音频...");
  
  // 检查当前状态
  if (state != AUDIO_PAUSED) {
    DEBUG_PRINTLN("当前状态不是暂停状态");
    return false;
  }
  
  // 检查音频驱动
  if (audioDriver == nullptr) {
    DEBUG_PRINTLN("音频驱动未初始化");
    return false;
  }
  
  // 恢复音频播放
  audioDriver->resumePlayback();
  
  // 更新状态
  state = AUDIO_PLAYING;
  
  DEBUG_PRINTLN("音频播放恢复");
  return true;
}

void AudioManager::setVolume(uint8_t volume) {
  // 限制音量范围
  if (volume > 100) {
    volume = 100;
  }
  
  this->volume = volume;
  
  // 应用音量设置
  if (audioDriver != nullptr) {
    audioDriver->setVolume(volume);
  }
  
  DEBUG_PRINT("音量设置为: ");
  DEBUG_PRINTLN(volume);
}

bool AudioManager::playAlarmSound() {
  DEBUG_PRINTLN("播放报警声音...");
  
  // 检查当前状态
  if (state != AUDIO_IDLE) {
    DEBUG_PRINTLN("当前状态不允许播放报警声音");
    return false;
  }
  
  // 检查音频驱动
  if (audioDriver == nullptr) {
    DEBUG_PRINTLN("音频驱动未初始化");
    return false;
  }
  
  // 播放报警声音
  if (audioDriver->startPlayback("alarm.wav")) {
    // 等待播放完成或超时
    unsigned long startTime = millis();
    while (audioDriver->isPlaying() && (millis() - startTime) < 2000) {
      audioDriver->loop();
      delay(100);
    }
    
    // 停止播放
    audioDriver->stopPlayback();
    
    DEBUG_PRINTLN("报警声音播放完成");
    return true;
  } else {
    DEBUG_PRINTLN("无法播放报警声音");
    return false;
  }
}

// 以下方法暂未实现
bool AudioManager::saveRecording(String filename) {
  return false;
}

bool AudioManager::loadAudioFile(String filename) {
  return false;
}

void AudioManager::updatePlayback() {
  // 已通过ESP32-audioI2S库实现
}

void AudioManager::updateRecording() {
  // 录音功能暂未实现
}