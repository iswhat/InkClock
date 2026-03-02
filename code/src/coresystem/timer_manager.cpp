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

// 处理定时器
void TimerManager::processTimers() {
  unsigned long now = platformGetMillis();

  // 使用索引遍历，避免迭代器失效问题
  for (size_t i = 0; i < timers.size(); ) {
    TimerItem& timer = timers[i];

    if (timer.enabled && !timer.paused && (now - timer.lastTriggerTime >= timer.interval)) {
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

// 创建定时器
uint32_t TimerManager::createTimer(unsigned long interval, TimerCallback callback, bool isOneShot) {
  TimerItem timer;
  timer.timerId = nextTimerId++;
  timer.interval = interval;
  timer.lastTriggerTime = platformGetMillis();
  timer.enabled = true;
  timer.isOneShot = isOneShot;
  timer.callback = callback;
  timer.paused = false;
  timer.pauseTime = 0;
  
  timers.push_back(timer);
  return timer.timerId;
}

// 创建微秒级定时器
uint32_t TimerManager::createMicrosecondTimer(unsigned long interval, TimerCallback callback, bool isOneShot) {
  // 注意：微秒级定时器在不同平台上的精度可能不同
  // 这里将微秒转换为毫秒，实际应用中可以根据平台特性进行优化
  unsigned long millisInterval = interval / 1000;
  if (millisInterval < 1) {
    millisInterval = 1; // 确保最小间隔为1毫秒
  }
  
  return createTimer(millisInterval, callback, isOneShot);
}

// 启动定时器
bool TimerManager::startTimer(uint32_t timerId) {
  for (auto& timer : timers) {
    if (timer.timerId == timerId) {
      timer.enabled = true;
      timer.lastTriggerTime = platformGetMillis();
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
        // 调整上次触发时间，补偿暂停的时间
        timer.lastTriggerTime += (now - timer.pauseTime);
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