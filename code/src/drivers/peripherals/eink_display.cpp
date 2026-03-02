#include "eink_display.h"

EinkDisplay::EinkDisplay() {
  #if DISPLAY_TYPE == EINK_42_INCH
    io = GxIO_Class(SPI, EINK_CS, EINK_DC, EINK_RST);
    display = GxGDEW042Z15_Class(io, EINK_BUSY);
    width = GxGDEW042Z15_WIDTH;
    height = GxGDEW042Z15_HEIGHT;
  #elif DISPLAY_TYPE == EINK_75_INCH
    io = GxIO_Class(SPI, EINK_CS, EINK_DC, EINK_RST);
    display = GxGDEW075Z09_Class(io, EINK_BUSY);
    width = GxGDEW075Z09_WIDTH;
    height = GxGDEW075Z09_HEIGHT;
  #endif
  
  // 初始化刷新区域管理
  refreshRegionEnabled = false;
  resetRefreshRegion();
}

EinkDisplay::~EinkDisplay() {
  // 清理资源
}

bool EinkDisplay::init() {
  DEBUG_PRINTLN("初始化墨水屏...");
  
  try {
    // 初始化墨水屏
    if (!display.init()) {
      DEBUG_PRINTLN("墨水屏初始化失败");
      return false;
    }
    
    // 初始化字体
    if (!fonts.init(display)) {
      DEBUG_PRINTLN("字体初始化失败");
      return false;
    }
    
    // 清空屏幕
    clear();
    update();
    
    DEBUG_PRINTLN("墨水屏初始化完成");
    return true;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("墨水屏初始化异常:");
    DEBUG_PRINTLN(e.what());
    return false;
  }
}

void EinkDisplay::update() {
  DEBUG_PRINTLN("全屏更新显示...");
  try {
    // 执行全屏更新
    display.update();
    DEBUG_PRINTLN("全屏显示更新完成");
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("全屏更新异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::update(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  DEBUG_PRINTLN("局部更新显示...");
  
  try {
    // 检查参数是否有效
    if (x < 0 || y < 0 || w <= 0 || h <= 0 || x + w > width || y + h > height) {
      DEBUG_PRINTLN("局部更新参数无效，使用全屏更新");
      display.update();
      DEBUG_PRINTLN("局部显示更新完成（使用全屏更新）");
      return;
    }
    
    // 根据墨水屏型号实现真正的局部更新
    #if DISPLAY_TYPE == EINK_42_INCH
      // 4.2寸墨水屏局部更新
      // 注意：GxGDEW042Z15库可能不直接支持局部更新
      // 这里使用updateWindow方法尝试局部更新
      display.updateWindow(x, y, x + w - 1, y + h - 1, true);
    #elif DISPLAY_TYPE == EINK_75_INCH
      // 7.5寸墨水屏局部更新
      // 注意：GxGDEW075Z09库可能不直接支持局部更新
      // 这里使用updateWindow方法尝试局部更新
      display.updateWindow(x, y, x + w - 1, y + h - 1, true);
    #else
      // 其他型号墨水屏，使用全屏更新
      display.update();
    #endif
    
    DEBUG_PRINTLN("局部显示更新完成");
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("局部更新异常，使用全屏更新:");
    DEBUG_PRINTLN(e.what());
    try {
      display.update();
      DEBUG_PRINTLN("局部显示更新完成（使用全屏更新）");
    } catch (const std::exception& e2) {
      DEBUG_PRINTLN("全屏更新也失败:");
      DEBUG_PRINTLN(e2.what());
    }
  }
}

void EinkDisplay::clear() {
  try {
    // 填充屏幕为白色
    display.fillScreen(GxEPD_WHITE);
    
    // 设置默认文本颜色
    display.setTextColor(GxEPD_BLACK);
    
    // 如果启用了刷新区域管理，更新刷新区域为整个屏幕
    if (refreshRegionEnabled) {
      updateRefreshRegion(0, 0, width, height);
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("清空屏幕异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::drawPixel(int16_t x, int16_t y, uint16_t color) {
  try {
    // 检查坐标是否在屏幕范围内
    if (x >= 0 && x < width && y >= 0 && y < height) {
      display.drawPixel(x, y, color);
      
      // 如果启用了刷新区域管理，更新刷新区域
      if (refreshRegionEnabled) {
        updateRefreshRegion(x, y, 1, 1);
      }
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("绘制像素异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) {
  try {
    display.drawLine(x0, y0, x1, y1, color);
    
    // 如果启用了刷新区域管理，更新刷新区域
    if (refreshRegionEnabled) {
      int16_t x = min(x0, x1);
      int16_t y = min(y0, y1);
      int16_t w = abs(x1 - x0) + 1;
      int16_t h = abs(y1 - y0) + 1;
      updateRefreshRegion(x, y, w, h);
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("绘制直线异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  try {
    // 检查参数是否有效
    if (w > 0 && h > 0) {
      display.drawRect(x, y, w, h, color);
      
      // 如果启用了刷新区域管理，更新刷新区域
      if (refreshRegionEnabled) {
        updateRefreshRegion(x, y, w, h);
      }
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("绘制矩形异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) {
  try {
    // 检查参数是否有效
    if (w > 0 && h > 0) {
      display.fillRect(x, y, w, h, color);
      
      // 如果启用了刷新区域管理，更新刷新区域
      if (refreshRegionEnabled) {
        updateRefreshRegion(x, y, w, h);
      }
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("填充矩形异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  try {
    // 检查半径是否有效
    if (r > 0) {
      display.drawCircle(x0, y0, r, color);
      
      // 如果启用了刷新区域管理，更新刷新区域
      if (refreshRegionEnabled) {
        int16_t x = x0 - r;
        int16_t y = y0 - r;
        int16_t w = r * 2 + 1;
        int16_t h = r * 2 + 1;
        updateRefreshRegion(x, y, w, h);
      }
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("绘制圆形异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) {
  try {
    // 检查半径是否有效
    if (r > 0) {
      display.fillCircle(x0, y0, r, color);
      
      // 如果启用了刷新区域管理，更新刷新区域
      if (refreshRegionEnabled) {
        int16_t x = x0 - r;
        int16_t y = y0 - r;
        int16_t w = r * 2 + 1;
        int16_t h = r * 2 + 1;
        updateRefreshRegion(x, y, w, h);
      }
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("填充圆形异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::drawString(int16_t x, int16_t y, const String& text, uint16_t textColor, uint16_t bgColor, uint8_t textSize) {
  try {
    // 检查文本是否为空
    if (text.isEmpty()) {
      return;
    }
    
    // 检查字体大小是否有效
    if (textSize == 0) {
      textSize = 1;
    }
    
    // 保存当前设置
    uint8_t oldTextSize = display.getTextSize();
    uint16_t oldTextColor = display.getTextColor();
    
    // 设置文本颜色和大小
    display.setTextColor(textColor, bgColor);
    display.setTextSize(textSize);
    
    // 计算行高
    int16_t lineHeight = display.fontHeight();
    
    // 处理多行文本
    int16_t currentX = x;
    int16_t currentY = y;
    int16_t minX = x;
    int16_t minY = y;
    int16_t maxX = x;
    int16_t maxY = y + lineHeight;
    
    for (unsigned int i = 0; i < text.length(); i++) {
      if (text[i] == '\n') {
        // 换行
        currentX = x;
        currentY += lineHeight;
        maxY = max(maxY, currentY + lineHeight);
      } else {
        // 检查是否需要换行（如果文本超出屏幕宽度）
        int16_t charWidth = display.charWidth(text[i]);
        if (currentX + charWidth > width) {
          // 自动换行
          currentX = x;
          currentY += lineHeight;
          maxY = max(maxY, currentY + lineHeight);
        }
        
        // 绘制字符
        display.setCursor(currentX, currentY);
        display.write(text[i]);
        
        // 更新X坐标
        currentX += charWidth;
        maxX = max(maxX, currentX);
      }
    }
    
    // 如果启用了刷新区域管理，更新刷新区域
    if (refreshRegionEnabled) {
      int16_t w = maxX - minX;
      int16_t h = maxY - minY;
      if (w > 0 && h > 0) {
        updateRefreshRegion(minX, minY, w, h);
      }
    }
    
    // 恢复默认设置
    display.setTextColor(GxEPD_BLACK, GxEPD_WHITE);
    display.setTextSize(oldTextSize);
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("绘制文本异常:");
    DEBUG_PRINTLN(e.what());
  }
}

int16_t EinkDisplay::getWidth() const {
  return width;
}

int16_t EinkDisplay::getHeight() const {
  return height;
}

int16_t EinkDisplay::measureTextWidth(const String& text, uint8_t textSize) const {
  // 更准确的文本宽度测量
  try {
    // 保存当前设置
    uint8_t oldTextSize = display.getTextSize();
    
    // 设置文本大小
    display.setTextSize(textSize);
    
    // 计算文本宽度
    int16_t width = 0;
    for (unsigned int i = 0; i < text.length(); i++) {
      if (text[i] == '\n') {
        // 换行符不占用宽度
        continue;
      }
      // 使用display的字符宽度测量
      width += display.charWidth(text[i]);
    }
    
    // 恢复原设置
    display.setTextSize(oldTextSize);
    
    return width;
  } catch (const std::exception& e) {
    // 如果出现异常，使用备用方法
    int16_t width = 0;
    for (unsigned int i = 0; i < text.length(); i++) {
      if (text[i] == '\n') {
        continue;
      }
      // 假设每个字符的宽度为6个像素，乘以字体大小
      width += 6 * textSize;
    }
    return width;
  }
}

int16_t EinkDisplay::measureTextHeight(const String& text, uint8_t textSize) const {
  // 更准确的文本高度测量
  try {
    // 保存当前设置
    uint8_t oldTextSize = display.getTextSize();
    
    // 设置文本大小
    display.setTextSize(textSize);
    
    // 计算文本中的换行符数量
    int lineCount = 1;
    for (unsigned int i = 0; i < text.length(); i++) {
      if (text[i] == '\n') {
        lineCount++;
      }
    }
    
    // 使用display的字符高度
    int16_t lineHeight = display.fontHeight();
    
    // 恢复原设置
    display.setTextSize(oldTextSize);
    
    // 计算总高度
    return lineHeight * lineCount;
  } catch (const std::exception& e) {
    // 如果出现异常，使用备用方法
    // 计算文本中的换行符数量
    int lineCount = 1;
    for (unsigned int i = 0; i < text.length(); i++) {
      if (text[i] == '\n') {
        lineCount++;
      }
    }
    // 假设每行的高度为8个像素，乘以字体大小和行数
    return 8 * textSize * lineCount;
  }
}

void EinkDisplay::drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) {
  try {
    // 检查字体大小是否有效
    if (size == 0) {
      size = 1;
    }
    
    // 保存当前设置
    uint8_t oldTextSize = display.getTextSize();
    uint16_t oldTextColor = display.getTextColor();
    
    // 设置文本颜色和大小
    display.setTextColor(color, bg);
    display.setTextSize(size);
    
    // 设置光标位置
    display.setCursor(x, y);
    
    // 绘制字符
    display.write(c);
    
    // 如果启用了刷新区域管理，更新刷新区域
    if (refreshRegionEnabled) {
      int16_t charWidth = display.charWidth(c);
      int16_t charHeight = display.fontHeight();
      updateRefreshRegion(x, y, charWidth, charHeight);
    }
    
    // 恢复默认设置
    display.setTextColor(GxEPD_BLACK, GxEPD_WHITE);
    display.setTextSize(oldTextSize);
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("绘制字符异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::sleep() {
  DEBUG_PRINTLN("墨水屏进入休眠模式...");
  
  try {
    // 清空屏幕，避免残影
    clear();
    update();
    
    // 进入休眠模式
    display.hibernate();
    
    DEBUG_PRINTLN("墨水屏已休眠");
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("墨水屏休眠异常:");
    DEBUG_PRINTLN(e.what());
  }
}

void EinkDisplay::wakeup() {
  DEBUG_PRINTLN("唤醒墨水屏...");
  
  try {
    // 墨水屏从休眠中唤醒通常需要重新初始化
    if (init()) {
      DEBUG_PRINTLN("墨水屏已唤醒");
    } else {
      DEBUG_PRINTLN("墨水屏唤醒失败");
    }
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("墨水屏唤醒异常:");
    DEBUG_PRINTLN(e.what());
  }
}

DisplayCategory EinkDisplay::getDisplayType() const {
  return DisplayCategory::DISPLAY_CATEGORY_EINK;
}

bool EinkDisplay::matchHardware() {
  // 更准确的硬件匹配检测
  try {
    // 检查SPI引脚是否正确配置
    #if defined(ESP32) || defined(ESP8266)
      // 检查SPI引脚是否有效
      if (EINK_CS < 0 || EINK_DC < 0 || EINK_RST < 0 || EINK_BUSY < 0) {
        DEBUG_PRINTLN("墨水屏引脚配置无效");
        return false;
      }
    #endif
    
    // 尝试初始化墨水屏以检测硬件是否存在
    // 注意：这里不实际初始化，只是检查硬件是否响应
    // 具体的硬件检测逻辑需要根据实际硬件情况调整
    
    // 检查显示尺寸是否合理
    if (width <= 0 || height <= 0) {
      DEBUG_PRINTLN("墨水屏尺寸无效");
      return false;
    }
    
    // 检查显示类型是否支持
    #if !defined(DISPLAY_TYPE) || (DISPLAY_TYPE != EINK_42_INCH && DISPLAY_TYPE != EINK_75_INCH)
      DEBUG_PRINTLN("不支持的显示类型");
      return false;
    #endif
    
    // 硬件匹配
    return true;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("硬件检测异常:");
    DEBUG_PRINTLN(e.what());
    return false;
  }
}

// 重置刷新区域
void EinkDisplay::resetRefreshRegion() {
  refreshX1 = width;
  refreshY1 = height;
  refreshX2 = 0;
  refreshY2 = 0;
}

// 更新刷新区域
void EinkDisplay::updateRefreshRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
  if (w <= 0 || h <= 0) {
    return;
  }
  
  uint16_t x2 = x + w - 1;
  uint16_t y2 = y + h - 1;
  
  // 确保坐标在屏幕范围内
  if (x < 0) x = 0;
  if (y < 0) y = 0;
  if (x2 >= width) x2 = width - 1;
  if (y2 >= height) y2 = height - 1;
  
  // 更新刷新区域
  if (x < refreshX1) refreshX1 = x;
  if (y < refreshY1) refreshY1 = y;
  if (x2 > refreshX2) refreshX2 = x2;
  if (y2 > refreshY2) refreshY2 = y2;
}

// 启用刷新区域管理
void EinkDisplay::enableRefreshRegion() {
  refreshRegionEnabled = true;
  resetRefreshRegion();
  DEBUG_PRINTLN("刷新区域管理已启用");
}

// 禁用刷新区域管理
void EinkDisplay::disableRefreshRegion() {
  refreshRegionEnabled = false;
  DEBUG_PRINTLN("刷新区域管理已禁用");
}

// 执行刷新区域更新
void EinkDisplay::updateRefreshRegion() {
  if (!refreshRegionEnabled) {
    // 如果刷新区域管理未启用，执行全屏更新
    update();
    return;
  }
  
  // 检查是否有需要刷新的区域
  if (refreshX1 >= width || refreshY1 >= height || refreshX2 < 0 || refreshY2 < 0 || refreshX1 > refreshX2 || refreshY1 > refreshY2) {
    // 没有需要刷新的区域
    DEBUG_PRINTLN("没有需要刷新的区域");
    return;
  }
  
  // 计算刷新区域的宽度和高度
  uint16_t w = refreshX2 - refreshX1 + 1;
  uint16_t h = refreshY2 - refreshY1 + 1;
  
  DEBUG_PRINTF("执行刷新区域更新: x=%d, y=%d, w=%d, h=%d\n", refreshX1, refreshY1, w, h);
  
  // 执行局部更新
  update(refreshX1, refreshY1, w, h);
  
  // 重置刷新区域
  resetRefreshRegion();
}

