#ifndef FEEDBACK_MANAGER_H
#define FEEDBACK_MANAGER_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "arduino_compat.h"
#endif

// 反馈类型枚举
enum FeedbackType {
  FEEDBACK_NONE,
  FEEDBACK_CLICK,         // 单击反馈
  FEEDBACK_DOUBLE_CLICK,  // 双击反馈
  FEEDBACK_TRIPLE_CLICK,  // 三连击反馈
  FEEDBACK_LONG_PRESS,    // 长按反馈
  FEEDBACK_POWER_OFF,     // 关机反馈
  FEEDBACK_SUCCESS,       // 操作成功反馈
  FEEDBACK_ERROR,         // 操作失败反馈
  FEEDBACK_WARNING,       // 警告反馈
  FEEDBACK_INFO           // 信息反馈
};

// 反馈模式枚举
enum FeedbackMode {
  FEEDBACK_MODE_SCREEN,    // 屏幕反馈
  FEEDBACK_MODE_LED,       // LED指示灯反馈
  FEEDBACK_MODE_BOTH       // 两者都有
};

// 状态反馈管理器类
class FeedbackManager {
public:
  FeedbackManager();
  ~FeedbackManager();
  
  // 初始化反馈管理器
  bool init();
  
  // 触发反馈
  void triggerFeedback(FeedbackType type, FeedbackMode mode = FEEDBACK_MODE_BOTH);
  
  // 设置反馈模式
  void setFeedbackMode(FeedbackMode mode);
  
  // 获取反馈模式
  FeedbackMode getFeedbackMode() const;
  
  // 更新反馈状态
  void update();
  
  // 设置LED引脚
  void setLEDPins(int powerLEDPin, int wifiLEDPin, int bluetoothLEDPin);
  
  // 设置屏幕驱动（可选）
  void setDisplayDriver(void* displayDriver);
  
private:
  // LED引脚
  int powerLEDPin;
  int wifiLEDPin;
  int bluetoothLEDPin;
  
  // 反馈模式
  FeedbackMode feedbackMode;
  
  // 屏幕驱动（可选）
  void* displayDriver;
  
  // 反馈状态
  struct FeedbackState {
    FeedbackType type;
    unsigned long startTime;
    unsigned long lastUpdateTime;
    int animationFrame;
    bool active;
  };
  
  FeedbackState currentFeedback;
  
  // 私有方法
  void initLEDs();
  void updateLEDs();
  void updateScreenFeedback();
  void resetFeedback();
};

#endif // FEEDBACK_MANAGER_H
