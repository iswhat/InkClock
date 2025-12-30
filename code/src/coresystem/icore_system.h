#pragma once

#include <Arduino.h>
#include <string>

// 核心系统状态枚举
enum CoreSystemState {
  SYSTEM_STATE_UNINITIALIZED,
  SYSTEM_STATE_INITIALIZING,
  SYSTEM_STATE_RUNNING,
  SYSTEM_STATE_LOW_POWER,
  SYSTEM_STATE_ERROR,
  SYSTEM_STATE_SHUTTING_DOWN
};

// 定时器回调函数类型
typedef std::function<void(uint32_t)> TimerCallback;

// 核心系统抽象接口
class ICoreSystem {
public:
  virtual ~ICoreSystem() {}
  
  // 初始化核心系统
  virtual bool init() = 0;
  
  // 运行核心系统
  virtual void run() = 0;
  
  // 进入低功耗模式
  virtual void enterLowPowerMode() = 0;
  
  // 退出低功耗模式
  virtual void exitLowPowerMode() = 0;
  
  // 关闭系统
  virtual void shutdown() = 0;
  
  // 重置系统
  virtual void reset() = 0;
  
  // 获取系统状态
  virtual CoreSystemState getState() const = 0;
  
  // 获取事件总线
  virtual EventBus* getEventBus() = 0;
  
  // 获取驱动注册表
  virtual DriverRegistry* getDriverRegistry() = 0;
  
  // 获取系统运行时间
  virtual unsigned long getUptime() const = 0;
  
  // 发送系统错误
  virtual void sendError(const String& message, int errorCode, const String& module) = 0;
  
  // 电源管理API
  virtual float getBatteryVoltage() const = 0;
  virtual int getBatteryPercentage() const = 0;
  virtual bool isChargingState() const = 0;
  virtual bool isInLowPowerMode() const = 0;
  
  // 配置管理API
  virtual String getConfig(const String& key, const String& defaultValue = "") = 0;
  virtual bool setConfig(const String& key, const String& value) = 0;
  virtual bool saveConfig() = 0;
  virtual bool resetConfig() = 0;
  
  // 定时器管理API
  virtual uint32_t addTimer(uint32_t intervalMs, TimerCallback callback, bool isOneShot = false, int priority = 5) = 0;
  virtual bool removeTimer(uint32_t timerId) = 0;
  virtual bool enableTimer(uint32_t timerId, bool enable = true) = 0;
  virtual bool isTimerEnabled(uint32_t timerId) const = 0;
  
  // 内存管理API
  virtual void* createMemoryPool(size_t blockSize, size_t blockCount) = 0;
  virtual void* allocateFromPool(void* poolPtr, size_t size) = 0;
  virtual void freeToPool(void* poolPtr, void* ptr) = 0;
  virtual void destroyMemoryPool(void* poolPtr) = 0;
  virtual void getMemoryPoolInfo(void* poolPtr, size_t& totalBlocks, size_t& freeBlocks) = 0;
  virtual void cleanupMemory() = 0;
  virtual void getMemoryStats(size_t& totalMemory, size_t& usedMemory, size_t& peakMemory) = 0;
  virtual void getMemoryInfo(size_t& freeHeap, size_t& minimumFreeHeap) = 0;
  
  // 运算资源管理API
  virtual uint32_t getCpuFrequencyMhz() = 0;
  virtual bool setCpuFrequencyMhz(uint32_t freqMHz) = 0;
  virtual void enableDynamicCpuFreq(bool enable) = 0;
  virtual void setCpuFreqRange(int minFreq, int maxFreq) = 0;
  virtual void adjustCpuFreqBasedOnLoad() = 0;
  virtual void setTaskPriority(uint32_t taskId, int priority) = 0;
  virtual int getTaskPriority(uint32_t taskId) = 0;
  virtual void setDefaultTaskPriority(int priority) = 0;
  
  // 线程管理API
  virtual SemaphoreHandle_t createMutex() = 0;
  virtual bool lockMutex(SemaphoreHandle_t mutex, uint32_t timeoutMs = portMAX_DELAY) = 0;
  virtual bool unlockMutex(SemaphoreHandle_t mutex) = 0;
  virtual void destroyMutex(SemaphoreHandle_t mutex) = 0;
  virtual SemaphoreHandle_t getSystemMutex() = 0;
  virtual void addActiveThread(uint32_t threadId) = 0;
  virtual void removeActiveThread(uint32_t threadId) = 0;
  virtual size_t getActiveThreadCount() = 0;
  
  // 功耗控制API
  virtual void enterDeepSleep(uint64_t sleepTimeMs) = 0;
  virtual void enterLightSleep(uint64_t sleepTimeMs) = 0;
  virtual void setLowPowerMode(bool enable) = 0;
  virtual void optimizePowerConsumption() = 0;
  
  // 系统信息API
  virtual uint32_t getChipId() = 0;
  virtual uint32_t getFlashChipSize() = 0;
  
  // 获取单例实例
  static ICoreSystem* getInstance();
};
