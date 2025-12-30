#ifndef DISPLAY_DRIVER_H
#define DISPLAY_DRIVER_H

#include <Arduino.h>

// 显示类型枚举
enum class DisplayType {
  DISPLAY_TYPE_UNKNOWN,
  DISPLAY_TYPE_EINK,      // 电子墨水屏（支持黑白、三色、四色）
};

// 显示驱动抽象接口
class IDisplayDriver {
public:
  virtual ~IDisplayDriver() {}
  
  // 初始化驱动
  virtual bool init() = 0;
  
  // 清除屏幕
  virtual void clear() = 0;
  
  // 绘制像素点
  virtual void drawPixel(int16_t x, int16_t y, uint16_t color) = 0;
  
  // 绘制字符
  virtual void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) = 0;
  
  // 绘制字符串
  virtual void drawString(int16_t x, int16_t y, const String& text, uint16_t color, uint16_t bg, uint8_t size) = 0;
  
  // 绘制矩形
  virtual void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
  
  // 填充矩形
  virtual void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) = 0;
  
  // 更新显示
  virtual void update() = 0;
  
  // 局部更新
  virtual void update(int16_t x, int16_t y, int16_t w, int16_t h) = 0;
  
  // 获取屏幕宽度
  virtual int16_t getWidth() const = 0;
  
  // 获取屏幕高度
  virtual int16_t getHeight() const = 0;
  
  // 测量文本宽度
  virtual int16_t measureTextWidth(const String& text, uint8_t size) const = 0;
  
  // 测量文本高度
  virtual int16_t measureTextHeight(const String& text, uint8_t size) const = 0;
  
  // 休眠
  virtual void sleep() = 0;
  
  // 唤醒
  virtual void wakeup() = 0;
  
  // 获取显示类型
  virtual DisplayType getType() const = 0;
  
  // 检测驱动与硬件是否匹配
  virtual bool matchHardware() = 0;
};

// 显示驱动工厂类
template <typename T>
class DisplayDriverFactory {
public:
  static IDisplayDriver* create() {
    return new T();
  }
};

#endif // DISPLAY_DRIVER_H