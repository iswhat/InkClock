#include "touch_manager.h"

// 触摸管理类实现
TouchManager::TouchManager() {
  // 初始化触摸数据
  currentTouch.x = 0;
  currentTouch.y = 0;
  currentTouch.pressed = false;
  currentEvent = TOUCH_EVENT_NONE;
  lastUpdate = 0;
  
  // 初始化校准参数
  xCalibration[0] = 1.0; // 斜率
  xCalibration[1] = 0.0; // 截距
  yCalibration[0] = 1.0; // 斜率
  yCalibration[1] = 0.0; // 截距
  
  // 初始化滤波数组
  for (int i = 0; i < FILTER_SIZE; i++) {
    xFilter[i] = 0;
    yFilter[i] = 0;
  }
  filterIndex = 0;
}

TouchManager::~TouchManager() {
  // 清理资源
}

void TouchManager::init() {
  DEBUG_PRINTLN("初始化触摸管理器...");
  
  // 检查当前硬件是否支持触摸
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    DEBUG_PRINTLN("当前硬件支持触摸功能");
    
    // 初始化触摸引脚
    pinMode(TOUCH_PIN_0, INPUT);
    pinMode(TOUCH_PIN_1, INPUT);
    pinMode(TOUCH_PIN_2, INPUT);
    pinMode(TOUCH_PIN_3, INPUT);
    
    // 校准触摸
    calibrateTouch();
    DEBUG_PRINTLN("触摸管理器初始化完成");
  #else
    DEBUG_PRINTLN("当前硬件不支持触摸功能");
  #endif
}

void TouchManager::update() {
  // 检查当前硬件是否支持触摸
  #if CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_DEFAULT || CURRENT_HARDWARE_MODEL == HARDWARE_MODEL_ESP32_S3_WROOM_1
    // 读取触摸数据
    TouchPoint rawPoint;
    rawPoint.pressed = false;
    
    // 读取触摸引脚状态
    int touchValue0 = touchRead(TOUCH_PIN_0);
    int touchValue1 = touchRead(TOUCH_PIN_1);
    int touchValue2 = touchRead(TOUCH_PIN_2);
    int touchValue3 = touchRead(TOUCH_PIN_3);
    
    // 判断是否触摸
    if (touchValue0 < 50 || touchValue1 < 50 || touchValue2 < 50 || touchValue3 < 50) {
      rawPoint.pressed = true;
      
      // 简化的触摸坐标计算（实际应用中需要更复杂的算法）
      rawPoint.x = map(touchValue0, 0, 100, 0, 800);
      rawPoint.y = map(touchValue1, 0, 100, 0, 480);
      
      // 应用校准
      rawPoint.x = rawPoint.x * xCalibration[0] + xCalibration[1];
      rawPoint.y = rawPoint.y * yCalibration[0] + yCalibration[1];
    }
    
    // 应用滤波
    TouchPoint filteredPoint = filterTouchData(rawPoint);
    
    // 更新触摸事件
    updateTouchEvent(filteredPoint);
    
    lastUpdate = millis();
  #endif
}

void TouchManager::loop() {
  // 定期更新触摸数据
  if (millis() - lastUpdate > 10) { // 10ms更新一次
    update();
  }
}

void TouchManager::reset() {
  currentTouch.x = 0;
  currentTouch.y = 0;
  currentTouch.pressed = false;
  currentEvent = TOUCH_EVENT_NONE;
}

void TouchManager::calibrateTouch() {
  // 简单的触摸校准
  DEBUG_PRINTLN("校准触摸...");
  
  // 默认校准值（实际应用中需要用户手动校准）
  xCalibration[0] = 1.0;
  xCalibration[1] = 0.0;
  yCalibration[0] = 1.0;
  yCalibration[1] = 0.0;
  
  DEBUG_PRINTLN("触摸校准完成");
}

TouchPoint TouchManager::filterTouchData(TouchPoint rawPoint) {
  // 简单的移动平均滤波
  TouchPoint filteredPoint = rawPoint;
  
  if (rawPoint.pressed) {
    // 更新滤波数组
    xFilter[filterIndex] = rawPoint.x;
    yFilter[filterIndex] = rawPoint.y;
    filterIndex = (filterIndex + 1) % FILTER_SIZE;
    
    // 计算平均值
    int xSum = 0;
    int ySum = 0;
    for (int i = 0; i < FILTER_SIZE; i++) {
      xSum += xFilter[i];
      ySum += yFilter[i];
    }
    
    filteredPoint.x = xSum / FILTER_SIZE;
    filteredPoint.y = ySum / FILTER_SIZE;
  }
  
  return filteredPoint;
}

void TouchManager::updateTouchEvent(TouchPoint newPoint) {
  // 更新触摸事件
  if (newPoint.pressed && !currentTouch.pressed) {
    // 按下事件
    currentEvent = TOUCH_EVENT_PRESS;
  } else if (!newPoint.pressed && currentTouch.pressed) {
    // 释放事件
    currentEvent = TOUCH_EVENT_RELEASE;
  } else if (newPoint.pressed && currentTouch.pressed) {
    // 移动事件（如果坐标变化超过阈值）
    int dx = abs(newPoint.x - currentTouch.x);
    int dy = abs(newPoint.y - currentTouch.y);
    if (dx > 5 || dy > 5) {
      currentEvent = TOUCH_EVENT_MOVE;
    } else {
      currentEvent = TOUCH_EVENT_NONE;
    }
  } else {
    currentEvent = TOUCH_EVENT_NONE;
  }
  
  // 更新当前触摸状态
  currentTouch = newPoint;
  
  // 调试输出
  #if DEBUG_ENABLED
    if (currentEvent != TOUCH_EVENT_NONE) {
      DEBUG_PRINT("触摸事件: ");
      switch (currentEvent) {
        case TOUCH_EVENT_PRESS:
          DEBUG_PRINT("PRESS");
          break;
        case TOUCH_EVENT_RELEASE:
          DEBUG_PRINT("RELEASE");
          break;
        case TOUCH_EVENT_MOVE:
          DEBUG_PRINT("MOVE");
          break;
        default:
          DEBUG_PRINT("UNKNOWN");
          break;
      }
      DEBUG_PRINT(" - X: ");
      DEBUG_PRINT(currentTouch.x);
      DEBUG_PRINT(" Y: ");
      DEBUG_PRINTLN(currentTouch.y);
    }
  #endif
}