/*
 * InkClock 简化测试固件
 * 用于测试基本编译功能，避免复杂依赖
 */

#include <Arduino.h>

// 定义调试宏
#define DEBUG_PRINTLN(x) Serial.println(x)
#define DEBUG_PRINT(x) Serial.print(x)

// 主类声明
class InkClock {
public:
    void init() {
        // 初始化串口
        Serial.begin(115200);
        while (!Serial) {
            delay(10);
        }
        
        DEBUG_PRINTLN("InkClock 简化测试固件初始化...");
        DEBUG_PRINTLN("固件版本: 1.0.0");
        DEBUG_PRINTLN("平台: " + String(ARDUINO_BOARD));
    }
    
    void loop() {
        // 简单的测试循环
        static unsigned long lastTime = 0;
        if (millis() - lastTime > 1000) {
            lastTime = millis();
            DEBUG_PRINTLN("运行中... " + String(millis() / 1000) + "秒");
        }
    }
};

// 创建全局实例
InkClock inkClock;

// ESP32-C3 需要的 app_main 函数
void app_main() {
    // 调用 Arduino 的 setup 函数
    setup();
    
    // 主循环
    while (1) {
        loop();
        delay(1);
    }
}

void setup() {
    inkClock.init();
}

void loop() {
    inkClock.loop();
}