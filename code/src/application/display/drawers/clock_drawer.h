#ifndef CLOCK_DRAWER_H
#define CLOCK_DRAWER_H

#include "display_driver.h"
#include <Arduino.h>

class ClockDrawer {
public:
    static void drawDigitalClock(IDisplayDriver* driver, int x, int y, String time, String date, int height);
    static void drawAnalogClock(IDisplayDriver* driver, int x, int y, int hour, int minute, int second, int millisecond, int radius);
    static void drawTextClock(IDisplayDriver* driver, int x, int y, int hour, int minute, int second, int height);
};

#endif // CLOCK_DRAWER_H