#include "clock_drawer.h"

void ClockDrawer::drawDigitalClock(IDisplayDriver* driver, int x, int y, String time, String date, int height) {
    if (driver == nullptr) {
        return;
    }
    
    // 根据屏幕尺寸设置字体大小
    int clockSize, dateSize;
    
    if (height < 400) {
        // 小屏幕
        clockSize = 4;
        dateSize = 1;
    } else {
        // 大屏幕
        clockSize = 7;
        dateSize = 2;
    }
    
    // 绘制时间
    driver->drawString(x, y, time, GxEPD_BLACK, GxEPD_WHITE, clockSize);
    
    // 绘制日期
    if (dateSize > 0) {
        int dateY = height < 400 ? y + 50 + (clockSize - 5) * 8 : y + 90 + (clockSize - 8) * 12;
        driver->drawString(x, dateY, date, GxEPD_RED, GxEPD_WHITE, dateSize);
    }
}

void ClockDrawer::drawAnalogClock(IDisplayDriver* driver, int x, int y, int hour, int minute, int second, int millisecond, int radius) {
    if (driver == nullptr) {
        return;
    }
    
    // 绘制时钟外圆
    driver->drawRect(x - radius, y - radius, radius * 2, radius * 2, GxEPD_BLACK);
    
    // 绘制时钟刻度
    for (int i = 0; i < 12; i++) {
        float angle = i * PI / 6 - PI / 2;
        int x1 = x + cos(angle) * (radius - 5);
        int y1 = y + sin(angle) * (radius - 5);
        int x2 = x + cos(angle) * radius;
        int y2 = y + sin(angle) * radius;
        driver->drawLine(x1, y1, x2, y2, GxEPD_BLACK);
    }
    
    // 计算精确的角度（支持平滑动画）
    float totalSeconds = hour * 3600 + minute * 60 + second + millisecond / 1000.0;
    float hourAngle = (totalSeconds / 43200.0) * 2 * PI - PI / 2;
    float minuteAngle = (totalSeconds / 3600.0) * 2 * PI - PI / 2;
    float secondAngle = (totalSeconds / 60.0) * 2 * PI - PI / 2;
    
    // 绘制时针
    int hourX = x + cos(hourAngle) * (radius - 20);
    int hourY = y + sin(hourAngle) * (radius - 20);
    driver->drawLine(x, y, hourX, hourY, GxEPD_BLACK);
    
    // 绘制分针
    int minuteX = x + cos(minuteAngle) * (radius - 10);
    int minuteY = y + sin(minuteAngle) * (radius - 10);
    driver->drawLine(x, y, minuteX, minuteY, GxEPD_BLACK);
    
    // 绘制秒针
    int secondX = x + cos(secondAngle) * (radius - 5);
    int secondY = y + sin(secondAngle) * (radius - 5);
    driver->drawLine(x, y, secondX, secondY, GxEPD_RED);
    
    // 绘制中心点
    driver->drawRect(x - 2, y - 2, 4, 4, GxEPD_BLACK);
}

void ClockDrawer::drawTextClock(IDisplayDriver* driver, int x, int y, int hour, int minute, int second, int height) {
    if (driver == nullptr) {
        return;
    }
    
    // 根据屏幕尺寸设置字体大小
    int textSize = height < 400 ? 2 : 3;
    int lineHeight = height < 400 ? 30 : 40;
    
    // 构建文字时钟内容
    String text;
    
    // 上午/下午
    String period = hour < 12 ? "上午" : "下午";
    
    // 小时转换为12小时制
    int hour12 = hour % 12;
    if (hour12 == 0) hour12 = 12;
    
    // 构建时间文字描述
    text = "现在是" + period + "" + String(hour12) + "点";
    
    if (minute > 0) {
        text += String(minute) + "分";
    }
    
    if (second > 0) {
        text += String(second) + "秒";
    }
    
    // 绘制文字时钟
    driver->drawString(x, y, text, GxEPD_BLACK, GxEPD_WHITE, textSize);
}