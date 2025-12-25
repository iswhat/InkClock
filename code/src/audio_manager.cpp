#include "audio_manager.h"
#include <driver/i2s.h>
#include <SPIFFS.h>

// I2S配置
#define I2S_NUM I2S_NUM_0
#define I2S_MODE (I2S_MODE_MASTER | I2S_MODE_TX | I2S_MODE_RX)
#define I2S_SAMPLE_BITS 16
#define I2S_CHANNEL_FORMAT I2S_CHANNEL_FMT_ONLY_LEFT // 单声道
#define I2S_COMM_FORMAT I2S_COMM_FORMAT_STAND_I2S

// 录音缓冲区配置
#define RECORD_BUFFER_SIZE 16384
#define RECORD_SAMPLE_RATE AUDIO_SAMPLE_RATE

// 播放缓冲区配置
#define PLAY_BUFFER_SIZE 16384
#define PLAY_SAMPLE_RATE AUDIO_SAMPLE_RATE

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
  deinitI2S();
}

void AudioManager::init() {
  DEBUG_PRINTLN("初始化音频管理器...");
  
  // 初始化SPIFFS文件系统
  if (!SPIFFS.begin(true)) {
    DEBUG_PRINTLN("SPIFFS初始化失败");
    return;
  }
  DEBUG_PRINTLN("SPIFFS初始化完成");
  
  // 初始化I2S
  initI2S();
  
  // 设置初始音量
  setVolume(volume);
  
  DEBUG_PRINTLN("音频管理器初始化完成");
}

void AudioManager::update() {
  // 根据当前状态更新音频处理
  switch (state) {
    case AUDIO_RECORDING:
      updateRecording();
      break;
    case AUDIO_PLAYING:
      updatePlayback();
      break;
    case AUDIO_PAUSED:
      // 暂停状态，无需更新
      break;
    case AUDIO_IDLE:
      // 空闲状态，无需更新
      break;
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
  
  // 生成默认文件名
  if (filename.isEmpty()) {
    char buffer[30];
    snprintf(buffer, sizeof(buffer), "rec_%lu.wav", millis());
    filename = String(buffer);
  }
  
  // 保存文件名
  currentFilename = filename;
  
  // 更新状态
  state = AUDIO_RECORDING;
  recordStartTime = millis();
  
  DEBUG_PRINT("录音文件名: ");
  DEBUG_PRINTLN(filename);
  return true;
}

bool AudioManager::stopRecording() {
  DEBUG_PRINTLN("停止录音...");
  
  // 检查当前状态
  if (state != AUDIO_RECORDING) {
    DEBUG_PRINTLN("当前状态不是录音状态");
    return false;
  }
  
  // 保存录音文件
  bool success = saveRecording(currentFilename);
  
  // 更新状态
  state = AUDIO_IDLE;
  
  if (success) {
    DEBUG_PRINTLN("录音保存成功");
  } else {
    DEBUG_PRINTLN("录音保存失败");
  }
  
  return success;
}

bool AudioManager::startPlaying(String filename) {
  DEBUG_PRINT("开始播放音频: ");
  DEBUG_PRINTLN(filename);
  
  // 检查当前状态
  if (state != AUDIO_IDLE && state != AUDIO_PAUSED) {
    DEBUG_PRINTLN("当前状态不允许播放");
    return false;
  }
  
  // 加载音频文件
  if (!loadAudioFile(filename)) {
    DEBUG_PRINTLN("音频文件加载失败");
    return false;
  }
  
  // 保存文件名
  currentFilename = filename;
  
  // 更新状态
  state = AUDIO_PLAYING;
  playPosition = 0;
  
  DEBUG_PRINTLN("音频播放开始");
  return true;
}

bool AudioManager::stopPlaying() {
  DEBUG_PRINTLN("停止播放音频...");
  
  // 检查当前状态
  if (state != AUDIO_PLAYING && state != AUDIO_PAUSED) {
    DEBUG_PRINTLN("当前状态不是播放状态");
    return false;
  }
  
  // 更新状态
  state = AUDIO_IDLE;
  
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
  
  // 更新状态
  state = AUDIO_PLAYING;
  
  DEBUG_PRINTLN("音频播放恢复");
  return true;
}

void AudioManager::setVolume(uint8_t volume) {
  // 限制音量范围
  if (volume > 10) {
    volume = 10;
  }
  
  this->volume = volume;
  
  // 应用音量设置
  // TODO: 实现音量调节逻辑
  
  DEBUG_PRINT("音量设置为: ");
  DEBUG_PRINTLN(volume);
}

void AudioManager::initI2S() {
  DEBUG_PRINTLN("初始化I2S接口...");
  
  // 配置I2S参数
  i2s_config_t i2s_config = {
    .mode = I2S_MODE,
    .sample_rate = RECORD_SAMPLE_RATE,
    .bits_per_sample = I2S_SAMPLE_BITS,
    .channel_format = I2S_CHANNEL_FORMAT,
    .communication_format = I2S_COMM_FORMAT,
    .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
    .dma_buf_count = 4,
    .dma_buf_len = 512,
    .use_apll = false,
    .tx_desc_auto_clear = true,
    .fixed_mclk = 0
  };
  
  // 安装I2S驱动
  i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
  
  // 配置I2S引脚
  i2s_pin_config_t pin_config = {
    .bck_io_num = I2S_BCLK,
    .ws_io_num = I2S_LRC,
    .data_out_num = I2S_DOUT,
    .data_in_num = I2S_DIN
  };
  
  i2s_set_pin(I2S_NUM, &pin_config);
  
  // 启动I2S
  i2s_start(I2S_NUM);
  
  DEBUG_PRINTLN("I2S接口初始化完成");
}

void AudioManager::deinitI2S() {
  DEBUG_PRINTLN("关闭I2S接口...");
  
  // 停止I2S
  i2s_stop(I2S_NUM);
  
  // 卸载I2S驱动
  i2s_driver_uninstall(I2S_NUM);
  
  DEBUG_PRINTLN("I2S接口已关闭");
}

bool AudioManager::saveRecording(String filename) {
  // 保存录音数据到文件
  // TODO: 实现录音数据保存逻辑
  
  // 这里只是一个示例，实际实现需要处理I2S录音数据
  File file = SPIFFS.open("/" + filename, FILE_WRITE);
  if (!file) {
    DEBUG_PRINTLN("无法创建录音文件");
    return false;
  }
  
  // 写入WAV文件头
  // TODO: 实现WAV文件头写入
  
  // 写入录音数据
  // TODO: 实现录音数据写入
  
  file.close();
  return true;
}

bool AudioManager::loadAudioFile(String filename) {
  // 加载音频文件
  // TODO: 实现音频文件加载逻辑
  
  File file = SPIFFS.open("/" + filename, FILE_READ);
  if (!file) {
    DEBUG_PRINTLN("无法打开音频文件");
    return false;
  }
  
  // 读取WAV文件头
  // TODO: 实现WAV文件头解析
  
  // 计算总播放时长
  totalDuration = file.size() / (RECORD_SAMPLE_RATE * 2); // 假设16位单声道
  
  file.close();
  return true;
}

void AudioManager::updatePlayback() {
  // 更新音频播放
  // TODO: 实现音频播放逻辑
  
  // 检查播放是否完成
  if (playPosition >= totalDuration) {
    // 播放完成
    stopPlaying();
    return;
  }
  
  // 播放音频数据
  // TODO: 实现音频数据播放
  
  // 更新播放进度
  playPosition++;
}

void AudioManager::updateRecording() {
  // 更新音频录音
  // TODO: 实现音频录音逻辑
  
  // 检查录音时长是否超过限制
  if (millis() - recordStartTime > AUDIO_RECORD_DURATION * 1000) {
    // 录音时长超过限制，自动停止
    stopRecording();
    return;
  }
  
  // 录制音频数据
  // TODO: 实现音频数据录制
}