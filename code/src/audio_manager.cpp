#include "audio_manager.h"
#include <SPIFFS.h>
#include "core/spiffs_manager.h"

// 音频回调函数
void AudioManager::audioInfoCallback(const char *info) {
  DEBUG_PRINT("音频信息: ");
  DEBUG_PRINTLN(info);
}

void AudioManager::audioErrorCallback(const char *info) {
  DEBUG_PRINT("音频错误: ");
  DEBUG_PRINTLN(info);
}

void AudioManager::audioStatusCallback(void* arg, int code, const char* status) {
  AudioManager* manager = static_cast<AudioManager*>(arg);
  DEBUG_PRINT("音频状态: ");
  DEBUG_PRINT(code);
  DEBUG_PRINT(" - ");
  DEBUG_PRINTLN(status);
  
  // 更新播放状态
  if (code == 200) {
    // 播放成功开始
    manager->state = AUDIO_PLAYING;
  } else if (code == 300) {
    // 播放完成
    manager->state = AUDIO_IDLE;
  }
}

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
}

AudioManager::~AudioManager() {
  // 清理资源
  audio.stopSong();
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
  
  // 设置音频回调函数
  audio.setInfoCallback(audioInfoCallback);
  audio.setErrorCallback(audioErrorCallback);
  audio.setStatusCallback(audioStatusCallback, this);
  
  // 配置I2S引脚
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT, I2S_DIN);
  
  // 设置初始音量
  setVolume(volume);
  
  // 订阅报警事件
  EVENT_SUBSCRIBE(EVENT_ALARM_TRIGGERED, [this](EventType type, std::shared_ptr<EventData> data) {
    if (type == EVENT_ALARM_TRIGGERED) {
      this->playAlarmSound();
    }
  }, "AudioManager");
  
  DEBUG_PRINTLN("音频管理器初始化完成");
}

void AudioManager::update() {
  // 运行音频库的loop函数
  audio.loop();
  
  // 更新播放进度
  if (state == AUDIO_PLAYING) {
    playPosition = audio.getAudioCurrentTime();
    totalDuration = audio.getAudioTotalTime();
  } else if (state == AUDIO_RECORDING) {
    // 检查录音时长
    if (millis() - recordStartTime > AUDIO_RECORD_DURATION * 1000) {
      // 录音时长已到，自动停止
      stopRecording();
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
  
  // 构建文件路径
  String filepath = "/" + filename;
  
  // 开始录音
  if (!audio.connectToRecord(filepath.c_str(), getSPIFFS(), AUDIO_SAMPLE_RATE)) {
    DEBUG_PRINTLN("开始录音失败");
    return false;
  }
  
  // 更新状态
  state = AUDIO_RECORDING;
  recordStartTime = millis();
  
  DEBUG_PRINTF("录音开始，文件: %s\n", filepath.c_str());
  return true;
}

bool AudioManager::stopRecording() {
  DEBUG_PRINTLN("停止录音...");
  
  // 检查当前状态
  if (state != AUDIO_RECORDING) {
    DEBUG_PRINTLN("当前状态不是录音状态");
    return false;
  }
  
  // 停止录音
  audio.stopRecord();
  
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
  
  // 停止当前播放
  audio.stopSong();
  
  // 保存文件名
  currentFilename = filename;
  
  // 使用ESP32-audioI2S库播放音频文件
  String filepath = "/" + filename;
  if (!audio.connecttoFS(getSPIFFS(), filepath.c_str())) {
    DEBUG_PRINTLN("无法播放音频文件");
    return false;
  }
  
  // 更新状态
  state = AUDIO_PLAYING;
  playPosition = 0;
  
  DEBUG_PRINTLN("音频播放开始");
  return true;
}

bool AudioManager::stopPlaying() {
  DEBUG_PRINTLN("停止播放音频...");
  
  // 停止音频播放
  audio.stopSong();
  
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
  
  // 暂停音频播放
  audio.pauseSong();
  
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
  
  // 恢复音频播放
  audio.resumeSong();
  
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
  audio.setVolume(volume);
  
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
  
  // 停止当前播放
  audio.stopSong();
  
  // 保存当前状态
  state = AUDIO_PLAYING;
  
  // 使用ESP32-audioI2S库播放一个简单的报警音调
  // 这里我们生成一个简单的WAV格式的报警声音
  
  // 生成一个1000Hz的方波报警声音
  // 注意：ESP32-audioI2S库支持播放WAV文件，我们可以生成一个简单的WAV格式的报警声音
  
  // 这里使用一个简单的实现：播放一个已知存在的报警WAV文件
  // 如果文件不存在，会自动失败
  String alarmFilePath = "/alarm.wav";
  
  if (audio.connecttoFS(getSPIFFS(), alarmFilePath.c_str())) {
    DEBUG_PRINTLN("报警声音播放开始");
    // 等待播放完成
    delay(1000); // 假设报警声音时长为1秒
    audio.stopSong();
  } else {
    // 如果无法播放WAV文件，使用蜂鸣器引脚生成简单的蜂鸣音
    DEBUG_PRINTLN("无法播放报警WAV文件，使用蜂鸣音");
    
    // 使用I2S_DOUT引脚作为蜂鸣器引脚
    pinMode(I2S_DOUT, OUTPUT);
    
    // 生成1000Hz的方波，持续500ms
    for (int i = 0; i < 500; i++) {
      digitalWrite(I2S_DOUT, HIGH);
      delayMicroseconds(500); // 1000Hz的半周期
      digitalWrite(I2S_DOUT, LOW);
      delayMicroseconds(500);
    }
    
    // 恢复引脚状态
    pinMode(I2S_DOUT, INPUT);
  }
  
  // 恢复状态
  state = AUDIO_IDLE;
  
  DEBUG_PRINTLN("报警声音播放完成");
  return true;
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