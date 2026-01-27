#pragma once

#include <stdint.h>
#include <string>

/**
 * @file platform_abstraction.h
 * @brief 平台抽象层，用于支持多种低功耗WiFi+BLE微控制器
 * @details 该文件定义了平台无关的API，屏蔽了不同微控制器的底层差异
 */

// 平台类型定义
#ifdef PLATFORM_ESP32
#undef PLATFORM_ESP32
#endif

#ifdef PLATFORM_ESP8266
#undef PLATFORM_ESP8266
#endif

#ifdef PLATFORM_NRF52
#undef PLATFORM_NRF52
#endif

#ifdef PLATFORM_STM32
#undef PLATFORM_STM32
#endif

#ifdef PLATFORM_RP2040
#undef PLATFORM_RP2040
#endif

enum class PlatformType {
    PLATFORM_ESP32,
    PLATFORM_ESP8266,
    PLATFORM_NRF52,
    PLATFORM_STM32,
    PLATFORM_RP2040,
    PLATFORM_UNKNOWN
};

// 获取当前平台类型
PlatformType getPlatformType();

/**
 * @brief 系统重启
 */
void platformReset();

/**
 * @brief 获取可用堆内存大小
 * @return 可用堆内存大小（字节）
 */
size_t platformGetFreeHeap();

/**
 * @brief 获取最小堆内存大小
 * @return 最小堆内存大小（字节）
 */
size_t platformGetMinFreeHeap();

/**
 * @brief 获取CPU频率
 * @return CPU频率（MHz）
 */
int platformGetCpuFreqMHz();

/**
 * @brief 获取芯片ID
 * @return 芯片ID
 */
uint32_t platformGetChipId();

/**
 * @brief 获取Flash芯片大小
 * @return Flash芯片大小（字节）
 */
uint32_t platformGetFlashChipSize();

/**
 * @brief 获取固件大小（已使用的Flash空间）
 * @return 固件大小（字节）
 */
uint32_t platformGetFirmwareSize();

/**
 * @brief 获取可用Flash空间
 * @return 可用Flash空间（字节）
 */
uint32_t platformGetFreeFlashSize();

/**
 * @brief 获取Flash使用情况
 * @param totalSize 总Flash大小（字节）
 * @param firmwareSize 固件大小（字节）
 * @param freeSize 可用Flash空间（字节）
 */
void platformGetFlashInfo(uint32_t& totalSize, uint32_t& firmwareSize, uint32_t& freeSize);

/**
 * @brief 动态调整CPU频率
 * @param freqMHz CPU频率（MHz）
 * @return 是否成功
 */
bool platformSetCpuFreqMHz(int freqMHz);

/**
 * @brief 进入深度睡眠模式
 * @param sleepTimeMs 睡眠时长（毫秒）
 */
void platformDeepSleep(uint64_t sleepTimeMs);

/**
 * @brief 进入轻度睡眠模式
 * @param sleepTimeMs 睡眠时长（毫秒）
 */
void platformLightSleep(uint64_t sleepTimeMs);

/**
 * @brief 获取系统运行时间
 * @return 系统运行时间（毫秒）
 */
unsigned long platformGetMillis();

/**
 * @brief 获取系统运行时间（微秒）
 * @return 系统运行时间（微秒）
 */
unsigned long platformGetMicros();

/**
 * @brief 延迟执行
 * @param delayMs 延迟时长（毫秒）
 */
void platformDelay(unsigned long delayMs);

/**
 * @brief 微秒级延迟执行
 * @param delayUs 延迟时长（微秒）
 */
void platformDelayMicroseconds(unsigned long delayUs);

/**
 * @brief 获取随机数
 * @return 随机数
 */
uint32_t platformRandom();

/**
 * @brief 设置随机数种子
 * @param seed 种子值
 */
void platformRandomSeed(uint32_t seed);

/**
 * @brief 获取平台名称
 * @return 平台名称字符串
 */
std::string platformGetName();

/**
 * @brief 获取平台版本
 * @return 平台版本字符串
 */
std::string platformGetVersion();
