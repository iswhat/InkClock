#ifndef FONT_MANAGER_H
#define FONT_MANAGER_H

#include <Arduino.h>
#include <vector>
#include <GxFonts/GxFonts.h>
#include "config.h"

struct FontInfo {
  String name;        // 字体名称
  String path;        // 字体文件路径
  int size;           // 字体大小（点）
  bool isBuiltIn;     // 是否为内置字体
  bool isDefault;     // 是否为默认字体
};

class FontManager {
public:
  FontManager();
  ~FontManager();
  
  void init();
  bool loadFont(String fontName);
  bool unloadFont(String fontName);
  bool setCurrentFont(String fontName);
  String getCurrentFont();
  std::vector<FontInfo> getFontList();
  bool addFont(String name, String path);
  bool removeFont(String name);
  bool uploadFont(String name, const uint8_t* data, size_t size);
  
private:
  std::vector<FontInfo> fonts;
  String currentFont;
  bool initialized;
  
  void loadBuiltInFonts();
  void scanFontDirectory();
  bool validateFont(String path);
};

#endif // FONT_MANAGER_H