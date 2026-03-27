#include "weather_drawer.h"

void WeatherDrawer::drawWeather(IDisplayDriver* driver, int x, int y, String city, String temp, String condition, String humidity, String wind, int height) {
    if (driver == nullptr) {
        return;
    }
    
    int textSize = height < 400 ? 1 : 2;
    int tempSize = height < 400 ? 3 : 4;
    
    // 绘制城市
    driver->drawString(x, y, city, GxEPD_BLACK, GxEPD_WHITE, textSize);
    
    // 绘制当前温度
    driver->drawString(x, y + (height < 400 ? 15 : 25), temp, GxEPD_BLACK, GxEPD_WHITE, tempSize);
    
    // 绘制天气状况
    driver->drawString(x, y + (height < 400 ? 35 : 55), condition, GxEPD_BLACK, GxEPD_WHITE, textSize);
    
    // 绘制天气图标
    String weatherIcon = "☀️";
    driver->drawString(x + (height < 400 ? 60 : 120), y + (height < 400 ? 25 : 45), weatherIcon, GxEPD_BLACK, GxEPD_WHITE, tempSize);
}