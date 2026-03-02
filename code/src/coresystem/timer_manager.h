#ifndef TIMER_MANAGER_H
#define TIMER_MANAGER_H

#include "platform_abstraction.h"
#include "event_bus.h"

// 定时器回调函数类型
typedef std::function<void(uint32_t)> TimerCallback;

// 定时器结构体
typedef struct {
  uint32_t timerId;
  unsigned long interval;      // 定时器间隔（毫秒）
  unsigned long lastTriggerTime; // 上次触发时间
  bool enabled;
  bool isOneShot;
  TimerCallback callback;
  bool paused;
  unsigned long pauseTime;     // 暂停时的时间
} TimerItem;

/**
 * @brief 定时器管理类
 * 
 * 负责管理系统中的定时器，包括创建、启动、停止、暂停和恢复定时器，
 * 以及处理定时器的触发回调。
 */
class TimerManager {
private:
  static TimerManager* instance;         // 单例实例
  
  // 定时器列表
  std::vector<TimerItem> timers;         // 定时器列表
  uint32_t nextTimerId;                  // 下一个定时器ID
  
  /**
   * @brief 构造函数
   * 
   * 私有构造函数，用于初始化定时器管理相关的变量。
   */
  TimerManager() {
    nextTimerId = 0;
  }
  
  /**
   * @brief 处理定时器
   * 
   * 检查所有定时器是否到期，并触发相应的回调函数。
   */
  void processTimers();
  
public:
  /**
   * @brief 获取单例实例
   * 
   * @return TimerManager* 定时器管理类的单例实例
   */
  static TimerManager* getInstance();
  
  /**
   * @brief 初始化定时器管理器
   * 
   * @return bool 初始化是否成功
   */
  bool init();
  
  /**
   * @brief 运行定时器管理
   * 
   * 处理所有定时器的触发逻辑。
   */
  void run();
  
  /**
   * @brief 创建定时器
   * 
   * @param interval 定时器间隔（毫秒）
   * @param callback 定时器回调函数
   * @param isOneShot 是否为一次性定时器
   * @return uint32_t 定时器ID
   */
  uint32_t createTimer(unsigned long interval, TimerCallback callback, bool isOneShot = false);
  
  /**
   * @brief 创建微秒级定时器
   * 
   * @param interval 定时器间隔（微秒）
   * @param callback 定时器回调函数
   * @param isOneShot 是否为一次性定时器
   * @return uint32_t 定时器ID
   */
  uint32_t createMicrosecondTimer(unsigned long interval, TimerCallback callback, bool isOneShot = false);
  
  /**
   * @brief 启动定时器
   * 
   * @param timerId 定时器ID
   * @return bool 启动是否成功
   */
  bool startTimer(uint32_t timerId);
  
  /**
   * @brief 停止定时器
   * 
   * @param timerId 定时器ID
   * @return bool 停止是否成功
   */
  bool stopTimer(uint32_t timerId);
  
  /**
   * @brief 暂停定时器
   * 
   * @param timerId 定时器ID
   * @return bool 暂停是否成功
   */
  bool pauseTimer(uint32_t timerId);
  
  /**
   * @brief 恢复定时器
   * 
   * @param timerId 定时器ID
   * @return bool 恢复是否成功
   */
  bool resumeTimer(uint32_t timerId);
  
  /**
   * @brief 删除定时器
   * 
   * @param timerId 定时器ID
   * @return bool 删除是否成功
   */
  bool deleteTimer(uint32_t timerId);
  
  /**
   * @brief 检查定时器是否运行
   * 
   * @param timerId 定时器ID
   * @return bool 定时器是否运行
   */
  bool isTimerRunning(uint32_t timerId);
  
  /**
   * @brief 检查定时器是否暂停
   * 
   * @param timerId 定时器ID
   * @return bool 定时器是否暂停
   */
  bool isTimerPaused(uint32_t timerId);
  
  /**
   * @brief 设置定时器间隔
   * 
   * @param timerId 定时器ID
   * @param interval 定时器间隔（毫秒）
   * @return bool 设置是否成功
   */
  bool setTimerInterval(uint32_t timerId, unsigned long interval);
  
  /**
   * @brief 获取定时器剩余时间
   * 
   * @param timerId 定时器ID
   * @return unsigned long 剩余时间（毫秒）
   */
  unsigned long getTimerRemaining(uint32_t timerId);
  
  /**
   * @brief 获取定时器数量
   * 
   * @return size_t 定时器数量
   */
  size_t getTimerCount();
  
  /**
   * @brief 清理所有定时器
   * 
   * 销毁所有定时器并重置定时器ID。
   */
  void clearAllTimers();
};

#endif // TIMER_MANAGER_H