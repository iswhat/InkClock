#ifndef ARDUINO_COMPAT_H
#define ARDUINO_COMPAT_H

#include <string>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <algorithm>
#include <cstddef>
#include <cstdint>

// 基本类型定义
typedef unsigned char byte;
typedef unsigned short word;

// Arduino String 类兼容
class String {
private:
    std::string _str;

public:
    String() : _str() {}
    String(const char* cstr) : _str(cstr) {}
    String(const std::string& str) : _str(str) {}
    String(int value) : _str(std::to_string(value)) {}
    String(long value) : _str(std::to_string(value)) {}
    String(float value) : _str(std::to_string(value)) {}
    String(double value) : _str(std::to_string(value)) {}

    // 操作符重载
    String& operator=(const char* cstr) {
        _str = cstr;
        return *this;
    }

    String& operator=(const std::string& str) {
        _str = str;
        return *this;
    }

    String& operator+=(const char* cstr) {
        _str += cstr;
        return *this;
    }

    String& operator+=(const String& other) {
        _str += other._str;
        return *this;
    }

    String& operator+=(char c) {
        _str += c;
        return *this;
    }

    String& operator+=(int value) {
        _str += std::to_string(value);
        return *this;
    }

    // 方法
    const char* c_str() const {
        return _str.c_str();
    }

    size_t length() const {
        return _str.length();
    }

    bool isEmpty() const {
        return _str.empty();
    }

    String substring(size_t beginIndex) const {
        if (beginIndex >= _str.length()) {
            return String();
        }
        return String(_str.substr(beginIndex));
    }

    String substring(size_t beginIndex, size_t endIndex) const {
        if (beginIndex >= _str.length()) {
            return String();
        }
        endIndex = std::min(endIndex, _str.length());
        return String(_str.substr(beginIndex, endIndex - beginIndex));
    }

    int toInt() const {
        try {
            return std::stoi(_str);
        } catch (...) {
            return 0;
        }
    }

    float toFloat() const {
        try {
            return std::stof(_str);
        } catch (...) {
            return 0.0f;
        }
    }

    // 转换为std::string
    std::string toStdString() const {
        return _str;
    }

    // 比较操作符
    bool operator==(const char* cstr) const {
        return _str == cstr;
    }

    bool operator!=(const char* cstr) const {
        return _str != cstr;
    }

    bool operator==(const String& other) const {
        return _str == other._str;
    }

    bool operator!=(const String& other) const {
        return _str != other._str;
    }
};

// 基本函数
void delay(unsigned long ms) {
    // 简单的延迟实现
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

unsigned long millis() {
    // 简单的毫秒计数实现
    static auto start_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
    return duration.count();
}

unsigned long micros() {
    // 简单的微秒计数实现
    static auto start_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
    return duration.count();
}

void yield() {
    // 空实现，用于兼容Arduino
}

// 随机数函数
int random(int max) {
    return rand() % max;
}

int random(int min, int max) {
    return min + rand() % (max - min);
}

void randomSeed(unsigned long seed) {
    srand(seed);
}

// 串口函数
class Serial_ {
public:
    void begin(unsigned long baud) {
        // 空实现，用于兼容Arduino
    }

    void end() {
        // 空实现，用于兼容Arduino
    }

    size_t print(const char* str) {
        std::cout << str;
        return strlen(str);
    }

    size_t print(const String& str) {
        std::cout << str.c_str();
        return str.length();
    }

    size_t print(int value, int base = 10) {
        std::cout << value;
        return std::to_string(value).length();
    }

    size_t print(long value, int base = 10) {
        std::cout << value;
        return std::to_string(value).length();
    }

    size_t print(float value, int digits = 2) {
        std::cout.precision(digits);
        std::cout << value;
        return std::to_string(value).length();
    }

    size_t println(const char* str) {
        std::cout << str << std::endl;
        return strlen(str) + 1;
    }

    size_t println(const String& str) {
        std::cout << str.c_str() << std::endl;
        return str.length() + 1;
    }

    size_t println(int value, int base = 10) {
        std::cout << value << std::endl;
        return std::to_string(value).length() + 1;
    }

    size_t println(long value, int base = 10) {
        std::cout << value << std::endl;
        return std::to_string(value).length() + 1;
    }

    size_t println(float value, int digits = 2) {
        std::cout.precision(digits);
        std::cout << value << std::endl;
        return std::to_string(value).length() + 1;
    }

    size_t println() {
        std::cout << std::endl;
        return 1;
    }

    bool available() {
        return false;
    }

    int read() {
        return -1;
    }
};

// 全局串口对象
extern Serial_ Serial;

// 其他常用宏和常量
#define HIGH 1
#define LOW 0

#define INPUT 0
#define OUTPUT 1
#define INPUT_PULLUP 2

#define true true
#define false false

#define PI 3.14159265358979323846
#define HALF_PI 1.57079632679489661923
#define TWO_PI 6.28318530717958647692

#define DEG_TO_RAD 0.017453292519943295
#define RAD_TO_DEG 57.29577951308232

// 常用函数
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// 数学函数
template<typename T>
T constrain(T x, T min_val, T max_val) {
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}

template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

template<typename T>
T min(T a, T b) {
    return (a < b) ? a : b;
}

// 运算符重载：String + char
String operator+(const String& lhs, char rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 运算符重载：String + const char*
String operator+(const String& lhs, const char* rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 运算符重载：String + String
String operator+(const String& lhs, const String& rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 运算符重载：const char* + String
String operator+(const char* lhs, const String& rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 运算符重载：char + String
String operator+(char lhs, const String& rhs) {
    String result;
    result += lhs;
    result += rhs;
    return result;
}

#endif // ARDUINO_COMPAT_H