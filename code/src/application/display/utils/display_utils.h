#ifndef DISPLAY_UTILS_H
#define DISPLAY_UTILS_H

#include <Arduino.h>

class DisplayUtils {
public:
    static String padStart(String str, unsigned int length, char padChar);
    static int getFirstWeekdayOfMonth(int year, int month);
    static int getDaysInMonth(int year, int month);
};

#endif // DISPLAY_UTILS_H