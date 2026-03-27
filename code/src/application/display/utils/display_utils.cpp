#include "display_utils.h"

// 自定义字符串填充函数，替代缺少的 padStart 方法
String DisplayUtils::padStart(String str, unsigned int length, char padChar) {
    if (str.length() >= length) {
        return str;
    }
    String result = "";
    for (unsigned int i = 0; i < length - str.length(); i++) {
        result += padChar;
    }
    result += str;
    return result;
}

// 获取指定月份的第一天是星期几（0-6，0表示星期日）
int DisplayUtils::getFirstWeekdayOfMonth(int year, int month) {
    struct tm timeinfo;
    timeinfo.tm_year = year - 1900;
    timeinfo.tm_mon = month - 1;
    timeinfo.tm_mday = 1;
    timeinfo.tm_hour = 0;
    timeinfo.tm_min = 0;
    timeinfo.tm_sec = 0;
    mktime(&timeinfo);
    return timeinfo.tm_wday;
}

// 获取指定月份的天数
int DisplayUtils::getDaysInMonth(int year, int month) {
    if (month == 2) {
        // 闰年判断
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
            return 29;
        } else {
            return 28;
        }
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else {
        return 31;
    }
}