#ifndef BUTTON_MANAGER_H
#define BUTTON_MANAGER_H

#include <Arduino.h>
#include "coresystem/config.h"
#include "coresystem/event_bus.h"
#include "coresystem/core_system.h"

// 按键常量定义
#define BUTTON_COUNT 1
#define BUTTON_PINS {0} // 使用引脚0作为按键引脚
#define BUTTON_DEBOUNCE_TIME 50
#define BUTTON_LONG_PRESS_TIME 3000 // 长按时间3秒
#define BUTTON_POWER_OFF_TIME 5000 // 关机时间5秒
#define BUTTON_CLICK_TIMEOUT 300 // 点击超时时间

// 按键事件类型
enum ButtonEvent {
  BUTTON_NONE,
  BUTTON_CLICK,
  BUTTON_DOUBLE_CLICK,
  BUTTON_TRIPLE_CLICK,
  BUTTON_LONG_PRESS,
  BUTTON_POWER_OFF
};

// 按键功能枚举
enum ButtonFunction {
  FUNCTION_NONE,
  FUNCTION_NEXT_PAGE,       // 切换下一页
  FUNCTION_PREVIOUS_PAGE,   // 切换上一页
  FUNCTION_TOGGLE_DISPLAY,  // 切换显示模式
  FUNCTION_ENTER_SETTINGS,   // 进入设置界面
  FUNCTION_TOGGLE_POWER,    // 切换电源状态
  FUNCTION_REFRESH_DISPLAY,  // 刷新显示
  FUNCTION_TOGGLE_WIFI,     // 切换WiFi状态
  FUNCTION_TOGGLE_BLUETOOTH, // 切换蓝牙状态
  FUNCTION_CUSTOM_1,         // 自定义功能1
  FUNCTION_CUSTOM_2,         // 自定义功能2
  FUNCTION_CUSTOM_3,         // 自定义功能3
  FUNCTION_CUSTOM_4,         // 自定义功能4
  FUNCTION_CUSTOM_5          // 自定义功能5
};

// 按键回调函数类型
typedef void (*ButtonCallback)(int buttonIndex, ButtonEvent event);

// 功能回调函数类型
typedef void (*FunctionCallback)(ButtonFunction function);

class ButtonManager {
public:
  ButtonManager();
  ~ButtonManager();
  
  void init();
  void update();
  void loop();
  
  // 设置按键回调函数
  void setCallback(ButtonCallback callback) { this->callback = callback; }
  
  // 设置功能回调函数
  void setFunctionCallback(FunctionCallback callback) { this->functionCallback = callback; }
  
  // 获取按键状态
  bool isButtonPressed(int buttonIndex);
  
  // 按钮映射功能
  bool setButtonMapping(int buttonIndex, ButtonEvent event, ButtonFunction function);
  ButtonFunction getButtonMapping(int buttonIndex, ButtonEvent event);
  bool saveButtonMappings();
  bool loadButtonMappings();
  void resetButtonMappings();
  
private:
  // 按键状态结构体
  struct ButtonState {
    bool currentState;
    bool lastState;
    unsigned long lastChangeTime;
    unsigned long pressStartTime;
    int clickCount;
    unsigned long lastClickTime;
    bool isLongPressDetected;
  };
  
  // 按键状态数组
  ButtonState buttonStates[BUTTON_COUNT];
  
  // 按键引脚数组
  int buttonPins[BUTTON_COUNT];
  
  // 按键回调函数
  ButtonCallback callback;
  
  // 功能回调函数
  FunctionCallback functionCallback;
  
  // 按钮映射数组：buttonIndex -> event -> function
  ButtonFunction buttonMappings[BUTTON_COUNT][6]; // 6种事件类型
  
  // 私有方法
  void readButtons();
  void processButtonEvents();
  void debounceButtons();
  void executeButtonFunction(int buttonIndex, ButtonEvent event);
};

#endif // BUTTON_MANAGER_H