/**
 * 图片解码器配置头文件
 * 
 * 此文件用于配置 ESP32 图片解码功能
 * 包含所有必要的头文件和宏定义
 */

#ifndef IMAGE_DECODER_CONFIG_H
#define IMAGE_DECODER_CONFIG_H

// ========================================
// 包含必要的头文件
// ========================================
#include <Arduino.h>
#include <SPI.h>
#include <SPIFFS.h>
#include <WiFi.h>
#include <HTTPClient.h>

// ========================================
// JPEG 解码器配置
// ========================================
#ifdef USE_JPEG_DECODER
  #include <JPEGDecoder.h>
  #define JPEG_DECODER_ENABLED 1
#else
  // 使用 ESP32 内置的 JPEG 解码功能
  #include <rom/tjpgd.h>
  #define JPEG_DECODER_ENABLED 1
#endif

// ========================================
// PNG 解码器配置
// ========================================
#ifdef USE_PNG_DECODER
  #include <PNGdec.h>
  #define PNG_DECODER_ENABLED 1
#else
  // 使用软件 PNG 解码
  #define PNG_DECODER_ENABLED 0
#endif

// ========================================
// GIF 解码器配置
// ========================================
#ifdef USE_GIF_DECODER
  #include <GIFDEC.h>
  #define GIF_DECODER_ENABLED 1
#else
  // 使用软件 GIF 解码
  #define GIF_DECODER_ENABLED 0
#endif

// ========================================
// 内存配置
// ========================================
// 最大图片缓冲区大小（字节）
#ifndef MAX_IMAGE_BUFFER_SIZE
  #define MAX_IMAGE_BUFFER_SIZE 524288  // 512KB
#endif

// 最大 GIF 帧数
#ifndef MAX_GIF_FRAMES
  #define MAX_GIF_FRAMES 256
#endif

// 是否使用 PSRAM
#ifndef USE_PSRAM
  #define USE_PSRAM 1
#endif

// ========================================
// 调试配置
// ========================================
#ifdef IMAGE_DECODER_DEBUG
  #define IMAGE_DEBUG_PRINT(...) Serial.printf(__VA_ARGS__)
  #define IMAGE_DEBUG_PRINTLN(...) Serial.println(__VA_ARGS__)
#else
  #define IMAGE_DEBUG_PRINT(...)
  #define IMAGE_DEBUG_PRINTLN(...)
#endif

// ========================================
// 辅助宏
// ========================================
// 检查是否支持某种格式
#define SUPPORTS_JPEG (JPEG_DECODER_ENABLED)
#define SUPPORTS_PNG (PNG_DECODER_ENABLED)
#define SUPPORTS_GIF (GIF_DECODER_ENABLED)
#define SUPPORTS_BMP (1)  // BMP 总是支持的

// ========================================
// 函数声明
// ========================================
#ifdef __cplusplus
extern "C" {
#endif

// JPEG 解码函数
int jpegDecode(const char* filename, int x, int y, int width, int height);
int jpegDecodeArray(const uint8_t* data, int size, int x, int y, int width, int height);

// PNG 解码函数
int pngDecode(const char* filename, int x, int y, int width, int height);
int pngDecodeArray(const uint8_t* data, int size, int x, int y, int width, int height);

// GIF 解码函数
int gifDecode(const char* filename, int x, int y, int width, int height);
int gifDecodeArray(const uint8_t* data, int size, int x, int y, int width, int height);

#ifdef __cplusplus
}
#endif

#endif // IMAGE_DECODER_CONFIG_H
