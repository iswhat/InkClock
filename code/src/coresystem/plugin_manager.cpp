#include "plugin_manager.h"

PluginManager::PluginManager() {
    pluginMutex = xSemaphoreCreateMutex();
}

PluginManager::~PluginManager() {
    shutdownAll();
    if (pluginMutex) {
        vSemaphoreDelete(pluginMutex);
    }
}

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

bool PluginManager::registerPlugin(std::unique_ptr<IPlugin> plugin) {
    if (!plugin) {
        return false;
    }
    
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    std::string name = plugin->getName();
    if (plugins.find(name) != plugins.end()) {
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
        return false;
    }
    
    IPlugin* pluginPtr = plugin.get();
    plugins[name] = std::move(plugin);
    
    // 更新插件信息
    updatePluginInfo(name, pluginPtr);
    
    // 解析依赖
    resolveDependencies();
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return true;
}

bool PluginManager::unregisterPlugin(const std::string& name) {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
        return false;
    }
    
    // 禁用插件及其依赖
    disablePluginWithDependents(name);
    
    // 关闭插件
    it->second->shutdown();
    plugins.erase(it);
    pluginInfos.erase(name);
    
    // 重新解析依赖
    resolveDependencies();
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return true;
}

void PluginManager::updatePluginInfo(const std::string& name, IPlugin* plugin) {
    PluginInfo info;
    info.name = name;
    info.version = plugin->getVersion();
    info.description = plugin->getDescription();
    info.enabled = plugin->isEnabled();
    info.initialized = false;
    info.priority = plugin->getPriority();
    info.dependencies = plugin->getDependencies();
    info.dependents = {};
    
    pluginInfos[name] = info;
}

void PluginManager::resolveDependencies() {
    // 重置所有依赖关系
    for (auto& [name, info] : pluginInfos) {
        info.dependents.clear();
        pluginInfos[name] = info;
    }
    
    // 建立依赖关系
    for (auto& [name, info] : pluginInfos) {
        for (const auto& dep : info.dependencies) {
            if (pluginInfos.find(dep) != pluginInfos.end()) {
                pluginInfos[dep].dependents.push_back(name);
            }
        }
    }
}

void PluginManager::initializeAll() {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    // 按优先级排序插件
    std::vector<IPlugin*> sortedPlugins = getPluginsByPriority();
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    for (auto plugin : sortedPlugins) {
        if (plugin->isEnabled() && checkDependencies(plugin->getName())) {
            plugin->initialize();
            
            if (pluginMutex) {
                xSemaphoreTake(pluginMutex, portMAX_DELAY);
            }
            if (pluginInfos.find(plugin->getName()) != pluginInfos.end()) {
                pluginInfos[plugin->getName()].initialized = true;
            }
            if (pluginMutex) {
                xSemaphoreGive(pluginMutex);
            }
        }
    }
}

void PluginManager::updateAll() {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    // 按优先级排序插件
    std::vector<IPlugin*> sortedPlugins = getPluginsByPriority();
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    for (auto plugin : sortedPlugins) {
        if (plugin->isEnabled() && plugin->isReady()) {
            plugin->update();
        }
    }
}

void PluginManager::shutdownAll() {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    // 按优先级逆序排序插件
    std::vector<IPlugin*> sortedPlugins = getPluginsByPriority();
    std::reverse(sortedPlugins.begin(), sortedPlugins.end());
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    for (auto plugin : sortedPlugins) {
        plugin->shutdown();
        
        if (pluginMutex) {
            xSemaphoreTake(pluginMutex, portMAX_DELAY);
        }
        if (pluginInfos.find(plugin->getName()) != pluginInfos.end()) {
            pluginInfos[plugin->getName()].initialized = false;
        }
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
    }
}

IPlugin* PluginManager::getPlugin(const std::string& name) {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    auto it = plugins.find(name);
    IPlugin* plugin = nullptr;
    if (it != plugins.end()) {
        plugin = it->second.get();
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return plugin;
}

std::vector<IPlugin*> PluginManager::getAllPlugins() {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    std::vector<IPlugin*> result;
    for (auto& [name, plugin] : plugins) {
        result.push_back(plugin.get());
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return result;
}

std::vector<IPlugin*> PluginManager::getEnabledPlugins() {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    std::vector<IPlugin*> result;
    for (auto& [name, plugin] : plugins) {
        if (plugin->isEnabled()) {
            result.push_back(plugin.get());
        }
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return result;
}

std::vector<PluginInfo> PluginManager::getPluginInfos() {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    std::vector<PluginInfo> result;
    for (auto& [name, info] : pluginInfos) {
        result.push_back(info);
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return result;
}

bool PluginManager::enablePlugin(const std::string& name) {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
        return false;
    }
    
    if (it->second->isEnabled()) {
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
        return true;
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    // 启用插件及其依赖
    return enablePluginWithDependencies(name);
}

bool PluginManager::disablePlugin(const std::string& name) {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
        return false;
    }
    
    if (!it->second->isEnabled()) {
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
        return true;
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    // 禁用插件及其依赖
    return disablePluginWithDependents(name);
}

bool PluginManager::checkDependencies(const std::string& pluginName) {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    auto it = pluginInfos.find(pluginName);
    if (it == pluginInfos.end()) {
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
        return false;
    }
    
    const PluginInfo& info = it->second;
    for (const auto& dep : info.dependencies) {
        auto depIt = pluginInfos.find(dep);
        if (depIt == pluginInfos.end() || !depIt->second.enabled || !depIt->second.initialized) {
            if (pluginMutex) {
                xSemaphoreGive(pluginMutex);
            }
            return false;
        }
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return true;
}

std::vector<IPlugin*> PluginManager::getPluginsByPriority() {
    std::vector<std::pair<int, IPlugin*>> pluginPairs;
    
    for (auto& [name, plugin] : plugins) {
        pluginPairs.emplace_back(plugin->getPriority(), plugin.get());
    }
    
    // 按优先级降序排序
    std::sort(pluginPairs.begin(), pluginPairs.end(), 
        [](const std::pair<int, IPlugin*>& a, const std::pair<int, IPlugin*>& b) {
            return a.first > b.first;
        });
    
    std::vector<IPlugin*> result;
    for (const auto& pair : pluginPairs) {
        result.push_back(pair.second);
    }
    
    return result;
}

bool PluginManager::canEnablePlugin(const std::string& pluginName) {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    auto it = pluginInfos.find(pluginName);
    if (it == pluginInfos.end()) {
        if (pluginMutex) {
            xSemaphoreGive(pluginMutex);
        }
        return false;
    }
    
    const PluginInfo& info = it->second;
    for (const auto& dep : info.dependencies) {
        auto depIt = pluginInfos.find(dep);
        if (depIt == pluginInfos.end()) {
            if (pluginMutex) {
                xSemaphoreGive(pluginMutex);
            }
            return false;
        }
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return true;
}

bool PluginManager::enablePluginWithDependencies(const std::string& pluginName) {
    if (!canEnablePlugin(pluginName)) {
        return false;
    }
    
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    // 先启用依赖
    auto it = pluginInfos.find(pluginName);
    if (it != pluginInfos.end()) {
        for (const auto& dep : it->second.dependencies) {
            auto depIt = plugins.find(dep);
            if (depIt != plugins.end() && !depIt->second->isEnabled()) {
                if (pluginMutex) {
                    xSemaphoreGive(pluginMutex);
                }
                if (!enablePluginWithDependencies(dep)) {
                    return false;
                }
                if (pluginMutex) {
                    xSemaphoreTake(pluginMutex, portMAX_DELAY);
                }
            }
        }
        
        // 启用当前插件
        auto pluginIt = plugins.find(pluginName);
        if (pluginIt != plugins.end()) {
            pluginIt->second->setEnabled(true);
            pluginIt->second->initialize();
            pluginInfos[pluginName].enabled = true;
            pluginInfos[pluginName].initialized = true;
        }
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return true;
}

bool PluginManager::disablePluginWithDependents(const std::string& pluginName) {
    if (pluginMutex) {
        xSemaphoreTake(pluginMutex, portMAX_DELAY);
    }
    
    // 先禁用依赖于当前插件的插件
    auto it = pluginInfos.find(pluginName);
    if (it != pluginInfos.end()) {
        for (const auto& dependent : it->second.dependents) {
            auto depIt = plugins.find(dependent);
            if (depIt != plugins.end() && depIt->second->isEnabled()) {
                if (pluginMutex) {
                    xSemaphoreGive(pluginMutex);
                }
                if (!disablePluginWithDependents(dependent)) {
                    return false;
                }
                if (pluginMutex) {
                    xSemaphoreTake(pluginMutex, portMAX_DELAY);
                }
            }
        }
        
        // 禁用当前插件
        auto pluginIt = plugins.find(pluginName);
        if (pluginIt != plugins.end()) {
            pluginIt->second->shutdown();
            pluginIt->second->setEnabled(false);
            pluginInfos[pluginName].enabled = false;
            pluginInfos[pluginName].initialized = false;
        }
    }
    
    if (pluginMutex) {
        xSemaphoreGive(pluginMutex);
    }
    
    return true;
}
