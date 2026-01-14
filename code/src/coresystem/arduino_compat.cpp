#include "arduino_compat.h"
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>
#include <cmath>

// 全局串口对象实例
Serial_ Serial;

// 实现延迟函数
void delay(unsigned long ms) {
    std::this_thread::sleep_for(std::chrono::milliseconds(ms));
}

// 实现毫秒计数函数
unsigned long millis() {
    static auto start_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(current_time - start_time);
    return duration.count();
}

// 实现微秒计数函数
unsigned long micros() {
    static auto start_time = std::chrono::steady_clock::now();
    auto current_time = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(current_time - start_time);
    return duration.count();
}

// 实现yield函数
void yield() {
    // 空实现，用于兼容Arduino
}

// 实现随机数函数
int random(int max) {
    return rand() % max;
}

int random(int min, int max) {
    return min + rand() % (max - min);
}

void randomSeed(unsigned long seed) {
    srand(seed);
}

// 实现map函数
long map(long x, long in_min, long in_max, long out_min, long out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// 实现串口函数
void Serial_::begin(unsigned long baud) {
    // 空实现，用于兼容Arduino
}

void Serial_::end() {
    // 空实现，用于兼容Arduino
}

size_t Serial_::print(const char* str) {
    std::cout << str;
    return strlen(str);
}

size_t Serial_::print(const String& str) {
    std::cout << str.c_str();
    return str.length();
}

size_t Serial_::print(int value, int base) {
    std::cout << value;
    return std::to_string(value).length();
}

size_t Serial_::print(long value, int base) {
    std::cout << value;
    return std::to_string(value).length();
}

size_t Serial_::print(float value, int digits) {
    std::cout.precision(digits);
    std::cout << value;
    return std::to_string(value).length();
}

size_t Serial_::println(const char* str) {
    std::cout << str << std::endl;
    return strlen(str) + 1;
}

size_t Serial_::println(const String& str) {
    std::cout << str.c_str() << std::endl;
    return str.length() + 1;
}

size_t Serial_::println(int value, int base) {
    std::cout << value << std::endl;
    return std::to_string(value).length() + 1;
}

size_t Serial_::println(long value, int base) {
    std::cout << value << std::endl;
    return std::to_string(value).length() + 1;
}

size_t Serial_::println(float value, int digits) {
    std::cout.precision(digits);
    std::cout << value << std::endl;
    return std::to_string(value).length() + 1;
}

size_t Serial_::println() {
    std::cout << std::endl;
    return 1;
}

bool Serial_::available() {
    return false;
}

int Serial_::read() {
    return -1;
}

// 实现运算符重载：String + char
String operator+(const String& lhs, char rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 实现运算符重载：String + const char*
String operator+(const String& lhs, const char* rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 实现运算符重载：String + String
String operator+(const String& lhs, const String& rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 实现运算符重载：const char* + String
String operator+(const char* lhs, const String& rhs) {
    String result = lhs;
    result += rhs;
    return result;
}

// 实现运算符重载：char + String
String operator+(char lhs, const String& rhs) {
    String result;
    result += lhs;
    result += rhs;
    return result;
}

// 实现三角函数
float sin(float x) {
    return std::sin(x);
}

float cos(float x) {
    return std::cos(x);
}

float tan(float x) {
    return std::tan(x);
}

float sqrt(float x) {
    return std::sqrt(x);
}

// 实现内存管理函数
void* malloc(size_t size) {
    return std::malloc(size);
}

void free(void* ptr) {
    std::free(ptr);
}

void* realloc(void* ptr, size_t size) {
    return std::realloc(ptr, size);
}