#ifndef STATUS_DRAWER_H
#define STATUS_DRAWER_H

#include "display_driver.h"
#include <Arduino.h>

class StatusDrawer {
public:
    static void drawStatusBar(IDisplayDriver* driver, int x, int y, float voltage, int percentage, bool isCharging, int messageCount, int height);
};

#endif // STATUS_DRAWER_H