#ifndef CONFIG_H
#define CONFIG_H

// Platform macros
#define PLATFORM_ESP32 1
#define PLATFORM_ESP8266 0
#define PLATFORM_NRF52 0
#define PLATFORM_STM32 0
#define PLATFORM_RP2040 0

// Audio module macros
#define AUDIO_MODULE_TYPE AUDIO_MODULE_ES8388

// Display macros
#define DISPLAY_TYPE EINK_154_INCH

// Sensor macros
#define ENABLE_DHT22 1
#define ENABLE_SHT30 0
#define ENABLE_BH1750 0
#define ENABLE_HC_SR501 0
#define ENABLE_MQ135 0
#define ENABLE_IR_FLAME 0
#define ENABLE_BME280 0

// Hardware module macros
#define ENABLE_TF_CARD 1

// Feature macros
#define ENABLE_WEBCLIENT 1
#define ENABLE_BLUETOOTH 1
#define ENABLE_TOUCH 0
#define ENABLE_PLUGIN 0
#define ENABLE_IPV6 0
#define ENABLE_FONT 0
#define ENABLE_FIRMWARE 1
#define ENABLE_WIFI 1
#define ENABLE_VOICE_MESSAGE 1
#define ENABLE_TF_CARD_MANAGEMENT 0
#define ENABLE_ALARM_DISPLAY 0
#define ENABLE_TEXT_MESSAGE 1
#define ENABLE_AUDIO 1
#define ENABLE_CAMERA 1
#define ENABLE_VIDEO_MESSAGE 1

#endif // CONFIG_H // CONFIG_H