#ifndef SIMULATOR_DISPLAY_H
#define SIMULATOR_DISPLAY_H

#include "display_driver.h"
#include <fstream>

// 模拟显示驱动，用于PC端预览
class SimulatorDisplay : public IDisplayDriver {
public:
  SimulatorDisplay();
  ~SimulatorDisplay();
  
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
  
  // 测量文本宽度
  int16_t measureTextWidth(const String& text, uint8_t size) const override;
  
  // 测量文本高度
  int16_t measureTextHeight(const String& text, uint8_t size) const override;
  
  // 休眠
  void sleep() override;
  
  // 唤醒
  void wakeup() override;
  
  // 获取显示类型
  DisplayType getType() const override;
  
  // 检测驱动与硬件是否匹配
  bool matchHardware() override;
  
  // 导出显示内容为HTML
  void exportToHtml(const char* filename);
  
  // 导出显示内容为SVG
  void exportToSvg(const char* filename);
  
private:
  int16_t width;
  int16_t height;
  uint16_t* frameBuffer;
  std::ofstream logFile;
  
  // 初始化帧缓冲区
  void initFrameBuffer();
  
  // 清理帧缓冲区
  void cleanupFrameBuffer();
  
  // 记录绘制操作
  void logDrawOperation(const char* operation, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color);
};

#endif // SIMULATOR_DISPLAY_H
