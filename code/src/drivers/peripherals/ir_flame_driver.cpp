#include "ir_flame_driver.h"

IRFlameDriver::IRFlameDriver() : pin(-1), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

IRFlameDriver::~IRFlameDriver() {
  // 析构函数，清理资源
}

bool IRFlameDriver::init(const SensorConfig& config) {
  this->config = config;
  
  // 使用配置中的引脚，或默认引脚
  pin = (config.pin != -1) ? config.pin : FLAME_SENSOR_PIN;
  
  // 设置引脚模式
  pinMode(pin, INPUT);
  
  initialized = true;
  return true;
}

bool IRFlameDriver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取火焰检测数据
  // 注意：IR火焰传感器通常是低电平有效，检测到火焰时输出LOW
  bool flameDetected = !digitalRead(pin);
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = platformGetMillis();
  data.temperature = 0.0; // IR火焰传感器不支持温度检测
  data.humidity = 0.0; // IR火焰传感器不支持湿度检测
  data.motionDetected = false; // IR火焰传感器不支持人体感应
  data.gasLevel = 0; // IR火焰传感器不支持气体检测
  data.flameDetected = flameDetected;
  data.lightLevel = 0; // IR火焰传感器不支持光照检测
  
  return true;
}

void IRFlameDriver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
  // IR火焰传感器不需要校准，这里只是为了满足接口要求
}

String IRFlameDriver::getTypeName() const {
  return "IR火焰传感器";
}

SensorType IRFlameDriver::getType() const {
  return SENSOR_TYPE_FLAME_IR;
}

void IRFlameDriver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    pinMode(pin, INPUT);
  }
}

SensorConfig IRFlameDriver::getConfig() const {
  return config;
}

bool IRFlameDriver::matchHardware() {
  DEBUG_PRINTLN("检测IR火焰传感器硬件匹配...");
  
  try {
    // 火焰传感器使用数字输入引脚，尝试检测常见引脚
    int testPins[] = {2, 4, 5, 12, 13, 14, 15, 25, 26, 27, 32, 33}; // 常见的GPIO引脚
    
    for (int testPin : testPins) {
      // 设置引脚模式为输入
      pinMode(testPin, INPUT);
      
      // 读取几次引脚状态，检查是否有稳定信号
      bool hasStableSignal = true;
      int previousState = digitalRead(testPin);
      
      for (int i = 0; i < 10; i++) {
        int currentState = digitalRead(testPin);
        
        // 如果状态变化超过2次，可能不是火焰传感器
        if (currentState != previousState) {
          hasStableSignal = false;
          break;
        }
        
        previousState = currentState;
        platformDelay(100); // 等待100ms再读取下一次
      }
      
      // 如果有稳定信号，认为硬件匹配
      if (hasStableSignal) {
        DEBUG_PRINTF("IR火焰传感器硬件匹配成功，引脚: %d\n", testPin);
        return true;
      }
    }
    
    DEBUG_PRINTLN("未检测到IR火焰传感器硬件");
    return false;
  } catch (const std::exception& e) {
    DEBUG_PRINTLN("IR火焰传感器硬件匹配失败: " + String(e.what()));
    return false;
  } catch (...) {
    DEBUG_PRINTLN("IR火焰传感器硬件匹配失败: 未知异常");
    return false;
  }
}