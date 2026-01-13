#include "feedback_manager.h"
#include "display_driver.h"

FeedbackManager::FeedbackManager() {
  // 初始化LED引脚
  powerLEDPin = -1;
  wifiLEDPin = -1;
  bluetoothLEDPin = -1;
  
  // 初始化反馈模式
  feedbackMode = FEEDBACK_MODE_BOTH;
  
  // 初始化屏幕驱动
  displayDriver = nullptr;
  
  // 初始化反馈状态
  currentFeedback = {
    FEEDBACK_NONE,
    0,
    0,
    0,
    false
  };
}

FeedbackManager::~FeedbackManager() {
  // 清理资源
}

bool FeedbackManager::init() {
  // 初始化LED
  initLEDs();
  
  // 初始化反馈状态
  resetFeedback();
  
  return true;
}

void FeedbackManager::initLEDs() {
  // 初始化LED引脚
  if (powerLEDPin != -1) {
    pinMode(powerLEDPin, OUTPUT);
    digitalWrite(powerLEDPin, LOW); // 默认关闭
  }
  
  if (wifiLEDPin != -1) {
    pinMode(wifiLEDPin, OUTPUT);
    digitalWrite(wifiLEDPin, LOW); // 默认关闭
  }
  
  if (bluetoothLEDPin != -1) {
    pinMode(bluetoothLEDPin, OUTPUT);
    digitalWrite(bluetoothLEDPin, LOW); // 默认关闭
  }
}

void FeedbackManager::triggerFeedback(FeedbackType type, FeedbackMode mode) {
  // 更新反馈状态
  currentFeedback = {
    type,
    millis(),
    millis(),
    0,
    true
  };
  
  // 更新反馈模式
  if (mode != FEEDBACK_MODE_NONE) {
    feedbackMode = mode;
  }
  
  // 立即更新反馈
  update();
}

void FeedbackManager::setFeedbackMode(FeedbackMode mode) {
  feedbackMode = mode;
}

FeedbackMode FeedbackManager::getFeedbackMode() const {
  return feedbackMode;
}

void FeedbackManager::update() {
  if (!currentFeedback.active) {
    return;
  }
  
  unsigned long currentTime = millis();
  
  // 检查反馈是否应该结束
  if (currentTime - currentFeedback.startTime > 2000) { // 2秒后结束反馈
    resetFeedback();
    return;
  }
  
  // 检查是否需要更新
  if (currentTime - currentFeedback.lastUpdateTime < 50) { // 50ms更新一次
    return;
  }
  
  // 更新LED反馈
  if (feedbackMode == FEEDBACK_MODE_LED || feedbackMode == FEEDBACK_MODE_BOTH) {
    updateLEDs();
  }
  
  // 更新屏幕反馈
  if (feedbackMode == FEEDBACK_MODE_SCREEN || feedbackMode == FEEDBACK_MODE_BOTH) {
    updateScreenFeedback();
  }
  
  // 更新时间戳和动画帧
  currentFeedback.lastUpdateTime = currentTime;
  currentFeedback.animationFrame++;
}

void FeedbackManager::updateLEDs() {
  // 根据反馈类型控制LED
  switch (currentFeedback.type) {
    case FEEDBACK_CLICK:
      // 单击：电源LED闪烁一次
      if (powerLEDPin != -1) {
        digitalWrite(powerLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
      }
      break;
      
    case FEEDBACK_DOUBLE_CLICK:
      // 双击：WiFi LED闪烁两次
      if (wifiLEDPin != -1) {
        digitalWrite(wifiLEDPin, currentFeedback.animationFrame % 4 < 2 ? HIGH : LOW);
      }
      break;
      
    case FEEDBACK_TRIPLE_CLICK:
      // 三连击：蓝牙LED闪烁三次
      if (bluetoothLEDPin != -1) {
        digitalWrite(bluetoothLEDPin, currentFeedback.animationFrame % 6 < 3 ? HIGH : LOW);
      }
      break;
      
    case FEEDBACK_LONG_PRESS:
      // 长按：所有LED同时闪烁
      if (powerLEDPin != -1) {
        digitalWrite(powerLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
      }
      if (wifiLEDPin != -1) {
        digitalWrite(wifiLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
      }
      if (bluetoothLEDPin != -1) {
        digitalWrite(bluetoothLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
      }
      break;
      
    case FEEDBACK_POWER_OFF:
      // 关机：所有LED快速闪烁三次后关闭
      if (currentFeedback.animationFrame < 6) {
        if (powerLEDPin != -1) {
          digitalWrite(powerLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
        }
        if (wifiLEDPin != -1) {
          digitalWrite(wifiLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
        }
        if (bluetoothLEDPin != -1) {
          digitalWrite(bluetoothLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
        }
      } else {
        // 关闭所有LED
        if (powerLEDPin != -1) {
          digitalWrite(powerLEDPin, LOW);
        }
        if (wifiLEDPin != -1) {
          digitalWrite(wifiLEDPin, LOW);
        }
        if (bluetoothLEDPin != -1) {
          digitalWrite(bluetoothLEDPin, LOW);
        }
      }
      break;
      
    case FEEDBACK_SUCCESS:
      // 成功：绿色LED（WiFi）常亮
      if (wifiLEDPin != -1) {
        digitalWrite(wifiLEDPin, HIGH);
      }
      break;
      
    case FEEDBACK_ERROR:
      // 错误：红色LED（电源）闪烁
      if (powerLEDPin != -1) {
        digitalWrite(powerLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
      }
      break;
      
    case FEEDBACK_WARNING:
      // 警告：黄色LED（蓝牙）闪烁
      if (bluetoothLEDPin != -1) {
        digitalWrite(bluetoothLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
      }
      break;
      
    case FEEDBACK_INFO:
      // 信息：蓝色LED（WiFi）闪烁
      if (wifiLEDPin != -1) {
        digitalWrite(wifiLEDPin, currentFeedback.animationFrame % 2 == 0 ? HIGH : LOW);
      }
      break;
      
    default:
      break;
  }
}

void FeedbackManager::updateScreenFeedback() {
  if (!displayDriver) {
    return;
  }
  
  IDisplayDriver* driver = static_cast<IDisplayDriver*>(displayDriver);
  int16_t width = driver->getWidth();
  int16_t height = driver->getHeight();
  
  // 根据反馈类型绘制屏幕效果
  switch (currentFeedback.type) {
    case FEEDBACK_CLICK:
      // 单击：屏幕中央闪烁一次
      if (currentFeedback.animationFrame % 2 == 0) {
        driver->fillRect(width/2 - 10, height/2 - 10, 20, 20, 0x0000); // 黑色
      } else {
        driver->fillRect(width/2 - 10, height/2 - 10, 20, 20, 0xFFFF); // 白色
      }
      driver->update(width/2 - 15, height/2 - 15, 30, 30);
      break;
      
    case FEEDBACK_DOUBLE_CLICK:
      // 双击：屏幕中央闪烁两次
      if (currentFeedback.animationFrame % 4 < 2) {
        driver->fillRect(width/2 - 15, height/2 - 15, 30, 30, 0x0000); // 黑色
      } else {
        driver->fillRect(width/2 - 15, height/2 - 15, 30, 30, 0xFFFF); // 白色
      }
      driver->update(width/2 - 20, height/2 - 20, 40, 40);
      break;
      
    case FEEDBACK_TRIPLE_CLICK:
      // 三连击：屏幕中央闪烁三次
      if (currentFeedback.animationFrame % 6 < 3) {
        driver->fillRect(width/2 - 20, height/2 - 20, 40, 40, 0x0000); // 黑色
      } else {
        driver->fillRect(width/2 - 20, height/2 - 20, 40, 40, 0xFFFF); // 白色
      }
      driver->update(width/2 - 25, height/2 - 25, 50, 50);
      break;
      
    case FEEDBACK_LONG_PRESS:
      // 长按：屏幕边缘闪烁
      if (currentFeedback.animationFrame % 2 == 0) {
        driver->drawRect(5, 5, width - 10, height - 10, 0x0000); // 黑色边框
      } else {
        driver->drawRect(5, 5, width - 10, height - 10, 0xFFFF); // 白色边框
      }
      driver->update(0, 0, width, height);
      break;
      
    case FEEDBACK_POWER_OFF:
      // 关机：屏幕中央显示关机图标
      driver->fillRect(width/2 - 40, height/2 - 40, 80, 80, 0xFFFF); // 白色背景
      driver->drawLine(width/2 - 30, height/2 - 30, width/2 + 30, height/2 + 30, 0x0000); // 黑色
      driver->drawLine(width/2 + 30, height/2 - 30, width/2 - 30, height/2 + 30, 0x0000); // 黑色
      driver->update(width/2 - 45, height/2 - 45, 90, 90);
      break;
      
    case FEEDBACK_SUCCESS:
      // 成功：屏幕中央显示绿色对勾
      driver->fillRect(width/2 - 30, height/2 - 30, 60, 60, 0xFFFF); // 白色背景
      driver->drawLine(width/2 - 20, height/2, width/2, height/2 + 20, 0x0000); // 黑色
      driver->drawLine(width/2, height/2 + 20, width/2 + 20, height/2 - 20, 0x0000); // 黑色
      driver->update(width/2 - 35, height/2 - 35, 70, 70);
      break;
      
    case FEEDBACK_ERROR:
      // 错误：屏幕中央显示红色叉号
      driver->fillRect(width/2 - 30, height/2 - 30, 60, 60, 0xFFFF); // 白色背景
      driver->drawLine(width/2 - 20, height/2 - 20, width/2 + 20, height/2 + 20, 0x0000); // 黑色
      driver->drawLine(width/2 + 20, height/2 - 20, width/2 - 20, height/2 + 20, 0x0000); // 黑色
      driver->update(width/2 - 35, height/2 - 35, 70, 70);
      break;
      
    case FEEDBACK_WARNING:
      // 警告：屏幕中央显示黄色感叹号
      driver->fillRect(width/2 - 30, height/2 - 30, 60, 60, 0xFFFF); // 白色背景
      driver->fillRect(width/2 - 5, height/2 - 20, 10, 40, 0x0000); // 黑色
      driver->fillCircle(width/2, height/2 + 20, 5, 0x0000); // 黑色
      driver->update(width/2 - 35, height/2 - 35, 70, 70);
      break;
      
    case FEEDBACK_INFO:
      // 信息：屏幕中央显示蓝色信息图标
      driver->fillRect(width/2 - 30, height/2 - 30, 60, 60, 0xFFFF); // 白色背景
      driver->fillCircle(width/2, height/2 - 10, 10, 0x0000); // 黑色
      driver->fillRect(width/2 - 5, height/2, 10, 20, 0x0000); // 黑色
      driver->update(width/2 - 35, height/2 - 35, 70, 70);
      break;
      
    default:
      break;
  }
}

void FeedbackManager::resetFeedback() {
  // 重置LED状态
  if (powerLEDPin != -1) {
    digitalWrite(powerLEDPin, LOW);
  }
  if (wifiLEDPin != -1) {
    digitalWrite(wifiLEDPin, LOW);
  }
  if (bluetoothLEDPin != -1) {
    digitalWrite(bluetoothLEDPin, LOW);
  }
  
  // 重置反馈状态
  currentFeedback = {
    FEEDBACK_NONE,
    0,
    0,
    0,
    false
  };
}

void FeedbackManager::setLEDPins(int powerLEDPin, int wifiLEDPin, int bluetoothLEDPin) {
  this->powerLEDPin = powerLEDPin;
  this->wifiLEDPin = wifiLEDPin;
  this->bluetoothLEDPin = bluetoothLEDPin;
  
  // 重新初始化LED
  initLEDs();
}

void FeedbackManager::setDisplayDriver(void* displayDriver) {
  this->displayDriver = displayDriver;
}
