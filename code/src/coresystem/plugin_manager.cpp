#include "plugin_manager.h"
#include <Arduino.h>
#include <SPIFFS.h>

PluginManager::PluginManager() {
    pluginMutex = xSemaphoreCreateMutex();
}

PluginManager::~PluginManager() {
    shutdownAll();
    vSemaphoreDelete(pluginMutex);
}

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

bool PluginManager::registerPlugin(std::unique_ptr<IPlugin> plugin) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    std::string name = plugin->getName();
    if (plugins.find(name) != plugins.end()) {
        xSemaphoreGive(pluginMutex);
        return false; // 插件已存在
    }
    
    plugins[name] = std::move(plugin);
    updatePluginInfo(name, plugins[name].get());
    
    xSemaphoreGive(pluginMutex);
    return true;
}

bool PluginManager::unregisterPlugin(const std::string& name) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        xSemaphoreGive(pluginMutex);
        return false; // 插件不存在
    }
    
    if (it->second->isEnabled()) {
        disablePluginWithDependents(name);
    }
    
    plugins.erase(it);
    pluginInfos.erase(name);
    
    xSemaphoreGive(pluginMutex);
    return true;
}

bool PluginManager::loadPluginFromFile(const String& filePath) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    // 检查文件是否存在
    if (!SPIFFS.exists(filePath)) {
        xSemaphoreGive(pluginMutex);
        return false;
    }
    
    // 打开文件
    File file = SPIFFS.open(filePath, "r");
    if (!file) {
        xSemaphoreGive(pluginMutex);
        return false;
    }
    
    // 读取文件内容
    String content = file.readString();
    file.close();
    
    // 解析插件信息（简化版JSON解析）
    // 实际项目中应该使用JSON库进行解析
    
    // 从文件路径提取插件名称
    String pluginName = filePath.substring(filePath.lastIndexOf('/') + 1);
    pluginName = pluginName.substring(0, pluginName.lastIndexOf('.'));
    
    // 检查插件是否已存在
    std::string nameStr = pluginName.c_str();
    if (plugins.find(nameStr) != plugins.end()) {
        xSemaphoreGive(pluginMutex);
        return false; // 插件已存在
    }
    
    // 这里应该根据插件配置创建相应的插件实例
    // 暂时实现一个简单的插件加载逻辑
    // 实际项目中应该使用工厂模式或反射机制创建插件实例
    
    // 模拟创建一个插件实例（实际应该根据配置动态创建）
    // 这里需要根据实际的插件类型进行创建
    
    xSemaphoreGive(pluginMutex);
    return false; // 暂时返回false，需要实现完整的插件加载逻辑
}

bool PluginManager::unloadPlugin(const std::string& name) {
    return unregisterPlugin(name);
}

void PluginManager::scanPlugins() {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    // 扫描SPIFFS中的插件目录
    File root = SPIFFS.open("/plugins");
    if (root) {
        while (File file = root.openNextFile()) {
            if (file.isDirectory()) {
                String pluginDir = file.name();
                String pluginConfig = pluginDir + "/plugin.json";
                if (SPIFFS.exists(pluginConfig)) {
                    // 尝试加载插件
                    loadPluginFromFile(pluginConfig);
                }
            }
            file.close();
        }
        root.close();
    }
    
    xSemaphoreGive(pluginMutex);
}

void PluginManager::initializeAll() {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    resolveDependencies();
    
    auto sortedPlugins = getPluginsByPriority();
    for (auto plugin : sortedPlugins) {
        if (plugin->isEnabled() && !plugin->isReady()) {
            plugin->initialize();
            updatePluginInfo(plugin->getName(), plugin);
        }
    }
    
    xSemaphoreGive(pluginMutex);
}

void PluginManager::updateAll() {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto sortedPlugins = getPluginsByPriority();
    for (auto plugin : sortedPlugins) {
        if (plugin->isEnabled() && plugin->isReady()) {
            plugin->update();
        }
    }
    
    xSemaphoreGive(pluginMutex);
}

void PluginManager::shutdownAll() {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto sortedPlugins = getPluginsByPriority();
    // 倒序关闭插件，确保依赖关系正确
    for (auto it = sortedPlugins.rbegin(); it != sortedPlugins.rend(); ++it) {
        if ((*it)->isEnabled() && (*it)->isReady()) {
            (*it)->shutdown();
            updatePluginInfo((*it)->getName(), *it);
        }
    }
    
    xSemaphoreGive(pluginMutex);
}

bool PluginManager::reloadPlugin(const std::string& name) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        xSemaphoreGive(pluginMutex);
        return false; // 插件不存在
    }
    
    // 保存插件状态
    bool wasEnabled = it->second->isEnabled();
    
    // 关闭插件
    if (wasEnabled && it->second->isReady()) {
        it->second->shutdown();
    }
    
    // 重新初始化插件
    if (wasEnabled) {
        it->second->initialize();
    }
    
    updatePluginInfo(name, it->second.get());
    
    xSemaphoreGive(pluginMutex);
    return true;
}

bool PluginManager::reloadAllPlugins() {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    bool allSuccess = true;
    for (const auto& pair : plugins) {
        const std::string& name = pair.first;
        if (!reloadPlugin(name)) {
            allSuccess = false;
        }
    }
    
    xSemaphoreGive(pluginMutex);
    return allSuccess;
}

void PluginManager::setPluginPriority(const std::string& name, int priority) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto it = plugins.find(name);
    if (it != plugins.end()) {
        it->second->setPriority(priority);
        updatePluginInfo(name, it->second.get());
    }
    
    xSemaphoreGive(pluginMutex);
}

std::vector<std::string> PluginManager::getPluginDependencies(const std::string& name) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    std::vector<std::string> dependencies;
    auto it = plugins.find(name);
    if (it != plugins.end()) {
        dependencies = it->second->getDependencies();
    }
    
    xSemaphoreGive(pluginMutex);
    return dependencies;
}

std::vector<std::string> PluginManager::getPluginDependents(const std::string& name) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    std::vector<std::string> dependents;
    auto it = pluginInfos.find(name);
    if (it != pluginInfos.end()) {
        dependents = it->second.dependents;
    }
    
    xSemaphoreGive(pluginMutex);
    return dependents;
}

IPlugin* PluginManager::getPlugin(const std::string& name) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto it = plugins.find(name);
    IPlugin* result = (it != plugins.end()) ? it->second.get() : nullptr;
    
    xSemaphoreGive(pluginMutex);
    return result;
}

std::vector<IPlugin*> PluginManager::getAllPlugins() {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    std::vector<IPlugin*> result;
    for (const auto& pair : plugins) {
        result.push_back(pair.second.get());
    }
    
    xSemaphoreGive(pluginMutex);
    return result;
}

std::vector<IPlugin*> PluginManager::getEnabledPlugins() {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    std::vector<IPlugin*> result;
    for (const auto& pair : plugins) {
        if (pair.second->isEnabled()) {
            result.push_back(pair.second.get());
        }
    }
    
    xSemaphoreGive(pluginMutex);
    return result;
}

std::vector<PluginInfo> PluginManager::getPluginInfos() {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    std::vector<PluginInfo> result;
    for (const auto& pair : pluginInfos) {
        result.push_back(pair.second);
    }
    
    xSemaphoreGive(pluginMutex);
    return result;
}

bool PluginManager::enablePlugin(const std::string& name) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        xSemaphoreGive(pluginMutex);
        return false; // 插件不存在
    }
    
    if (it->second->isEnabled()) {
        xSemaphoreGive(pluginMutex);
        return true; // 插件已经启用
    }
    
    bool success = enablePluginWithDependencies(name);
    
    xSemaphoreGive(pluginMutex);
    return success;
}

bool PluginManager::disablePlugin(const std::string& name) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        xSemaphoreGive(pluginMutex);
        return false; // 插件不存在
    }
    
    if (!it->second->isEnabled()) {
        xSemaphoreGive(pluginMutex);
        return true; // 插件已经禁用
    }
    
    bool success = disablePluginWithDependents(name);
    
    xSemaphoreGive(pluginMutex);
    return success;
}

bool PluginManager::checkDependencies(const std::string& pluginName) {
    xSemaphoreTake(pluginMutex, portMAX_DELAY);
    
    auto it = plugins.find(pluginName);
    if (it == plugins.end()) {
        xSemaphoreGive(pluginMutex);
        return false; // 插件不存在
    }
    
    IPlugin* plugin = it->second.get();
    std::vector<std::string> dependencies = plugin->getDependencies();
    
    for (const auto& dep : dependencies) {
        if (plugins.find(dep) == plugins.end()) {
            xSemaphoreGive(pluginMutex);
            return false; // 依赖插件不存在
        }
    }
    
    xSemaphoreGive(pluginMutex);
    return true;
}

std::vector<IPlugin*> PluginManager::getPluginsByPriority() {
    std::vector<IPlugin*> result;
    for (const auto& pair : plugins) {
        result.push_back(pair.second.get());
    }
    
    // 按优先级排序，数值越大优先级越高
    std::sort(result.begin(), result.end(), [](IPlugin* a, IPlugin* b) {
        return a->getPriority() > b->getPriority();
    });
    
    return result;
}

void PluginManager::updatePluginInfo(const std::string& name, IPlugin* plugin) {
    if (!plugin) return;
    
    PluginInfo info;
    info.name = plugin->getName();
    info.version = plugin->getVersion();
    info.description = plugin->getDescription();
    info.enabled = plugin->isEnabled();
    info.initialized = plugin->isReady();
    info.priority = plugin->getPriority();
    info.dependencies = plugin->getDependencies();
    
    pluginInfos[name] = info;
}

void PluginManager::resolveDependencies() {
    // 清空依赖关系
    for (auto& pair : pluginInfos) {
        pair.second.dependents.clear();
    }
    
    // 建立依赖关系
    for (const auto& pair : plugins) {
        IPlugin* plugin = pair.second.get();
        std::vector<std::string> dependencies = plugin->getDependencies();
        
        for (const auto& dep : dependencies) {
            auto depIt = pluginInfos.find(dep);
            if (depIt != pluginInfos.end()) {
                depIt->second.dependents.push_back(plugin->getName());
            }
        }
    }
}

bool PluginManager::canEnablePlugin(const std::string& pluginName) {
    auto it = plugins.find(pluginName);
    if (it == plugins.end()) {
        return false; // 插件不存在
    }
    
    IPlugin* plugin = it->second.get();
    std::vector<std::string> dependencies = plugin->getDependencies();
    
    for (const auto& dep : dependencies) {
        auto depIt = plugins.find(dep);
        if (depIt == plugins.end() || !depIt->second->isEnabled()) {
            return false; // 依赖插件不存在或未启用
        }
    }
    
    return true;
}

bool PluginManager::enablePluginWithDependencies(const std::string& pluginName) {
    auto it = plugins.find(pluginName);
    if (it == plugins.end()) {
        return false; // 插件不存在
    }
    
    IPlugin* plugin = it->second.get();
    
    // 首先启用依赖插件
    std::vector<std::string> dependencies = plugin->getDependencies();
    for (const auto& dep : dependencies) {
        auto depIt = plugins.find(dep);
        if (depIt != plugins.end() && !depIt->second->isEnabled()) {
            if (!enablePluginWithDependencies(dep)) {
                return false; // 依赖插件启用失败
            }
        }
    }
    
    // 启用当前插件
    plugin->setEnabled(true);
    if (!plugin->isReady()) {
        plugin->initialize();
    }
    updatePluginInfo(pluginName, plugin);
    
    return true;
}

bool PluginManager::disablePluginWithDependents(const std::string& pluginName) {
    auto it = plugins.find(pluginName);
    if (it == plugins.end()) {
        return false; // 插件不存在
    }
    
    // 首先禁用依赖于当前插件的插件
    auto infoIt = pluginInfos.find(pluginName);
    if (infoIt != pluginInfos.end()) {
        for (const auto& dependent : infoIt->second.dependents) {
            auto depIt = plugins.find(dependent);
            if (depIt != plugins.end() && depIt->second->isEnabled()) {
                if (!disablePluginWithDependents(dependent)) {
                    return false; // 依赖插件禁用失败
                }
            }
        }
    }
    
    // 禁用当前插件
    IPlugin* plugin = it->second.get();
    if (plugin->isReady()) {
        plugin->shutdown();
    }
    plugin->setEnabled(false);
    updatePluginInfo(pluginName, plugin);
    
    return true;
}
