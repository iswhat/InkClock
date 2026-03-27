#ifndef SENSOR_DRAWER_H
#define SENSOR_DRAWER_H

#include "display_driver.h"
#include <Arduino.h>

class SensorDrawer {
public:
    static void drawSensorData(IDisplayDriver* driver, int x, int y, float temperature, float humidity, int height);
};

#endif // SENSOR_DRAWER_H