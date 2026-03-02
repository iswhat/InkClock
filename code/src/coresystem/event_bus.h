#ifndef EVENT_BUS_H
#define EVENT_BUS_H

#ifdef ARDUINO
#include <Arduino.h>
#else
#include "arduino_compat.h"
#endif

#include <vector>
#include <functional>
#include <memory>
#include "data_types.h"
#include "../drivers/peripherals/sensor_driver.h"

// 事件类型枚举
enum EventType {
  // 系统事件
  EVENT_SYSTEM_STARTUP,
  EVENT_SYSTEM_ERROR,
  EVENT_SYSTEM_SHUTDOWN,
  EVENT_SYSTEM_RESET,
  EVENT_SYSTEM_IDLE,
  EVENT_SYSTEM_ACTIVE,
  EVENT_SYSTEM_DEEP_SLEEP,
  EVENT_SYSTEM_LIGHT_SLEEP,
  EVENT_SYSTEM_WAKEUP,
  EVENT_SYSTEM_LOW_POWER,
  EVENT_SYSTEM_NORMAL_POWER,
  
  // 网络事件
  EVENT_WIFI_CONNECTED,
  EVENT_WIFI_DISCONNECTED,
  EVENT_IP_ACQUIRED,
  EVENT_NETWORK_ERROR,
  EVENT_NETWORK_RECOVERED,
  
  // 时间事件
  EVENT_TIME_UPDATED,
  EVENT_TIME_SYNCED,
  EVENT_TIMER_EXPIRED,
  
  // 传感器事件
  EVENT_SENSOR_DATA_UPDATED,
  EVENT_SENSOR_CONFIG_UPDATED,
  EVENT_SENSOR_DISCOVERED,
  EVENT_SENSOR_CONNECTED,
  EVENT_SENSOR_DISCONNECTED,
  EVENT_SENSOR_ERROR,
  EVENT_SENSOR_CALIBRATED,
  
  // 报警事件
  EVENT_ALARM_TRIGGERED,
  EVENT_ALARM_CLEARED,
  EVENT_ALARM_ACKNOWLEDGED,
  
  // 输入事件
  EVENT_BUTTON_PRESSED,
  EVENT_BUTTON_RELEASED,
  EVENT_BUTTON_CLICKED,
  EVENT_BUTTON_DOUBLE_CLICKED,
  EVENT_BUTTON_LONG_PRESSED,
  EVENT_TOUCH_EVENT,
  
  // 显示事件
  EVENT_DISPLAY_UPDATED,
  EVENT_DISPLAY_ERROR,
  EVENT_DISPLAY_REFRESH,
  EVENT_DISPLAY_CLEARED,
  
  // 电源事件
  EVENT_LOW_POWER_ENTER,
  EVENT_LOW_POWER_EXIT,
  EVENT_CHARGING_STARTED,
  EVENT_CHARGING_STOPPED,
  EVENT_BATTERY_LOW,
  EVENT_BATTERY_OK,
  EVENT_POWER_STATE_CHANGED,
  EVENT_LOW_POWER_SENSOR_ADJUST,
  
  // 驱动事件
  EVENT_DRIVER_REGISTERED,
  EVENT_DRIVER_UNREGISTERED,
  EVENT_DRIVER_ENABLED,
  EVENT_DRIVER_DISABLED,
  EVENT_DRIVER_ERROR,
  EVENT_DRIVER_UPDATED,
  
  // 设备事件
  EVENT_DEVICE_DISCOVERED,
  EVENT_DEVICE_CONNECTED,
  EVENT_DEVICE_DISCONNECTED,
  EVENT_DEVICE_DATA_RECEIVED,
  EVENT_DEVICE_CONTROL,
  EVENT_DEVICE_STATUS_CHANGED,
  
  // 数据传输事件
  EVENT_DATA_TRANSMIT,
  EVENT_DATA_RECEIVE,
  EVENT_DATA_ERROR,
  EVENT_DATA_COMPLETE,
  
  // 配置事件
  EVENT_CONFIG_UPDATED,
  EVENT_CONFIG_RESET,
  EVENT_CONFIG_SAVED,
  EVENT_CONFIG_LOADED,
  
  // 应用事件
  EVENT_APP_STARTED,
  EVENT_APP_STOPPED,
  EVENT_APP_PAUSED,
  EVENT_APP_RESUMED,
  
  // 插件事件
  EVENT_PLUGIN_LOADED,
  EVENT_PLUGIN_UNLOADED,
  EVENT_PLUGIN_ENABLED,
  EVENT_PLUGIN_DISABLED,
  EVENT_PLUGIN_ERROR,
  
  // 存储事件
  EVENT_STORAGE_READ,
  EVENT_STORAGE_WRITE,
  EVENT_STORAGE_ERROR,
  EVENT_STORAGE_FULL,
  
  // 更新事件
  EVENT_UPDATE_AVAILABLE,
  EVENT_UPDATE_STARTED,
  EVENT_UPDATE_PROGRESS,
  EVENT_UPDATE_COMPLETE,
  EVENT_UPDATE_FAILED,
  
  // 硬件变化事件
  EVENT_HARDWARE_CHANGED,
  
  // 模块事件
  EVENT_MODULE_REGISTERED,
  EVENT_MODULE_UNREGISTERED,
  EVENT_MODULE_ENABLED,
  EVENT_MODULE_DISABLED,
  EVENT_MODULE_STATUS_CHANGED
};

// 事件数据基类
class EventData {
public:
  virtual ~EventData() {}
};

// 按钮事件数据
class ButtonEventData : public EventData {
public:
  int buttonIndex;
  int eventType; // BUTTON_PRESS, BUTTON_RELEASE, BUTTON_CLICK, BUTTON_DOUBLE_CLICK, BUTTON_LONG_PRESS
  unsigned long duration;
  ButtonEventData(int btnIndex, int evtType, unsigned long dur = 0) : 
    buttonIndex(btnIndex), eventType(evtType), duration(dur) {}
};

// 报警事件数据
class AlarmEventData : public EventData {
public:
  String alarmType;
  String message;
  AlarmEventData(const String& type, const String& msg) : alarmType(type), message(msg) {}
};

// 驱动事件数据
class DriverEventData : public EventData {
public:
  String driverName;
  String driverType;
  DriverEventData(const String& name, const String& type) : driverName(name), driverType(type) {}
};

// 设备事件数据
class DeviceEventData : public EventData {
public:
  String deviceName;
  String deviceType;
  String deviceId;
  DeviceEventData(const String& name, const String& type, const String& id) : 
    deviceName(name), deviceType(type), deviceId(id) {}
};

// 系统错误事件数据
class SystemErrorEventData : public EventData {
public:
  String errorMessage;
  int errorCode;
  String moduleName;
  SystemErrorEventData(const String& msg, int code, const String& module) : 
    errorMessage(msg), errorCode(code), moduleName(module) {}
};

// 设备数据事件数据
class DeviceDataEventData : public EventData {
public:
  String deviceId;
  String dataType;
  String data;
  DeviceDataEventData(const String& devId, const String& type, const String& data) : 
    deviceId(devId), dataType(type), data(data) {}
};

// 设备控制事件数据
class DeviceControlEventData : public EventData {
public:
  String deviceId;
  String command;
  String params;
  DeviceControlEventData(const String& devId, const String& cmd, const String& params) : 
    deviceId(devId), command(cmd), params(params) {}
};

// 配置事件数据
class ConfigEventData : public EventData {
public:
  String configKey;
  String configValue;
  ConfigEventData(const String& key, const String& value) : 
    configKey(key), configValue(value) {}
};

// 数据传输事件数据
class DataTransferEventData : public EventData {
public:
  String source;
  String destination;
  String data;
  int dataSize;
  DataTransferEventData(const String& src, const String& dest, const String& data, int size) : 
    source(src), destination(dest), data(data), dataSize(size) {}
};

// 插件事件数据
class PluginEventData : public EventData {
public:
  String pluginName;
  String pluginVersion;
  String pluginStatus;
  PluginEventData(const String& name, const String& version, const String& status) : 
    pluginName(name), pluginVersion(version), pluginStatus(status) {}
};

// 模块事件数据
class ModuleEventData : public EventData {
public:
  String moduleName;
  int moduleType;
  ModuleEventData(const String& name, int type) : 
    moduleName(name), moduleType(type) {}
};

// 电源状态事件数据
class PowerStateEventData : public EventData {
public:
  int batteryPercentage;
  bool isCharging;
  bool isLowPower;
  PowerStateEventData(int battery, bool charging, bool lowPower) : 
    batteryPercentage(battery), isCharging(charging), isLowPower(lowPower) {}
};

// 时间更新事件数据
class TimeDataEventData : public EventData {
public:
  TimeData timeData;
  TimeDataEventData(TimeData data) : timeData(data) {}
};

// 天气更新事件数据
class WeatherDataEventData : public EventData {
public:
  WeatherData weatherData;
  WeatherDataEventData(WeatherData data) : weatherData(data) {}
};

// 传感器更新事件数据
class SensorDataEventData : public EventData {
public:
  SensorData sensorData;
  SensorDataEventData(SensorData data) : sensorData(data) {}
};

// 传感器配置更新事件数据
class SensorConfigEventData : public EventData {
public:
  SensorConfig config;
  SensorConfigEventData(SensorConfig cfg) : config(cfg) {}
};

// 事件处理器类型
typedef std::function<void(EventType, std::shared_ptr<EventData>)> EventHandler;

// 事件订阅信息
  typedef struct EventSubscriptionStruct {
    EventType type;
    EventHandler handler;
    const char* moduleName;
    uint32_t priority; // 订阅优先级，数值越大优先级越高
  } EventSubscription;

/**
 * @brief 事件总线类
 * 
 * 负责事件的发布和订阅，是系统各模块之间通信的核心组件。
 * 使用单例模式，确保系统中只有一个事件总线实例。
 */
class EventBus {
private:
  std::unordered_map<EventType, std::vector<EventSubscription>> subscriptionsMap; // 事件订阅映射
  bool isProcessingEvents = false;                           // 是否正在处理事件
  std::vector<std::pair<EventType, std::shared_ptr<EventData>>> eventQueue; // 事件队列，用于处理递归事件
  SemaphoreHandle_t eventMutex = nullptr;                    // 事件处理互斥锁
  
  /**
   * @brief 构造函数
   * 
   * 私有构造函数，用于初始化事件总线。
   */
  EventBus() {
    eventMutex = xSemaphoreCreateMutex();
  }
  
public:
  /**
   * @brief 获取单例实例
   * 
   * @return EventBus* 事件总线的单例实例
   */
  static EventBus* getInstance() {
    static EventBus instance;
    return &instance;
  }
  
  /**
   * @brief 订阅事件
   * 
   * @param type 事件类型
   * @param handler 事件处理函数
   * @param moduleName 模块名称，用于标识订阅者
   * @param priority 订阅优先级，默认5（中等）
   */
  void subscribe(EventType type, EventHandler handler, const char* moduleName, uint32_t priority = 5) {
    if (eventMutex) {
      xSemaphoreTake(eventMutex, portMAX_DELAY);
    }
    
    EventSubscription sub = { type, handler, moduleName, priority };
    subscriptionsMap[type].push_back(sub);
    
    // 按优先级排序，优先级高的排在前面
    std::sort(subscriptionsMap[type].begin(), subscriptionsMap[type].end(), 
      [](const EventSubscription& a, const EventSubscription& b) {
        return a.priority > b.priority;
      });
    
    if (eventMutex) {
      xSemaphoreGive(eventMutex);
    }
  }
  
  /**
   * @brief 发布事件
   * 
   * @param type 事件类型
   * @param data 事件数据，默认为nullptr
   */
  void publish(EventType type, std::shared_ptr<EventData> data = nullptr) {
    if (isProcessingEvents) {
      // 如果正在处理事件，将事件加入队列，避免递归调用
      if (eventMutex) {
        xSemaphoreTake(eventMutex, portMAX_DELAY);
      }
      eventQueue.emplace_back(type, data);
      if (eventMutex) {
        xSemaphoreGive(eventMutex);
      }
      return;
    }
    
    isProcessingEvents = true;
    
    // 处理当前事件
    std::vector<EventSubscription> subs;
    if (eventMutex) {
      xSemaphoreTake(eventMutex, portMAX_DELAY);
      auto it = subscriptionsMap.find(type);
      if (it != subscriptionsMap.end()) {
        subs = it->second;
      }
      xSemaphoreGive(eventMutex);
    } else {
      auto it = subscriptionsMap.find(type);
      if (it != subscriptionsMap.end()) {
        subs = it->second;
      }
    }
    
    // 处理订阅者
    for (const auto& sub : subs) {
      sub.handler(type, data);
    }
    
    // 处理队列中的事件
    while (true) {
      std::pair<EventType, std::shared_ptr<EventData>> event;
      bool hasEvent = false;
      
      if (eventMutex) {
        xSemaphoreTake(eventMutex, portMAX_DELAY);
        if (!eventQueue.empty()) {
          event = eventQueue.front();
          eventQueue.erase(eventQueue.begin());
          hasEvent = true;
        }
        xSemaphoreGive(eventMutex);
      } else {
        if (!eventQueue.empty()) {
          event = eventQueue.front();
          eventQueue.erase(eventQueue.begin());
          hasEvent = true;
        }
      }
      
      if (!hasEvent) {
        break;
      }
      
      std::vector<EventSubscription> queueSubs;
      if (eventMutex) {
        xSemaphoreTake(eventMutex, portMAX_DELAY);
        auto eventIt = subscriptionsMap.find(event.first);
        if (eventIt != subscriptionsMap.end()) {
          queueSubs = eventIt->second;
        }
        xSemaphoreGive(eventMutex);
      } else {
        auto eventIt = subscriptionsMap.find(event.first);
        if (eventIt != subscriptionsMap.end()) {
          queueSubs = eventIt->second;
        }
      }
      
      for (const auto& sub : queueSubs) {
        sub.handler(event.first, event.second);
      }
    }
    
    isProcessingEvents = false;
  }
  
  /**
   * @brief 取消订阅
   * 
   * @param type 事件类型
   * @param handler 事件处理函数
   */
  void unsubscribe(EventType type, EventHandler handler) {
    if (eventMutex) {
      xSemaphoreTake(eventMutex, portMAX_DELAY);
    }
    
    auto it = subscriptionsMap.find(type);
    if (it != subscriptionsMap.end()) {
      auto& subs = it->second;
      // 简化的取消订阅逻辑，移除所有该事件类型的订阅
      subs.clear();
      
      // 如果该事件类型没有订阅者了，从映射中移除
      if (subs.empty()) {
        subscriptionsMap.erase(it);
      }
    }
    
    if (eventMutex) {
      xSemaphoreGive(eventMutex);
    }
  }
  
  /**
   * @brief 按模块取消订阅
   * 
   * @param moduleName 模块名称
   */
  void unsubscribeByModule(const char* moduleName) {
    if (eventMutex) {
      xSemaphoreTake(eventMutex, portMAX_DELAY);
    }
    
    for (auto& pair : subscriptionsMap) {
      auto& subs = pair.second;
      subs.erase(std::remove_if(subs.begin(), subs.end(), 
        [moduleName](const EventSubscription& sub) {
          return strcmp(sub.moduleName, moduleName) == 0;
        }), subs.end());
      
      // 如果该事件类型没有订阅者了，从映射中移除
      if (subs.empty()) {
        subscriptionsMap.erase(pair.first);
      }
    }
    
    if (eventMutex) {
      xSemaphoreGive(eventMutex);
    }
  }
  
  /**
   * @brief 取消所有订阅
   * 
   * 清空所有事件订阅和事件队列。
   */
  void clear() {
    if (eventMutex) {
      xSemaphoreTake(eventMutex, portMAX_DELAY);
    }
    
    subscriptionsMap.clear();
    eventQueue.clear();
    
    if (eventMutex) {
      xSemaphoreGive(eventMutex);
    }
  }
  
  /**
   * @brief 获取订阅数量
   * 
   * @return size_t 总订阅数量
   */
  size_t getSubscriptionCount() const {
    size_t count = 0;
    for (const auto& pair : subscriptionsMap) {
      count += pair.second.size();
    }
    return count;
  }
  
  /**
   * @brief 获取特定事件类型的订阅数量
   * 
   * @param type 事件类型
   * @return size_t 该事件类型的订阅数量
   */
  size_t getSubscriptionCount(EventType type) const {
    auto it = subscriptionsMap.find(type);
    if (it != subscriptionsMap.end()) {
      return it->second.size();
    }
    return 0;
  }
  
  /**
   * @brief 获取特定模块的订阅数量
   * 
   * @param moduleName 模块名称
   * @return size_t 该模块的订阅数量
   */
  size_t getSubscriptionCountByModule(const char* moduleName) const {
    size_t count = 0;
    for (const auto& pair : subscriptionsMap) {
      for (const auto& sub : pair.second) {
        if (strcmp(sub.moduleName, moduleName) == 0) {
          count++;
        }
      }
    }
    return count;
  }
  
  /**
   * @brief 析构函数
   * 
   * 清理所有订阅和事件队列。
   */
  ~EventBus() {
    clear();
    if (eventMutex) {
      vSemaphoreDelete(eventMutex);
    }
  }
};



// 简化的事件发布宏
#define EVENT_PUBLISH(type, data) EventBus::getInstance()->publish(type, data)

// 简化的事件订阅宏
#define EVENT_SUBSCRIBE(type, handler, module) EventBus::getInstance()->subscribe(type, handler, module)

#endif // EVENT_BUS_H