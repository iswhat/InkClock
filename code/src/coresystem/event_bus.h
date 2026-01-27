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
} EventSubscription;

// 事件总线类
class EventBus {
private:
  static EventBus* instance;
  std::vector<EventSubscription> subscriptions;
  
  EventBus() {}
  
public:
  static EventBus* getInstance() {
    if (instance == nullptr) {
      instance = new EventBus();
    }
    return instance;
  }
  
  // 订阅事件
  void subscribe(EventType type, EventHandler handler, const char* moduleName) {
    EventSubscription sub = { type, handler, moduleName };
    subscriptions.push_back(sub);
  }
  
  // 发布事件
  void publish(EventType type, std::shared_ptr<EventData> data = nullptr) {
    for (const auto& sub : subscriptions) {
      if (sub.type == type) {
        sub.handler(type, data);
      }
    }
  }
  
  // 取消订阅
  void unsubscribe(EventType type, EventHandler handler) {
    auto it = subscriptions.begin();
    while (it != subscriptions.end()) {
      if (it->type == type) {
        it = subscriptions.erase(it);
      } else {
        ++it;
      }
    }
  }
  
  // 取消所有订阅
  void clear() {
    subscriptions.clear();
  }
  
  // 获取订阅数量
  size_t getSubscriptionCount() const {
    return subscriptions.size();
  }
  
  // 析构函数
  ~EventBus() {
    clear();
  }
};

// 初始化单例实例
EventBus* EventBus::instance = nullptr;

// 简化的事件发布宏
#define EVENT_PUBLISH(type, data) EventBus::getInstance()->publish(type, data)

// 简化的事件订阅宏
#define EVENT_SUBSCRIBE(type, handler, module) EventBus::getInstance()->subscribe(type, handler, module)

#endif // EVENT_BUS_H