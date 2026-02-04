#include "memory_pool.h"

// 静态成员初始化
MemoryPool* MemoryPool::instance = nullptr;

// 私有构造函数
MemoryPool::MemoryPool() {
  totalSize = 0;
  usedSize = 0;
  freeBlocks = 0;
  usedBlocks = 0;
  lastAllocationTime = 0;
  lastFreeTime = 0;
  
  // 默认配置
  config.blockSize = 128;
  config.blockCount = 16;
  config.autoExpand = true;
  config.expandBlockCount = 8;
  config.maxPoolSize = 1024 * 1024; // 1MB
}

// 获取单例实例
MemoryPool* MemoryPool::getInstance() {
  if (instance == nullptr) {
    instance = new MemoryPool();
  }
  return instance;
}

// 初始化内存池
bool MemoryPool::init(MemoryPoolConfig config) {
  this->config = config;
  
  // 分配初始内存块
  if (!allocateBlocks(config.blockCount, config.blockSize)) {
    return false;
  }
  
  updateStats();
  return true;
}

// 分配内存块
bool MemoryPool::allocateBlocks(int count, size_t blockSize) {
  // Security: Parameter validation
  if (count <= 0 || blockSize == 0) {
    Serial.println("MemoryPool: Invalid parameters");
    return false;
  }

  // Security: Check for overflow
  if (static_cast<size_t>(count) > SIZE_MAX / blockSize) {
    Serial.println("MemoryPool: Potential integer overflow");
    return false;
  }

  // Security: Emergency release if needed
  if (usedSize >= totalSize * 0.9) {
    Serial.println("MemoryPool: Emergency release triggered");
    emergencyRelease();
  }

  for (int i = 0; i < count; i++) {
    // For ESP32, prefer ps_malloc (PSRAM) if available
    void* address = ps_malloc(blockSize);
    if (address == nullptr) {
      // Fallback to regular malloc
      address = malloc(blockSize);
      if (address == nullptr) {
        Serial.println("MemoryPool: Memory allocation failed");
        // Call emergency callback if configured
        if (emergencyCallback) {
          emergencyCallback();
        }
        return false;
      }
    }

    MemoryBlock block;
    block.address = address;
    block.size = blockSize;
    block.status = BLOCK_STATUS_FREE;
    block.allocTime = 0;
    block.owner = "";

    blocks.push_back(block);
    totalSize += blockSize;
  }

  return true;
}

// 查找可用内存块
MemoryBlock* MemoryPool::findFreeBlock(size_t size) {
  for (auto& block : blocks) {
    if (block.status == BLOCK_STATUS_FREE && block.size >= size) {
      return &block;
    }
  }
  return nullptr;
}

// 分配内存
void* MemoryPool::allocate(size_t size, const String& owner) {
  // 查找可用内存块
  MemoryBlock* block = findFreeBlock(size);
  
  // 如果没有找到可用内存块，尝试自动扩展
  if (block == nullptr && config.autoExpand) {
    size_t requiredSize = (size + config.blockSize - 1) / config.blockSize * config.blockSize;
    int requiredBlocks = (requiredSize + config.blockSize - 1) / config.blockSize;
    
    if (totalSize + requiredBlocks * config.blockSize <= config.maxPoolSize) {
      if (allocateBlocks(requiredBlocks, config.blockSize)) {
        block = findFreeBlock(size);
      }
    }
  }
  
  // 如果仍然没有找到可用内存块，返回nullptr
  if (block == nullptr) {
    return nullptr;
  }
  
  // 标记内存块为已使用
  block->status = BLOCK_STATUS_USED;
  block->allocTime = millis();
  block->owner = owner;
  
  // 更新统计信息
  lastAllocationTime = millis();
  updateStats();
  
  return block->address;
}

// 释放内存
bool MemoryPool::free(void* ptr) {
  for (auto& block : blocks) {
    if (block.address == ptr) {
      block.status = BLOCK_STATUS_FREE;
      block.allocTime = 0;
      block.owner = "";
      
      // 更新统计信息
      lastFreeTime = millis();
      updateStats();
      return true;
    }
  }
  return false;
}

// 保留内存
bool MemoryPool::reserve(size_t size, const String& owner) {
  void* ptr = allocate(size, owner);
  if (ptr != nullptr) {
    // 标记为保留状态
    for (auto& block : blocks) {
      if (block.address == ptr) {
        block.status = BLOCK_STATUS_RESERVED;
        updateStats();
        return true;
      }
    }
  }
  return false;
}

// 更新统计信息
void MemoryPool::updateStats() {
  usedSize = 0;
  freeBlocks = 0;
  usedBlocks = 0;
  
  for (const auto& block : blocks) {
    if (block.status == BLOCK_STATUS_USED || block.status == BLOCK_STATUS_RESERVED) {
      usedSize += block.size;
      usedBlocks++;
    } else if (block.status == BLOCK_STATUS_FREE) {
      freeBlocks++;
    }
  }
}

// 获取内存池状态
size_t MemoryPool::getTotalSize() {
  return totalSize;
}

size_t MemoryPool::getUsedSize() {
  updateStats();
  return usedSize;
}

size_t MemoryPool::getFreeSize() {
  updateStats();
  return totalSize - usedSize;
}

int MemoryPool::getFreeBlockCount() {
  updateStats();
  return freeBlocks;
}

int MemoryPool::getUsedBlockCount() {
  updateStats();
  return usedBlocks;
}

float MemoryPool::getUsagePercentage() {
  updateStats();
  if (totalSize == 0) {
    return 0.0;
  }
  return (float)usedSize / totalSize * 100.0;
}

// 获取内存块信息
std::vector<MemoryBlock> MemoryPool::getBlocksInfo() {
  return blocks;
}

// 清理内存池
void MemoryPool::cleanup() {
  // 释放所有未使用的内存块
  for (auto it = blocks.begin(); it != blocks.end();) {
    if (it->status == BLOCK_STATUS_FREE) {
      free(it->address);
      totalSize -= it->size;
      it = blocks.erase(it);
    } else {
      ++it;
    }
  }
  
  updateStats();
}

// 调整内存池大小
bool MemoryPool::resize(int blockCount, size_t blockSize) {
  // 清理未使用的内存块
  cleanup();
  
  // 计算当前块数
  int currentBlocks = blocks.size();
  
  if (blockCount > currentBlocks) {
    // 需要增加内存块
    int addBlocks = blockCount - currentBlocks;
    if (!allocateBlocks(addBlocks, blockSize)) {
      return false;
    }
  } else if (blockCount < currentBlocks) {
    // 需要减少内存块
    int removeBlocks = currentBlocks - blockCount;
    int removed = 0;
    
    for (auto it = blocks.begin(); it != blocks.end() && removed < removeBlocks;) {
      if (it->status == BLOCK_STATUS_FREE) {
        free(it->address);
        totalSize -= it->size;
        it = blocks.erase(it);
        removed++;
      } else {
        ++it;
      }
    }
  }
  
  // 更新配置
  config.blockSize = blockSize;
  config.blockCount = blocks.size();
  
  updateStats();
  return true;
}

// 检查内存泄漏
void MemoryPool::checkMemoryLeaks() {
  unsigned long currentTime = millis();
  unsigned long leakThreshold = 60000; // 60秒
  
  for (const auto& block : blocks) {
    if (block.status == BLOCK_STATUS_USED && block.allocTime > 0) {
      if (currentTime - block.allocTime > leakThreshold) {
        Serial.printf("内存泄漏警告: 内存块 %p 被 %s 占用超过 %ld 毫秒\n", 
                     block.address, block.owner.c_str(), currentTime - block.allocTime);
      }
    }
  }
}

// 获取内存池配置
MemoryPoolConfig MemoryPool::getConfig() {
  return config;
}

// 设置内存池配置
bool MemoryPool::setConfig(MemoryPoolConfig config) {
  this->config = config;
  return true;
}
