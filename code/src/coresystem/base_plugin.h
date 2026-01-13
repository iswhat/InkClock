#ifndef BASE_PLUGIN_H
#define BASE_PLUGIN_H

#include "plugin_manager.h"

class BasePlugin : public IPlugin {
public:
    BasePlugin(const std::string& name, const std::string& version, const std::string& description) 
        : name(name), version(version), description(description), enabled(true) {}
    
    virtual ~BasePlugin() = default;
    
    std::string getName() const override {
        return name;
    }
    
    std::string getVersion() const override {
        return version;
    }
    
    std::string getDescription() const override {
        return description;
    }
    
    bool isEnabled() const override {
        return enabled;
    }
    
    void setEnabled(bool enabled) override {
        this->enabled = enabled;
    }
    
    bool initialize() override {
        return true;
    }
    
    void update() override {
    }
    
    void shutdown() override {
    }
    
protected:
    std::string name;
    std::string version;
    std::string description;
    bool enabled;
};

#endif // BASE_PLUGIN_H
