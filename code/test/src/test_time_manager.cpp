#include <Arduino.h>
#include <ArduinoUnit.h>

// 包含要测试的模块头文件
#include "../../src/application/time_manager.h"

// 创建测试套件
TestSuite timeManagerTests;

// 测试时间管理器初始化
test(timeManagerTests, TestInitialize) {
    TimeManager timeManager;
    timeManager.init();
    // 简单验证初始化没有崩溃
    assertTrue(true);
}

// 测试获取时间字符串
test(timeManagerTests, TestGetTimeString) {
    TimeManager timeManager;
    timeManager.init();
    
    String timeStr = timeManager.getTimeString();
    // 验证时间字符串不为空
    assertFalse(timeStr.isEmpty());
    // 验证时间字符串格式（HH:MM:SS）
    assertEquals(8, timeStr.length()); // HH:MM:SS 格式长度为8
    assertEquals(':', timeStr.charAt(2));
    assertEquals(':', timeStr.charAt(5));
}

// 测试获取日期字符串
test(timeManagerTests, TestGetDateString) {
    TimeManager timeManager;
    timeManager.init();
    
    String dateStr = timeManager.getDateString();
    // 验证日期字符串不为空
    assertFalse(dateStr.isEmpty());
}

// 主函数，设置测试环境
void setup() {
    Serial.begin(115200);
    while (!Serial) {
        // 等待串口连接
    }
    
    // 运行所有测试
    TestSuiteRunner::run(timeManagerTests);
}

// 循环函数，不需要做任何事情
void loop() {
    // 测试运行一次即可
    delay(1000);
}