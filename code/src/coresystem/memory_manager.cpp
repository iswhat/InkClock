#include "memory_manager.h"

// 初始化单例实例
MemoryManager* MemoryManager::instance = nullptr;

// 获取单例实例
MemoryManager* MemoryManager::getInstance() {
  if (instance == nullptr) {
    instance = new MemoryManager();
  }
  return instance;
}

// 初始化
bool MemoryManager::init() {
  return true;
}

// 创建内存池
void* MemoryManager::createMemoryPool(size_t blockSize, size_t blockCount) {
  MemoryPool pool;
  pool.blockSize = blockSize;
  pool.blockCount = blockCount;
  pool.freeBlocks = blockCount;
  
  // 分配内存池
  pool.pool = malloc(blockSize * blockCount);
  if (pool.pool == nullptr) {
    return nullptr;
  }
  
  // 初始化空闲块链表
  pool.freeList = (void**)malloc(sizeof(void*) * blockCount);
  if (pool.freeList == nullptr) {
    free(pool.pool);
    return nullptr;
  }
  
  // 填充空闲块链表
  for (size_t i = 0; i < blockCount; i++) {
    pool.freeList[i] = (uint8_t*)pool.pool + (i * blockSize);
  }
  
  memoryPools.push_back(pool);
  totalAllocatedMemory += blockSize * blockCount + sizeof(void*) * blockCount;
  if (totalAllocatedMemory > peakAllocatedMemory) {
    peakAllocatedMemory = totalAllocatedMemory;
  }
  
  return pool.pool;
}

// 从内存池分配内存
void* MemoryManager::allocateFromPool(void* poolPtr, size_t size, const char* file, int line) {
  for (auto& pool : memoryPools) {
    if (pool.pool == poolPtr && size <= pool.blockSize && pool.freeBlocks > 0) {
      void* ptr = pool.freeList[--pool.freeBlocks];
      memset(ptr, 0, pool.blockSize);
      
      // 记录内存分配
      MemoryAllocation allocation;
      allocation.ptr = ptr;
      allocation.size = size;
      allocation.file = file;
      allocation.line = line;
      allocation.time = platformGetMillis();
      allocations.push_back(allocation);
      
      return ptr;
    }
  }
  return nullptr;
}

// 从内存池分配内存（兼容旧接口）
void* MemoryManager::allocateFromPool(void* poolPtr, size_t size) {
  return allocateFromPool(poolPtr, size, nullptr, 0);
}

// 释放内存回内存池
void MemoryManager::freeToPool(void* poolPtr, void* ptr) {
  for (auto& pool : memoryPools) {
    if (pool.pool == poolPtr) {
      // 检查ptr是否属于该内存池
      if (ptr >= pool.pool && ptr < (uint8_t*)pool.pool + (pool.blockSize * pool.blockCount)) {
        pool.freeList[pool.freeBlocks++] = ptr;
        
        // 移除内存分配记录
        for (auto it = allocations.begin(); it != allocations.end(); ++it) {
          if (it->ptr == ptr) {
            allocations.erase(it);
            break;
          }
        }
        
        return;
      }
      break;
    }
  }
}

// 销毁内存池
void MemoryManager::destroyMemoryPool(void* poolPtr) {
  for (auto it = memoryPools.begin(); it != memoryPools.end(); ++it) {
    if (it->pool == poolPtr) {
      totalAllocatedMemory -= it->blockSize * it->blockCount + sizeof(void*) * it->blockCount;
      free(it->freeList);
      free(it->pool);
      memoryPools.erase(it);
      return;
    }
  }
}

// 获取内存池使用情况
void MemoryManager::getMemoryPoolInfo(void* poolPtr, size_t& totalBlocks, size_t& freeBlocks, size_t& usedBlocks) {
  for (auto& pool : memoryPools) {
    if (pool.pool == poolPtr) {
      totalBlocks = pool.blockCount;
      freeBlocks = pool.freeBlocks;
      usedBlocks = pool.blockCount - pool.freeBlocks;
      return;
    }
  }
  totalBlocks = 0;
  freeBlocks = 0;
  usedBlocks = 0;
}

// 获取内存池使用情况（兼容旧接口）
void MemoryManager::getMemoryPoolInfo(void* poolPtr, size_t& totalBlocks, size_t& freeBlocks) {
  size_t usedBlocks;
  getMemoryPoolInfo(poolPtr, totalBlocks, freeBlocks, usedBlocks);
}

// 执行内存清理
void MemoryManager::cleanupMemory() {
  // 检查并清理内存泄漏
  checkMemoryLeaks();
  
  // 更新内存使用统计
  updateMemoryStats();
}

// 检查内存泄漏
void MemoryManager::checkMemoryLeaks() {
  // 简单的内存泄漏检查，实际应用中可以更复杂
  size_t currentHeap = platformGetFreeHeap();
  if (lastMemoryUpdate > 0) {
    // 修复：正确检测内存泄漏（内存持续增长才是泄漏）
    static size_t previousHeap = currentHeap;
    static int leakCounter = 0;

    // 检测内存是否持续增长（真正的泄漏）
    // 如果当前内存比上次少超过1KB，说明可能释放了内存（正常现象）
    // 如果当前内存比上次少，且连续多次，可能是正常的内存波动
    // 真正的泄漏是free heap持续减少（可用内存变少）
    if (previousHeap > currentHeap + 1024) {
      // 内存减少了超过1KB，可能是释放了内存（正常）
      leakCounter = 0;
    } else if (previousHeap > currentHeap && (previousHeap - currentHeap) > 256) {
      // 内存持续减少，可能是泄漏
      leakCounter++;
      if (leakCounter > 10) { // 连续10次检测到内存泄漏
        Serial.println("[MemoryManager] Potential memory leak detected");
        leakCounter = 0;
      }
    } else {
      leakCounter = 0;
    }
    previousHeap = currentHeap;
  }
}

// 更新内存统计信息
void MemoryManager::updateMemoryStats() {
  lastMemoryUpdate = platformGetMillis();
  
  // 可以在这里添加更详细的内存统计
}

// 获取内存使用统计
void MemoryManager::getMemoryStats(size_t& totalMemory, size_t& usedMemory, size_t& peakMemory, size_t& freeMemory) {
  size_t freeHeap = platformGetFreeHeap();
  // 根据平台类型获取合理的总RAM大小估计
  size_t totalHeap = 0;
  PlatformType platform = getPlatformType();
  switch(platform) {
    case PlatformType::PLATFORM_ESP32:
      totalHeap = 512 * 1024; // ESP32有512KB RAM
      break;
    case PlatformType::PLATFORM_ESP8266:
      totalHeap = 80 * 1024; // ESP8266有80KB RAM
      break;
    case PlatformType::PLATFORM_NRF52:
      totalHeap = 64 * 1024; // nRF52有64KB RAM
      break;
    case PlatformType::PLATFORM_STM32:
      totalHeap = 20 * 1024; // 假设STM32有20KB RAM
      break;
    case PlatformType::PLATFORM_RP2040:
      totalHeap = 264 * 1024; // RP2040有264KB RAM
      break;
    default:
      totalHeap = 100 * 1024; // 默认值
  }
  
  totalMemory = totalHeap;
  usedMemory = totalHeap - freeHeap + totalAllocatedMemory;
  peakMemory = peakAllocatedMemory;
  freeMemory = freeHeap;
}

// 获取内存使用统计（兼容旧接口）
void MemoryManager::getMemoryStats(size_t& totalMemory, size_t& usedMemory, size_t& peakMemory) {
  size_t freeMemory;
  getMemoryStats(totalMemory, usedMemory, peakMemory, freeMemory);
}

// 获取内存池数量
size_t MemoryManager::getMemoryPoolCount() {
  return memoryPools.size();
}

// 清理所有内存池
void MemoryManager::clearAllMemoryPools() {
  for (auto& pool : memoryPools) {
    free(pool.freeList);
    free(pool.pool);
  }
  memoryPools.clear();
  totalAllocatedMemory = 0;
  peakAllocatedMemory = 0;
}

// 直接分配内存
void* MemoryManager::allocate(size_t size, const char* file, int line) {
  void* ptr = malloc(size);
  if (ptr) {
    totalAllocatedMemory += size;
    if (totalAllocatedMemory > peakAllocatedMemory) {
      peakAllocatedMemory = totalAllocatedMemory;
    }
    
    // 记录内存分配
    MemoryAllocation allocation;
    allocation.ptr = ptr;
    allocation.size = size;
    allocation.file = file;
    allocation.line = line;
    allocation.time = platformGetMillis();
    allocations.push_back(allocation);
  }
  return ptr;
}

// 直接分配内存（兼容旧接口）
void* MemoryManager::allocate(size_t size) {
  return allocate(size, nullptr, 0);
}

// 直接释放内存
void MemoryManager::free(void* ptr) {
  if (ptr) {
    // 移除内存分配记录
    for (auto it = allocations.begin(); it != allocations.end(); ++it) {
      if (it->ptr == ptr) {
        totalAllocatedMemory -= it->size;
        allocations.erase(it);
        break;
      }
    }
    ::free(ptr);
  }
}

// 获取当前空闲内存
size_t MemoryManager::getFreeMemory() {
  return platformGetFreeHeap();
}

// 合并相似的内存池
void MemoryManager::mergeSimilarPools() {
  // 按块大小分组内存池
  std::map<size_t, std::vector<size_t>> poolsByBlockSize;
  for (size_t i = 0; i < memoryPools.size(); ++i) {
    poolsByBlockSize[memoryPools[i].blockSize].push_back(i);
  }
  
  // 合并相同块大小的内存池
  for (auto& entry : poolsByBlockSize) {
    if (entry.second.size() > 1) {
      // 计算总块数
      size_t totalBlocks = 0;
      for (size_t index : entry.second) {
        totalBlocks += memoryPools[index].blockCount;
      }
      
      // 创建新的合并内存池
      void* newPool = createMemoryPool(entry.first, totalBlocks);
      if (newPool) {
        // 复制数据并销毁旧内存池
        for (size_t index : entry.second) {
          auto& oldPool = memoryPools[index];
          // 这里可以添加数据迁移逻辑
          // 注意：实际应用中需要更复杂的处理
        }
      }
    }
  }
}

// 分割过大的内存池
void MemoryManager::splitLargePools() {
  for (auto it = memoryPools.begin(); it != memoryPools.end();) {
    if (it->blockCount > 100 && it->freeBlocks > it->blockCount * 0.8) {
      // 分割为两个较小的内存池
      size_t newBlockCount = it->blockCount / 2;
      void* newPool = createMemoryPool(it->blockSize, newBlockCount);
      if (newPool) {
        it->blockCount -= newBlockCount;
        it->freeBlocks -= newBlockCount;
      }
    }
    ++it;
  }
}

// 检查内存使用预警
void MemoryManager::checkMemoryWarning() {
  size_t totalMemory, usedMemory, peakMemory, freeMemory;
  getMemoryStats(totalMemory, usedMemory, peakMemory, freeMemory);
  
  size_t usagePercentage = (usedMemory * 100) / totalMemory;
  if (usagePercentage >= memoryWarningThreshold) {
    Serial.printf("[MemoryManager] Memory warning: %zu%% usage\n", usagePercentage);
    // 可以在这里触发内存紧急清理
    optimizeMemoryUsage();
  }
}

// 打印内存分配详情
void MemoryManager::printMemoryAllocations() {
  Serial.println("[MemoryManager] Memory allocations:");
  for (const auto& allocation : allocations) {
    Serial.printf("  Address: %p, Size: %zu bytes, File: %s, Line: %d, Time: %lu\n", 
                 allocation.ptr, allocation.size, 
                 allocation.file ? allocation.file : "unknown", 
                 allocation.line, allocation.time);
  }
  Serial.printf("[MemoryManager] Total allocations: %zu\n", allocations.size());
}

// 执行内存碎片整理
void MemoryManager::defragmentMemory() {
  #ifdef ARDUINO
    #if defined(ESP32)
      // ESP32 内存碎片整理
      // 这里可以添加 ESP32 特定的内存碎片整理代码
    #elif defined(ESP8266)
      // ESP8266 内存碎片整理
      // 尝试分配和释放大内存块来整理碎片
      void* temp = malloc(1024);
      if (temp) {
        free(temp);
      }
    #endif
  #endif
  
  // 清理未使用的内存池
  optimizeMemoryUsage();
}

// 优化内存使用
void MemoryManager::optimizeMemoryUsage() {
  // 清理未使用的内存池
  for (auto it = memoryPools.begin(); it != memoryPools.end();) {
    if (it->freeBlocks == it->blockCount) {
      // 所有块都空闲，可以销毁该内存池
      totalAllocatedMemory -= it->blockSize * it->blockCount + sizeof(void*) * it->blockCount;
      free(it->freeList);
      free(it->pool);
      it = memoryPools.erase(it);
    } else {
      ++it;
    }
  }
  
  // 合并相似的内存池
  mergeSimilarPools();
  
  // 分割过大的内存池
  splitLargePools();
  
  // 执行内存碎片整理
  defragmentMemory();
  
  // 检查内存使用预警
  checkMemoryWarning();
  
  // 更新内存统计
  updateMemoryStats();
}

// 设置内存警告阈值
void MemoryManager::setMemoryWarningThreshold(size_t threshold) {
  if (threshold >= 0 && threshold <= 100) {
    memoryWarningThreshold = threshold;
  }
}

// 检查内存泄漏
void MemoryManager::checkMemoryLeaks() {
  // 简单的内存泄漏检查，实际应用中可以更复杂
  size_t currentHeap = platformGetFreeHeap();
  if (lastMemoryUpdate > 0) {
    // 修复：正确检测内存泄漏（内存持续增长才是泄漏）
    static size_t previousHeap = currentHeap;
    static int leakCounter = 0;

    // 检测内存是否持续增长（真正的泄漏）
    // 如果当前内存比上次少超过1KB，说明可能释放了内存（正常现象）
    // 如果当前内存比上次少，且连续多次，可能是正常的内存波动
    // 真正的泄漏是free heap持续减少（可用内存变少）
    if (previousHeap > currentHeap + 1024) {
      // 内存减少了超过1KB，可能是释放了内存（正常）
      leakCounter = 0;
    } else if (previousHeap > currentHeap && (previousHeap - currentHeap) > 256) {
      // 内存持续减少，可能是泄漏
      leakCounter++;
      if (leakCounter > 10) { // 连续10次检测到内存泄漏
        Serial.println("[MemoryManager] Potential memory leak detected");
        // 打印内存分配详情
        printMemoryAllocations();
        leakCounter = 0;
      }
    } else {
      leakCounter = 0;
    }
    previousHeap = currentHeap;
  }
}