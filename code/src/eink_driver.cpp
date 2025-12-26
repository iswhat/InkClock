#include "eink_driver.h"

EinkDriver::EinkDriver() : io(SPI, EINK_CS, EINK_DC, EINK_RST), initialized(false) {
  // 构造函数，初始化IO对象
}

EinkDriver::~EinkDriver() {
  // 析构函数，清理资源
  if (initialized) {
    sleep();
  }
}

bool EinkDriver::init() {
  DEBUG_PRINTLN("初始化墨水屏驱动...");
  
  // 初始化显示对象
  display.init();
  
  // 初始化字体
  fonts.init(display);
  
  // 清除屏幕
  clear();
  update();
  
  initialized = true;
  DEBUG_PRINTLN("墨水屏驱动初始化完成");
  return true;
}

void EinkDriver::clear() {
  if (!initialized) {
    return;
  }
  
  display.fillScreen(GxEPD_WHITE);
}

void EinkDriver::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (!initialized) {
    return;
  }
  
  display.drawPixel(x, y, color);
}

void EinkDriver::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
  if (!initialized) {
    return;
  }
  
  display.setCursor(x, y);
  display.setTextColor(color, bg);
  display.setTextSize(size);
  display.write(c);
}

void EinkDriver::drawString(int16_t x, int16_t y, const String& text, uint16_t color, uint16_t bg, uint8_t size) {
  if (!initialized) {
    return;
  }
  
  display.setCursor(x, y);
  display.setTextColor(color, bg);
  display.setTextSize(size);
  display.print(text);
}

void EinkDriver::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (!initialized) {
    return;
  }
  
  display.drawRect(x, y, w, h, color);
}

void EinkDriver::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (!initialized) {
    return;
  }
  
  display.fillRect(x, y, w, h, color);
}

void EinkDriver::update() {
  if (!initialized) {
    return;
  }
  
  display.update();
}

void EinkDriver::update(int16_t x, int16_t y, int16_t w, int16_t h) {
  if (!initialized) {
    return;
  }
  
  // 注意：不同的墨水屏库可能对局部刷新的支持不同
  // 这里使用通用的update方法，实际使用时需要根据墨水屏型号调整
  display.update();
}

int16_t EinkDriver::getWidth() const {
  return SCREEN_WIDTH;
}

int16_t EinkDriver::getHeight() const {
  return SCREEN_HEIGHT;
}

void EinkDriver::sleep() {
  if (!initialized) {
    return;
  }
  
  display.powerOff();
}

void EinkDriver::wakeup() {
  if (!initialized) {
    return;
  }
  
  display.powerOn();
}

EinkDisplayType EinkDriver::getType() const {
  // 返回当前配置的显示类型
  return DISPLAY_TYPE;
}