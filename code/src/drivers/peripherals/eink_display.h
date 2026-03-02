#ifndef EINK_DISPLAY_H
#define EINK_DISPLAY_H

#include <Arduino.h>
#include <GxGDEW042Z15/GxGDEW042Z15.h> // 4.2寸三色墨水屏库
#include <GxGDEW075Z09/GxGDEW075Z09.h> // 7.5寸三色墨水屏库
#include <GxIO/GxIO_SPI/GxIO_SPI.h>
#include <GxIO/GxIO.h>
#include <GxFonts/GxFonts.h>
#include "../core/config.h"
#include "display_driver.h"

/**
 * @brief 墨水屏显示驱动类
 * 
 * 负责墨水屏的底层硬件操作，包括初始化、更新、绘制等功能。
 * 实现了IDisplayDriver接口，提供了统一的显示驱动接口。
 */
class EinkDisplay : public IDisplayDriver {
public:
  /**
   * @brief 构造函数
   * 
   * 根据配置的显示类型初始化相应的墨水屏对象。
   */
  EinkDisplay();
  
  /**
   * @brief 析构函数
   * 
   * 清理墨水屏相关资源。
   */
  ~EinkDisplay();
  
  // IDisplayDriver接口实现
  /**
   * @brief 初始化墨水屏
   * 
   * @return bool 初始化是否成功
   */
  bool init() override;
  
  /**
   * @brief 全屏更新显示
   */
  void update() override;
  
  /**
   * @brief 局部更新显示
   * 
   * @param x 起始X坐标
   * @param y 起始Y坐标
   * @param w 宽度
   * @param h 高度
   */
  void update(uint16_t x, uint16_t y, uint16_t w, uint16_t h) override;
  
  /**
   * @brief 清空屏幕
   */
  void clear() override;
  
  /**
   * @brief 绘制像素点
   * 
   * @param x X坐标
   * @param y Y坐标
   * @param color 颜色
   */
  void drawPixel(int16_t x, int16_t y, uint16_t color) override;
  
  /**
   * @brief 绘制直线
   * 
   * @param x0 起始X坐标
   * @param y0 起始Y坐标
   * @param x1 结束X坐标
   * @param y1 结束Y坐标
   * @param color 颜色
   */
  void drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color) override;
  
  /**
   * @brief 绘制矩形
   * 
   * @param x 起始X坐标
   * @param y 起始Y坐标
   * @param w 宽度
   * @param h 高度
   * @param color 颜色
   */
  void drawRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  
  /**
   * @brief 填充矩形
   * 
   * @param x 起始X坐标
   * @param y 起始Y坐标
   * @param w 宽度
   * @param h 高度
   * @param color 颜色
   */
  void fillRect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color) override;
  
  /**
   * @brief 绘制圆形
   * 
   * @param x0 圆心X坐标
   * @param y0 圆心Y坐标
   * @param r 半径
   * @param color 颜色
   */
  void drawCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override;
  
  /**
   * @brief 填充圆形
   * 
   * @param x0 圆心X坐标
   * @param y0 圆心Y坐标
   * @param r 半径
   * @param color 颜色
   */
  void fillCircle(int16_t x0, int16_t y0, int16_t r, uint16_t color) override;
  
  /**
   * @brief 绘制字符串
   * 
   * @param x 起始X坐标
   * @param y 起始Y坐标
   * @param text 文本内容
   * @param textColor 文本颜色
   * @param bgColor 背景颜色
   * @param textSize 文本大小
   */
  void drawString(int16_t x, int16_t y, const String& text, uint16_t textColor, uint16_t bgColor, uint8_t textSize) override;
  
  /**
   * @brief 获取屏幕宽度
   * 
   * @return int16_t 屏幕宽度
   */
  int16_t getWidth() const override;
  
  /**
   * @brief 获取屏幕高度
   * 
   * @return int16_t 屏幕高度
   */
  int16_t getHeight() const override;
  
  /**
   * @brief 测量文本宽度
   * 
   * @param text 文本内容
   * @param textSize 文本大小
   * @return int16_t 文本宽度
   */
  int16_t measureTextWidth(const String& text, uint8_t textSize) const override;
  
  /**
   * @brief 测量文本高度
   * 
   * @param text 文本内容
   * @param textSize 文本大小
   * @return int16_t 文本高度
   */
  int16_t measureTextHeight(const String& text, uint8_t textSize) const override;
  
  /**
   * @brief 绘制字符
   * 
   * @param x 起始X坐标
   * @param y 起始Y坐标
   * @param c 字符
   * @param color 字符颜色
   * @param bg 背景颜色
   * @param size 字符大小
   */
  void drawChar(int16_t x, int16_t y, unsigned char c, uint16_t color, uint16_t bg, uint8_t size) override;
  
  /**
   * @brief 使墨水屏进入休眠模式
   */
  void sleep() override;
  
  /**
   * @brief 唤醒墨水屏
   */
  void wakeup() override;
  
  /**
   * @brief 获取显示类型
   * 
   * @return DisplayCategory 显示类型
   */
  DisplayCategory getDisplayType() const override;
  
  /**
   * @brief 检测驱动与硬件是否匹配
   * 
   * @return bool 是否匹配
   */
  bool matchHardware() override;
  
  /**
   * @brief 启用刷新区域管理
   * 
   * 启用后，只会刷新有内容变化的区域，减少功耗
   */
  void enableRefreshRegion();
  
  /**
   * @brief 禁用刷新区域管理
   * 
   * 禁用后，每次都会进行全屏刷新
   */
  void disableRefreshRegion();
  
  /**
   * @brief 执行刷新区域更新
   * 
   * 只刷新标记为需要更新的区域
   */
  void updateRefreshRegion();
  

  
public:
  /**
   * @brief 刷新模式
   */
  enum RefreshMode {
    REFRESH_MODE_FULL,     // 全屏刷新
    REFRESH_MODE_PARTIAL,  // 局部刷新
    REFRESH_MODE_FAST      // 快速刷新（如果支持）
  };
  
  /**
   * @brief 设置刷新模式
   * 
   * @param mode 刷新模式
   */
  void setRefreshMode(RefreshMode mode);
  
  /**
   * @brief 获取当前刷新模式
   * 
   * @return RefreshMode 当前刷新模式
   */
  RefreshMode getRefreshMode() const;
  
  /**
   * @brief 启用自动刷新管理
   * 
   * 启用后，系统会根据内容变化自动决定何时刷新
   */
  void enableAutoRefresh();
  
  /**
   * @brief 禁用自动刷新管理
   * 
   * 禁用后，需要手动调用update()或updateRefreshRegion()进行刷新
   */
  void disableAutoRefresh();
  
  /**
   * @brief 设置自动刷新间隔
   * 
   * @param interval 自动刷新间隔（毫秒）
   */
  void setAutoRefreshInterval(unsigned long interval);
  
  /**
   * @brief 获取显示状态
   * 
   * @return bool 显示是否就绪
   */
  bool isDisplayReady() const;
  
  /**
   * @brief 获取最后刷新时间
   * 
   * @return unsigned long 最后刷新时间（毫秒）
   */
  unsigned long getLastRefreshTime() const;
  
  /**
   * @brief 强制全屏刷新
   * 
   * 忽略刷新区域管理，强制进行全屏刷新
   */
  void forceFullRefresh();
  
private:
  // 墨水屏对象
  #if DISPLAY_TYPE == EINK_42_INCH
    GxIO_Class io;              // 4.2寸墨水屏IO对象
    GxGDEW042Z15_Class display; // 4.2寸墨水屏对象
  #elif DISPLAY_TYPE == EINK_75_INCH
    GxIO_Class io;              // 7.5寸墨水屏IO对象
    GxGDEW075Z09_Class display; // 7.5寸墨水屏对象
  #endif
  GxFonts fonts;                // 字体对象
  
  // 显示尺寸
  uint16_t width;               // 屏幕宽度
  uint16_t height;              // 屏幕高度
  
  // 刷新区域管理
  bool refreshRegionEnabled;    // 是否启用刷新区域管理
  uint16_t refreshX1;           // 刷新区域左上角X坐标
  uint16_t refreshY1;           // 刷新区域左上角Y坐标
  uint16_t refreshX2;           // 刷新区域右下角X坐标
  uint16_t refreshY2;           // 刷新区域右下角Y坐标
  
  // 刷新模式
  RefreshMode currentRefreshMode; // 当前刷新模式
  
  // 自动刷新管理
  bool autoRefreshEnabled;      // 是否启用自动刷新
  unsigned long autoRefreshInterval; // 自动刷新间隔
  unsigned long lastRefreshTime; // 最后刷新时间
  unsigned long lastDrawTime;    // 最后绘制时间
  
  // 显示状态
  bool displayReady;             // 显示是否就绪
  
  // 性能统计
  unsigned long refreshCount;    // 刷新次数
  unsigned long totalRefreshTime; // 总刷新时间
  
  // 私有方法
  void displayFullRefresh();                    // 全屏刷新
  void displayPartialRefresh(uint16_t x, uint16_t y, uint16_t w, uint16_t h); // 局部刷新
  void displayFastRefresh(uint16_t x, uint16_t y, uint16_t w, uint16_t h);    // 快速刷新
  void resetRefreshRegion();                    // 重置刷新区域
  void updateRefreshRegion(uint16_t x, uint16_t y, uint16_t w, uint16_t h);   // 更新刷新区域
  void checkAutoRefresh();                      // 检查自动刷新
  void mergeRefreshRegions();                   // 合并刷新区域
  void optimizeRefreshRegion();                 // 优化刷新区域
  void updateDisplayState();                    // 更新显示状态
};

#endif // EINK_DISPLAY_H