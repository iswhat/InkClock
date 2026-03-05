#include "example_plugin.h"
#include "coresystem/dependency_injection.h"

ExamplePlugin::ExamplePlugin() : BasePlugin("Example Plugin", "1.0.0", "An example plugin for demonstration purposes") {
    enabled = true;
    counter = 0;
    lastUpdateTime = 0;
}

ExamplePlugin::~ExamplePlugin() {
    shutdown();
}

std::string ExamplePlugin::getName() const {
    return "Example Plugin";
}

std::string ExamplePlugin::getVersion() const {
    return "1.0.0";
}

std::string ExamplePlugin::getDescription() const {
    return "An example plugin for demonstration purposes";
}

bool ExamplePlugin::initialize() {
    if (!BasePlugin::initialize()) {
        return false;
    }
    
    lastUpdateTime = millis();
    counter = 0;
    
    // 获取其他管理器实例
    auto displayManager = DependencyInjectionContainer::getInstance()->getDisplayManager();
    if (displayManager) {
        // 可以通过显示管理器添加消息
    }
    
    return true;
}

void ExamplePlugin::update() {
    BasePlugin::update();
    
    // 每5秒更新一次
    if (millis() - lastUpdateTime >= 5000) {
        lastUpdateTime = millis();
        counter++;
        
        // 可以在这里添加插件的更新逻辑
    }
}

void ExamplePlugin::shutdown() {
    BasePlugin::shutdown();
    
    // 清理插件资源
}

bool ExamplePlugin::isEnabled() const {
    return enabled;
}

void ExamplePlugin::setEnabled(bool enabled) {
    this->enabled = enabled;
}

void ExamplePlugin::doSomething() {
    // 插件特定方法的实现
}


