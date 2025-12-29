#include "button_manager.h"
#include "app/display_manager.h"

// 外部全局对象
extern DisplayManager displayManager;

ButtonManager::ButtonManager() {
  // 初始化按键引脚数组
  int pins[] = BUTTON_PINS;
  for (int i = 0; i < BUTTON_COUNT; i++) {
    buttonPins[i] = pins[i];
  }
  
  // 初始化按键状态
  for (int i = 0; i < BUTTON_COUNT; i++) {
    buttonStates[i].currentState = false;
    buttonStates[i].lastState = false;
    buttonStates[i].lastChangeTime = 0;
    buttonStates[i].pressStartTime = 0;
    buttonStates[i].clickCount = 0;
    buttonStates[i].lastClickTime = 0;
  }
  
  // 初始化回调函数
  callback = NULL;
}

ButtonManager::~ButtonManager() {
  // 清理资源
}

void ButtonManager::init() {
  DEBUG_PRINTLN("初始化按键管理器...");
  
  // 设置按键引脚为输入模式，带上拉电阻
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  
  DEBUG_PRINTLN("按键管理器初始化完成");
}

void ButtonManager::update() {
  // 读取按键状态
  readButtons();
  
  // 消抖处理
  debounceButtons();
  
  // 处理按键事件
  processButtonEvents();
}

void ButtonManager::loop() {
  // 定期更新按键状态
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 10) { // 每10ms检查一次按键状态
    lastUpdate = millis();
    update();
  }
}

bool ButtonManager::isButtonPressed(int buttonIndex) {
  if (buttonIndex >= 0 && buttonIndex < BUTTON_COUNT) {
    return buttonStates[buttonIndex].currentState;
  }
  return false;
}

void ButtonManager::readButtons() {
  // 读取所有按键的当前状态
  for (int i = 0; i < BUTTON_COUNT; i++) {
    // 按键按下时为低电平，释放时为高电平
    bool state = !digitalRead(buttonPins[i]);
    
    // 检查状态是否变化
    if (state != buttonStates[i].lastState) {
      buttonStates[i].lastChangeTime = millis();
      buttonStates[i].lastState = state;
    }
  }
}

void ButtonManager::debounceButtons() {
  // 消抖处理，只有当状态稳定一段时间后才更新当前状态
  for (int i = 0; i < BUTTON_COUNT; i++) {
    if (millis() - buttonStates[i].lastChangeTime > BUTTON_DEBOUNCE_TIME) {
      buttonStates[i].currentState = buttonStates[i].lastState;
    }
  }
}

void ButtonManager::processButtonEvents() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    ButtonState &state = buttonStates[i];
    
    // 处理按键按下事件
    if (state.currentState && !state.lastState) {
      state.pressStartTime = millis();
      state.lastState = true;
    }
    
    // 处理按键释放事件
    if (!state.currentState && state.lastState) {
      unsigned long pressDuration = millis() - state.pressStartTime;
      
      if (pressDuration < BUTTON_LONG_PRESS_TIME) {
        // 短按，检查是否是双击
        state.clickCount++;
        state.lastClickTime = millis();
        
        // 延迟一段时间检查是否有第二次点击
        if (state.clickCount == 1) {
          // 第一次点击，等待第二次点击
        } else if (state.clickCount == 2) {
          // 双击事件
          if (callback != NULL) {
            callback(i, BUTTON_DOUBLE_CLICK);
          }
          state.clickCount = 0;
        }
      } else {
        // 长按事件
        if (callback != NULL) {
          callback(i, BUTTON_LONG_PRESS);
        }
        state.clickCount = 0;
      }
      
      state.lastState = false;
    }
    
    // 处理单击事件（延迟一段时间后，如果没有第二次点击）
    if (state.clickCount == 1 && millis() - state.lastClickTime > 300) {
      if (callback != NULL) {
        callback(i, BUTTON_CLICK);
      }
      state.clickCount = 0;
    }
    
    // 处理长按持续事件
    if (state.currentState && state.lastState) {
      unsigned long pressDuration = millis() - state.pressStartTime;
      if (pressDuration >= BUTTON_LONG_PRESS_TIME) {
        // 长按持续事件
        // TODO: 实现长按持续事件处理
      }
    }
  }
}