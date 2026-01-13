#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#include <Arduino.h>
#include <vector>
#include <memory>

// 内存块状态枚举
enum MemoryBlockStatus {
  BLOCK_STATUS_FREE,
  BLOCK_STATUS_USED,
  BLOCK_STATUS_RESERVED
};

// 内存块结构
typedef struct {
  void* address;
  size_t size;
  MemoryBlockStatus status;
  unsigned long allocTime;
  String owner;
} MemoryBlock;

// 内存池配置结构
typedef struct {
  size_t blockSize;
  int blockCount;
  bool autoExpand;
  int expandBlockCount;
  size_t maxPoolSize;
} MemoryPoolConfig;

// 内存池类
class MemoryPool {
private:
  static MemoryPool* instance;
  
  // 内存块列表
  std::vector<MemoryBlock> blocks;
  
  // 内存池配置
  MemoryPoolConfig config;
  
  // 内存池统计信息
  size_t totalSize;
  size_t usedSize;
  int freeBlocks;
  int usedBlocks;
  unsigned long lastAllocationTime;
  unsigned long lastFreeTime;
  
  // 私有方法
  bool allocateBlocks(int count, size_t blockSize);
  MemoryBlock* findFreeBlock(size_t size);
  void updateStats();
  
  // 私有构造函数
  MemoryPool();
  
public:
  // 获取单例实例
  static MemoryPool* getInstance();
  
  // 初始化内存池
  bool init(MemoryPoolConfig config);
  
  // 分配内存
  void* allocate(size_t size, const String& owner = "unknown");
  
  // 释放内存
  bool free(void* ptr);
  
  // 保留内存
  bool reserve(size_t size, const String& owner = "unknown");
  
  // 获取内存池状态
  size_t getTotalSize();
  size_t getUsedSize();
  size_t getFreeSize();
  int getFreeBlockCount();
  int getUsedBlockCount();
  float getUsagePercentage();
  
  // 获取内存块信息
  std::vector<MemoryBlock> getBlocksInfo();
  
  // 清理内存池
  void cleanup();
  
  // 调整内存池大小
  bool resize(int blockCount, size_t blockSize);
  
  // 检查内存泄漏
  void checkMemoryLeaks();
  
  // 获取内存池配置
  MemoryPoolConfig getConfig();
  
  // 设置内存池配置
  bool setConfig(MemoryPoolConfig config);
};

// 内存池智能指针模板
template <typename T>
class PooledPtr {
private:
  T* ptr;
  MemoryPool* pool;
  
public:
  PooledPtr() : ptr(nullptr), pool(MemoryPool::getInstance()) {}
  
  PooledPtr(size_t size, const String& owner = "unknown") : pool(MemoryPool::getInstance()) {
    ptr = static_cast<T*>(pool->allocate(size, owner));
  }
  
  ~PooledPtr() {
    if (ptr) {
      pool->free(ptr);
    }
  }
  
  T* get() const {
    return ptr;
  }
  
  T& operator*() const {
    return *ptr;
  }
  
  T* operator->() const {
    return ptr;
  }
  
  operator bool() const {
    return ptr != nullptr;
  }
  
  void reset() {
    if (ptr) {
      pool->free(ptr);
      ptr = nullptr;
    }
  }
  
  void reset(size_t size, const String& owner = "unknown") {
    if (ptr) {
      pool->free(ptr);
    }
    ptr = static_cast<T*>(pool->allocate(size, owner));
  }
};

#endif // MEMORY_POOL_H