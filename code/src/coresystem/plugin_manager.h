#ifndef PLUGIN_MANAGER_H
#define PLUGIN_MANAGER_H

#include <string>
#include <vector>
#include <memory>
#include <map>

class IPlugin {
public:
    virtual ~IPlugin() = default;
    
    virtual std::string getName() const = 0;
    virtual std::string getVersion() const = 0;
    virtual std::string getDescription() const = 0;
    
    virtual bool initialize() = 0;
    virtual void update() = 0;
    virtual void shutdown() = 0;
    
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
};

class PluginManager {
public:
    static PluginManager& getInstance();
    
    bool registerPlugin(std::unique_ptr<IPlugin> plugin);
    bool unregisterPlugin(const std::string& name);
    
    void initializeAll();
    void updateAll();
    void shutdownAll();
    
    IPlugin* getPlugin(const std::string& name);
    std::vector<IPlugin*> getAllPlugins();
    std::vector<IPlugin*> getEnabledPlugins();
    
    bool enablePlugin(const std::string& name);
    bool disablePlugin(const std::string& name);
    
private:
    PluginManager() = default;
    ~PluginManager() = default;
    
    PluginManager(const PluginManager&) = delete;
    PluginManager& operator=(const PluginManager&) = delete;
    
    std::map<std::string, std::unique_ptr<IPlugin>> plugins;
};

#define REGISTER_PLUGIN(PluginClass) 
    class PluginClass##Registrar {
    public:
        PluginClass##Registrar() {
            PluginManager::getInstance().registerPlugin(std::make_unique<PluginClass>());
        }
    };
    static PluginClass##Registrar pluginRegistrar_##PluginClass;

#endif // PLUGIN_MANAGER_H
