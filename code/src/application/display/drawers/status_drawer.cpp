#include "status_drawer.h"

void StatusDrawer::drawStatusBar(IDisplayDriver* driver, int x, int y, float voltage, int percentage, bool isCharging, int messageCount, int height) {
    if (driver == nullptr) {
        return;
    }
    
    int batteryWidth = height < 400 ? 24 : 28;
    int batteryHeight = height < 400 ? 12 : 14;
    int textSize = height < 400 ? 1 : 2;
    
    // 绘制电池图标
    driver->drawRect(x, y, batteryWidth, batteryHeight, GxEPD_BLACK);
    // 电池正极
    driver->drawRect(x + batteryWidth, y + (height < 400 ? 2 : 3), 
                     (height < 400 ? 3 : 4), batteryHeight - (height < 400 ? 4 : 6), GxEPD_BLACK);
    
    // 绘制电池电量
    int batteryLevelWidth = (batteryWidth - (height < 400 ? 4 : 6)) * percentage / 100;
    driver->fillRect(x + (height < 400 ? 2 : 3), y + (height < 400 ? 2 : 3), 
                     batteryLevelWidth, batteryHeight - (height < 400 ? 4 : 6), GxEPD_BLACK);
    
    // 绘制电量百分比文字
    int textX = x + batteryWidth + (height < 400 ? 8 : 10);
    int textY = y + (height < 400 ? 10 : 12);
    driver->drawString(textX, textY, String(percentage) + "%", GxEPD_BLACK, GxEPD_WHITE, textSize);
    
    // 绘制消息数量（在电量右侧）
    if (messageCount > 0) {
        int messageX = textX + (height < 400 ? 40 : 50);
        driver->drawString(messageX, textY, "🔔 " + String(messageCount), GxEPD_BLACK, GxEPD_WHITE, textSize);
    }
}