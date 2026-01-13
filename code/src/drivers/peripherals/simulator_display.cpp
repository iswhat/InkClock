#include "simulator_display.h"
#include <iostream>
#include <string>

SimulatorDisplay::SimulatorDisplay() : width(800), height(480), frameBuffer(nullptr)
#ifdef USE_SDL2
, window(nullptr), renderer(nullptr), texture(nullptr)
#endif
 {
  // 初始化日志文件
  logFile.open("simulator_display.log");
  if (logFile.is_open()) {
    logFile << "SimulatorDisplay initialized" << std::endl;
  }
  
  // 初始化帧缓冲区
  initFrameBuffer();
  
  // 初始化SDL2
  #ifdef USE_SDL2
  initSDL2();
  #endif
}

SimulatorDisplay::~SimulatorDisplay() {
  // 清理SDL2
  #ifdef USE_SDL2
  cleanupSDL2();
  #endif
  
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
  // 简单的字符绘制实现
  // 注意：这是一个简化版本，实际应用中可能需要使用字体库
  for (uint8_t i = 0; i < 5; i++) { // 字符宽度
    uint8_t line = 0;
    // 这里使用一个简单的字体映射，实际应用中应该使用字体库
    switch (c) {
      case 'A': line = 0x7E; break;
      case 'B': line = 0x3F; break;
      case 'C': line = 0x1C; break;
      case 'D': line = 0x3F; break;
      case 'E': line = 0x79; break;
      case 'F': line = 0x71; break;
      case 'G': line = 0x1E; break;
      case 'H': line = 0x77; break;
      case 'I': line = 0x30; break;
      case 'J': line = 0x0E; break;
      case 'K': line = 0x76; break;
      case 'L': line = 0x18; break;
      case 'M': line = 0x55; break;
      case 'N': line = 0x57; break;
      case 'O': line = 0x1C; break;
      case 'P': line = 0x73; break;
      case 'Q': line = 0x3D; break;
      case 'R': line = 0x7B; break;
      case 'S': line = 0x4E; break;
      case 'T': line = 0x31; break;
      case 'U': line = 0x1F; break;
      case 'V': line = 0x3B; break;
      case 'W': line = 0x6B; break;
      case 'X': line = 0x76; break;
      case 'Y': line = 0x33; break;
      case 'Z': line = 0x49; break;
      case '0': line = 0x3E; break;
      case '1': line = 0x06; break;
      case '2': line = 0x7A; break;
      case '3': line = 0x7E; break;
      case '4': line = 0x46; break;
      case '5': line = 0x6E; break;
      case '6': line = 0x6F; break;
      case '7': line = 0x70; break;
      case '8': line = 0x7F; break;
      case '9': line = 0x7E; break;
      default: line = 0x00; break;
    }
    
    for (uint8_t j = 0; j < 8; j++) { // 字符高度
      if (line & (1 << j)) {
        for (uint8_t sx = 0; sx < size; sx++) {
          for (uint8_t sy = 0; sy < size; sy++) {
            drawPixel(x + i * size + sx, y + j * size + sy, color);
          }
        }
      } else if (bg != color) {
        for (uint8_t sx = 0; sx < size; sx++) {
          for (uint8_t sy = 0; sy < size; sy++) {
            drawPixel(x + i * size + sx, y + j * size + sy, bg);
          }
        }
      }
    }
  }
  
  logDrawOperation("drawChar", x, y, 8 * size, 16 * size, color);
}

// 绘制字符串
void SimulatorDisplay::drawString(int16_t x, int16_t y, const String& text, uint16_t color, uint16_t bg, uint8_t size) {
  int16_t currentX = x;
  for (size_t i = 0; i < text.length(); i++) {
    drawChar(currentX, y, text[i], color, bg, size);
    currentX += 6 * size; // 字符间距
  }
  logDrawOperation("drawString", x, y, text.length() * 8 * size, 16 * size, color);
}

// 绘制矩形
void SimulatorDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  // 绘制矩形的四条边
  drawLine(x, y, x + w - 1, y, color);           // 上边
  drawLine(x, y + h - 1, x + w - 1, y + h - 1, color); // 下边
  drawLine(x, y, x, y + h - 1, color);           // 左边
  drawLine(x + w - 1, y, x + w - 1, y + h - 1, color); // 右边
  logDrawOperation("drawRect", x, y, w, h, color);
}

// 填充矩形
void SimulatorDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  // 填充矩形区域
  for (int16_t i = x; i < x + w; i++) {
    for (int16_t j = y; j < y + h; j++) {
      drawPixel(i, j, color);
    }
  }
  logDrawOperation("fillRect", x, y, w, h, color);
}

// 绘制直线
void SimulatorDisplay::drawLine(int16_t x1, int16_t y1, int16_t x2, int16_t y2, uint16_t color) {
  // 简化的直线绘制（使用Bresenham算法）
  int dx = abs(x2 - x1);
  int dy = abs(y2 - y1);
  int sx = x1 < x2 ? 1 : -1;
  int sy = y1 < y2 ? 1 : -1;
  int err = dx - dy;
  
  int x = x1;
  int y = y1;
  
  while (x != x2 || y != y2) {
    drawPixel(x, y, color);
    int e2 = 2 * err;
    if (e2 > -dy) {
      err -= dy;
      x += sx;
    }
    if (e2 < dx) {
      err += dx;
      y += sy;
    }
  }
  drawPixel(x2, y2, color);
  
  logDrawOperation("drawLine", x1, y1, x2 - x1, y2 - y1, color);
}

// 更新显示
void SimulatorDisplay::update() {
  std::cout << "SimulatorDisplay update" << std::endl;
  exportToHtml("simulator_display.html");
  exportToSvg("simulator_display.svg");
  
  // 更新SDL2显示
  #ifdef USE_SDL2
  updateSDL2();
  #endif
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

#ifdef USE_SDL2

// 初始化SDL2
bool SimulatorDisplay::initSDL2() {
  if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
    return false;
  }
  
  window = SDL_CreateWindow(
    "InkClock Simulator Display",
    SDL_WINDOWPOS_UNDEFINED,
    SDL_WINDOWPOS_UNDEFINED,
    width,
    height,
    SDL_WINDOW_SHOWN
  );
  
  if (!window) {
    std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
    SDL_Quit();
    return false;
  }
  
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    std::cerr << "SDL_CreateRenderer failed: " << SDL_GetError() << std::endl;
    SDL_DestroyWindow(window);
    SDL_Quit();
    return false;
  }
  
  texture = SDL_CreateTexture(
    renderer,
    SDL_PIXELFORMAT_RGB565,
    SDL_TEXTUREACCESS_STREAMING,
    width,
    height
  );
  
  if (!texture) {
    std::cerr << "SDL_CreateTexture failed: " << SDL_GetError() << std::endl;
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return false;
  }
  
  std::cout << "SDL2 initialized successfully" << std::endl;
  return true;
}

// 清理SDL2
void SimulatorDisplay::cleanupSDL2() {
  if (texture) {
    SDL_DestroyTexture(texture);
    texture = nullptr;
  }
  
  if (renderer) {
    SDL_DestroyRenderer(renderer);
    renderer = nullptr;
  }
  
  if (window) {
    SDL_DestroyWindow(window);
    window = nullptr;
  }
  
  SDL_Quit();
  std::cout << "SDL2 cleanup completed" << std::endl;
}

// 更新SDL2显示
void SimulatorDisplay::updateSDL2() {
  if (!texture || !renderer) {
    return;
  }
  
  SDL_UpdateTexture(texture, nullptr, frameBuffer, width * sizeof(uint16_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
  
  // 处理SDL事件
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    if (event.type == SDL_QUIT) {
      // 这里可以添加退出处理逻辑
    }
  }
}

#endif // USE_SDL2
