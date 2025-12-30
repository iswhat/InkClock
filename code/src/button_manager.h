#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "coresystem/config.h"

// 按键事件类型
enum ButtonEvent {
  BUTTON_NONE,
  BUTTON_CLICK,
  BUTTON_DOUBLE_CLICK,
  BUTTON_LONG_PRESS
};

// 按键回调函数类型
typedef void (*ButtonCallback)(int buttonIndex, ButtonEvent event);

class ButtonManager {
public:
  ButtonManager();
  ~ButtonManager();
  
  void init();
  void update();
  void loop();
  
  // 设置按键回调函数
  void setCallback(ButtonCallback callback) { this->callback = callback; }
  
  // 获取按键状态
  bool isButtonPressed(int buttonIndex);
  
private:
  // 按键状态结构体
  struct ButtonState {
    bool currentState;
    bool lastState;
    unsigned long lastChangeTime;
    unsigned long pressStartTime;
    int clickCount;
    unsigned long lastClickTime;
  };
  
  // 按键状态数组
  ButtonState buttonStates[BUTTON_COUNT];
  
  // 按键引脚数组
  int buttonPins[BUTTON_COUNT];
  
  // 按键回调函数
  ButtonCallback callback;
  
  // 私有方法
  void readButtons();
  void processButtonEvents();
  void debounceButtons();
};

#endif // BUTTON_MANAGER_H