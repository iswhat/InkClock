#include "platform_abstraction.h"
#include <Arduino.h>
#include "esp_pm.h"
#include "esp_sleep.h"
#include "esp_clk.h"

/**
 * @file platform_abstraction.cpp
 * @brief 平台抽象层实现
 * @details 该文件根据不同的平台类型提供相应的实现
 */

// 获取当前平台类型
PlatformType getPlatformType() {
    #ifdef ESP32
        return PlatformType::PLATFORM_ESP32;
    #elif defined(ESP8266)
        return PlatformType::PLATFORM_ESP8266;
    #elif defined(NRF52)
        return PlatformType::PLATFORM_NRF52;
    #elif defined(STM32)
        return PlatformType::PLATFORM_STM32;
    #elif defined(RP2040)
        return PlatformType::PLATFORM_RP2040;
    #elif defined(ARDUINO_ARCH_ESP32)
        return PlatformType::PLATFORM_ESP32;
    #elif defined(ARDUINO_ARCH_ESP8266)
        return PlatformType::PLATFORM_ESP8266;
    #elif defined(ARDUINO_ARCH_NRF52)
        return PlatformType::PLATFORM_NRF52;
    #elif defined(ARDUINO_ARCH_STM32)
        return PlatformType::PLATFORM_STM32;
    #elif defined(ARDUINO_ARCH_RP2040)
        return PlatformType::PLATFORM_RP2040;
    #else
        return PlatformType::PLATFORM_UNKNOWN;
    #endif
}

/**
 * @brief 系统重启实现
 */
void platformReset() {
    #ifdef ESP32
        ESP.restart();
    #elif defined(ESP8266)
        ESP.restart();
    #elif defined(NRF52)
        // NRF52重启实现
        NVIC_SystemReset();
    #elif defined(STM32)
        // STM32重启实现
        NVIC_SystemReset();
    #elif defined(RP2040)
        // RP2040重启实现
        reset_usb_boot(0, 0);
    #else
        // 默认实现
        // 不使用while(1)避免系统卡死
        // 尝试使用标准的系统重置方法
        #if defined(__AVR__)
            wdt_enable(WDTO_15MS);
            while(1);
        #elif defined(__arm__)
            __asm__ __volatile__("bkpt #0");
        #else
            // 对于其他平台，使用空循环但添加超时
            unsigned long start = platformGetMillis();
            while(platformGetMillis() - start < 1000);
        #endif
    #endif
}

/**
 * @brief 获取可用堆内存大小实现
 */
size_t platformGetFreeHeap() {
    #ifdef ESP32
        return ESP.getFreeHeap();
    #elif defined(ESP8266)
        return ESP.getFreeHeap();
    #elif defined(NRF52)
        // NRF52获取堆内存实现
        return heap_caps_get_free_size(MALLOC_CAP_DEFAULT);
    #elif defined(STM32)
        // STM32获取堆内存实现
        #if defined(STM32duino)
            return freeMemory();
        #elif defined(HAL_H)
            // 使用HAL库的内存检测
            return xPortGetFreeHeapSize();
        #else
            // 尝试使用更准确的堆内存计算
            extern char _estack;
            extern char _sheap;
            extern char _eheap;
            size_t heapSize = &_eheap - &_sheap;
            size_t stackSize = &_estack - &_eheap;
            return heapSize;
        #endif
    #elif defined(RP2040)
        // RP2040获取堆内存实现
        struct mallinfo info = mallinfo();
        return info.fordblks;
    #elif defined(ARDUINO_ARCH_ESP32)
        return ESP.getFreeHeap();
    #elif defined(ARDUINO_ARCH_ESP8266)
        return ESP.getFreeHeap();
    #else
        // 默认实现
        return 0;
    #endif
}

/**
 * @brief 获取最小堆内存大小实现
 */
size_t platformGetMinFreeHeap() {
    #ifdef ESP32
        return ESP.getMinFreeHeap();
    #elif defined(ESP8266)
        return ESP.getMinFreeHeap();
    #elif defined(NRF52)
        // NRF52获取最小堆内存实现
        return heap_caps_get_minimum_free_size(MALLOC_CAP_DEFAULT);
    #elif defined(STM32)
        // STM32获取最小堆内存实现
        #if defined(STM32duino)
            static size_t minFreeHeap = 0;
            size_t currentFree = freeMemory();
            if (minFreeHeap == 0 || currentFree < minFreeHeap) {
                minFreeHeap = currentFree;
            }
            return minFreeHeap;
        #elif defined(HAL_H)
            // 使用HAL库的内存检测
            static size_t minFreeHeap = 0;
            size_t currentFree = xPortGetFreeHeapSize();
            if (minFreeHeap == 0 || currentFree < minFreeHeap) {
                minFreeHeap = currentFree;
            }
            return minFreeHeap;
        #else
            static size_t minFreeHeap = 0;
            extern char _estack;
            extern char _sheap;
            extern char _eheap;
            size_t currentFree = &_eheap - &_sheap;
            if (minFreeHeap == 0 || currentFree < minFreeHeap) {
                minFreeHeap = currentFree;
            }
            return minFreeHeap;
        #endif
    #elif defined(RP2040)
        // RP2040获取最小堆内存实现
        static size_t minFreeHeap = 0;
        struct mallinfo info = mallinfo();
        size_t currentFree = info.fordblks;
        if (minFreeHeap == 0 || currentFree < minFreeHeap) {
            minFreeHeap = currentFree;
        }
        return minFreeHeap;
    #elif defined(ARDUINO_ARCH_ESP32)
        return ESP.getMinFreeHeap();
    #elif defined(ARDUINO_ARCH_ESP8266)
        return ESP.getMinFreeHeap();
    #else
        // 默认实现
        static size_t minFreeHeap = 0;
        return minFreeHeap;
    #endif
}

/**
 * @brief 获取CPU频率实现
 */
int platformGetCpuFreqMHz() {
    #ifdef ESP32
        return ESP.getCpuFreqMHz();
    #elif defined(ESP8266)
        return ESP.getCpuFreqMHz();
    #elif defined(NRF52)
        // NRF52获取CPU频率实现
        return NRF_CLOCK->HFCLKSTAT == CLOCK_HFCLKSTAT_STATE_Running ? 64 : 16;
    #elif defined(STM32)
        // STM32获取CPU频率实现
        return SystemCoreClock / 1000000;
    #elif defined(RP2040)
        // RP2040获取CPU频率实现
        return frequency_count_khz(CLOCKS_FC0_SRC_VALUE_CLK_SYS) / 1000;
    #else
        // 默认实现
        return 0;
    #endif
}

/**
 * @brief 获取芯片ID实现
 */
uint32_t platformGetChipId() {
    #ifdef ESP32
        return 0;
    #elif defined(ESP8266)
        return ESP.getChipId();
    #elif defined(NRF52)
        // NRF52获取芯片ID实现
        return NRF_FICR->DEVICEID[0] ^ NRF_FICR->DEVICEID[1];
    #elif defined(STM32)
        // STM32获取芯片ID实现
        return *(uint32_t*)0x1FFF7A10;
    #elif defined(RP2040)
        // RP2040获取芯片ID实现
        return rp2040_chip_unique_id();
    #else
        // 默认实现
        return 0;
    #endif
}

/**
 * @brief 获取Flash芯片大小实现
 */
uint32_t platformGetFlashChipSize() {
    #ifdef ESP32
        return ESP.getFlashChipSize();
    #elif defined(ESP8266)
        return ESP.getFlashChipSize();
    #elif defined(NRF52)
        // NRF52获取Flash大小实现
        return NRF_FICR->CODESIZE;
    #elif defined(STM32)
        // STM32获取Flash大小实现
        return *(uint16_t*)0x1FFF7A22 * 1024;
    #elif defined(RP2040)
        // RP2040获取Flash大小实现
        return flash_get_size();
    #else
        // 默认实现
        return 0;
    #endif
}

/**
 * @brief 获取固件大小实现
 */
uint32_t platformGetFirmwareSize() {
    #ifdef ESP32
        return ESP.getSketchSize();
    #elif defined(ESP8266)
        return ESP.getSketchSize();
    #elif defined(NRF52)
        // NRF52固件大小实现 - 假设固件从0地址开始
        extern uint32_t __etext; // 固件结束地址
        return (uint32_t)&__etext;
    #elif defined(STM32)
        // STM32固件大小实现 - 假设固件从0地址开始
        extern uint32_t __etext; // 固件结束地址
        return (uint32_t)&__etext;
    #elif defined(RP2040)
        // RP2040固件大小实现
        // 对于RP2040，固件大小通常在链接脚本中定义
        extern uint32_t __binary_info_size;
        return (uint32_t)&__binary_info_size;
    #else
        // 默认实现
        return 0;
    #endif
}

/**
 * @brief 获取可用Flash空间实现
 */
uint32_t platformGetFreeFlashSize() {
    #ifdef ESP32
        return ESP.getFreeSketchSpace();
    #elif defined(ESP8266)
        return ESP.getFreeSketchSpace();
    #elif defined(NRF52)
        // NRF52可用Flash空间实现
        return platformGetFlashChipSize() - platformGetFirmwareSize();
    #elif defined(STM32)
        // STM32可用Flash空间实现
        return platformGetFlashChipSize() - platformGetFirmwareSize();
    #elif defined(RP2040)
        // RP2040可用Flash空间实现
        return platformGetFlashChipSize() - platformGetFirmwareSize();
    #else
        // 默认实现
        return 0;
    #endif
}

/**
 * @brief 获取Flash使用情况实现
 */
void platformGetFlashInfo(uint32_t& totalSize, uint32_t& firmwareSize, uint32_t& freeSize) {
    totalSize = platformGetFlashChipSize();
    firmwareSize = platformGetFirmwareSize();
    freeSize = platformGetFreeFlashSize();
}

/**
 * @brief 动态调整CPU频率实现
 */
bool platformSetCpuFreqMHz(int freqMHz) {
    #ifdef ESP32
        // ESP32设置CPU频率
        return false;
    #elif defined(ESP8266)
        // ESP8266设置CPU频率
        switch(freqMHz) {
            case 80:
                setCpuFrequencyMhz(80);
                return true;
            case 160:
                setCpuFrequencyMhz(160);
                return true;
            default:
                return false;
        }
    #elif defined(NRF52)
        // NRF52设置CPU频率
        if(freqMHz == 64) {
            NRF_CLOCK->TASKS_HFCLKSTART = 1;
            while(NRF_CLOCK->EVENTS_HFCLKSTARTED == 0);
            NRF_CLOCK->EVENTS_HFCLKSTARTED = 0;
            return true;
        } else if(freqMHz == 16) {
            NRF_CLOCK->TASKS_HFCLKSTOP = 1;
            return true;
        }
        return false;
    #elif defined(STM32)
        // STM32设置CPU频率实现
        // 这里需要根据具体的STM32型号实现
        return false;
    #elif defined(RP2040)
        // RP2040设置CPU频率实现
        return set_sys_clock_khz(freqMHz * 1000, true) == 0;
    #else
        // 默认实现
        return false;
    #endif
}

/**
 * @brief 进入深度睡眠模式实现
 */
void platformDeepSleep(uint64_t sleepTimeMs) {
    #ifdef ESP32
        // ESP32深度睡眠
        esp_sleep_enable_timer_wakeup(sleepTimeMs * 1000);
        esp_deep_sleep_start();
    #elif defined(ESP8266)
        // ESP8266深度睡眠
        ESP.deepSleep(sleepTimeMs * 1000);
    #elif defined(NRF52)
        // NRF52深度睡眠实现
        nrf_pwr_mgmt_run();
    #elif defined(STM32)
        // STM32深度睡眠实现
        HAL_PWR_EnterDEEPSLEEPMode(PWR_MAINREGULATOR_ON, PWR_DEEPSLEEPENTRY_WFI);
    #elif defined(RP2040)
        // RP2040深度睡眠实现
        sleep_run_from_xosc();
        sleep_ms(sleepTimeMs);
    #else
        // 默认实现
        platformDelay(sleepTimeMs);
    #endif
}

/**
 * @brief 进入轻度睡眠模式实现
 */
void platformLightSleep(uint64_t sleepTimeMs) {
    #ifdef ESP32
        // ESP32轻度睡眠
        esp_sleep_enable_timer_wakeup(sleepTimeMs * 1000);
        esp_light_sleep_start();
    #elif defined(ESP8266)
        // ESP8266轻度睡眠
        ESP.deepSleep(sleepTimeMs * 1000, WAKE_RF_DEFAULT);
    #elif defined(NRF52)
        // NRF52轻度睡眠实现
        nrf_pwr_mgmt_run();
    #elif defined(STM32)
        // STM32轻度睡眠实现
        HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);
    #elif defined(RP2040)
        // RP2040轻度睡眠实现
        sleep_ms(sleepTimeMs);
    #else
        // 默认实现
        platformDelay(sleepTimeMs);
    #endif
}

/**
 * @brief 获取系统运行时间实现
 */
unsigned long platformGetMillis() {
    #ifdef ESP32
        return millis();
    #elif defined(ESP8266)
        return millis();
    #elif defined(NRF52)
        // NRF52获取系统时间实现
        return app_timer_cnt_get() / (APP_TIMER_TICKS_PER_SEC / 1000);
    #elif defined(STM32)
        // STM32获取系统时间实现
        return HAL_GetTick();
    #elif defined(RP2040)
        // RP2040获取系统时间实现
        return to_ms_since_boot(get_absolute_time());
    #else
        // 默认实现
        return 0;
    #endif
}

/**
 * @brief 获取系统运行时间（微秒）实现
 */
unsigned long platformGetMicros() {
    #ifdef ESP32
        return micros();
    #elif defined(ESP8266)
        return micros();
    #elif defined(NRF52)
        // NRF52获取微秒时间实现
        return app_timer_cnt_get() / (APP_TIMER_TICKS_PER_SEC / 1000000);
    #elif defined(STM32)
        // STM32获取微秒时间实现
        return HAL_GetTick() * 1000 + (SysTick->LOAD - SysTick->VAL) * 1000 / SysTick->LOAD;
    #elif defined(RP2040)
        // RP2040获取微秒时间实现
        return to_us_since_boot(get_absolute_time());
    #else
        // 默认实现
        return 0;
    #endif
}

/**
 * @brief 延迟执行实现
 */
void platformDelay(unsigned long delayMs) {
    #ifdef ESP32
        delay(delayMs);
    #elif defined(ESP8266)
        delay(delayMs);
    #elif defined(NRF52)
        // NRF52延迟实现
        nrf_delay_ms(delayMs);
    #elif defined(STM32)
        // STM32延迟实现
        HAL_Delay(delayMs);
    #elif defined(RP2040)
        // RP2040延迟实现
        sleep_ms(delayMs);
    #else
        // 默认实现
        unsigned long start = platformGetMillis();
        while(platformGetMillis() - start < delayMs);
    #endif
}

/**
 * @brief 微秒级延迟执行实现
 */
void platformDelayMicroseconds(unsigned long delayUs) {
    #ifdef ESP32
        delayMicroseconds(delayUs);
    #elif defined(ESP8266)
        delayMicroseconds(delayUs);
    #elif defined(NRF52)
        // NRF52微秒延迟实现
        nrf_delay_us(delayUs);
    #elif defined(STM32)
        // STM32微秒延迟实现
        HAL_Delay(delayUs / 1000);
    #elif defined(RP2040)
        // RP2040微秒延迟实现
        sleep_us(delayUs);
    #else
        // 默认实现
        unsigned long start = platformGetMicros();
        while(platformGetMicros() - start < delayUs);
    #endif
}

/**
 * @brief 获取随机数实现
 */
uint32_t platformRandom() {
    #ifdef ESP32
        return random();
    #elif defined(ESP8266)
        return random();
    #elif defined(NRF52)
        // NRF52随机数实现
        return nrf_rng_random_int_get();
    #elif defined(STM32)
        // STM32随机数实现
        return HAL_RNG_GetRandomNumber(&hrng);
    #elif defined(RP2040)
        // RP2040随机数实现
        return rp2040.random();
    #else
        // 默认实现
        static uint32_t seed = 1;
        seed = seed * 1103515245 + 12345;
        return seed;
    #endif
}

/**
 * @brief 设置随机数种子实现
 */
void platformRandomSeed(uint32_t seed) {
    #ifdef ESP32
        randomSeed(seed);
    #elif defined(ESP8266)
        randomSeed(seed);
    #elif defined(NRF52)
        // NRF52设置随机数种子实现
        // NRF52使用硬件RNG，不需要种子
        (void)seed;
    #elif defined(STM32)
        // STM32设置随机数种子实现
        // STM32使用硬件RNG，不需要种子
        (void)seed;
    #elif defined(RP2040)
        // RP2040设置随机数种子实现
        rp2040.random_seed(seed);
    #else
        // 默认实现
        // 软件RNG使用种子
        srand(seed);
    #endif
}

/**
 * @brief 获取平台名称实现
 */
std::string platformGetName() {
    #ifdef ESP32
        return "ESP32";
    #elif defined(ESP8266)
        return "ESP8266";
    #elif defined(NRF52)
        return "NRF52";
    #elif defined(STM32)
        return "STM32";
    #elif defined(RP2040)
        return "RP2040";
    #else
        return "Unknown";
    #endif
}

/**
 * @brief 获取平台版本实现
 */
std::string platformGetVersion() {
    #ifdef ESP32
        return ESP.getSdkVersion();
    #elif defined(ESP8266)
        return ESP.getSdkVersion();
    #elif defined(NRF52)
        return "1.0.0";
    #elif defined(STM32)
        return "1.0.0";
    #elif defined(RP2040)
        return "1.0.0";
    #else
        return "1.0.0";
    #endif
}
