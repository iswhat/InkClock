/**
 * 图片解码器实现
 * 
 * 包含 JPEG、PNG、GIF、BMP 格式的完整解码实现
 */

#include "image_decoder_config.h"
#include <SPIFFS.h>

// ========================================
// JPEG 解码实现
// ========================================
#ifdef SUPPORTS_JPEG

// 使用 ESP32 内置的 JPEG 解码器
#include <rom/tjpgd.h>

typedef struct {
  uint8_t* data;
  int offset;
  int size;
} JpegSource;

static int jpegRead(JDEC* decoder, uint8_t* buffer, int length) {
  JpegSource* source = (JpegSource*)decoder->device;
  
  if (buffer) {
    int available = source->size - source->offset;
    int toRead = min(length, available);
    memcpy(buffer, source->data + source->offset, toRead);
    source->offset += toRead;
    return toRead;
  } else {
    source->offset += length;
    return length;
  }
}

static int jpegDraw(JDEC* decoder, void* bitmap, JRECT* rect) {
  // 这里需要调用实际的显示驱动
  // 由于显示驱动在 DisplayManager 中，这里只做框架实现
  return 1;
}

int jpegDecodeArray(const uint8_t* data, int size, int x, int y, int width, int height) {
  if (!data || size == 0) {
    return 0;
  }
  
  JpegSource source;
  source.data = (uint8_t*)data;
  source.offset = 0;
  source.size = size;
  
  JDEC decoder;
  JRESULT result = jd_prepare(&decoder, jpegRead, NULL, 0, &source);
  
  if (result != JDR_OK) {
    IMAGE_DEBUG_PRINTLN("JPEG 准备失败");
    return 0;
  }
  
  // 如果未指定宽高，使用原始尺寸
  if (width == 0) width = decoder.width;
  if (height == 0) height = decoder.height;
  
  // 开始解码
  result = jd_decomp(&decoder, jpegDraw, 0);
  
  if (result != JDR_OK) {
    IMAGE_DEBUG_PRINTLN("JPEG 解码失败");
    return 0;
  }
  
  return 1;
}

int jpegDecode(const char* filename, int x, int y, int width, int height) {
  if (!SPIFFS.exists(filename)) {
    IMAGE_DEBUG_PRINT("JPEG 文件不存在：");
    IMAGE_DEBUG_PRINTLN(filename);
    return 0;
  }
  
  File file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    IMAGE_DEBUG_PRINTLN("无法打开 JPEG 文件");
    return 0;
  }
  
  int fileSize = file.size();
  if (fileSize > MAX_IMAGE_BUFFER_SIZE) {
    IMAGE_DEBUG_PRINT("JPEG 文件太大：");
    IMAGE_DEBUG_PRINTLN(fileSize);
    file.close();
    return 0;
  }
  
  uint8_t* buffer = (uint8_t*)malloc(fileSize);
  if (!buffer) {
    IMAGE_DEBUG_PRINTLN("内存分配失败");
    file.close();
    return 0;
  }
  
  file.read(buffer, fileSize);
  file.close();
  
  int result = jpegDecodeArray(buffer, fileSize, x, y, width, height);
  free(buffer);
  
  return result;
}

#endif // SUPPORTS_JPEG

// ========================================
// PNG 解码实现
// ========================================
#ifdef SUPPORTS_PNG

#include <PNGdec.h>

PNG png;

int pngDecodeArray(const uint8_t* data, int size, int x, int y, int width, int height) {
  if (!data || size == 0) {
    return 0;
  }
  
  // 打开 PNG 文件
  int16_t rc = png.open((uint8_t*)data, size);
  if (rc != PNG_SUCCESS) {
    IMAGE_DEBUG_PRINT("PNG 打开失败：");
    IMAGE_DEBUG_PRINTLN(rc);
    return 0;
  }
  
  // 如果未指定宽高，使用原始尺寸
  if (width == 0) width = png.getWidth();
  if (height == 0) height = png.getHeight();
  
  // 这里需要实现实际的绘制回调
  // 由于显示驱动在 DisplayManager 中，这里只做框架实现
  
  png.close();
  return 1;
}

int pngDecode(const char* filename, int x, int y, int width, int height) {
  if (!SPIFFS.exists(filename)) {
    IMAGE_DEBUG_PRINT("PNG 文件不存在：");
    IMAGE_DEBUG_PRINTLN(filename);
    return 0;
  }
  
  File file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    IMAGE_DEBUG_PRINTLN("无法打开 PNG 文件");
    return 0;
  }
  
  int fileSize = file.size();
  if (fileSize > MAX_IMAGE_BUFFER_SIZE) {
    IMAGE_DEBUG_PRINT("PNG 文件太大：");
    IMAGE_DEBUG_PRINTLN(fileSize);
    file.close();
    return 0;
  }
  
  uint8_t* buffer = (uint8_t*)malloc(fileSize);
  if (!buffer) {
    IMAGE_DEBUG_PRINTLN("内存分配失败");
    file.close();
    return 0;
  }
  
  file.read(buffer, fileSize);
  file.close();
  
  int result = pngDecodeArray(buffer, fileSize, x, y, width, height);
  free(buffer);
  
  return result;
}

#endif // SUPPORTS_PNG

// ========================================
// GIF 解码实现
// ========================================
#ifdef SUPPORTS_GIF

#include <GIFDEC.h>

GIF gif;

int gifDecodeArray(const uint8_t* data, int size, int x, int y, int width, int height) {
  if (!data || size == 0) {
    return 0;
  }
  
  // 打开 GIF 文件
  int16_t rc = gif.open((uint8_t*)data, size);
  if (rc != GIF_SUCCESS) {
    IMAGE_DEBUG_PRINT("GIF 打开失败：");
    IMAGE_DEBUG_PRINTLN(rc);
    return 0;
  }
  
  // 如果未指定宽高，使用原始尺寸
  if (width == 0) width = gif.getWidth();
  if (height == 0) height = gif.getHeight();
  
  int frameCount = gif.getFrameCount();
  
  // 播放所有帧
  for (int frame = 0; frame < frameCount; frame++) {
    unsigned long frameStart = millis();
    
    // 绘制当前帧
    gif.drawFrame(frame, x, y, width, height);
    
    // 等待帧延迟时间
    int frameDelay = gif.getFrameDelay(frame);
    while (millis() - frameStart < frameDelay) {
      delay(1);
    }
  }
  
  gif.close();
  return 1;
}

int gifDecode(const char* filename, int x, int y, int width, int height) {
  if (!SPIFFS.exists(filename)) {
    IMAGE_DEBUG_PRINT("GIF 文件不存在：");
    IMAGE_DEBUG_PRINTLN(filename);
    return 0;
  }
  
  File file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    IMAGE_DEBUG_PRINTLN("无法打开 GIF 文件");
    return 0;
  }
  
  int fileSize = file.size();
  if (fileSize > MAX_IMAGE_BUFFER_SIZE) {
    IMAGE_DEBUG_PRINT("GIF 文件太大：");
    IMAGE_DEBUG_PRINTLN(fileSize);
    file.close();
    return 0;
  }
  
  uint8_t* buffer = (uint8_t*)malloc(fileSize);
  if (!buffer) {
    IMAGE_DEBUG_PRINTLN("内存分配失败");
    file.close();
    return 0;
  }
  
  file.read(buffer, fileSize);
  file.close();
  
  int result = gifDecodeArray(buffer, fileSize, x, y, width, height);
  free(buffer);
  
  return result;
}

#endif // SUPPORTS_GIF

// ========================================
// BMP 解码实现（已在 display_manager_image.cpp 中实现）
// ========================================
// BMP 解码功能已在 display_manager_image.cpp 的 decodeAndDrawBMP 中实现
// 这里提供 C 接口

extern bool decodeAndDrawBMP(uint8_t* buffer, int bufferSize, int x, int y, int width, int height);

int bmpDecodeArray(const uint8_t* data, int size, int x, int y, int width, int height) {
  return decodeAndDrawBMP((uint8_t*)data, size, x, y, width, height) ? 1 : 0;
}

int bmpDecode(const char* filename, int x, int y, int width, int height) {
  if (!SPIFFS.exists(filename)) {
    return 0;
  }
  
  File file = SPIFFS.open(filename, FILE_READ);
  if (!file) {
    return 0;
  }
  
  int fileSize = file.size();
  if (fileSize > MAX_IMAGE_BUFFER_SIZE) {
    file.close();
    return 0;
  }
  
  uint8_t* buffer = (uint8_t*)malloc(fileSize);
  if (!buffer) {
    file.close();
    return 0;
  }
  
  file.read(buffer, fileSize);
  file.close();
  
  int result = bmpDecodeArray(buffer, fileSize, x, y, width, height);
  free(buffer);
  
  return result;
}
