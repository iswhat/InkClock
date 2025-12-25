#ifndef SENSOR_MANAGER_H
#define SENSOR_MANAGER_H

#include <Arduino.h>
#include "config.h"

// 传感器数据结构
typedef struct {
  float temperature; // 温度（摄氏度）
  float humidity;    // 湿度（%）
  bool valid;        // 数据是否有效
  unsigned long timestamp; // 数据采集时间戳
} SensorData;

class SensorManager {
public:
  SensorManager();
  ~SensorManager();
  
  void init();
  void update();
  void loop();
  
  // 获取传感器数据
  SensorData getSensorData() { return currentData; }
  
  // 校准传感器
  void calibrate(float tempOffset = 0.0, float humOffset = 0.0);
  
private:
  // 传感器数据
  SensorData currentData;
  
  // 校准偏移量
  float tempOffset;
  float humOffset;
  
  // 更新标志
  unsigned long lastUpdate;
  bool dataUpdated;
  
  // 私有方法
  bool readDHT22();
  bool readSHT30();
  void filterData();
};

#endif // SENSOR_MANAGER_H