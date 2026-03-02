#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include "platform_abstraction.h"

// 内存池结构体
typedef struct {
  void* pool;          // 内存池指针
  size_t blockSize;    // 块大小
  size_t blockCount;   // 块数量
  size_t freeBlocks;   // 空闲块数量
  void** freeList;     // 空闲块链表
} MemoryPool;

/**
 * @brief 内存管理类
 * 
 * 负责管理系统内存，包括内存池的创建、分配和释放，
 * 以及内存使用情况的监控和优化。
 */
// 内存分配记录结构体
typedef struct {
  void* ptr;          // 内存指针
  size_t size;        // 分配大小
  const char* file;   // 分配文件
  int line;           // 分配行号
  unsigned long time; // 分配时间
} MemoryAllocation;

class MemoryManager {
private:
  static MemoryManager* instance;         // 单例实例
  
  // 内存池列表
  std::vector<MemoryPool> memoryPools;    // 内存池列表
  size_t totalAllocatedMemory;            // 总分配内存大小
  size_t peakAllocatedMemory;             // 峰值分配内存大小
  unsigned long lastMemoryUpdate;         // 上次内存更新时间
  size_t memoryWarningThreshold;          // 内存警告阈值
  
  // 内存分配跟踪
  std::vector<MemoryAllocation> allocations;  // 内存分配记录
  
  /**
   * @brief 构造函数
   * 
   * 私有构造函数，用于初始化内存管理相关的变量。
   */
  MemoryManager() {
    totalAllocatedMemory = 0;
    peakAllocatedMemory = 0;
    lastMemoryUpdate = 0;
    memoryWarningThreshold = 90; // 90% 内存使用率警告
  }
  
  /**
   * @brief 检查内存泄漏
   * 
   * 检查系统是否存在内存泄漏情况。
   */
  void checkMemoryLeaks();
  
  /**
   * @brief 更新内存统计信息
   * 
   * 更新内存使用统计信息，包括总分配内存和峰值内存。
   */
  void updateMemoryStats();
  
  /**
   * @brief 合并相似的内存池
   * 
   * 合并具有相同块大小的内存池，减少内存池数量。
   */
  void mergeSimilarPools();
  
  /**
   * @brief 分割过大的内存池
   * 
   * 分割过大的内存池，提高内存利用率。
   */
  void splitLargePools();
  
  /**
   * @brief 检查内存使用预警
   * 
   * 检查内存使用是否达到预警阈值。
   */
  void checkMemoryWarning();
  
public:
  /**
   * @brief 获取单例实例
   * 
   * @return MemoryManager* 内存管理类的单例实例
   */
  static MemoryManager* getInstance();
  
  /**
   * @brief 初始化内存管理器
   * 
   * @return bool 初始化是否成功
   */
  bool init();
  
  /**
   * @brief 创建内存池
   * 
   * @param blockSize 块大小
   * @param blockCount 块数量
   * @return void* 内存池指针
   */
  void* createMemoryPool(size_t blockSize, size_t blockCount);
  
  /**
   * @brief 从内存池分配内存
   * 
   * @param poolPtr 内存池指针
   * @param size 分配大小
   * @param file 调用文件
   * @param line 调用行号
   * @return void* 分配的内存指针
   */
  void* allocateFromPool(void* poolPtr, size_t size, const char* file = nullptr, int line = 0);
  
  /**
   * @brief 释放内存回内存池
   * 
   * @param poolPtr 内存池指针
   * @param ptr 要释放的内存指针
   */
  void freeToPool(void* poolPtr, void* ptr);
  
  /**
   * @brief 销毁内存池
   * 
   * @param poolPtr 内存池指针
   */
  void destroyMemoryPool(void* poolPtr);
  
  /**
   * @brief 获取内存池使用情况
   * 
   * @param poolPtr 内存池指针
   * @param totalBlocks 总块数
   * @param freeBlocks 空闲块数
   * @param usedBlocks 已使用块数
   */
  void getMemoryPoolInfo(void* poolPtr, size_t& totalBlocks, size_t& freeBlocks, size_t& usedBlocks);
  
  /**
   * @brief 执行内存清理
   * 
   * 检查内存泄漏并更新内存统计信息。
   */
  void cleanupMemory();
  
  /**
   * @brief 获取内存使用统计
   * 
   * @param totalMemory 总内存
   * @param usedMemory 已使用内存
   * @param peakMemory 峰值内存
   * @param freeMemory 空闲内存
   */
  void getMemoryStats(size_t& totalMemory, size_t& usedMemory, size_t& peakMemory, size_t& freeMemory);
  
  /**
   * @brief 获取内存池数量
   * 
   * @return size_t 内存池数量
   */
  size_t getMemoryPoolCount();
  
  /**
   * @brief 清理所有内存池
   * 
   * 销毁所有内存池并释放相关资源。
   */
  void clearAllMemoryPools();
  
  /**
   * @brief 直接分配内存
   * 
   * @param size 分配大小
   * @param file 调用文件
   * @param line 调用行号
   * @return void* 分配的内存指针
   */
  void* allocate(size_t size, const char* file = nullptr, int line = 0);
  
  /**
   * @brief 直接释放内存
   * 
   * @param ptr 要释放的内存指针
   */
  void free(void* ptr);
  
  /**
   * @brief 获取当前空闲内存
   * 
   * @return size_t 当前空闲内存大小
   */
  size_t getFreeMemory();
  
  /**
   * @brief 优化内存使用
   * 
   * 清理未使用的内存池并执行内存碎片整理。
   */
  void optimizeMemoryUsage();
  
  /**
   * @brief 设置内存警告阈值
   * 
   * @param threshold 内存使用率阈值（0-100）
   */
  void setMemoryWarningThreshold(size_t threshold);
  
  /**
   * @brief 打印内存分配详情
   * 
   * 打印当前所有内存分配的详细信息。
   */
  void printMemoryAllocations();
  
  /**
   * @brief 执行内存碎片整理
   * 
   * 尝试整理内存碎片，提高内存利用率。
   */
  void defragmentMemory();
};

#endif // MEMORY_MANAGER_H