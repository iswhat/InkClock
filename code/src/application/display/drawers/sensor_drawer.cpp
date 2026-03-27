#include "sensor_drawer.h"

void SensorDrawer::drawSensorData(IDisplayDriver* driver, int x, int y, float temperature, float humidity, int height) {
    if (driver == nullptr) {
        return;
    }
    
    int textSize = height < 400 ? 1 : 2;
    
    // 绘制温度
    String tempStr = "温度: " + String(temperature, 1) + "°C";
    driver->drawString(x, y, tempStr, GxEPD_BLACK, GxEPD_WHITE, textSize);
    
    // 绘制湿度
    String humStr = "湿度: " + String(humidity, 1) + "%";
    driver->drawString(x, y + (height < 400 ? 20 : 30), humStr, GxEPD_BLACK, GxEPD_WHITE, textSize);
}