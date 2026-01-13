#include "button_manager.h"
#include "application/display_manager.h"

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
    buttonStates[i].isLongPressDetected = false;
  }
  
  // 初始化回调函数
  callback = NULL;
  functionCallback = NULL;
  
  // 初始化按钮映射
  resetButtonMappings();
}

ButtonManager::~ButtonManager() {
  // 清理资源
}

void ButtonManager::resetButtonMappings() {
  DEBUG_PRINTLN("重置按钮映射为默认值");
  
  // 设置默认的按钮映射
  for (int i = 0; i < BUTTON_COUNT; i++) {
    // 单击：切换下一页
    buttonMappings[i][BUTTON_CLICK] = FUNCTION_NEXT_PAGE;
    // 双击：切换显示模式
    buttonMappings[i][BUTTON_DOUBLE_CLICK] = FUNCTION_TOGGLE_DISPLAY;
    // 三连击：进入设置界面
    buttonMappings[i][BUTTON_TRIPLE_CLICK] = FUNCTION_ENTER_SETTINGS;
    // 长按：切换电源状态
    buttonMappings[i][BUTTON_LONG_PRESS] = FUNCTION_TOGGLE_POWER;
    // 关机：保持默认功能
    buttonMappings[i][BUTTON_POWER_OFF] = FUNCTION_TOGGLE_POWER;
  }
}

void ButtonManager::init() {
  DEBUG_PRINTLN("初始化按键管理器...");
  
  // 设置按键引脚为输入模式，带上拉电阻
  for (int i = 0; i < BUTTON_COUNT; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
  }
  
  // 加载按钮映射
  loadButtonMappings();
  
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
  if (platformGetMillis() - lastUpdate > 10) { // 每10ms检查一次按键状态
    lastUpdate = platformGetMillis();
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
      buttonStates[i].lastChangeTime = platformGetMillis();
      buttonStates[i].lastState = state;
    }
  }
}

void ButtonManager::executeButtonFunction(int buttonIndex, ButtonEvent event) {
  // 执行按钮映射的功能
  ButtonFunction function = buttonMappings[buttonIndex][event];
  if (function != FUNCTION_NONE && functionCallback != NULL) {
    functionCallback(function);
  }
}

bool ButtonManager::setButtonMapping(int buttonIndex, ButtonEvent event, ButtonFunction function) {
  if (buttonIndex < 0 || buttonIndex >= BUTTON_COUNT) {
    return false;
  }
  
  if (event < BUTTON_CLICK || event > BUTTON_POWER_OFF) {
    return false;
  }
  
  buttonMappings[buttonIndex][event] = function;
  DEBUG_PRINTF("设置按钮%d的%s事件映射为%s\n", buttonIndex, 
               event == BUTTON_CLICK ? "单击" : 
               event == BUTTON_DOUBLE_CLICK ? "双击" : 
               event == BUTTON_TRIPLE_CLICK ? "三连击" : 
               event == BUTTON_LONG_PRESS ? "长按" : "关机",
               function == FUNCTION_NONE ? "无" :
               function == FUNCTION_NEXT_PAGE ? "下一页" :
               function == FUNCTION_PREVIOUS_PAGE ? "上一页" :
               function == FUNCTION_TOGGLE_DISPLAY ? "切换显示" :
               function == FUNCTION_ENTER_SETTINGS ? "设置" :
               function == FUNCTION_TOGGLE_POWER ? "电源" :
               function == FUNCTION_REFRESH_DISPLAY ? "刷新" :
               function == FUNCTION_TOGGLE_WIFI ? "WiFi" :
               function == FUNCTION_TOGGLE_BLUETOOTH ? "蓝牙" :
               function == FUNCTION_CUSTOM_1 ? "自定义1" :
               function == FUNCTION_CUSTOM_2 ? "自定义2" :
               function == FUNCTION_CUSTOM_3 ? "自定义3" :
               function == FUNCTION_CUSTOM_4 ? "自定义4" : "自定义5"
              );
  
  return true;
}

ButtonFunction ButtonManager::getButtonMapping(int buttonIndex, ButtonEvent event) {
  if (buttonIndex < 0 || buttonIndex >= BUTTON_COUNT) {
    return FUNCTION_NONE;
  }
  
  if (event < BUTTON_CLICK || event > BUTTON_POWER_OFF) {
    return FUNCTION_NONE;
  }
  
  return buttonMappings[buttonIndex][event];
}

bool ButtonManager::saveButtonMappings() {
  DEBUG_PRINTLN("保存按钮映射到文件");
  
  // 这里可以实现将按钮映射保存到SPIFFS或其他存储介质
  // 暂时返回true，表示保存成功
  return true;
}

bool ButtonManager::loadButtonMappings() {
  DEBUG_PRINTLN("从文件加载按钮映射");
  
  // 这里可以实现从SPIFFS或其他存储介质加载按钮映射
  // 暂时返回false，表示使用默认映射
  return false;
}

void ButtonManager::debounceButtons() {
  // 消抖处理，只有当状态稳定一段时间后才更新当前状态
  for (int i = 0; i < BUTTON_COUNT; i++) {
    if (platformGetMillis() - buttonStates[i].lastChangeTime > BUTTON_DEBOUNCE_TIME) {
      buttonStates[i].currentState = buttonStates[i].lastState;
    }
  }
}

void ButtonManager::processButtonEvents() {
  for (int i = 0; i < BUTTON_COUNT; i++) {
    ButtonState &state = buttonStates[i];
    
    // 处理按键按下事件
    if (state.currentState && !state.lastState) {
      state.pressStartTime = platformGetMillis();
      state.lastState = true;
      state.isLongPressDetected = false;
    }
    
    // 处理按键释放事件
    if (!state.currentState && state.lastState) {
      unsigned long pressDuration = platformGetMillis() - state.pressStartTime;
      
      if (pressDuration < BUTTON_LONG_PRESS_TIME) {
        // 短按，检查点击次数
        state.clickCount++;
        state.lastClickTime = platformGetMillis();
        
        // 三连击事件
        if (state.clickCount == 3) {
          if (callback != NULL) {
            callback(i, BUTTON_TRIPLE_CLICK);
          }
          // 执行按钮映射的功能
          executeButtonFunction(i, BUTTON_TRIPLE_CLICK);
          state.clickCount = 0;
        }
        // 双击和单击事件会在后续检查中处理
      } else if (pressDuration >= BUTTON_LONG_PRESS_TIME && pressDuration < BUTTON_POWER_OFF_TIME) {
        // 处理长按事件（3-5秒）
        if (callback != NULL) {
          callback(i, BUTTON_LONG_PRESS);
        }
        // 执行按钮映射的功能
        executeButtonFunction(i, BUTTON_LONG_PRESS);
        state.clickCount = 0;
      } else if (pressDuration >= BUTTON_POWER_OFF_TIME && pressDuration < 10000) {
        // 处理关机事件（5秒以上，10秒以下）
        if (callback != NULL) {
          callback(i, BUTTON_POWER_OFF);
        }
        // 执行按钮映射的功能
        executeButtonFunction(i, BUTTON_POWER_OFF);
        state.clickCount = 0;
      } else if (pressDuration >= 10000) {
        // 长按持续时间超过10秒，触发恢复出厂设置
        DEBUG_PRINTLN("长按超过10秒，触发恢复出厂设置");
        // 发布恢复出厂设置事件
        EventBus* eventBus = EventBus::getInstance();
        eventBus->publish(EVENT_SYSTEM_RESET, nullptr);
        
        // 直接调用CoreSystem的resetConfig和reset方法
        CoreSystem* coreSystem = CoreSystem::getInstance();
        coreSystem->resetConfig();
        coreSystem->reset();
        state.clickCount = 0;
      }
      
      state.lastState = false;
    }
    
    // 处理双击事件（延迟一段时间后，如果没有第三次点击）
    if (state.clickCount == 2 && platformGetMillis() - state.lastClickTime > BUTTON_CLICK_TIMEOUT) {
      if (callback != NULL) {
        callback(i, BUTTON_DOUBLE_CLICK);
      }
      // 执行按钮映射的功能
      executeButtonFunction(i, BUTTON_DOUBLE_CLICK);
      state.clickCount = 0;
    }
    
    // 处理单击事件（延迟一段时间后，如果没有第二次点击）
    if (state.clickCount == 1 && platformGetMillis() - state.lastClickTime > BUTTON_CLICK_TIMEOUT) {
      if (callback != NULL) {
        callback(i, BUTTON_CLICK);
      }
      // 执行按钮映射的功能
      executeButtonFunction(i, BUTTON_CLICK);
      state.clickCount = 0;
    }
    
    // 处理长按持续事件
    if (state.currentState && state.lastState) {
      unsigned long pressDuration = platformGetMillis() - state.pressStartTime;
      if (pressDuration >= BUTTON_LONG_PRESS_TIME && !state.isLongPressDetected) {
        // 长按持续事件，用于LED状态反馈
        state.isLongPressDetected = true;
        // 这里可以添加LED状态反馈逻辑
        // 例如：每500毫秒闪烁一次LED
        DEBUG_PRINTLN("长按持续中，LED状态反馈");
      }
    }
  }
}