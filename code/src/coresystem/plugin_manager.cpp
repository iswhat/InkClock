#include "plugin_manager.h"

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

bool PluginManager::registerPlugin(std::unique_ptr<IPlugin> plugin) {
    if (!plugin) {
        return false;
    }
    
    std::string name = plugin->getName();
    if (plugins.find(name) != plugins.end()) {
        return false;
    }
    
    plugins[name] = std::move(plugin);
    return true;
}

bool PluginManager::unregisterPlugin(const std::string& name) {
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        return false;
    }
    
    it->second->shutdown();
    plugins.erase(it);
    return true;
}

void PluginManager::initializeAll() {
    for (auto& [name, plugin] : plugins) {
        if (plugin->isEnabled()) {
            plugin->initialize();
        }
    }
}

void PluginManager::updateAll() {
    for (auto& [name, plugin] : plugins) {
        if (plugin->isEnabled()) {
            plugin->update();
        }
    }
}

void PluginManager::shutdownAll() {
    for (auto& [name, plugin] : plugins) {
        plugin->shutdown();
    }
}

IPlugin* PluginManager::getPlugin(const std::string& name) {
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        return nullptr;
    }
    return it->second.get();
}

std::vector<IPlugin*> PluginManager::getAllPlugins() {
    std::vector<IPlugin*> result;
    for (auto& [name, plugin] : plugins) {
        result.push_back(plugin.get());
    }
    return result;
}

std::vector<IPlugin*> PluginManager::getEnabledPlugins() {
    std::vector<IPlugin*> result;
    for (auto& [name, plugin] : plugins) {
        if (plugin->isEnabled()) {
            result.push_back(plugin.get());
        }
    }
    return result;
}

bool PluginManager::enablePlugin(const std::string& name) {
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        return false;
    }
    
    if (!it->second->isEnabled()) {
        it->second->setEnabled(true);
        it->second->initialize();
    }
    return true;
}

bool PluginManager::disablePlugin(const std::string& name) {
    auto it = plugins.find(name);
    if (it == plugins.end()) {
        return false;
    }
    
    if (it->second->isEnabled()) {
        it->second->shutdown();
        it->second->setEnabled(false);
    }
    return true;
}
