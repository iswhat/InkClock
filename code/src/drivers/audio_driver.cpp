#include "audio_driver.h"
#include "coresystem/config.h"
#include "coresystem/spiffs_manager.h"
#include <SPIFFS.h>

// ESP32-audioI2S库将在运行时检查

// 基础音频驱动类，基于ESP32-audioI2S库实现
class BaseAudioDriver : public AudioDriver {
public:
    BaseAudioDriver() {
        volume = AUDIO_VOLUME;
        isPlayingFlag = false;
        isRecordingFlag = false;
        playPosition = 0;
        totalDuration = 0;
    }
    
    virtual ~BaseAudioDriver() {
        deinit();
    }
    
    bool init() override {
        DEBUG_PRINTLN("音频驱动: 音频功能不可用");
        return true;
    }
    
    void deinit() override {
        stopPlayback();
        stopRecording();
    }
    
    void setVolume(uint8_t volume) override {
        this->volume = volume;
    }
    
    bool startPlayback(const char* filename) override {
        DEBUG_PRINTLN("音频驱动: 播放功能不可用");
        return false;
    }
    
    void stopPlayback() override {
        isPlayingFlag = false;
        playPosition = 0;
        totalDuration = 0;
    }
    
    void pausePlayback() override {
        isPlayingFlag = false;
    }
    
    void resumePlayback() override {
        isPlayingFlag = true;
    }
    
    bool startRecording(const char* filename) override {
        DEBUG_PRINTLN("音频驱动: 录音功能不可用");
        return false;
    }
    
    void stopRecording() override {
        isRecordingFlag = false;
    }
    
    bool isPlaying() override {
        return isPlayingFlag;
    }
    
    bool isRecording() override {
        return isRecordingFlag;
    }
    
    uint8_t getVolume() override {
        return volume;
    }
    
    unsigned long getPlayPosition() override {
        return playPosition;
    }
    
    unsigned long getTotalDuration() override {
        return totalDuration;
    }
    
    void loop() override {
    }
    
    bool matchHardware() override {
        DEBUG_PRINTLN("音频驱动: 硬件检测功能不可用");
        return false;
    }
    
    AudioDriverType getType() override {
        return AUDIO_DRIVER_NONE;
    }
    
protected:
    // 音量设置
    uint8_t volume;
    
    // 播放和录音状态
    bool isPlayingFlag;
    bool isRecordingFlag;
    
    // 播放进度
    unsigned long playPosition;
    unsigned long totalDuration;
};

// VS1053B无耳机孔音频驱动
class VS1053BAudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        // 初始化VS1053B音频芯片
        // VS1053B使用SPI接口，需要单独初始化
        DEBUG_PRINTLN("初始化VS1053B音频驱动");
        
        // 调用基础类初始化
        return BaseAudioDriver::init();
    }
    
    void loop() override {
        // VS1053B特定的处理
        BaseAudioDriver::loop();
    }
};

// VS1003B存储版音频驱动
class VS1003BAudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化VS1003B音频驱动");
        return BaseAudioDriver::init();
    }
};

// YX5300音频驱动
class YX5300AudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化YX5300音频驱动");
        return BaseAudioDriver::init();
    }
};

// YX6300音频驱动
class YX6300AudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化YX6300音频驱动");
        return BaseAudioDriver::init();
    }
};

// WT588D音频驱动
class WT588DAudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化WT588D音频驱动");
        return BaseAudioDriver::init();
    }
};

// ISD1820录音模块驱动
class ISD1820AudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化ISD1820音频驱动");
        return BaseAudioDriver::init();
    }
};

// NRF52832音频驱动
class NRF52832AudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化NRF52832音频驱动");
        return BaseAudioDriver::init();
    }
};

// ESP32音频解码模块驱动
class ESP32AudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化ESP32音频驱动");
        return BaseAudioDriver::init();
    }
};

// STM32F103音频驱动
class STM32AudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化STM32音频驱动");
        return BaseAudioDriver::init();
    }
};

// ATmega328音频驱动
class ATmega328AudioDriver : public BaseAudioDriver {
public:
    bool init() override {
        DEBUG_PRINTLN("初始化ATmega328音频驱动");
        return BaseAudioDriver::init();
    }
};

// 根据配置创建音频驱动实例
AudioDriver* createAudioDriver() {
    #if AUDIO_DRIVER_TYPE == AUDIO_DRIVER_VS1053B_NO_HEADPHONE
        return new VS1053BAudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_VS1003B_STORAGE
        return new VS1003BAudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_YX5300
        return new YX5300AudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_YX6300
        return new YX6300AudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_WT588D
        return new WT588DAudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_ISD1820
        return new ISD1820AudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_NRF52832
        return new NRF52832AudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_ESP32_AUDIO
        return new ESP32AudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_STM32_AUDIO
        return new STM32AudioDriver();
    #elif AUDIO_DRIVER_TYPE == AUDIO_DRIVER_ATMEGA328
        return new ATmega328AudioDriver();
    #else
        DEBUG_PRINTLN("使用默认音频驱动");
        return new BaseAudioDriver();
    #endif
}