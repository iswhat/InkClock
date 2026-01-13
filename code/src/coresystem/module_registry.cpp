#include "module_registry.h"

// 初始化单例实例
ModuleRegistry* ModuleRegistry::instance = nullptr;

// 更新模块状态
void ModuleRegistry::updateModuleStatus(const String& moduleName, ModuleStatus status) {
  for (auto& module : modules) {
    if (module.name == moduleName) {
      module.status = status;
      module.lastActiveTime = millis();
      
      // 发布模块状态变化事件
      auto moduleData = std::make_shared<ModuleEventData>(moduleName, module.type);
      eventBus->publish(EVENT_MODULE_STATUS_CHANGED, moduleData);
      break;
    }
  }
}

// 注册模块
bool ModuleRegistry::registerModule(IModule* module) {
  if (module == nullptr) {
    Serial.println("Error: Attempt to register null module");
    return false;
  }
  
  // 检查模块是否已经注册
  for (auto& existingModule : modules) {
    if (existingModule.name == module->getName()) {
      Serial.printf("Warning: Module already registered: %s\n", module->getName().c_str());
      return false;
    }
  }
  
  // 创建模块信息
  ModuleInfo moduleInfo;
  moduleInfo.name = module->getName();
  moduleInfo.type = module->getModuleType();
  moduleInfo.status = MODULE_STATUS_UNINITIALIZED;
  moduleInfo.enabled = false;
  moduleInfo.loaded = false;
  moduleInfo.lastActiveTime = 0;
  moduleInfo.startTime = 0;
  moduleInfo.errorCount = 0;
  moduleInfo.modulePtr = module;
  
  modules.push_back(moduleInfo);
  
  // 发布模块注册事件
  auto moduleData = std::make_shared<ModuleEventData>(module->getName(), module->getModuleType());
  eventBus->publish(EVENT_MODULE_REGISTERED, moduleData);
  
  Serial.printf("Module registered: %s\n", module->getName().c_str());
  return true;
}

// 卸载模块
bool ModuleRegistry::unregisterModule(const String& moduleName) {
  for (auto it = modules.begin(); it != modules.end(); ++it) {
    if (it->name == moduleName) {
      // 发布模块卸载事件
      auto moduleData = std::make_shared<ModuleEventData>(moduleName, it->type);
      eventBus->publish(EVENT_MODULE_UNREGISTERED, moduleData);
      
      // 更新模块状态
      updateModuleStatus(moduleName, MODULE_STATUS_UNLOADED);
      
      // 清理模块资源
      IModule* module = static_cast<IModule*>(it->modulePtr);
      delete module;
      
      modules.erase(it);
      
      Serial.printf("Module unregistered: %s\n", moduleName.c_str());
      return true;
    }
  }
  
  Serial.printf("Error: Module not found for unregistration: %s\n", moduleName.c_str());
  return false;
}

// 加载模块
bool ModuleRegistry::loadModule(const String& moduleName) {
  for (auto& module : modules) {
    if (module.name == moduleName && !module.loaded) {
      updateModuleStatus(moduleName, MODULE_STATUS_INITIALIZING);
      
      try {
        IModule* modulePtr = static_cast<IModule*>(module.modulePtr);
        modulePtr->init();
        
        module.loaded = true;
        module.enabled = true;
        module.status = MODULE_STATUS_READY;
        module.startTime = millis();
        module.lastActiveTime = millis();
        
        updateModuleStatus(moduleName, MODULE_STATUS_READY);
        
        Serial.printf("Module loaded: %s\n", moduleName.c_str());
        return true;
      } catch (const std::exception& e) {
        Serial.printf("Error loading module %s: %s\n", moduleName.c_str(), e.what());
        updateModuleStatus(moduleName, MODULE_STATUS_ERROR);
        return false;
      }
    }
  }
  
  Serial.printf("Error: Module not found or already loaded: %s\n", moduleName.c_str());
  return false;
}

// 卸载模块
bool ModuleRegistry::unloadModule(const String& moduleName) {
  for (auto& module : modules) {
    if (module.name == moduleName && module.loaded) {
      module.loaded = false;
      module.enabled = false;
      updateModuleStatus(moduleName, MODULE_STATUS_UNLOADED);
      
      Serial.printf("Module unloaded: %s\n", moduleName.c_str());
      return true;
    }
  }
  
  Serial.printf("Error: Module not found or not loaded: %s\n", moduleName.c_str());
  return false;
}

// 启用模块
bool ModuleRegistry::enableModule(const String& moduleName) {
  for (auto& module : modules) {
    if (module.name == moduleName && !module.enabled) {
      module.enabled = true;
      if (module.loaded) {
        updateModuleStatus(moduleName, MODULE_STATUS_READY);
      }
      
      // 发布模块启用事件
      auto moduleData = std::make_shared<ModuleEventData>(moduleName, module.type);
      eventBus->publish(EVENT_MODULE_ENABLED, moduleData);
      
      Serial.printf("Module enabled: %s\n", moduleName.c_str());
      return true;
    }
  }
  
  Serial.printf("Error: Module not found or already enabled: %s\n", moduleName.c_str());
  return false;
}

// 禁用模块
bool ModuleRegistry::disableModule(const String& moduleName) {
  for (auto& module : modules) {
    if (module.name == moduleName && module.enabled) {
      module.enabled = false;
      if (module.loaded) {
        updateModuleStatus(moduleName, MODULE_STATUS_DISABLED);
      }
      
      // 发布模块禁用事件
      auto moduleData = std::make_shared<ModuleEventData>(moduleName, module.type);
      eventBus->publish(EVENT_MODULE_DISABLED, moduleData);
      
      Serial.printf("Module disabled: %s\n", moduleName.c_str());
      return true;
    }
  }
  
  Serial.printf("Error: Module not found or not enabled: %s\n", moduleName.c_str());
  return false;
}

// 根据名称获取模块
void* ModuleRegistry::getModuleByName(const String& moduleName) {
  for (auto& module : modules) {
    if (module.name == moduleName && module.loaded && module.enabled) {
      return module.modulePtr;
    }
  }
  return nullptr;
}

// 根据类型获取模块
void* ModuleRegistry::getModuleByType(ModuleType type) {
  for (auto& module : modules) {
    if (module.type == type && module.loaded && module.enabled) {
      return module.modulePtr;
    }
  }
  return nullptr;
}

// 获取所有模块信息
std::vector<ModuleInfo> ModuleRegistry::getModulesInfo() {
  return modules;
}

// 运行所有启用的模块
void ModuleRegistry::runModules() {
  for (auto& module : modules) {
    if (module.loaded && module.enabled && module.status == MODULE_STATUS_READY) {
      try {
        IModule* modulePtr = static_cast<IModule*>(module.modulePtr);
        if (modulePtr->shouldRun()) {
          modulePtr->loop();
          module.lastActiveTime = millis();
          updateModuleStatus(module.name, MODULE_STATUS_RUNNING);
        }
      } catch (const std::exception& e) {
        Serial.printf("Error running module %s: %s\n", module.name.c_str(), e.what());
        module.errorCount++;
        updateModuleStatus(module.name, MODULE_STATUS_ERROR);
      }
    }
  }
}

// 清理未使用的模块
void ModuleRegistry::cleanupUnusedModules() {
  Serial.println("Cleaning up unused modules...");
  
  for (auto it = modules.begin(); it != modules.end();) {
    if (!it->loaded && millis() - it->lastActiveTime > 300000) { // 5分钟未使用
      Serial.printf("Cleaning up unused module: %s\n", it->name.c_str());
      
      // 发布模块卸载事件
      auto moduleData = std::make_shared<ModuleEventData>(it->name, it->type);
      eventBus->publish(EVENT_MODULE_UNREGISTERED, moduleData);
      
      // 清理模块资源
      IModule* module = static_cast<IModule*>(it->modulePtr);
      delete module;
      
      it = modules.erase(it);
    } else {
      ++it;
    }
  }
}

// 初始化所有模块
void ModuleRegistry::initAllModules() {
  Serial.println("Initializing all modules...");
  
  for (auto& module : modules) {
    if (!module.loaded) {
      loadModule(module.name);
    }
  }
}

// 关闭所有模块
void ModuleRegistry::shutdownAllModules() {
  Serial.println("Shutting down all modules...");
  
  for (auto& module : modules) {
    if (module.loaded) {
      unloadModule(module.name);
    }
  }
}
