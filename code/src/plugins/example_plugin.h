#ifndef EXAMPLE_PLUGIN_H
#define EXAMPLE_PLUGIN_H

#include "../coresystem/base_plugin.h"

class ExamplePlugin : public BasePlugin {
public:
    ExamplePlugin() : BasePlugin("ExamplePlugin", "1.0.0", "示例插件，展示插件系统使用方法") {}
    
    bool initialize() override;
    void update() override;
    void shutdown() override;
    
private:
    unsigned long lastUpdateTime;
    int counter;
};

#endif // EXAMPLE_PLUGIN_H
