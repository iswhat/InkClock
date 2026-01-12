#include "simulator_display.h"
#include <iostream>
#include <string>

SimulatorDisplay::SimulatorDisplay() : width(800), height(480), frameBuffer(nullptr) {
  // 初始化日志文件
  logFile.open("simulator_display.log");
  if (logFile.is_open()) {
    logFile << "SimulatorDisplay initialized" << std::endl;
  }
  
  // 初始化帧缓冲区
  initFrameBuffer();
}

SimulatorDisplay::~SimulatorDisplay() {
  // 清理帧缓冲区
  cleanupFrameBuffer();
  
  // 关闭日志文件
  if (logFile.is_open()) {
    logFile.close();
  }
}

// 初始化帧缓冲区
void SimulatorDisplay::initFrameBuffer() {
  frameBuffer = new uint16_t[width * height];
  clear();
}

// 清理帧缓冲区
void SimulatorDisplay::cleanupFrameBuffer() {
  if (frameBuffer) {
    delete[] frameBuffer;
    frameBuffer = nullptr;
  }
}

// 记录绘制操作
void SimulatorDisplay::logDrawOperation(const char* operation, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  if (logFile.is_open()) {
    logFile << operation << " at (" << x << ", " << y << ") size (" << w << ", " << h << ") color: " << color << std::endl;
  }
}

// 初始化驱动
bool SimulatorDisplay::init() {
  std::cout << "SimulatorDisplay initialized successfully" << std::endl;
  return true;
}

// 清除屏幕
void SimulatorDisplay::clear() {
  if (frameBuffer) {
    for (int i = 0; i < width * height; i++) {
      frameBuffer[i] = 0xFFFF; // 白色
    }
  }
  logDrawOperation("clear", 0, 0, width, height, 0xFFFF);
}

// 绘制像素点
void SimulatorDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
  if (x >= 0 && x < width && y >= 0 && y < height && frameBuffer) {
    frameBuffer[y * width + x] = color;
  }
  logDrawOperation("drawPixel", x, y, 1, 1, color);
}

// 绘制字符
void SimulatorDisplay::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
  // 简化的字符绘制，实际应用中可能需要更复杂的实现
  logDrawOperation("drawChar", x, y, 8 * size, 16 * size, color);
}

// 绘制字符串
void SimulatorDisplay::drawString(int16_t x, int16_t y, const String& text, uint16_t color, uint16_t bg, uint8_t size) {
  // 简化的字符串绘制
  logDrawOperation("drawString", x, y, text.length() * 8 * size, 16 * size, color);
}

// 绘制矩形
void SimulatorDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  // 简化的矩形绘制
  logDrawOperation("drawRect", x, y, w, h, color);
}

// 填充矩形
void SimulatorDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  // 简化的填充矩形
  logDrawOperation("fillRect", x, y, w, h, color);
}

// 更新显示
void SimulatorDisplay::update() {
  std::cout << "SimulatorDisplay update" << std::endl;
  exportToHtml("simulator_display.html");
  exportToSvg("simulator_display.svg");
}

// 局部更新
void SimulatorDisplay::update(int16_t x, int16_t y, int16_t w, int16_t h) {
  std::cout << "SimulatorDisplay partial update at (" << x << ", " << y << ") size (" << w << ", " << h << ")" << std::endl;
}

// 获取屏幕宽度
int16_t SimulatorDisplay::getWidth() const {
  return width;
}

// 获取屏幕高度
int16_t SimulatorDisplay::getHeight() const {
  return height;
}

// 测量文本宽度
int16_t SimulatorDisplay::measureTextWidth(const String& text, uint8_t size) const {
  return text.length() * 8 * size;
}

// 测量文本高度
int16_t SimulatorDisplay::measureTextHeight(const String& text, uint8_t size) const {
  return 16 * size;
}

// 休眠
void SimulatorDisplay::sleep() {
  std::cout << "SimulatorDisplay sleep" << std::endl;
}

// 唤醒
void SimulatorDisplay::wakeup() {
  std::cout << "SimulatorDisplay wakeup" << std::endl;
}

// 获取显示类型
DisplayType SimulatorDisplay::getType() const {
  return DisplayType::DISPLAY_TYPE_EINK;
}

// 检测驱动与硬件是否匹配
bool SimulatorDisplay::matchHardware() {
  // 模拟驱动始终匹配
  return true;
}

// 导出显示内容为HTML
void SimulatorDisplay::exportToHtml(const char* filename) {
  std::ofstream htmlFile(filename);
  if (htmlFile.is_open()) {
    htmlFile << "<!DOCTYPE html>" << std::endl;
    htmlFile << "<html>" << std::endl;
    htmlFile << "<head>" << std::endl;
    htmlFile << "<title>Simulator Display</title>" << std::endl;
    htmlFile << "<style>" << std::endl;
    htmlFile << ".display { width: " << width << "px; height: " << height << "px; border: 1px solid #000; background-color: #fff; }" << std::endl;
    htmlFile << ".pixel { width: 1px; height: 1px; float: left; }" << std::endl;
    htmlFile << "</style>" << std::endl;
    htmlFile << "</head>" << std::endl;
    htmlFile << "<body>" << std::endl;
    htmlFile << "<h1>Simulator Display</h1>" << std::endl;
    htmlFile << "<div class=\"display\">" << std::endl;
    
    // 生成像素数据
    if (frameBuffer) {
      for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
          uint16_t color = frameBuffer[y * width + x];
          // 简单的颜色转换
          int r = (color >> 11) & 0x1F;
          int g = (color >> 5) & 0x3F;
          int b = color & 0x1F;
          r = (r * 255) / 31;
          g = (g * 255) / 63;
          b = (b * 255) / 31;
          htmlFile << "<div class=\"pixel\" style=\"background-color: rgb(" << r << ", " << g << ", " << b << ");\"></div>" << std::endl;
        }
        htmlFile << "<br style=\"clear: both;\">" << std::endl;
      }
    }
    
    htmlFile << "</div>" << std::endl;
    htmlFile << "</body>" << std::endl;
    htmlFile << "</html>" << std::endl;
    htmlFile.close();
    std::cout << "Display exported to " << filename << std::endl;
  }
}

// 导出显示内容为SVG
void SimulatorDisplay::exportToSvg(const char* filename) {
  std::ofstream svgFile(filename);
  if (svgFile.is_open()) {
    svgFile << "<svg width=\"" << width << "\" height=\"" << height << "\" xmlns=\"http://www.w3.org/2000/svg\">" << std::endl;
    svgFile << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>" << std::endl;
    
    // 这里可以添加更复杂的SVG绘制逻辑
    svgFile << "<text x=\"10\" y=\"30\" font-family=\"Arial\" font-size=\"24\" fill=\"black\">Simulator Display</text>" << std::endl;
    svgFile << "<text x=\"10\" y=\"60\" font-family=\"Arial\" font-size=\"16\" fill=\"gray\">Width: " << width << "px, Height: " << height << "px</text>" << std::endl;
    svgFile << "<text x=\"10\" y=\"90\" font-family=\"Arial\" font-size=\"16\" fill=\"gray\">This is a simulated display for preview purposes</text>" << std::endl;
    
    svgFile << "</svg>" << std::endl;
    svgFile.close();
    std::cout << "Display exported to " << filename << std::endl;
  }
}
