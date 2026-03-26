#ifndef BASE_MQ_SENSOR_DRIVER_H
#define BASE_MQ_SENSOR_DRIVER_H

#include "base_sensor_driver.h"

/**
 * @brief 基础MQ系列气体传感器驱动类
 * 
 * 这个类为所有MQ系列气体传感器提供通用的实现，包括MQ-2、MQ-5、MQ-7、MQ-135等。
 * 具体的传感器驱动可以继承这个类，只需要实现特定的传感器类型和名称即可。
 */
class BaseMQSensorDriver : public BaseSensorDriver {
protected:
  String typeName;               ///< 传感器类型名称
  int threshold;                 ///< 检测阈值
  int pin;                       ///< 传感器引脚
  float r0;                      ///< 传感器在清洁空气中的电阻
  float rLoad;                   ///< 负载电阻值
  float calibrationFactor;       ///< 校准因子
  float temperature;             ///< 环境温度
  float humidity;                ///< 环境湿度
  
  // 历史数据用于滤波
  int historyValues[10];
  int historyIndex;
  bool historyInitialized;
  
public:
  /**
   * @brief 构造函数
   * 
   * @param typeName 传感器类型名称
   */
  BaseMQSensorDriver(const String& typeName) : BaseSensorDriver(), 
    typeName(typeName), threshold(512), pin(-1), 
    r0(10000.0), rLoad(10000.0), calibrationFactor(1.0),
    temperature(25.0), humidity(50.0), historyIndex(0), historyInitialized(false) {
    // 初始化历史数据
    for (int i = 0; i < 10; i++) {
      historyValues[i] = 0;
    }
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
    // 调用基类初始化
    if (!BaseSensorDriver::init(config)) {
      return false;
    }
    
    // 使用配置中的引脚，或默认引脚
    pin = (this->config.pin != -1) ? this->config.pin : 36; // 使用GPIO36作为默认模拟输入引脚
    
    // 设置传感器引脚为输入
    pinMode(pin, INPUT);
    
    // 如果配置了气体阈值，则使用配置的阈值
    if (this->config.gasThreshold > 0) {
      threshold = this->config.gasThreshold;
    }
    
    // 预热传感器
    Serial.printf("%s气体传感器预热中...\n", typeName.c_str());
    for (int i = 0; i < 30; i++) {
      analogRead(pin); // 读取值以预热
      delay(1000);
      if (i % 10 == 0) {
        Serial.printf("预热进度: %d%%\n", (i/30)*100);
      }
    }
    
    // 初始化历史数据
    for (int i = 0; i < 10; i++) {
      historyValues[i] = analogRead(pin);
      delay(100);
    }
    historyInitialized = true;
    
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
    if (!isInitialized()) {
      recordError();
      return false;
    }
    
    // 读取模拟值
    int rawValue = analogRead(pin);
    
    // 应用移动平均滤波
    int filteredValue = applyFilter(rawValue);
    
    // 计算气体浓度
    float gasConcentration = calculateGasConcentration(filteredValue);
    
    // 使用基类的fillSensorData方法填充数据
    fillSensorData(data, 0.0, 0.0, false, filteredValue, false, 0);
    
    // 根据传感器类型设置相应的气体浓度值
    setGasConcentration(data, gasConcentration);
    
    recordSuccess();
    return true;
  }
  
  /**
   * @brief 校准传感器
   * 
   * 在清洁空气中校准传感器的零点
   * 
   * @return 校准是否成功
   */
  bool calibrate() {
    if (!isInitialized()) {
      return false;
    }
    
    Serial.printf("%s气体传感器校准中...\n", typeName.c_str());
    
    // 读取多次值并平均
    int sum = 0;
    for (int i = 0; i < 50; i++) {
      sum += analogRead(pin);
      delay(100);
    }
    int avgValue = sum / 50;
    
    // 计算R0值（清洁空气中的电阻）
    float vout = (float)avgValue * 3.3 / 4095.0;
    float rs = (3.3 - vout) * rLoad / vout;
    r0 = rs / getCleanAirRatio();
    
    Serial.printf("%s气体传感器校准完成，R0: %.2f欧姆\n", typeName.c_str(), r0);
    return true;
  }
  
  /**
   * @brief 设置环境参数
   * 
   * @param temp 温度（摄氏度）
   * @param hum 湿度（百分比）
   */
  void setEnvironment(float temp, float hum) {
    temperature = temp;
    humidity = hum;
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
   * @brief 硬件匹配检测
   * 
   * 检测传感器是否与当前硬件匹配
   * 
   * @return 硬件是否匹配
   */
  bool matchHardware() override {
    DEBUG_PRINTF("检测%s硬件匹配...\n", typeName.c_str());
    
    // MQ系列传感器使用模拟输入引脚，尝试常见的引脚
    int testPins[] = {36, 37, 38, 39, 32, 33}; // ESP32常见的模拟输入引脚
    
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
  }
  
protected:
  /**
   * @brief 应用移动平均滤波
   * 
   * @param value 当前读取值
   * @return 滤波后的值
   */
  int applyFilter(int value) {
    if (!historyInitialized) {
      return value;
    }
    
    // 更新历史数据
    historyValues[historyIndex] = value;
    historyIndex = (historyIndex + 1) % 10;
    
    // 计算平均值
    int sum = 0;
    for (int i = 0; i < 10; i++) {
      sum += historyValues[i];
    }
    return sum / 10;
  }
  
  /**
   * @brief 计算气体浓度
   * 
   * @param rawValue 原始模拟值
   * @return 气体浓度
   */
  float calculateGasConcentration(int rawValue) {
    // 计算传感器电阻
    float vout = (float)rawValue * 3.3 / 4095.0;
    float rs = (3.3 - vout) * rLoad / vout;
    
    // 计算Rs/R0比值
    float ratio = rs / r0;
    
    // 应用温度和湿度补偿
    float compensatedRatio = compensateTemperatureHumidity(ratio);
    
    // 根据传感器类型计算气体浓度
    return calculateSpecificGasConcentration(compensatedRatio);
  }
  
  /**
   * @brief 温度和湿度补偿
   * 
   * @param ratio Rs/R0比值
   * @return 补偿后的比值
   */
  float compensateTemperatureHumidity(float ratio) {
    // 简单的温度湿度补偿公式
    // 实际应用中可能需要更复杂的补偿算法
    float tempCompensation = 1.0 + 0.01 * (temperature - 25.0);
    float humCompensation = 1.0 + 0.005 * (50.0 - humidity);
    return ratio * tempCompensation * humCompensation;
  }
  
  /**
   * @brief 获取清洁空气中的Rs/R0比值
   * 
   * @return 清洁空气中的比值
   */
  virtual float getCleanAirRatio() {
    // 默认值，具体传感器需要重写
    return 1.0;
  }
  
  /**
   * @brief 计算特定气体的浓度
   * 
   * @param ratio Rs/R0比值
   * @return 气体浓度
   */
  virtual float calculateSpecificGasConcentration(float ratio) {
    // 默认实现，具体传感器需要重写
    return ratio * 1000;
  }
  
  /**
   * @brief 根据传感器类型设置气体浓度值
   * 
   * @param data 传感器数据结构
   * @param concentration 气体浓度
   */
  virtual void setGasConcentration(SensorData& data, float concentration) {
    // 默认设置为gasLevel，具体传感器可以设置其他字段
    data.gasLevel = (int)concentration;
  }
};

#endif // BASE_MQ_SENSOR_DRIVER_H