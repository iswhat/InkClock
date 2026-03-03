#include "hc_sr505_driver.h"
#include "coresystem/platform_abstraction.h"

bool HC_SR505Driver::matchHardware() {
  DEBUG_PRINTLN("检测HC-SR505硬件匹配...");
  
  // 人体感应传感器使用数字输入引脚，尝试检测常见引脚
  int testPins[] = {2, 4, 5, 12, 13, 14, 15, 25, 26, 27, 32, 33}; // 常见的GPIO引脚
  
  for (int testPin : testPins) {
    // 设置引脚模式为输入
    pinMode(testPin, INPUT);
    
    // 读取几次引脚状态，检查是否有信号
    for (int i = 0; i < 10; i++) {
      bool state = digitalRead(testPin);
      
      // 如果检测到高电平（有人或物体移动），则认为硬件匹配
      if (state) {
        DEBUG_PRINTF("HC-SR505硬件匹配成功，引脚: %d\n", testPin);
        return true;
      }
      
      platformDelay(100); // 等待100ms再读取下一次
    }
  }
  
  DEBUG_PRINTLN("未检测到HC-SR505硬件");
  return false;
}

HC_SR505Driver::HC_SR505Driver() : pin(-1), tempOffset(0.0), humOffset(0.0), initialized(false) {
  // 构造函数
}

HC_SR505Driver::~HC_SR505Driver() {
  // 析构函数，清理资源
}

bool HC_SR505Driver::init(const SensorConfig& config) {
  this->config = config;
  
  // 使用配置中的引脚，或默认引脚
  pin = (config.pin != -1) ? config.pin : PIR_SENSOR_PIN;
  
  // 设置引脚模式
  pinMode(pin, INPUT);
  
  initialized = true;
  return true;
}

bool HC_SR505Driver::readData(SensorData& data) {
  if (!initialized) {
    return false;
  }
  
  // 读取人体感应数据
  bool motionDetected = digitalRead(pin);
  
  // 填充传感器数据
  data.valid = true;
  data.timestamp = platformGetMillis();
  data.temperature = 0.0; // HC-SR505不支持温度检测
  data.humidity = 0.0; // HC-SR505不支持湿度检测
  data.motionDetected = motionDetected;
  data.gasLevel = 0; // HC-SR505不支持气体检测
  data.flameDetected = false; // HC-SR505不支持火焰检测
  data.lightLevel = 0; // HC-SR505不支持光照检测
  
  return true;
}

void HC_SR505Driver::calibrate(float tempOffset, float humOffset) {
  this->tempOffset = tempOffset;
  this->humOffset = humOffset;
  // HC-SR505不需要校准，这里只是为了满足接口要求
}

String HC_SR505Driver::getTypeName() const {
  return "HC-SR505人体感应传感器";
}

SensorType HC_SR505Driver::getType() const {
  return SENSOR_TYPE_HC_SR505;
}

void HC_SR505Driver::setConfig(const SensorConfig& config) {
  this->config = config;
  
  // 如果已经初始化，重新初始化传感器
  if (initialized) {
    pinMode(pin, INPUT);
  }
}

SensorConfig HC_SR505Driver::getConfig() const {
  return config;
}