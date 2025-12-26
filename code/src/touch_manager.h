#ifndef TOUCH_MANAGER_H
#define TOUCH_MANAGER_H

#include <Arduino.h>
#include "config.h"

// 触摸事件类型
enum TouchEventType {
  TOUCH_EVENT_NONE,
  TOUCH_EVENT_PRESS,
  TOUCH_EVENT_RELEASE,
  TOUCH_EVENT_MOVE
};

// 触摸点结构体
typedef struct {
  int x;
  int y;
  bool pressed;
} TouchPoint;

class TouchManager {
private:
  TouchPoint currentTouch;
  TouchEventType currentEvent;
  unsigned long lastUpdate;
  
  // 触摸校准参数
  float xCalibration[2]; // 斜率和截距
  float yCalibration[2]; // 斜率和截距
  
  // 触摸过滤参数
  static const int FILTER_SIZE = 5;
  int xFilter[FILTER_SIZE];
  int yFilter[FILTER_SIZE];
  int filterIndex;
  
  // 私有方法
  void calibrateTouch();
  TouchPoint filterTouchData(TouchPoint rawPoint);
  void updateTouchEvent(TouchPoint newPoint);
  
public:
  TouchManager();
  ~TouchManager();
  
  void init();
  void update();
  void loop();
  
  // 获取触摸状态
  TouchPoint getTouchPoint() { return currentTouch; }
  TouchEventType getTouchEvent() { return currentEvent; }
  
  // 检查是否触摸
  bool isTouched() { return currentTouch.pressed; }
  
  // 重置触摸状态
  void reset();
};

#endif // TOUCH_MANAGER_H