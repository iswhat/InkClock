#ifndef EINK_DRIVER_H
#define EINK_DRIVER_H

#include "display_driver.h"
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxFonts/GxFonts.h>
#include "../core/config.h"

// 根据显示类型选择合适的墨水屏库
#if DISPLAY_TYPE == EINK_102_INCH
  #include <GxGDEW0102T4/GxGDEW0102T4.h> // 1.02寸单色墨水屏库
#elif DISPLAY_TYPE == EINK_144_INCH
  #include <GxGDEW0144Z07/GxGDEW0144Z07.h> // 1.44寸单色墨水屏库
#elif DISPLAY_TYPE == EINK_154_INCH || DISPLAY_TYPE == ESL_154_INCH_DUAL
  #include <GxGDEW0154M09/GxGDEW0154M09.h> // 1.54寸单色墨水屏库
#elif DISPLAY_TYPE == EINK_213_INCH || DISPLAY_TYPE == ESL_213_INCH_DUAL
  #include <GxGDEW0213Z16/GxGDEW0213Z16.h> // 2.13寸双色墨水屏库
#elif DISPLAY_TYPE == EINK_266_INCH || DISPLAY_TYPE == ESL_266_INCH_DUAL
  #include <GxGDEW0266T90/GxGDEW0266T90.h> // 2.66寸双色墨水屏库
#elif DISPLAY_TYPE == EINK_27_INCH
  #include <GxGDEW027W3/GxGDEW027W3.h> // 2.7寸单色墨水屏库
#elif DISPLAY_TYPE == EINK_29_INCH || DISPLAY_TYPE == ESL_29_INCH_DUAL
  #include <GxGDEW029Z10/GxGDEW029Z10.h> // 2.9寸双色墨水屏库
#elif DISPLAY_TYPE == EINK_312_INCH || DISPLAY_TYPE == ESL_312_INCH_DUAL
  #include <GxGDEW031Z15/GxGDEW031Z15.h> // 3.12寸双色墨水屏库
#elif DISPLAY_TYPE == EINK_37_INCH
  #include <GxGDEW0371W7/GxGDEW0371W7.h> // 3.7寸单色墨水屏库
#elif DISPLAY_TYPE == EINK_42_INCH || DISPLAY_TYPE == ESL_42_INCH_COLOR
  #include <GxGDEW042Z15/GxGDEW042Z15.h> // 4.2寸三色墨水屏库
#elif DISPLAY_TYPE == EINK_437_INCH
  #include <GxGDEW0437Z90/GxGDEW0437Z90.h> // 4.37寸三色墨水屏库
#elif DISPLAY_TYPE == EINK_54_INCH
  #include <GxGDEW054Z01/GxGDEW054Z01.h> // 5.4寸双色墨水屏库
#elif DISPLAY_TYPE == EINK_583_INCH || DISPLAY_TYPE == ESL_583_INCH_COLOR
  #include <GxGDEW0583T7/GxGDEW0583T7.h> // 5.83寸三色墨水屏库
#elif DISPLAY_TYPE == EINK_60_INCH || DISPLAY_TYPE == READER_6_INCH_MONO || DISPLAY_TYPE == READER_6_INCH_COLOR
  #include <GxGDEW060Z10/GxGDEW060Z10.h> // 6.0寸双色墨水屏库
#elif DISPLAY_TYPE == EINK_75_INCH
  #include <GxGDEW075Z09/GxGDEW075Z09.h> // 7.5寸三色墨水屏库
#elif DISPLAY_TYPE == EINK_78_INCH || DISPLAY_TYPE == READER_78_INCH_MONO || DISPLAY_TYPE == READER_78_INCH_COLOR
  #include <GxGDEW078Z21/GxGDEW078Z21.h> // 7.8寸单色墨水屏库
#elif DISPLAY_TYPE == EINK_97_INCH
  #include <GxGDEW097T4/GxGDEW097T4.h> // 9.7寸单色墨水屏库
#elif DISPLAY_TYPE == EINK_103_INCH || DISPLAY_TYPE == READER_103_INCH_MONO || DISPLAY_TYPE == READER_103_INCH_COLOR
  #include <GxGDEW103Z07/GxGDEW103Z07.h> // 10.3寸单色墨水屏库
#elif DISPLAY_TYPE == EINK_1248_INCH
  #include <GxGDEW1248Z21/GxGDEW1248Z21.h> // 12.48寸单色墨水屏库
#endif

// 墨水屏驱动实现类
class EinkDriver : public IDisplayDriver {
public:
  EinkDriver();
  ~EinkDriver() override;
  
  // 初始化驱动
  bool init() override;
  
  // 清除屏幕
  void clear() override;
  
  // 绘制像素点
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  
  // 绘制字符
  void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) override;
  
  // 绘制字符串
  void drawString(int16_t x, int16_t y, const String& text, uint16_t color, uint16_t bg, uint8_t size) override;
  
  // 绘制矩形
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  
  // 填充矩形
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  
  // 更新显示
  void update() override;
  
  // 局部更新
  void update(int16_t x, int16_t y, int16_t w, int16_t h) override;
  
  // 获取屏幕宽度
  int16_t getWidth() const override;
  
  // 获取屏幕高度
  int16_t getHeight() const override;
  
  // 休眠
  void sleep() override;
  
  // 唤醒
  void wakeup() override;
  
  // 获取显示类型
  EinkDisplayType getType() const override;
  
private:
  // 墨水屏IO和显示对象
  GxIO_Class io;
  
  #if DISPLAY_TYPE == EINK_102_INCH
    GxGDEW0102T4_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW0102T4_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW0102T4_HEIGHT;
  #elif DISPLAY_TYPE == EINK_144_INCH
    GxGDEW0144Z07_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW0144Z07_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW0144Z07_HEIGHT;
  #elif DISPLAY_TYPE == EINK_154_INCH || DISPLAY_TYPE == ESL_154_INCH_DUAL
    GxGDEW0154M09_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW0154M09_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW0154M09_HEIGHT;
  #elif DISPLAY_TYPE == EINK_213_INCH || DISPLAY_TYPE == ESL_213_INCH_DUAL
    GxGDEW0213Z16_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW0213Z16_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW0213Z16_HEIGHT;
  #elif DISPLAY_TYPE == EINK_266_INCH || DISPLAY_TYPE == ESL_266_INCH_DUAL
    GxGDEW0266T90_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW0266T90_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW0266T90_HEIGHT;
  #elif DISPLAY_TYPE == EINK_27_INCH
    GxGDEW027W3_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW027W3_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW027W3_HEIGHT;
  #elif DISPLAY_TYPE == EINK_29_INCH || DISPLAY_TYPE == ESL_29_INCH_DUAL
    GxGDEW029Z10_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW029Z10_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW029Z10_HEIGHT;
  #elif DISPLAY_TYPE == EINK_312_INCH || DISPLAY_TYPE == ESL_312_INCH_DUAL
    GxGDEW031Z15_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW031Z15_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW031Z15_HEIGHT;
  #elif DISPLAY_TYPE == EINK_37_INCH
    GxGDEW0371W7_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW0371W7_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW0371W7_HEIGHT;
  #elif DISPLAY_TYPE == EINK_42_INCH || DISPLAY_TYPE == ESL_42_INCH_COLOR
    GxGDEW042Z15_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW042Z15_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW042Z15_HEIGHT;
  #elif DISPLAY_TYPE == EINK_437_INCH
    GxGDEW0437Z90_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW0437Z90_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW0437Z90_HEIGHT;
  #elif DISPLAY_TYPE == EINK_54_INCH
    GxGDEW054Z01_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW054Z01_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW054Z01_HEIGHT;
  #elif DISPLAY_TYPE == EINK_583_INCH || DISPLAY_TYPE == ESL_583_INCH_COLOR
    GxGDEW0583T7_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW0583T7_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW0583T7_HEIGHT;
  #elif DISPLAY_TYPE == EINK_60_INCH || DISPLAY_TYPE == READER_6_INCH_MONO || DISPLAY_TYPE == READER_6_INCH_COLOR
    GxGDEW060Z10_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW060Z10_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW060Z10_HEIGHT;
  #elif DISPLAY_TYPE == EINK_75_INCH
    GxGDEW075Z09_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW075Z09_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW075Z09_HEIGHT;
  #elif DISPLAY_TYPE == EINK_78_INCH || DISPLAY_TYPE == READER_78_INCH_MONO || DISPLAY_TYPE == READER_78_INCH_COLOR
    GxGDEW078Z21_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW078Z21_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW078Z21_HEIGHT;
  #elif DISPLAY_TYPE == EINK_97_INCH
    GxGDEW097T4_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW097T4_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW097T4_HEIGHT;
  #elif DISPLAY_TYPE == EINK_103_INCH || DISPLAY_TYPE == READER_103_INCH_MONO || DISPLAY_TYPE == READER_103_INCH_COLOR
    GxGDEW103Z07_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW103Z07_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW103Z07_HEIGHT;
  #elif DISPLAY_TYPE == EINK_1248_INCH
    GxGDEW1248Z21_Class display;
    static const int16_t SCREEN_WIDTH = GxGDEW1248Z21_WIDTH;
    static const int16_t SCREEN_HEIGHT = GxGDEW1248Z21_HEIGHT;
  #endif
  
  GxFonts fonts;
  bool initialized;
};

#endif // EINK_DRIVER_H