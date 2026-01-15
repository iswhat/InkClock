#ifndef EXAMPLE_PLUGIN_H
#define EXAMPLE_PLUGIN_H

#include "../coresystem/base_plugin.h"

class ExamplePlugin : public BasePlugin {
public:
    ExamplePlugin();
    ~ExamplePlugin() override;

    // 插件接口方法
    std::string getName() const override;
    std::string getVersion() const override;
    std::string getDescription() const override;

    bool initialize() override;
    void update() override;
    void shutdown() override;

    bool isEnabled() const override;
    void setEnabled(bool enabled) override;

    // 插件特定方法
    void doSomething();

private:
    bool enabled;
    int counter;
};

// 注册插件
REGISTER_PLUGIN(ExamplePlugin);

#endif // EXAMPLE_PLUGIN_H