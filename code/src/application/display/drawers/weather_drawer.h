#ifndef WEATHER_DRAWER_H
#define WEATHER_DRAWER_H

#include "display_driver.h"
#include <Arduino.h>

class WeatherDrawer {
public:
    static void drawWeather(IDisplayDriver* driver, int x, int y, String city, String temp, String condition, String humidity, String wind, int height);
};

#endif // WEATHER_DRAWER_H