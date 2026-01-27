#include "core_system.h"

// 实现ICoreSystem接口的getInstance()方法
ICoreSystem* ICoreSystem::getInstance() {
  return CoreSystem::getInstance();
}


