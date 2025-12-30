#ifndef BASE_MQ_SENSOR_DRIVER_H
#define BASE_MQ_SENSOR_DRIVER_H

#include "sensor_driver.h"

/**
 * @brief 基础MQ系列气体传感器驱动类
 * 
 * 这个类为所有MQ系列气体传感器提供通用的实现，包括MQ-2、MQ-5、MQ-7、MQ-135等。
 * 具体的传感器驱动可以继承这个类，只需要实现特定的传感器类型和名称即可。
 */
class BaseMQSensorDriver : public ISensorDriver {
protected:
  SensorConfig config;            ///< 传感器配置
  bool initialized;              ///< 初始化状态标志
  String typeName;               ///< 传感器类型名称
  int threshold;                 ///< 检测阈值
  int pin;                       ///< 传感器引脚
  
public:
  /**
   * @brief 构造函数
   * 
   * @param typeName 传感器类型名称
   */
  BaseMQSensorDriver(const String& typeName) : initialized(false), typeName(typeName), threshold(512), pin(-1) {
  }
  
  /**
   * @brief 析构函数
   */
  virtual ~BaseMQSensorDriver() {
  }
  
  /**
   * @brief 初始化传感器
   * 
   * @param config 传感器配置
   * @return 初始化是否成功
   */
  bool init(const SensorConfig& config) override {
    // 保存配置
    this->config = config;
    
    // 使用配置中的引脚，或默认引脚
    pin = (config.pin != -1) ? config.pin : A0;
    
    // 设置传感器引脚为输入
    pinMode(pin, INPUT);
    initialized = true;
    
    // 如果配置了气体阈值，则使用配置的阈值
    if (config.gasThreshold > 0) {
      threshold = config.gasThreshold;
    }
    
    Serial.printf("%s气体传感器初始化成功，引脚: %d，阈值: %d\n", typeName.c_str(), pin, threshold);
    
    return true;
  }
  
  /**
   * @brief 读取传感器数据
   * 
   * @param data 传感器数据结构，用于存储读取到的数据
   * @return 读取是否成功
   */
  bool readData(SensorData& data) override {
    if (!initialized) {
      return false;
    }
    
    // 读取模拟值
    int gasValue = analogRead(pin);
    
    // 设置传感器数据
    data.valid = true;
    data.timestamp = millis();
    data.gasLevel = gasValue;
    data.temperature = 0.0; // MQ系列传感器不支持温度检测
    data.humidity = 0.0; // MQ系列传感器不支持湿度检测
    data.motionDetected = false; // MQ系列传感器不支持人体感应
    data.flameDetected = false; // MQ系列传感器不支持火焰检测
    data.lightLevel = 0; // MQ系列传感器不支持光照检测
    
    return true;
  }
  
  /**
   * @brief 校准传感器
   * 
   * MQ系列传感器不需要温湿度校准，所以这里不做任何操作
   * 
   * @param tempOffset 温度偏移量
   * @param humOffset 湿度偏移量
   */
  void calibrate(float tempOffset, float humOffset) override {
    // MQ系列传感器不需要温湿度校准
  }
  
  /**
   * @brief 获取传感器类型名称
   * 
   * @return 传感器类型名称
   */
  String getTypeName() const override {
    return typeName;
  }
  
  /**
   * @brief 设置传感器配置
   * 
   * @param config 传感器配置
   */
  void setConfig(const SensorConfig& config) override {
    this->config = config;
    // 重新初始化传感器
    init(config);
  }
  
  /**
   * @brief 获取传感器配置
   * 
   * @return 传感器配置
   */
  SensorConfig getConfig() const override {
    return config;
  }
  
  /**
   * @brief 硬件匹配检测
   * 
   * 检测传感器是否与当前硬件匹配
   * 
   * @return 硬件是否匹配
   */
  bool matchHardware() override {
    DEBUG_PRINTF("检测%s硬件匹配...\n", typeName.c_str());
    
    try {
      // MQ系列传感器使用模拟输入引脚，尝试常见的引脚
      int testPins[] = {A0, A1, A2, A3, A4, A5};
      
      for (int pin : testPins) {
        // 设置引脚模式为输入
        pinMode(pin, INPUT);
        
        // 读取多次值，检查是否有合理的变化
        int values[10];
        for (int i = 0; i < 10; i++) {
          values[i] = analogRead(pin);
          delay(100);
        }
        
        // 计算标准差，检查数据是否有合理的变化
        float sum = 0;
        for (int value : values) {
          sum += value;
        }
        float avg = sum / 10;
        
        float variance = 0;
        for (int value : values) {
          variance += pow(value - avg, 2);
        }
        variance /= 10;
        float stdDev = sqrt(variance);
        
        // 如果标准差在合理范围内，说明可能连接了传感器
        if (stdDev > 5 && stdDev < 200) {
          DEBUG_PRINTF("%s硬件匹配成功，引脚: %d\n", typeName.c_str(), pin);
          return true;
        }
      }
      
      DEBUG_PRINTF("未检测到%s硬件\n", typeName.c_str());
      return false;
    } catch (const std::exception& e) {
      DEBUG_PRINTF("%s硬件匹配失败: %s\n", typeName.c_str(), e.what());
      return false;
    } catch (...) {
      DEBUG_PRINTF("%s硬件匹配未知错误\n", typeName.c_str());
      return false;
    }
  }
};

#endif // BASE_MQ_SENSOR_DRIVER_H