# 固件单元测试框架

本目录包含InkClock固件的单元测试代码，使用ArduinoUnit测试框架。

## 目录结构

```
test/
├── lib/               # 测试依赖库
├── src/               # 测试源代码
├── ArduinoUnit/       # ArduinoUnit测试框架
├── platformio.ini     # PlatformIO配置文件
└── README.md          # 本说明文件
```

## 测试框架

- **ArduinoUnit**: 用于Arduino/ESP32平台的单元测试框架
- **PlatformIO**: 用于构建和运行测试

## 如何运行测试

1. 安装PlatformIO IDE或CLI
2. 打开项目根目录
3. 运行测试命令：
   ```
   pio test
   ```

## 测试类型

- **单元测试**: 测试单个函数或类
- **集成测试**: 测试多个模块的协作
- **模拟测试**: 使用模拟对象测试外部依赖

## 测试命名规范

- 测试文件命名: `test_模块名.cpp`
- 测试用例命名: `test_功能名`
- 测试组命名: `test_模块名`

## 示例测试

```cpp
#include <ArduinoUnit.h>
#include "../src/application/time_manager.h"

TestSuite timeManagerTests;

test(timeManagerTests, TestInitialize) {
    TimeManager timeManager;
    assertTrue(timeManager.initialize());
}

test(timeManagerTests, TestGetCurrentTime) {
    TimeManager timeManager;
    timeManager.initialize();
    time_t currentTime = timeManager.getCurrentTime();
    assertTrue(currentTime > 0);
}

void setup() {
    Serial.begin(115200);
    while (!Serial) {}
}

void loop() {
    TestSuiteRunner::run(timeManagerTests);
}
```
