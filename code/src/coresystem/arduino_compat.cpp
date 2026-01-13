#include "arduino_compat.h"
#include <thread>
#include <chrono>
#include <cstdlib>
#include <cstring>

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

// 实现constrain函数
template<typename T>
T constrain(T x, T min_val, T max_val) {
    if (x < min_val) return min_val;
    if (x > max_val) return max_val;
    return x;
}

// 显式实例化常用类型
template int constrain(int, int, int);
template long constrain(long, long, long);
template float constrain(float, float, float);
template double constrain(double, double, double);

// 实现max函数
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

// 显式实例化常用类型
template int max(int, int);
template long max(long, long);
template float max(float, float);
template double max(double, double);

// 实现min函数
template<typename T>
T min(T a, T b) {
    return (a < b) ? a : b;
}

// 显式实例化常用类型
template int min(int, int);
template long min(long, long);
template float min(float, float);
template double min(double, double);

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