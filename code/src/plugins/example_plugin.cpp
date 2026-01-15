#include "example_plugin.h"
#include "dependency_injection.h"

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

// 注册插件
REGISTER_PLUGIN(ExamplePlugin);
