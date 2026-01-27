#include "font_manager.h"
#include "spiffs_manager.h"
#include <FS.h>

FontManager::FontManager() {
  currentFont = DEFAULT_FONT;
  initialized = false;
}

FontManager::~FontManager() {
  // 清理资源
}

void FontManager::init() {
  if (initialized) return;
  
  DEBUG_PRINTLN("初始化字体管理器...");
  
  // 加载内置字体
  loadBuiltInFonts();
  
  // 扫描字体目录
  scanFontDirectory();
  
  initialized = true;
  DEBUG_PRINTLN("字体管理器初始化完成");
}

void FontManager::loadBuiltInFonts() {
  // 添加内置字体
  FontInfo font;
  
  font.name = "Roboto-Regular";
  font.path = "built-in";
  font.size = 16;
  font.isBuiltIn = true;
  font.isDefault = (font.name == DEFAULT_FONT);
  fonts.push_back(font);
  
  font.name = "Roboto-Bold";
  font.path = "built-in";
  font.size = 16;
  font.isBuiltIn = true;
  font.isDefault = false;
  fonts.push_back(font);
  
  font.name = "Arial";
  font.path = "built-in";
  font.size = 16;
  font.isBuiltIn = true;
  font.isDefault = false;
  fonts.push_back(font);
  
  DEBUG_PRINTLN("加载内置字体完成");
}

void FontManager::scanFontDirectory() {
  // 确保SPIFFS已挂载
  if (!isSPIFFSMounted()) {
    DEBUG_PRINTLN("SPIFFS挂载失败，无法扫描字体目录");
    return;
  }
  
  FS& fs = getSPIFFS();
  
  // 检查字体目录是否存在
  if (!fs.exists(FONT_DIR)) {
    DEBUG_PRINTLN("字体目录不存在，创建目录...");
    if (!fs.mkdir(FONT_DIR)) {
      DEBUG_PRINTLN("创建字体目录失败");
      return;
    }
  }
  
  // 扫描字体目录中的文件
  File root = fs.open(FONT_DIR);
  if (!root || !root.isDirectory()) {
    DEBUG_PRINTLN("打开字体目录失败");
    return;
  }
  
  File file = root.openNextFile();
  while (file) {
    if (!file.isDirectory()) {
      String fileName = file.name();
      String fontName = fileName.substring(fileName.lastIndexOf('/') + 1);
      fontName = fontName.substring(0, fontName.lastIndexOf('.'));
      
      // 检查是否已存在相同名称的字体
      bool exists = false;
      for (const auto& font : fonts) {
        if (font.name == fontName) {
          exists = true;
          break;
        }
      }
      
      if (!exists) {
        FontInfo font;
        font.name = fontName;
        font.path = fileName;
        font.size = 16; // 默认大小
        font.isBuiltIn = false;
        font.isDefault = (fontName == DEFAULT_FONT);
        fonts.push_back(font);
        
        DEBUG_PRINTF("发现字体文件: %s\n", fileName.c_str());
      }
    }
    file = root.openNextFile();
  }
  
  DEBUG_PRINTLN("扫描字体目录完成");
}

bool FontManager::validateFont(String path) {
  // 简单的字体文件验证
  if (path == "built-in") {
    return true; // 内置字体总是有效的
  }
  
  // 检查SPIFFS是否挂载
  if (!isSPIFFSMounted()) {
    DEBUG_PRINTLN("SPIFFS未挂载，无法验证字体文件");
    return false;
  }
  
  FS& fs = getSPIFFS();
  
  // 检查文件是否存在
  if (!fs.exists(path)) {
    DEBUG_PRINTF("字体文件不存在: %s\n", path.c_str());
    return false;
  }
  
  // 检查文件大小是否合理
  File file = fs.open(path);
  if (!file) {
    DEBUG_PRINTF("打开字体文件失败: %s\n", path.c_str());
    return false;
  }
  
  size_t fileSize = file.size();
  file.close();
  
  // 字体文件大小应在合理范围内（1KB-1MB）
  if (fileSize < 1024 || fileSize > 1024 * 1024) {
    DEBUG_PRINTF("字体文件大小不合理: %zu bytes\n", fileSize);
    return false;
  }
  
  // 验证文件格式（GxFonts格式验证）
  file = fs.open(path, FILE_READ);
  if (!file) {
    DEBUG_PRINTF("打开字体文件失败: %s\n", path.c_str());
    return false;
  }
  
  // 读取GxFonts文件头
  // GxFonts文件格式：
  // 前4字节：字体名称长度
  // 接下来是字体名称
  // 然后是字体高度、宽度等信息
  
  // 读取字体名称长度
  uint8_t nameLen = 0;
  if (file.readBytes((char*)&nameLen, 1) != 1) {
    file.close();
    DEBUG_PRINTF("读取字体文件头失败: %s\n", path.c_str());
    return false;
  }
  
  // 读取字体名称
  if (nameLen > 0) {
    char fontName[256];
    if (file.readBytes(fontName, nameLen) != nameLen) {
      file.close();
      DEBUG_PRINTF("读取字体名称失败: %s\n", path.c_str());
      return false;
    }
  }
  
  // 读取字体高度
  uint8_t fontHeight = 0;
  if (file.readBytes((char*)&fontHeight, 1) != 1) {
    file.close();
    DEBUG_PRINTF("读取字体高度失败: %s\n", path.c_str());
    return false;
  }
  
  // 验证字体高度是否合理（8-128像素）
  if (fontHeight < 8 || fontHeight > 128) {
    file.close();
    DEBUG_PRINTF("字体高度不合理: %d像素\n", fontHeight);
    return false;
  }
  
  file.close();
  
  DEBUG_PRINTF("字体文件验证通过: %s\n", path.c_str());
  return true;
}

bool FontManager::loadFont(String fontName) {
  if (!initialized) {
    init();
  }
  
  // 查找字体
  for (const auto& font : fonts) {
    if (font.name == fontName) {
      if (validateFont(font.path)) {
        DEBUG_PRINTF("加载字体: %s\n", fontName.c_str());
        return true;
      }
      break;
    }
  }
  
  DEBUG_PRINTF("加载字体失败: %s\n", fontName.c_str());
  return false;
}

bool FontManager::unloadFont(String fontName) {
  // 内置字体不能卸载
  for (const auto& font : fonts) {
    if (font.name == fontName && font.isBuiltIn) {
      DEBUG_PRINTF("内置字体不能卸载: %s\n", fontName.c_str());
      return false;
    }
  }
  
  DEBUG_PRINTF("卸载字体: %s\n", fontName.c_str());
  return true;
}

bool FontManager::setCurrentFont(String fontName) {
  if (!loadFont(fontName)) {
    return false;
  }
  
  currentFont = fontName;
  DEBUG_PRINTF("当前字体已设置为: %s\n", fontName.c_str());
  return true;
}

String FontManager::getCurrentFont() {
  return currentFont;
}

std::vector<FontInfo> FontManager::getFontList() {
  if (!initialized) {
    init();
  }
  
  return fonts;
}

bool FontManager::addFont(String name, String path) {
  if (!initialized) {
    init();
  }
  
  // 检查是否已存在同名字体
  for (const auto& font : fonts) {
    if (font.name == name) {
      DEBUG_PRINTF("字体已存在: %s\n", name.c_str());
      return false;
    }
  }
  
  FontInfo font;
  font.name = name;
  font.path = path;
  font.size = 16; // 默认大小
  font.isBuiltIn = false;
  font.isDefault = false;
  
  fonts.push_back(font);
  DEBUG_PRINTF("添加字体成功: %s\n", name.c_str());
  return true;
}

bool FontManager::removeFont(String name) {
  if (!initialized) {
    init();
  }
  
  for (auto it = fonts.begin(); it != fonts.end(); ++it) {
    if (it->name == name) {
      if (it->isBuiltIn) {
        DEBUG_PRINTF("内置字体不能删除: %s\n", name.c_str());
        return false;
      }
      
      // 删除字体文件
      if (isSPIFFSMounted()) {
        FS& fs = getSPIFFS();
        if (fs.exists(it->path)) {
          fs.remove(it->path);
          DEBUG_PRINTF("删除字体文件: %s\n", it->path.c_str());
        }
      }
      
      fonts.erase(it);
      DEBUG_PRINTF("删除字体成功: %s\n", name.c_str());
      return true;
    }
  }
  
  DEBUG_PRINTF("删除字体失败: %s\n", name.c_str());
  return false;
}

bool FontManager::uploadFont(String name, const uint8_t* data, size_t size) {
  if (!initialized) {
    init();
  }
  
  // 检查字体数量是否已达上限
  if (fonts.size() >= MAX_FONTS) {
    DEBUG_PRINTLN("字体数量已达上限");
    return false;
  }
  
  // 检查SPIFFS是否挂载
  if (!isSPIFFSMounted()) {
    DEBUG_PRINTLN("SPIFFS未挂载，无法上传字体文件");
    return false;
  }
  
  FS& fs = getSPIFFS();
  
  // 构建字体文件路径
  String path = String(FONT_DIR) + "/" + name + ".bin";
  
  // 检查文件是否已存在
  if (fs.exists(path)) {
    DEBUG_PRINTF("字体文件已存在: %s\n", path.c_str());
    return false;
  }
  
  // 创建字体文件
  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    DEBUG_PRINTF("创建字体文件失败: %s\n", path.c_str());
    return false;
  }
  
  // 写入字体数据
  size_t written = file.write(data, size);
  file.close();
  
  if (written != size) {
    DEBUG_PRINTLN("写入字体数据失败");
    fs.remove(path); // 删除不完整的文件
    return false;
  }
  
  // 添加到字体列表
  return addFont(name, path);
}