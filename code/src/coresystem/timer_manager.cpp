#include "timer_manager.h"

// 初始化单例实例
TimerManager* TimerManager::instance = nullptr;

// 获取单例实例
TimerManager* TimerManager::getInstance() {
  if (instance == nullptr) {
    instance = new TimerManager();
  }
  return instance;
}

// 初始化
bool TimerManager::init() {
  return true;
}

// 运行定时器管理
void TimerManager::run() {
  processTimers();
}

// 按优先级排序定时器
void TimerManager::sortTimersByPriority() {
  // 按优先级降序排序，确保高优先级定时器先被处理
  std::sort(timers.begin(), timers.end(), [](const TimerItem& a, const TimerItem& b) {
    return a.priority > b.priority;
  });
}

// 处理毫秒级定时器
void TimerManager::processMillisecondTimers(unsigned long now) {
  // 使用索引遍历，避免迭代器失效问题
  for (size_t i = 0; i < timers.size(); ) {
    TimerItem& timer = timers[i];

    if (!timer.isMicrosecond && timer.enabled && !timer.paused && (now - timer.lastTriggerTime >= timer.interval)) {
      // 触发定时器回调
      if (timer.callback) {
        timer.callback(timer.timerId);
      }

      // 更新最后触发时间
      timer.lastTriggerTime = now;

      // 如果是一次性定时器，禁用它
      if (timer.isOneShot) {
        timer.enabled = false;

        // 发布定时器到期事件
        auto timerData = std::make_shared<SystemErrorEventData>("Timer expired", 0, "Timer" + String(timer.timerId));
        EventBus::getInstance()->publish(EVENT_TIMER_EXPIRED, timerData);
      }
    }

    // 移除已禁用的一次性定时器
    if (timer.isOneShot && !timer.enabled) {
      timers.erase(timers.begin() + i);
      // 不递增i，因为元素被移除后，下一个元素移动到了当前位置
    } else {
      i++; // 移动到下一个元素
    }
  }
}

// 处理微秒级定时器
void TimerManager::processMicrosecondTimers(unsigned long nowMicros) {
  // 使用索引遍历，避免迭代器失效问题
  for (size_t i = 0; i < timers.size(); ) {
    TimerItem& timer = timers[i];

    if (timer.isMicrosecond && timer.enabled && !timer.paused && (nowMicros - timer.lastMicrosecondTrigger >= timer.microsecondInterval)) {
      // 触发定时器回调
      if (timer.callback) {
        timer.callback(timer.timerId);
      }

      // 更新最后触发时间
      timer.lastMicrosecondTrigger = nowMicros;

      // 如果是一次性定时器，禁用它
      if (timer.isOneShot) {
        timer.enabled = false;

        // 发布定时器到期事件
        auto timerData = std::make_shared<SystemErrorEventData>("Microsecond timer expired", 0, "MicroTimer" + String(timer.timerId));
        EventBus::getInstance()->publish(EVENT_TIMER_EXPIRED, timerData);
      }
    }

    // 移除已禁用的一次性定时器
    if (timer.isOneShot && !timer.enabled) {
      timers.erase(timers.begin() + i);
      // 不递增i，因为元素被移除后，下一个元素移动到了当前位置
    } else {
      i++; // 移动到下一个元素
    }
  }
}

// 处理定时器
void TimerManager::processTimers() {
  unsigned long now = platformGetMillis();
  unsigned long nowMicros = platformGetMicros();

  // 按优先级排序定时器
  sortTimersByPriority();

  // 处理毫秒级定时器
  processMillisecondTimers(now);

  // 处理微秒级定时器
  processMicrosecondTimers(nowMicros);

  lastProcessTime = now;
}

// 创建定时器
uint32_t TimerManager::createTimer(unsigned long interval, TimerCallback callback, bool isOneShot, TimerPriority priority) {
  if (timers.size() >= maxTimers) {
    Serial.println("[TimerManager] Max timers reached");
    return 0;
  }
  
  TimerItem timer;
  timer.timerId = nextTimerId++;
  timer.interval = interval;
  timer.lastTriggerTime = platformGetMillis();
  timer.enabled = true;
  timer.isOneShot = isOneShot;
  timer.callback = callback;
  timer.paused = false;
  timer.pauseTime = 0;
  timer.priority = priority;
  timer.isMicrosecond = false;
  timer.microsecondInterval = 0;
  timer.lastMicrosecondTrigger = 0;
  
  timers.push_back(timer);
  return timer.timerId;
}

// 创建微秒级定时器
uint32_t TimerManager::createMicrosecondTimer(unsigned long interval, TimerCallback callback, bool isOneShot, TimerPriority priority) {
  if (timers.size() >= maxTimers) {
    Serial.println("[TimerManager] Max timers reached");
    return 0;
  }
  
  TimerItem timer;
  timer.timerId = nextTimerId++;
  timer.interval = 0; // 微秒级定时器不使用毫秒间隔
  timer.lastTriggerTime = 0;
  timer.enabled = true;
  timer.isOneShot = isOneShot;
  timer.callback = callback;
  timer.paused = false;
  timer.pauseTime = 0;
  timer.priority = priority;
  timer.isMicrosecond = true;
  timer.microsecondInterval = interval;
  timer.lastMicrosecondTrigger = platformGetMicros();
  
  timers.push_back(timer);
  return timer.timerId;
}

// 创建定时器（兼容旧接口）
uint32_t TimerManager::createTimer(unsigned long interval, TimerCallback callback, bool isOneShot) {
  return createTimer(interval, callback, isOneShot, TIMER_PRIORITY_NORMAL);
}

// 创建微秒级定时器（兼容旧接口）
uint32_t TimerManager::createMicrosecondTimer(unsigned long interval, TimerCallback callback, bool isOneShot) {
  return createMicrosecondTimer(interval, callback, isOneShot, TIMER_PRIORITY_NORMAL);
}

// 启动定时器
bool TimerManager::startTimer(uint32_t timerId) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      timer.enabled = true;
      timer.paused = false;
      if (timer.isMicrosecond) {
        timer.lastMicrosecondTrigger = platformGetMicros();
      } else {
        timer.lastTriggerTime = platformGetMillis();
      }
      timer.pauseTime = 0;
      return true;
    }
  }
  return false;
}

// 停止定时器
bool TimerManager::stopTimer(uint32_t timerId) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      timer.enabled = false;
      return true;
    }
  }
  return false;
}

// 删除定时器
bool TimerManager::deleteTimer(uint32_t timerId) {
  for (auto it = timers.begin(); it != timers.end(); ++it) {
    if (it->timerId == timerId) {
      timers.erase(it);
      return true;
    }
  }
  return false;
}

// 检查定时器是否运行
bool TimerManager::isTimerRunning(uint32_t timerId) {
  for (const auto& timer : timers) {
    if (timer.timerId == timerId) {
      return timer.enabled;
    }
  }
  return false;
}

// 设置定时器间隔
bool TimerManager::setTimerInterval(uint32_t timerId, unsigned long interval) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      timer.interval = interval;
      return true;
    }
  }
  return false;
}

// 获取定时器数量
size_t TimerManager::getTimerCount() {
  return timers.size();
}

// 暂停定时器
bool TimerManager::pauseTimer(uint32_t timerId) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      if (timer.enabled && !timer.paused) {
        timer.paused = true;
        timer.pauseTime = platformGetMillis();
        return true;
      }
      break;
    }
  }
  return false;
}

// 恢复定时器
bool TimerManager::resumeTimer(uint32_t timerId) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      if (timer.enabled && timer.paused) {
        unsigned long now = platformGetMillis();
        unsigned long nowMicros = platformGetMicros();
        
        // 调整上次触发时间，补偿暂停的时间
        if (timer.isMicrosecond) {
          unsigned long pausedTimeMicros = (now - timer.pauseTime) * 1000;
          timer.lastMicrosecondTrigger += pausedTimeMicros;
        } else {
          timer.lastTriggerTime += (now - timer.pauseTime);
        }
        
        timer.paused = false;
        timer.pauseTime = 0;
        return true;
      }
      break;
    }
  }
  return false;
}

// 检查定时器是否暂停
bool TimerManager::isTimerPaused(uint32_t timerId) {
  for (const auto& timer : timers) {
    if (timer.timerId == timerId) {
      return timer.paused;
    }
  }
  return false;
}

// 获取定时器剩余时间
unsigned long TimerManager::getTimerRemaining(uint32_t timerId) {
  for (const auto& timer : timers) {
    if (timer.timerId == timerId) {
      if (!timer.enabled) {
        return 0;
      }
      if (timer.paused) {
        unsigned long pausedTime = platformGetMillis() - timer.pauseTime;
        unsigned long elapsed = timer.pauseTime - timer.lastTriggerTime;
        if (elapsed >= timer.interval) {
          return 0;
        }
        return timer.interval - elapsed;
      } else {
        unsigned long now = platformGetMillis();
        unsigned long elapsed = now - timer.lastTriggerTime;
        if (elapsed >= timer.interval) {
          return 0;
        }
        return timer.interval - elapsed;
      }
    }
  }
  return 0;
}

// 清理所有定时器
void TimerManager::clearAllTimers() {
  timers.clear();
  nextTimerId = 0;
}

// 设置微秒级定时器间隔
bool TimerManager::setTimerMicrosecondInterval(uint32_t timerId, unsigned long interval) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId && timer.isMicrosecond) {
      timer.microsecondInterval = interval;
      return true;
    }
  }
  return false;
}

// 获取微秒级定时器剩余时间
unsigned long TimerManager::getTimerMicrosecondRemaining(uint32_t timerId) {
  for (const auto& timer : timers) {
    if (timer.timerId == timerId && timer.isMicrosecond) {
      if (!timer.enabled) {
        return 0;
      }
      if (timer.paused) {
        unsigned long pausedTime = platformGetMillis() - timer.pauseTime;
        unsigned long pausedTimeMicros = pausedTime * 1000;
        unsigned long elapsed = timer.pauseTime * 1000 - timer.lastMicrosecondTrigger;
        if (elapsed >= timer.microsecondInterval) {
          return 0;
        }
        return timer.microsecondInterval - elapsed;
      } else {
        unsigned long nowMicros = platformGetMicros();
        unsigned long elapsed = nowMicros - timer.lastMicrosecondTrigger;
        if (elapsed >= timer.microsecondInterval) {
          return 0;
        }
        return timer.microsecondInterval - elapsed;
      }
    }
  }
  return 0;
}

// 设置最大定时器数量
void TimerManager::setMaxTimers(size_t max) {
  maxTimers = max;
}

// 获取定时器统计信息
void TimerManager::getTimerStats(size_t& runningCount, size_t& pausedCount, size_t& oneShotCount) {
  runningCount = 0;
  pausedCount = 0;
  oneShotCount = 0;
  
  for (const auto& timer : timers) {
    if (timer.enabled) {
      runningCount++;
      if (timer.paused) {
        pausedCount++;
      }
    }
    if (timer.isOneShot) {
      oneShotCount++;
    }
  }
}