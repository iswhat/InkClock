# InkClock - Hardware Material Selection Guide

[中文版本](硬件物料选型指南.md)

## 1. Core Components

### 1.1 Main Control Chips

| Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-------------|-----------------------|
| **ESP8266 Series** | | | | |
| ESP8266-12F | Espressif | Single-core 32-bit processor, 80MHz, 80KB SRAM, 4MB Flash, WiFi | 10×10×1.2mm | 8-12 yuan | Low cost, suitable for simple WiFi applications |
| ESP8266-07 | Espressif | Single-core 32-bit processor, 80MHz, 80KB SRAM, 4MB Flash, WiFi | 10×10×1.2mm | 10-15 yuan | Small size package, suitable for space-limited applications |
| **ESP32 Series** | | | | |
| ESP32-C3-WROOM-02 | Espressif | Single-core 32-bit processor, 160MHz, 400KB SRAM, 8MB Flash, WiFi+Bluetooth dual-mode | 8×8×1.2mm | 13-18 yuan | Low cost, low power consumption, suitable for simple applications |
| **ESP8266 Series** | | | | |
| ESP8266-WROOM-02 | Espressif | Single-core 32-bit processor, 80MHz, 80KB SRAM, 4MB Flash, WiFi | 10×10×1.2mm | 13-18 yuan | Stable and reliable, suitable for mass production applications |
| **ESP32 Series** | | | | |
| ESP32-S2-WROOM-1 | Espressif | Single-core 32-bit processor, 240MHz, 320KB SRAM, 16MB Flash, WiFi | 8×8×1.2mm | 16-22 yuan | Low cost, suitable for WiFi applications |
| **STM32 Series** | | | | |
| STM32L432KC | STMicroelectronics | Single-core 32-bit processor, 80MHz, 64KB SRAM, 256KB Flash | 7×7×0.8mm | 16-22 yuan | Low power consumption, suitable for battery-powered applications |
| **ESP32 Series** | | | | |
| ESP32-S3-WROOM-1 | Espressif | Dual-core 32-bit processor, 240MHz, 512KB SRAM, 16MB Flash, WiFi+Bluetooth dual-mode | 8×8×1.2mm | 18-25 yuan | Powerful performance, low power consumption, supports WiFi and Bluetooth, suitable for IoT applications |
| ESP32-C6-WROOM-1 | Espressif | Single-core 32-bit processor, 160MHz, 512KB SRAM, 8MB Flash, WiFi+Bluetooth dual-mode, supports IPv6 | 8×8×1.2mm | 18-25 yuan | Supports IPv6, suitable for IoT applications |
| **NRF52 Series** | | | | |
| NRF52832-QFAA | Nordic Semiconductor | Single-core 32-bit processor, 64MHz, 64KB SRAM, 512KB Flash, Bluetooth 5.0 | 5×5×0.8mm | 18-25 yuan | Low power consumption, suitable for Bluetooth applications |
| **RP2040 Series** | | | | |
| Raspberry Pi Pico W | Raspberry Pi | Dual-core ARM Cortex-M0+, 133MHz, 264KB SRAM, 2MB Flash, WiFi+Bluetooth | 5×5×0.8mm | 18-25 yuan | Open source hardware, rich community support |
| **STM32 Series** | | | | |
| STM32G431KB | STMicroelectronics | Single-core 32-bit processor, 170MHz, 128KB SRAM, 512KB Flash | 7×7×0.8mm | 20-28 yuan | High performance, suitable for complex applications |
| **ESP32 Series** | | | | |
| ESP32-S3-WROOM-2 | Espressif | Dual-core 32-bit processor, 240MHz, 512KB SRAM, 8MB PSRAM, 16MB Flash, WiFi+Bluetooth dual-mode | 8×8×1.2mm | 22-30 yuan | Built-in PSRAM, suitable for applications requiring large memory |
| **STM32 Series** | | | | |
| STM32F411CEU6 | STMicroelectronics | Single-core 32-bit processor, 100MHz, 128KB SRAM, 512KB Flash | 7×7×0.8mm | 22-30 yuan | Classic model, rich resources |
| **RP2040 Series** | | | | |
| Adafruit Feather RP2040 | Adafruit | Dual-core ARM Cortex-M0+, 133MHz, 264KB SRAM, 2MB Flash | 5×5×0.8mm | 25-35 yuan | Feather ecosystem, easy to expand |
| **NRF52 Series** | | | | |
| NRF52840-QIAA | Nordic Semiconductor | Single-core 32-bit processor, 64MHz, 256KB SRAM, 1MB Flash, Bluetooth 5.0, 802.15.4 | 7×7×0.8mm | 32-40 yuan | High performance, supports multiple wireless protocols |

### 1.2 Development Boards

| Model | Brand | CPU | ROM | RAM | Integrated Components | Size | Price Range | Recommendation Reason |
|-------|-------|-----|-----|-----|----------------------|------|-------------|-----------------------|
| **ESP8266 Series Development Boards** | | | | | | | |
| ESP8266-DevKitC | Espressif | Single-core 32-bit processor, 80MHz | 4MB Flash | 80KB SRAM | WiFi 802.11b/g/n, USB Type-C interface | 25×68×3mm | 18-25 yuan | Official design, reliable quality |
| ESP8266-DevKitC | Espressif | Single-core 32-bit processor, 80MHz | 4MB Flash | 80KB SRAM | WiFi 802.11b/g/n, USB Type-C interface | 25×68×3mm | 18-25 yuan | Official design, reliable quality |
| ESP8266-DevKitC | Espressif | Single-core 32-bit processor, 80MHz | 4MB Flash | 80KB SRAM | WiFi 802.11b/g/n, USB Type-C interface | 25×68×3mm | 18-25 yuan | Official design, reliable quality |
| ESP8266-DevKitC | Espressif | Single-core 32-bit processor, 80MHz | 4MB Flash | 80KB SRAM | WiFi 802.11b/g/n, USB Type-C interface | 25×68×3mm | 18-25 yuan | Official design, reliable quality |
| **ESP32 Series Development Boards** | | | | | | | |
| ESP32-C3-DevKitC-02 | Espressif | Single-core 32-bit processor, 160MHz | 8MB Flash | 400KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×40×3mm | 18-25 yuan | Cost-effective, suitable for beginners |
| ESP32-C3-DevKitC-02 | Espressif | Single-core 32-bit processor, 160MHz | 8MB Flash | 400KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×40×3mm | 18-25 yuan | Cost-effective, suitable for beginners |
| ESP32-C3-DevKitC-02 | Espressif | Single-core 32-bit processor, 160MHz | 8MB Flash | 400KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×40×3mm | 18-25 yuan | Cost-effective, suitable for beginners |
| ESP32-C3-DevKitC-02 | Espressif | Single-core 32-bit processor, 160MHz | 8MB Flash | 400KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×40×3mm | 18-25 yuan | Cost-effective, suitable for beginners |
| ESP32-S2-DevKitC-1 | Espressif | Single-core 32-bit processor, 240MHz | 16MB Flash | 320KB SRAM | WiFi 802.11b/g/n, USB Type-C interface | 25.4×60×3mm | 20-28 yuan | Low cost, suitable for WiFi applications |
| ESP32-S2-DevKitC-1 | Espressif | Single-core 32-bit processor, 240MHz | 16MB Flash | 320KB SRAM | WiFi 802.11b/g/n, USB Type-C interface | 25.4×60×3mm | 20-28 yuan | Low cost, suitable for WiFi applications |
| ESP32-S2-DevKitC-1 | Espressif | Single-core 32-bit processor, 240MHz | 16MB Flash | 320KB SRAM | WiFi 802.11b/g/n, USB Type-C interface | 25.4×60×3mm | 20-28 yuan | Low cost, suitable for WiFi applications |
| ESP32-S2-DevKitC-1 | Espressif | Single-core 32-bit processor, 240MHz | 16MB Flash | 320KB SRAM | WiFi 802.11b/g/n, USB Type-C interface | 25.4×60×3mm | 20-28 yuan | Low cost, suitable for WiFi applications |
| ESP32-C6-DevKitC-1 | Espressif | Single-core 32-bit processor, 160MHz | 8MB Flash | 512KB SRAM | WiFi 6, Bluetooth 5.0, supports IPv6, USB Type-C interface | 25.4×60×3mm | 22-30 yuan | Supports IPv6, suitable for IoT applications |
| ESP32-C6-DevKitC-1 | Espressif | Single-core 32-bit processor, 160MHz | 8MB Flash | 512KB SRAM | WiFi 6, Bluetooth 5.0, supports IPv6, USB Type-C interface | 25.4×60×3mm | 22-30 yuan | Supports IPv6, suitable for IoT applications |
| ESP32-C6-DevKitC-1 | Espressif | Single-core 32-bit processor, 160MHz | 8MB Flash | 512KB SRAM | WiFi 6, Bluetooth 5.0, supports IPv6, USB Type-C interface | 25.4×60×3mm | 22-30 yuan | Supports IPv6, suitable for IoT applications |
| ESP32-C6-DevKitC-1 | Espressif | Single-core 32-bit processor, 160MHz | 8MB Flash | 512KB SRAM | WiFi 6, Bluetooth 5.0, supports IPv6, USB Type-C interface | 25.4×60×3mm | 22-30 yuan | Supports IPv6, suitable for IoT applications |
| **RP2040 Series Development Boards** | | | | | | | |
| Adafruit Feather RP2040 | Adafruit | Dual-core ARM Cortex-M0+, 133MHz | 2MB Flash | 264KB SRAM | USB Type-C interface, LiPo battery charging circuit | 25.4×50.8×3mm | 25-35 yuan | Easy to expand, suitable for IoT applications |
| Adafruit Feather RP2040 | Adafruit | Dual-core ARM Cortex-M0+, 133MHz | 2MB Flash | 264KB SRAM | USB Type-C interface, LiPo battery charging circuit | 25.4×50.8×3mm | 25-35 yuan | Easy to expand, suitable for IoT applications |
| Adafruit Feather RP2040 | Adafruit | Dual-core ARM Cortex-M0+, 133MHz | 2MB Flash | 264KB SRAM | USB Type-C interface, LiPo battery charging circuit | 25.4×50.8×3mm | 25-35 yuan | Easy to expand, suitable for IoT applications |
| Adafruit Feather RP2040 | Adafruit | Dual-core ARM Cortex-M0+, 133MHz | 2MB Flash | 264KB SRAM | USB Type-C interface, LiPo battery charging circuit | 25.4×50.8×3mm | 25-35 yuan | Easy to expand, suitable for IoT applications |
| **ESP32 Series Development Boards** | | | | | | | |
| ESP32-S3-DevKitC-1 | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×60×3mm | 28-35 yuan | Espressif official development board, reliable quality, suitable for development and debugging |
| ESP32-S3-DevKitC-1 | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×60×3mm | 28-35 yuan | Espressif official development board, reliable quality, suitable for development and debugging |
| ESP32-S3-DevKitC-1 | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×60×3mm | 28-35 yuan | Espressif official development board, reliable quality, suitable for development and debugging |
| ESP32-S3-DevKitC-1 | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×60×3mm | 28-35 yuan | Espressif official development board, reliable quality, suitable for development and debugging |
| Seeed Studio XIAO ESP32-S3 | Seeed Studio | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, castellated hole design, USB Type-C interface | 17×20×3mm | 28-35 yuan | Castellated hole design, suitable for SMT installation |
| Seeed Studio XIAO ESP32-S3 | Seeed Studio | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, castellated hole design, USB Type-C interface | 17×20×3mm | 28-35 yuan | Castellated hole design, suitable for SMT installation |
| Seeed Studio XIAO ESP32-S3 | Seeed Studio | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, castellated hole design, USB Type-C interface | 17×20×3mm | 28-35 yuan | Castellated hole design, suitable for SMT installation |
| Seeed Studio XIAO ESP32-S3 | Seeed Studio | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM | WiFi 802.11b/g/n, Bluetooth 5.0, castellated hole design, USB Type-C interface | 17×20×3mm | 28-35 yuan | Castellated hole design, suitable for SMT installation |
| **General Development Boards** | | | | | | | |
| Elecrow ESP32-S3 | Elecrow | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, RGB LED, USB Type-C interface | 25.4×60×3mm | 40-55 yuan | Suitable for creative electronic projects |
| Elecrow ESP32-S3 | Elecrow | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, RGB LED, USB Type-C interface | 25.4×60×3mm | 40-55 yuan | Suitable for creative electronic projects |
| Elecrow ESP32-S3 | Elecrow | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, RGB LED, USB Type-C interface | 25.4×60×3mm | 40-55 yuan | Suitable for creative electronic projects |
| Elecrow ESP32-S3 | Elecrow | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, RGB LED, USB Type-C interface | 25.4×60×3mm | 40-55 yuan | Suitable for creative electronic projects |
| ESP32-S3-Pro | MakerBase | Dual-core 32-bit processor, 240MHz | 32MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×60×3mm | 35-50 yuan | Powerful performance, suitable for complex applications |
| ESP32-S3-Pro | MakerBase | Dual-core 32-bit processor, 240MHz | 32MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×60×3mm | 35-50 yuan | Powerful performance, suitable for complex applications |
| ESP32-S3-Pro | MakerBase | Dual-core 32-bit processor, 240MHz | 32MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×60×3mm | 35-50 yuan | Powerful performance, suitable for complex applications |
| ESP32-S3-Pro | MakerBase | Dual-core 32-bit processor, 240MHz | 32MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 25.4×60×3mm | 35-50 yuan | Powerful performance, suitable for complex applications |
| ESP32-S3-LCD-EV-Board | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | LCD controller, WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 50×70×4mm | 45-60 yuan | Designed for display applications, supports multiple display interfaces |
| ESP32-S3-LCD-EV-Board | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | LCD controller, WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 50×70×4mm | 45-60 yuan | Designed for display applications, supports multiple display interfaces |
| ESP32-S3-LCD-EV-Board | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | LCD controller, WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 50×70×4mm | 45-60 yuan | Designed for display applications, supports multiple display interfaces |
| ESP32-S3-LCD-EV-Board | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | LCD controller, WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 50×70×4mm | 45-60 yuan | Designed for display applications, supports multiple display interfaces |
| DFRobot FireBeetle 2 ESP32-S3 | DFRobot | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, LiPo battery interface, USB Type-C interface | 25.4×50.8×3mm | 50-65 yuan | Low power design, suitable for battery-powered applications |
| DFRobot FireBeetle 2 ESP32-S3 | DFRobot | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, LiPo battery interface, USB Type-C interface | 25.4×50.8×3mm | 50-65 yuan | Low power design, suitable for battery-powered applications |
| DFRobot FireBeetle 2 ESP32-S3 | DFRobot | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, LiPo battery interface, USB Type-C interface | 25.4×50.8×3mm | 50-65 yuan | Low power design, suitable for battery-powered applications |
| DFRobot FireBeetle 2 ESP32-S3 | DFRobot | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | WiFi 802.11b/g/n, Bluetooth 5.0, LiPo battery interface, USB Type-C interface | 25.4×50.8×3mm | 50-65 yuan | Low power design, suitable for battery-powered applications |
| **NRF52 Series Development Boards** | | | | | | | |
| Adafruit Feather nRF52832 | Adafruit | Single-core 32-bit processor, 64MHz | 512KB Flash | 64KB SRAM | Bluetooth 5.0, USB Type-C interface | 25.4×50.8×3mm | 55-70 yuan | Easy to expand, suitable for IoT applications |
| Adafruit Feather nRF52832 | Adafruit | Single-core 32-bit processor, 64MHz | 512KB Flash | 64KB SRAM | Bluetooth 5.0, USB Type-C interface | 25.4×50.8×3mm | 55-70 yuan | Easy to expand, suitable for IoT applications |
| Adafruit Feather nRF52832 | Adafruit | Single-core 32-bit processor, 64MHz | 512KB Flash | 64KB SRAM | Bluetooth 5.0, USB Type-C interface | 25.4×50.8×3mm | 55-70 yuan | Easy to expand, suitable for IoT applications |
| Adafruit Feather nRF52832 | Adafruit | Single-core 32-bit processor, 64MHz | 512KB Flash | 64KB SRAM | Bluetooth 5.0, USB Type-C interface | 25.4×50.8×3mm | 55-70 yuan | Easy to expand, suitable for IoT applications |
| **General Development Boards** | | | | | | | |
| ESP32-S3-Box | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | 2.4 inch TFT screen, OV2640 camera, microphone array, WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 80×80×15mm | 140-180 yuan | High integration, suitable for rapid prototyping |
| ESP32-S3-Box | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | 2.4 inch TFT screen, OV2640 camera, microphone array, WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 80×80×15mm | 140-180 yuan | High integration, suitable for rapid prototyping |
| ESP32-S3-Box | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | 2.4 inch TFT screen, OV2640 camera, microphone array, WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 80×80×15mm | 140-180 yuan | High integration, suitable for rapid prototyping |
| ESP32-S3-Box | Espressif | Dual-core 32-bit processor, 240MHz | 16MB Flash | 512KB SRAM + 8MB PSRAM | 2.4 inch TFT screen, OV2640 camera, microphone array, WiFi 802.11b/g/n, Bluetooth 5.0, USB Type-C interface | 80×80×15mm | 140-180 yuan | High integration, suitable for rapid prototyping |

## 2. Display Components

### 2.1 E-Paper Display

| Size | Model | Brand | Resolution | Interface | New Price | Used/Recycled Price | Type | Recommendation Reason | Available Driver Boards |
|------|-------|-------|------------|-----------|-----------|---------------------|------|-----------------------|------------------------|
| 1.02 inch | GDEW0102T4 | Good Display | 80×128 | SPI | About 20 yuan | About 8-10 yuan | New | Ultra-small size, suitable for micro devices | LILYGO e-Paper Driver, Waveshare e-Paper Driver HAT |
| 1.44 inch | GDEW0144Z07 | Good Display | 128×128 | SPI | About 25 yuan | About 10-12 yuan | New | Square design, suitable for displaying simple information | LILYGO e-Paper Driver, Good Display Extension Board |
| 1.54 inch | GDEW0154M09 | Good Display | 200×200 | SPI | About 30 yuan | About 12-15 yuan | New | Small size, suitable for small devices | Waveshare e-Paper Driver HAT, Seeed Studio Grove e-Paper Driver |
| 1.54 inch | Waveshare 1.54inch e-Paper | Waveshare | 200×200 | SPI | About 35 yuan | About 15-18 yuan | New | Compatible with multiple development boards, rich documentation | Waveshare e-Paper Driver HAT |
| 2.13 inch | GDEW0213M09 | Good Display | 212×104 | SPI | About 40 yuan | About 18-22 yuan | New | Wide screen design, suitable for displaying time and weather | LILYGO e-Paper Driver, MakerBase e-Paper Controller |
| 2.13 inch | GDEW0213Z19 | Good Display | 250×122 | SPI | About 45 yuan | About 20-25 yuan | New | High contrast, suitable for clear text display | Good Display Extension Board, Elecrow e-Paper Driver Board |
| 2.13 inch | Waveshare 2.13inch e-Paper HAT | Waveshare | 250×122 | SPI | About 50 yuan | About 22-28 yuan | New | With driver board, easy to use | Built-in driver board |
| 2.9 inch | GDEW029T5 | Good Display | 296×128 | SPI | About 50 yuan | About 25-30 yuan | New | Cost-effective, suitable for most applications | Waveshare e-Paper Driver HAT, LILYGO e-Paper Driver |
| 2.9 inch | GDEW029Z13 | Good Display | 296×128 | SPI | About 55 yuan | About 28-32 yuan | New | Fast refresh, suitable for dynamic content | Good Display Extension Board, MakerBase e-Paper Controller |
| 2.9 inch | Waveshare 2.9inch e-Paper V2 | Waveshare | 296×128 | SPI | About 60 yuan | About 30-35 yuan | New | Improved design, stable performance | Waveshare e-Paper Driver HAT |
| 3.7 inch | GDEW0371W7 | Good Display | 480×280 | SPI | About 70 yuan | About 35-40 yuan | New | Medium size, suitable for desktop applications | Elecrow e-Paper Driver Board, Seeed Studio Grove e-Paper Driver |
| 4.2 inch | GDEW042T2 | Good Display | 400×300 | SPI | About 80 yuan | About 40-45 yuan | New | Large display area, suitable for displaying more content | Waveshare e-Paper Driver HAT, LILYGO e-Paper Driver |
| 4.2 inch | GDEW042Z15 | Good Display | 400×300 | SPI | About 90 yuan | About 45-50 yuan | New | Black, white and red colors, richer display | Good Display Extension Board, MakerBase e-Paper Controller |
| 5.83 inch | GDEW0583T7 | Good Display | 600×448 | SPI | About 100 yuan | About 50-60 yuan | New | Large size, suitable for clear image and text display | Elecrow e-Paper Driver Board, Seeed Studio Grove e-Paper Driver |
| 7.5 inch | GDEW075T7 | Good Display | 800×480 | SPI | About 120 yuan | About 60-70 yuan | New | Large screen, suitable for living room and other places | Waveshare e-Paper Driver HAT, LILYGO e-Paper Driver |
| 7.5 inch | GDEW075Z09 | Good Display | 800×480 | SPI | About 130 yuan | About 65-75 yuan | New | Fast refresh, suitable for frequent content updates | Good Display Extension Board, MakerBase e-Paper Controller |
| 7.5 inch | Waveshare 7.5inch e-Paper HAT | Waveshare | 800×480 | SPI | About 140 yuan | About 70-80 yuan | New | With driver board, supports multiple interfaces | Built-in driver board |
| 9.7 inch | GDEW097T4 | Good Display | 1200×825 | SPI | About 250 yuan | About 120-150 yuan | New | Extra large size, suitable for displaying detailed information | Elecrow e-Paper Driver Board, Seeed Studio Grove e-Paper Driver |
| 10.3 inch | GDEW103T2 | Good Display | 1872×1404 | SPI | About 300 yuan | About 150-180 yuan | New | Close to A4 paper size, suitable for document reading | Waveshare e-Paper Driver HAT, LILYGO e-Paper Driver |
| 12.48 inch | GDEW1248Z17 | Good Display | 1304×984 | SPI | About 400 yuan | About 200-250 yuan | New | Super large area, suitable for wall display | Good Display Extension Board, MakerBase e-Paper Controller |
| 6.0 inch | Kindle Paperwhite recycled screen | Amazon | 1024×758 | SPI | - | About 30-40 yuan | Used/recycled | Second-hand e-reader screen, cost-effective | Waveshare e-Paper Driver HAT, DIY driver board |
| 7.8 inch | Kindle Oasis recycled screen | Amazon | 1264×1680 | SPI | - | About 50-70 yuan | Used/recycled | High resolution, suitable for document reading | Waveshare e-Paper Driver HAT, Seeed Studio Grove e-Paper Driver |
| 10.3 inch | Remarkable 2 recycled screen | Remarkable | 1872×1404 | SPI | - | About 100-150 yuan | Used/recycled | High-quality electronic paper, suitable for writing and reading | Good Display Extension Board, Elecrow e-Paper Driver Board |
| 2.13 inch | Hema electronic price tag screen | Hema | 250×122 | SPI | About 30 yuan | About 0.7-5 yuan | New/Used | Supermarket electronic price tag,大量 available on Xianyu, extremely cost-effective | LILYGO e-Paper Driver, Waveshare e-Paper Driver HAT |
| 2.66 inch | Hema electronic price tag screen | Hema | 296×152 | SPI | About 35 yuan | About 1-6 yuan | New/Used | Supermarket electronic price tag, moderate display area | Good Display Extension Board, MakerBase e-Paper Controller |
| 3.12 inch | Hema electronic price tag screen | Hema | 400×300 | SPI | About 45 yuan | About 2-8 yuan | New/Used | Supermarket electronic price tag, high resolution | Elecrow e-Paper Driver Board, Seeed Studio Grove e-Paper Driver |
| 4.2 inch | Hema electronic price tag screen | Hema | 400×300 | SPI | About 60 yuan | About 3-10 yuan | New/Used | Supermarket electronic price tag, three-color display, suitable for displaying more information | LILYGO e-Paper Driver, Waveshare e-Paper Driver HAT |
| 4.2 inch | 7Fresh electronic price tag screen | JD 7Fresh | 400×300 | SPI | About 55 yuan | About 2-9 yuan | Used | JD supermarket electronic price tag, good condition | Good Display Extension Board, MakerBase e-Paper Controller |
| 2.9 inch | Xiaomi Home electronic price tag screen | Xiaomi | 296×128 | SPI | About 40 yuan | About 1-6 yuan | Used | Xiaomi offline store electronic price tag, good compatibility | Elecrow e-Paper Driver Board, Seeed Studio Grove e-Paper Driver |
| 6.0 inch | Kindle Paperwhite 3 recycled screen | Amazon | 1024×758 | SPI | - | About 30-45 yuan | Used/recycled | Amazon e-reader recycled screen, good display effect | Waveshare e-Paper Driver HAT, DIY driver board |
| 7.8 inch | Kindle Oasis 2 recycled screen | Amazon | 1264×1680 | SPI | - | About 50-75 yuan | Used/recycled | Amazon high-end e-reader recycled screen, high resolution | Good Display Extension Board, Elecrow e-Paper Driver Board |
| 10.3 inch | Remarkable 2 recycled screen | Remarkable | 1872×1404 | SPI | - | About 100-160 yuan | Used/recycled | Electronic note device recycled screen, high-quality electronic paper | LILYGO e-Paper Driver, Waveshare e-Paper Driver HAT |
| 1.54 inch | Xianyu homemade e-paper screen | Personal DIY | 200×200 | SPI | About 20 yuan | About 8-12 yuan | New/Used | Homemade by Xianyu experts, cost-effective | MakerBase e-Paper Controller, DIY driver board |
| 4.2 inch | Xianyu homemade e-paper screen | Personal DIY | 400×300 | SPI | About 50 yuan | About 20-30 yuan | New/Used | Xianyu homemade, supports multiple driver boards | Waveshare e-Paper Driver HAT, Good Display Extension Board |

### 2.2 E-Paper Display Driver Boards

| Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-------------|-----------------------|
| DIY driver board | Homemade | Designed based on ESP32-S3 | 35×25×2mm | 8-15 yuan | Low cost, customizable according to needs |
| MakerBase e-Paper Controller | MakerBase | Low-cost e-paper display driver board | 40×25×2mm | 10-15 yuan | Cost-effective, suitable for entry-level applications |
| Good Display Extension Board | Good Display | Designed specifically for Good Display e-paper displays | 50×25×2mm | 13-18 yuan | Perfectly compatible with Good Display e-paper displays |
| LILYGO e-Paper Driver | LILYGO | Supports 1.54-7.5 inch e-paper displays | 40×25×2mm | 16-22 yuan | Cost-effective, suitable for multiple e-paper displays |
| Waveshare e-Paper Driver HAT | Waveshare | Supports multiple sizes of e-paper displays | 65×30×2mm | 18-25 yuan | Good compatibility, supports multiple e-paper display sizes |
| Elecrow e-Paper Driver Board | Elecrow | Supports multiple e-paper displays, Type-C interface | 45×30×2mm | 20-28 yuan | Good compatibility, suitable for DIY projects |
| Seeed Studio Grove e-Paper Driver | Seeed Studio | Grove interface, supports multiple e-paper displays | 30×20×2mm | 22-30 yuan | Modular design, easy to expand |

## 3. Power Components

### 3.1 Soft-pack Polymer Battery

| Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-------------|-----------------------|
| 103450 | MY | 3.7V, 2000mAh, ternary lithium battery | 10×34×5mm | 3-6 yuan | Cost-effective, suitable for e-books, portable devices |
| 502080 | Pure cobalt | 3.7V, 1000mAh, pure cobalt battery cell | 50×20×8mm | 4-8 yuan | Pure cobalt battery cell, excellent performance |
| HM 102050 | Haoming | 3.7V, 1000mAh, ternary cathode material | 10×20×5mm | 6-10 yuan | High discharge platform, good cycling performance, lifespan >500 cycles, suitable for small devices |
| 303450 | Zhongsunxin | 3.7V, 2000mAh, high-temperature soft-pack | 30×34×5mm | 6-10 yuan | High-temperature resistant design, suitable for various environments |
| 203048 | Eve Energy | 3.7V, 1500mAh, well-known brand | 20×30×4.8mm | 8-12 yuan | Well-known brand, reliable quality, thin design |
| 505573 | Tianqin New Energy | 3.7V, 2500mAh, dedicated to smart devices | 50×55×7.3mm | 10-15 yuan | Designed specifically for smart devices, stable performance |
| 253048 | Guoxuan Hi-Tech | 3.7V, 1800mAh, thin design | 25×30×4.8mm | 10-15 yuan | Thin design, suitable for thin devices |
| 523450 | Zhongsunxin | 3.7V, 4000mAh, large capacity | 52×34×5mm | 13-18 yuan | Large capacity, suitable for high-power consumption devices |
| 103565 | Dajiamanyi | 3.7V, 3000mAh, no swelling design | 10×35×6.5mm | 14-20 yuan | No swelling, long-lasting battery life, suitable for long-term use |
| 355060 | CATL | 3.7V, 3500mAh, large capacity | 35×50×6mm | 18-25 yuan | Large capacity, long lifespan, suitable for long-term use |
| 405065 | BAK Battery | 3.7V, 4000mAh, high capacity | 40×50×6.5mm | 20-28 yuan | High capacity, cost-effective |
| 626078 | Amprius | 3.7V, 5000mAh, high-quality battery cell | 62×60×7.8mm | 22-30 yuan | High-quality battery cell, lightweight and compact |

### 3.2 Charging Modules

| Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-------------|-----------------------|
| AMS1117-3.3 | Elecrow | 3.3V voltage regulator module, suitable for small current applications | 10×8×2mm | 1.5-3 yuan | Low cost, suitable for simple circuits |
| TP4056 charging board | Seeed Studio | Classic TP4056 charging IC, Micro-USB interface | 25×15×2mm | 2-5 yuan | Low cost, suitable for simple applications |
| TP4056+Type-C | MakerBase | USB-Type-C interface, TP4056 charging IC, supports 10W charging | 25×15×2mm | 4-7 yuan | Cost-effective, suitable for entry-level applications |
| Type-C charging module | Waveshare | USB-Type-C interface, 5V/2A, supports 10W charging | 25×15×2mm | 8-12 yuan | Good compatibility, suitable for multiple devices |
| QC3.0+Type-C | DFRobot | USB-Type-C interface, supports QC2.0/QC3.0 fast charging, 10W-20W | 30×20×3mm | 13-18 yuan | Supports fast charging, fast charging speed |
| AXP192+Type-C | LILYGO | USB-Type-C interface, AXP192 power management IC, supports 10W-15W charging | 30×20×3mm | 18-25 yuan | Powerful functions, suitable for complex systems |

### 3.3 Power Management Modules

| Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-------------|-----------------------|
| AMS1117-5.0 | Domestic brand | 5V voltage regulator module, suitable for small current applications | 10×8×2mm | 1.5-3 yuan | Low cost, suitable for simple circuits |
| MT3608 boost module | Domestic brand | 5V input, 3.3V-24V adjustable output | 15×10×2mm | 2-5 yuan | Low cost, suitable for boost applications |
| Lithium battery protection board | MakerBase | Supports overcharge, overdischarge, overcurrent, short circuit protection | 25×15×1mm | 4-7 yuan | Protects battery safety, extends lifespan |
| Type-C power management | Waveshare | USB-Type-C input, supports 10W-20W charging, with battery protection | 30×20×3mm | 13-18 yuan | High integration, easy to use |
| AXP192 module | LILYGO | Designed based on AXP192, supports 6 DC-DC outputs, 2 LDO outputs | 30×20×3mm | 18-25 yuan | Powerful functions, suitable for multi-channel power supply needs |
| BQ24074 module | TI | Texas Instruments BQ24074 charging IC, supports fast charging | 25×15×2mm | 22-30 yuan | High performance, suitable for high-end applications |

**Note: This device only supports USB-Type-C charging interface, does not support DC direct plug-in power supply, charging power range is 10W-20W.**

## 4. Sensor Components

### 4.1 Temperature and Humidity Sensors

| Model | Brand | Specification | Size | Interface | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-----------|-------------|-----------------------|
| DHT22 | AOSONG | Temperature: -40℃~80℃, Humidity: 0%~100%RH, Accuracy: ±0.5℃/±2%RH | 22×15×5mm | Single bus | 4-7 yuan | Cost-effective, widely used |
| AM2302 | AOSONG | Encapsulated version of DHT22, Temperature: -40℃~80℃, Humidity: 0%~100%RH | 22×15×5mm | Single bus | 6-10 yuan | Waterproof design, suitable for outdoor applications |
| SHT20 | Sensirion | Temperature: -40℃~125℃, Humidity: 0%~100%RH, Accuracy: ±0.5℃/±3%RH | 3×3×0.9mm | I2C | 6-10 yuan | Small size, suitable for space-limited applications |
| BME280 | Bosch | Pressure: 300-1100 hPa, Temperature: -40℃~85℃, Humidity: 0%~100%RH | 3.5×3.5×0.9mm | I2C/SPI | 8-12 yuan | Multi-functional, suitable for weather prediction |
| HTU21D | Measurement Specialties | Temperature: -40℃~125℃, Humidity: 0%~100%RH, Accuracy: ±0.3℃/±2%RH | 3×3×0.9mm | I2C | 8-12 yuan | Compatible with SHT21 |
| SI7021 | Silicon Labs | Temperature: -40℃~125℃, Humidity: 0%~100%RH, Accuracy: ±0.4℃/±3%RH | 3×3×0.9mm | I2C | 8-12 yuan | Compatible with SHT21 |
| SHT30 | Sensirion | Temperature: -40℃~125℃, Humidity: 0%~100%RH, Accuracy: ±0.3℃/±2%RH | 5×5×1.3mm | I2C | 8-12 yuan | High accuracy, fast response speed, suitable for high-precision applications |
| HDC1080 | Texas Instruments | Temperature: -40℃~125℃, Humidity: 0%~100%RH, Accuracy: ±0.2℃/±2%RH | 3×3×0.8mm | I2C | 10-15 yuan | Low power consumption, suitable for battery-powered applications |
| SHT21 | Sensirion | Temperature: -40℃~125℃, Humidity: 0%~100%RH, Accuracy: ±0.3℃/±2%RH | 3×3×0.9mm | I2C | 10-15 yuan | High accuracy, suitable for professional applications |
| BME680 | Bosch | Temperature, humidity, pressure and gas sensor, Temperature: -40℃~85℃, Humidity: 0%~100%RH, Pressure: 300-1100 hPa | 3.5×3.5×0.9mm | I2C/SPI | 12-18 yuan | Rich functions, supports gas detection |
| SHT40 | Sensirion | Temperature: -40℃~125℃, Humidity: 0%~100%RH, Accuracy: ±0.3℃/±2%RH | 3×3×0.9mm | I2C | 12-18 yuan | Fast response speed, suitable for dynamic environment monitoring |

### 4.2 Human Presence Sensors

| Model | Brand | Specification | Size | Interface | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-----------|-------------|-----------------------|
| HC-SR505 | Guangzhou Huicheng | Miniaturized design, Detection distance: 2-4 meters, Detection angle: 80° | 12×20×5mm | Digital output | 1.5-3 yuan | Small size, suitable for space-limited applications |
| HC-SR501 | Shenzhen Jieshun | Detection distance: 3-7 meters, Detection angle: 110° | 32×24×15mm | Digital output | 2-5 yuan | Low price, widely used |
| RCWL-0516 | Shenzhen Jinshunchang | Microwave radar sensor module, Detection distance: 5-7 meters | 25×25×5mm | Digital output | 3-5 yuan | Non-contact detection, suitable for hidden installation |
| RE200B | Excelitas | High-sensitivity infrared sensor, Detection distance: up to 10 meters | 5×5×2mm | Analog output | 4-7 yuan | High sensitivity, suitable for long-distance detection |
| BH1750 | Rohm | Human presence + light sensor 2-in-1 | 3×3×0.9mm | I2C | 6-10 yuan | Multi-functional, saves space |
| LD2410 | Espressif | Millimeter wave radar, supports distance detection, Detection range: 0-8 meters | 15×25×3mm | UART | 22-30 yuan | Supports distance detection, strong anti-interference ability |

### 4.3 Light Sensors

| Model | Brand | Specification | Size | Interface | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-----------|-------------|-----------------------|
| BH1750 | Rohm | 1-65535 lx, Accuracy: ±20% | 3×3×0.9mm | I2C | 4-6 yuan | High accuracy, fast response speed |
| GY30 | Hangzhou Jinghua | High-precision light sensor, 0-65535 lx | 3×3×0.9mm | I2C | 4-6 yuan | Cost-effective |
| TSL2561 | ams | Wide dynamic range, 0.1-40000 lx | 3×3×0.8mm | I2C | 8-12 yuan | Suitable for complex lighting environments |
| SI1145 | Silicon Labs | Ambient light, UV, proximity detection 3-in-1 | 3×3×0.8mm | I2C | 10-15 yuan | Multi-functional sensor |
| OPT3001 | Texas Instruments | High-precision ambient light sensor, 0.01-83k lx | 3×3×0.8mm | I2C | 12-18 yuan | High precision, suitable for professional applications |

### 4.4 Gas Sensors

| Model | Brand | Specification | Size | Interface | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-----------|-------------|-----------------------|
| MQ-2 | Wuhan Minxin | Detects smoke, gas, methane and other gases | 32×24×15mm | Analog output | 6-10 yuan | Wide detection range, suitable for home safety |
| MQ-5 | Shenzhen Weisheng | Detects liquefied petroleum gas, methane, propane and other gases | 32×24×15mm | Analog output | 6-10 yuan | Suitable for gas leakage detection |
| MQ-7 | Guangzhou Hanwei | Detects carbon monoxide gas, high sensitivity | 32×24×15mm | Analog output | 6-10 yuan | Suitable for carbon monoxide concentration detection |
| MQ-135 | Zhengzhou Weisheng | Detects formaldehyde, benzene, toluene, NH3, NOx and other harmful gases | 32×24×15mm | Analog output | 8-12 yuan | Wide detection range, suitable for indoor air quality monitoring |
| TGS2600 | Figaro | Detects various volatile organic compounds | 10×10×5mm | Analog output | 12-18 yuan | High accuracy, fast response speed |
| SGP30 | Sensirion | Digital air quality sensor, detects CO2 and VOC | 3×3×0.9mm | I2C | 25-35 yuan | Digital output, high accuracy, suitable for high-end applications |

### 4.5 Flame Sensors

| Model | Brand | Specification | Size | Interface | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-----------|-------------|-----------------------|
| IR flame sensor | Shenzhen Jieshun | Detects infrared flame, Detection distance: 0.1-1 meter | 20×15×8mm | Digital output | 2-4 yuan | Low price, widely used |
| YG1006 | Hangzhou Jinghua | High-sensitivity infrared flame sensor, Detection distance: 0.1-2 meters | 20×15×8mm | Digital output | 4-6 yuan | High sensitivity |
| UV flame sensor | Guangzhou Huicheng | Detects ultraviolet flame, Detection distance: up to 10 meters | 25×20×10mm | Digital/Analog output | 6-10 yuan | Suitable for long-distance detection of open flames |
| MQ-2 | Wuhan Minxin | Can detect smoke and flames | 32×24×15mm | Analog output | 6-10 yuan | Multi-functional, cost-effective |
| TGS2600 | Figaro | Can detect smoke generated by fire | 10×10×5mm | Analog output | 12-18 yuan | High accuracy, reliable |

### 4.6 Pressure Sensors

| Model | Brand | Specification | Size | Interface | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-----------|-------------|-----------------------|
| LPS25HB | STMicroelectronics | Pressure: 260-1260 hPa, Temperature: -30℃~105℃ | 3×3×0.8mm | I2C/SPI | 6-10 yuan | Low power consumption, suitable for battery-powered applications |
| BMP280 | Bosch | Pressure: 300-1100 hPa, Temperature: -40℃~85℃ | 3.5×3.5×0.9mm | I2C/SPI | 8-12 yuan | High accuracy, suitable for weather prediction |
| BMP388 | Bosch | Pressure: 300-1100 hPa, Temperature: -40℃~85℃, higher accuracy | 3.5×3.5×0.9mm | I2C/SPI | 10-15 yuan | New generation product, better performance |

## 5. Multimedia Components

### 5.1 Camera Modules

| Model | Brand | Specification | Interface | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|-----------|------|-------------|-----------------------|
| GC0308 micro camera | Domestic | 300,000 pixels, 640×480 resolution, low power consumption | SCCB/I2C | 18×18×3mm | 8-12 yuan | Ultra-small size, thickness only 3mm, suitable for applications with extremely high volume requirements |
| OV2640 micro camera | Domestic | 2 million pixels, 1600×1200 resolution, supports JPEG output | SCCB/I2C | 25×25×4mm | 12-18 yuan | Ultra-small size, suitable for micro devices, thickness only 4mm, meets the overall thickness requirements of the device |
| OV5640 micro camera | Domestic | 5 million pixels, 2592×1944 resolution, supports JPEG output | SCCB/I2C | 28×28×5mm | 18-25 yuan | High resolution, thickness 5mm, suitable for applications requiring high-quality images |
| ESP32-CAM dedicated camera | Espressif | 2 million pixels, supports WiFi+Bluetooth, built-in OV2640 | Integrated | 24×40×4mm | 22-30 yuan | Designed specifically for ESP32, high integration, thickness 4mm, suitable for ESP32 development boards |

### 5.2 Microphone Modules

| Model | Brand | Specification | Interface | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|-----------|------|-------------|-----------------------|
| MAX4466 | Maxim | Analog microphone amplifier, adjustable gain | Analog | 9×9×2mm | 4-6 yuan | Low cost, thickness 2mm, suitable for simple applications |
| SPH0645LM4H-1 | Knowles | I2S digital microphone, -26dBFS sensitivity | I2S | 4×3×1.2mm | 6-10 yuan | Ultra-small size, thickness only 1.2mm, suitable for micro devices |
| INMP441 | InvenSense | I2S digital microphone, -26dBFS sensitivity, low power consumption | I2S | 3.5×2.65×0.98mm | 8-12 yuan | Extremely small size, thickness less than 1mm, suitable for applications with extremely high thickness requirements |
| WM8960 | Wolfson | Audio codec, built-in microphone amplifier | I2S | 5×5×0.8mm | 12-18 yuan | High integration, thickness only 0.8mm, suitable for applications requiring high-quality audio |

### 5.3 Speaker Modules

| Model | Brand | Specification | Interface | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|-----------|------|-------------|-----------------------|
| PAM8403 micro amplifier | Domestic | 3W output power, mono, 5V power supply | Analog | 8×8×2mm | 2.5-4 yuan | Ultra-small size, thickness 2mm, suitable for micro devices |
| WS8113 | Weipu Innovation | 3W Class D amplifier, mono, 5V power supply | Analog | 7×7×2mm | 3-5 yuan | Small size, thickness 2mm, cost-effective |
| NS4168 | NXP | 2.5W Class D amplifier, mono, 2.5V-5.5V power supply | Analog | 6×6×1.6mm | 4-6 yuan | Small size, thickness 1.6mm, suitable for low power consumption applications |
| MAX98357A | Maxim | 3.2W Class D amplifier, mono, 3.3V-5V power supply | I2S | 4×4×1.2mm | 6-10 yuan | Extremely small size, thickness only 1.2mm, suitable for applications with extremely high thickness requirements |

### 5.4 Audio Decoder Modules

| Model | Brand | Specification (Including Components) | Interface | Size | Price Range | Recommendation Reason |
|-------|-------|-------------------------------------|-----------|------|-------------|-----------------------|
| ISD1820 recording module | Domestic | Integrated ISD1820 recording chip, SPI storage, microphone input | Button control | 20×25×3mm | 6-12 yuan | Low-cost recording solution, built-in SPI storage, no 3.5mm headphone jack, suitable for simple recording applications |
| YX5300-24SS | Domestic | Integrated YX5300 decoding chip, SPI Flash storage, audio output | UART | 20×25×3mm | 8-15 yuan | Serial control, supports multiple audio formats, built-in SPI Flash storage, no 3.5mm headphone jack |
| VS1003B storage version | Domestic | Integrated VS1003B decoding chip, MicroSD card slot | SPI | 25×35×4mm | 10-20 yuan | Low-cost audio decoding solution, supports MP3 format, with MicroSD card storage, no 3.5mm headphone jack |
| YX6300-24SS | Domestic | Integrated YX6300 decoding chip, TF card slot, audio amplifier | UART | 25×30×4mm | 10-20 yuan | Supports TF card storage, built-in amplifier, no 3.5mm headphone jack, suitable for audio playback applications |
| WT588D-M02 | Domestic | Integrated WT588D voice chip, SPI Flash storage, audio output | IO port control | 20×25×3mm | 12-20 yuan | Supports recording and playback, built-in SPI Flash storage, no 3.5mm headphone jack, suitable for voice prompt applications |
| VS1053B no headphone jack module | Domestic | Integrated VS1053B decoding chip, MicroSD card slot, audio amplifier | SPI | 30×40×5mm | 12-25 yuan | Supports multiple audio formats such as MP3/WMA/WAV, built-in audio amplifier and MicroSD card storage, no 3.5mm headphone jack |
| ATmega328 audio player | Domestic | Integrated ATmega328 chip, SD card slot, audio output | I2S | 25×30×4mm | 15-25 yuan | Based on Arduino design, supports SD card storage, no 3.5mm headphone jack, suitable for DIY audio applications |
| NRF52832 audio module | Domestic | Integrated NRF52832 chip, Flash storage, Bluetooth audio transmission | Bluetooth | 25×30×4mm | 15-30 yuan | Supports Bluetooth audio transmission, built-in Flash storage, no 3.5mm headphone jack, suitable for wireless audio applications |
| STM32F103 audio module | Domestic | Integrated STM32F103 chip, Flash storage, I2S audio output | I2S | 25×30×4mm | 18-30 yuan | Based on STM32 design, built-in Flash storage, no 3.5mm headphone jack, suitable for high-quality audio applications |
| ESP32 audio decoding module | Domestic | Integrated ESP32 chip, TF card slot, I2S audio output | I2S/WiFi/Bluetooth | 30×40×5mm | 20-35 yuan | Based on ESP32 design, supports WiFi and Bluetooth, with TF card storage, no 3.5mm headphone jack, suitable for IoT audio applications |

## 6. Input/Output Components

### 6.1 Buttons

| Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|-------|-------|---------------|------|-------------|-----------------------|
| Tactile button | Dongguan Angui | 6×6×5mm | 6×6×5mm | 0.08-0.15 yuan/piece | Low price, good operating feel |
| Tactile button | Shenzhen Dongqiang | 12×12×7mm | 12×12×7mm | 0.2-0.4 yuan/piece | Large size, suitable for elderly people |
| Illuminated tactile button | Taiwan Yuanda | 6×6×5mm, with LED indicator | 6×6×5mm | 0.4-0.6 yuan/piece | With indicator light, operation status visualization |
| Waterproof tactile button | Zhejiang Feida | Waterproof level: IP67 | 8×8×6mm | 0.8-1.2 yuan/piece | Suitable for outdoor applications |

### 6.2 Indicators

| Type | Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|------|-------|-------|---------------|------|-------------|-----------------------|
| SMD LED | Suzhou Jingfang | 0805, Current: 10mA | Red/Green/Blue/Yellow | 2×1.25×0.8mm | 0.03-0.08 yuan/piece | Small size, suitable for SMT installation |
| LED indicator | Shenzhen Huaqiang | 3mm, Current: 20mA | Red/Green/Blue/Yellow | 3×10mm | 0.08-0.15 yuan/piece | Low price, widely used |
| LED indicator | Hangzhou Hongyan | 5mm, Current: 20mA | Red/Green/Blue/Yellow | 5×12mm | 0.15-0.25 yuan/piece | High brightness, suitable for long-distance viewing |
| High-brightness LED | Taiwan Everlight | 3mm, Current: 20mA, Brightness: 2000mcd | Red/Green/Blue/Yellow | 3×10mm | 0.2-0.4 yuan/piece | Ultra-high brightness, suitable for strong light environments |

### 6.3 Buzzers

| Type | Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|------|-------|-------|---------------|------|-------------|-----------------------|
| Passive buzzer | Shanghai Huahong | 5V, Frequency: 1-5KHz | Piezoelectric | 9×9×3mm | 0.4-0.6 yuan | Requires external driver, can produce various frequencies of sound |
| Active buzzer | Shenzhen Ketong | 5V, Frequency: 2300Hz | Electromagnetic | 9×9×6mm | 0.8-1.2 yuan | Directly connected to power supply can sound, easy to use |
| SMD buzzer | Guangzhou Jinghua | 5V, Volume: 12×9×5mm | Electromagnetic | 12×9×5mm | 1.5-2.5 yuan | Small size, suitable for SMT installation |
| Waterproof buzzer | Zhejiang Feida | 5V, Waterproof level: IP67 | Electromagnetic | 12×12×8mm | 2.5-3.5 yuan | Waterproof design, suitable for outdoor applications |

## 7. Communication Components

### 7.1 WiFi Antennas

| Type | Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|------|-------|-------|---------------|------|-------------|-----------------------|
| Built-in antenna | PCB-ANT-24G | Shanghai Amphenol | 2.4GHz, Gain: 1dBi | 10×30×0.8mm | 1.5-3 yuan | Small size, easy to install |
| Ceramic antenna | CER-ANT-24G | Taiwan Sunlord | 2.4GHz, Gain: 1.5dBi | 5×5×1.5mm | 2.5-4 yuan | Stable performance, suitable for high-precision applications |
| External antenna | ANT-2400-2DBI | Shenzhen Sunway | 2.4GHz, Gain: 2dBi | 5×60mm | 4-7 yuan | Good signal, suitable for long-distance transmission |
| High-gain antenna | ANT-2400-5DBI | Hangzhou Hikvision | 2.4GHz, Gain: 5dBi | 8×100mm | 8-12 yuan | Ultra-high gain, suitable for long-distance transmission |

### 7.2 Bluetooth Antennas

| Type | Model | Brand | Specification | Size | Price Range | Recommendation Reason |
|------|-------|-------|---------------|------|-------------|-----------------------|
| Built-in Bluetooth antenna | PCB-BT-24G | Shanghai TE | 2.4GHz, Gain: 1dBi | 10×30×0.8mm | 1.5-3 yuan | Small size, easy to install |
| Ceramic Bluetooth antenna | CER-BT-24G | Taiwan Yageo | 2.4GHz, Gain: 1.5dBi | 5×5×1.5mm | 2.5-4 yuan | Stable performance, suitable for high-precision applications |
| Bluetooth antenna | BT-ANT-2400 | Shenzhen Luxshare | 2.4GHz, Gain: 2dBi | 5×60mm | 4-7 yuan | Good signal, suitable for long-distance transmission |

### 7.3 Connection Wires

| Type | Specification | Brand | Size | Price Range | Recommendation Reason |
|------|---------------|-------|------|-------------|-----------------------|
| Silicone wire | 22AWG, multiple colors | Zhejiang Zhongce | 0.64mm diameter | 0.8-1.2 yuan/meter | Good flexibility, suitable for frequent bending occasions |
| Shielded wire | 24AWG, with shield layer | Shenzhen Jinxinnuo | 1.2mm diameter | 1.5-2.5 yuan/meter | Strong anti-interference ability, suitable for high-speed signal transmission |
| Ribbon cable | 10cm, 10Pin | Suzhou Yizhou | 1.0mm diameter | 4-6 yuan/meter | Suitable for fixed connections, neat and tidy |
| Dupont wire | 20cm, male to male | Shenzhen Qiuyeyuan | 0.8mm diameter | 8-12 yuan/100 pieces | Low price, suitable for development and debugging |
| Dupont wire | 20cm, male to female | Shanghai Lvlian | 0.8mm diameter | 10-15 yuan/100 pieces | Low price, suitable for development and debugging |
| Dupont wire | 20cm, female to female | Guangzhou Shanze | 0.8mm diameter | 12-18 yuan/100 pieces | Low price, suitable for development and debugging |

## 8. Structural Components

### 8.1 Enclosures

| Type | Material | Brand | Size | Price Range | Recommendation Reason |
|------|----------|-------|------|-------------|-----------------------|
| 3D printed enclosure | PLA | Shenzhen Creality | Custom size | 8-12 yuan (material cost) | Customizable, suitable for DIY |
| 3D printed enclosure | ABS | Hangzhou Shanzu | Custom size | 12-18 yuan (material cost) | High strength, suitable for mass production |
| Acrylic enclosure | Acrylic | Shanghai Acrylic | Custom size | 15-30 yuan | Transparent, suitable for displaying internal structure |
| Plastic enclosure | ABS | Dongguan Weichuang | Custom size | 20-50 yuan | Beautiful, suitable for mass production products |
| Wooden enclosure | Solid wood | Suzhou Muyi | Custom size | 30-100 yuan | Good texture, suitable for high-end products |
| Aluminum alloy enclosure | Aluminum alloy | Shenzhen Lvye | Custom size | 50-100 yuan | Good heat dissipation, suitable for high-performance devices |

### 8.2 Fasteners

| Type | Specification | Material | Brand | Size | Price Range | Recommendation Reason |
|------|---------------|----------|-------|------|-------------|-----------------------|
| Nut | M2/M2.5/M3 | Stainless steel | Hebei Standard Parts | M2/M2.5/M3 | 0.04-0.06 yuan/piece | Used with screws |
| Washer | φ2.5/φ3/φ4 | Stainless steel | Jiangsu Screw | φ2.5/φ3/φ4mm | 0.04-0.06 yuan/piece | Prevents screw loosening, protects component surface |
| Self-tapping screw | ST2.5×10 | Carbon steel | Taiwan Hongquan | ST2.5×10mm | 0.06-0.1 yuan/piece | Suitable for plastic enclosure fixing |
| Screw | M2×6 | Stainless steel | Shenzhen Standard Parts | M2×6mm | 0.08-0.15 yuan/piece | Suitable for fixing small components |
| Screw | M2.5×8 | Stainless steel | Shanghai Fasteners | M2.5×8mm | 0.12-0.2 yuan/piece | Suitable for fixing development boards and modules |
| Screw | M3×10 | Stainless steel | Zhejiang Dongming | M3×10mm | 0.15-0.25 yuan/piece | Suitable for fixing enclosures and large components |

### 8.3 Thermal Materials

| Type | Specification | Material | Brand | Size | Price Range | Recommendation Reason |
|------|---------------|----------|-------|------|-------------|-----------------------|
| Thermal paste | 2g pack | Silicone grease | Shanghai Thermal | 2g pack | 2.5-3.5 yuan | Good thermal conductivity, suitable for filling between CPU and heat sink |
| Thermal silicone pad | 0.5mm, 100×100mm | Silicone | Shenzhen Thermal Technology | 0.5×100×100mm | 4-6 yuan | Good thermal conductivity, insulated, suitable for chip cooling |
| Thermal tape | 0.3mm, 100×100mm | Pressure-sensitive adhesive | Hangzhou Thermal | 0.3×100×100mm | 6-10 yuan | Good thermal conductivity, easy to install |
| Graphite heat sink | 0.2mm, 100×100mm | Graphite | Dongguan Graphite | 0.2×100×100mm | 8-12 yuan | High thermal conductivity, suitable for ultra-thin devices |

## 10. Tools and Accessories

### 10.1 Welding Tools

| Type | Model | Brand | Price Range | Recommendation Reason |
|------|-------|-------|-------------|-----------------------|
| Soldering flux | 10ml | Domestic brand | 4-7 yuan | Improves welding quality, prevents oxidation |
| Desoldering pump | Manual/electric | Domestic brand | 5-50 yuan | Used to remove excess solder |
| Solder wire | 0.8mm, 63/37 | Domestic brand | 8-15 yuan/roll | Low melting point, good welding quality |
| Soldering iron | 60W | Huanghua/Irontip | 30-100 yuan | Suitable for soldering components |
| Hot air gun | 858D | Domestic brand | 100-300 yuan | Suitable for soldering SMD components and desoldering |
| Soldering station | 936/937 | Hakko/Quick | 100-500 yuan | Stable temperature, suitable for long-time welding |

### 10.2 Measurement Tools

| Type | Model | Brand | Price Range | Recommendation Reason |
|------|-------|-------|-------------|-----------------------|
| USB data cable | Domestic brand | None | 10-30 yuan | Used to connect development board and computer |
| Power adapter | 5V/2A | Domestic brand | 10-30 yuan | Used to power the development board |
| Battery tester | Domestic brand | Can test voltage and capacity | 10-50 yuan | Used to test battery performance |
| Multimeter | DT9205A | Victor/UNI-T | 50-200 yuan | Used to measure voltage, current, resistance, etc. |
| Logic analyzer | DSLogic Plus | DreamSourceLab | 200-500 yuan | Used to analyze digital signals |
| Oscilloscope | DS1102E | Rigol | 1000-3000 yuan | Used for debugging signals |

### 10.3 Accessories

| Type | Specification | Price Range | Recommendation Reason |
|------|---------------|-------------|-----------------------|
| Insulating tape | Electrical insulating tape | 1.5-3 yuan/roll | Used for insulation and fixing |
| Rosin | Block | 1.5-3 yuan | Used for soldering flux |
| Double-sided tape | Strong double-sided tape | 2.5-4 yuan/roll | Used for fixing components |
| Heat shrink tubing | Multiple specifications | 4-6 yuan/meter | Used to protect wires and solder joints |
| Cable tie | 3×100mm | 8-12 yuan/100 pieces | Used for organizing wires |
| PCB cleaner | 500ml | 8-12 yuan | Used for cleaning circuit boards |

## 11. Recommended Configuration Schemes

### 11.1 Basic Version

| Component | Model | Quantity | Price |
|-----------|-------|----------|-------|
| ESP32-S3-DevKitC-1 | Espressif | 1 | 30 yuan |
| 2.9 inch e-paper display | GDEW029T5 | 1 | 50 yuan |
| DHT22 temperature and humidity sensor | AOSONG | 1 | 5 yuan |
| Tactile button | 6×6×5mm | 4 | 0.4 yuan |
| 10KΩ resistor | 1/4W | 4 | 0.2 yuan |
| Soft-pack polymer battery | 3.7V 2000mAh | 1 | 20 yuan |
| Type-C charging module | USB-Type-C interface, 10W charging | 1 | 5 yuan |
| Dupont wire | 20cm | 20 | 2 yuan |
| 3D printed enclosure | 2.9 inch | 1 | 10 yuan (material cost) |
| Screw | M2.5×8 | 4 | 0.4 yuan |
| **Total** | | | 123 yuan |

### 11.2 Advanced Version

| Component | Model | Quantity | Price |
|-----------|-------|----------|-------|
| ESP32-S3-WROOM-2 | Espressif | 1 | 25 yuan |
| 4.2 inch e-paper display | GDEW042T2 | 1 | 80 yuan |
| SHT30 temperature and humidity sensor | Sensirion | 1 | 10 yuan |
| Tactile button | 12×12×7mm | 4 | 1.2 yuan |
| 10KΩ resistor | 1/4W | 4 | 0.2 yuan |
| Soft-pack polymer battery | 3.7V 3000mAh | 1 | 25 yuan |
| AXP192+Type-C power management module | Domestic brand | 1 | 25 yuan |
| BH1750 light sensor | Rohm | 1 | 5 yuan |
| Dupont wire | 20cm | 30 | 3 yuan |
| Plastic enclosure | 4.2 inch | 1 | 30 yuan |
| Screw | M3×10 | 6 | 1.2 yuan |
| **Total** | | | 205.6 yuan |

### 11.3 Premium Version

| Component | Model | Quantity | Price |
|-----------|-------|----------|-------|
| ESP32-S3-Box | Espressif | 1 | 150 yuan |
| 7.5 inch e-paper display | GDEW075T7 | 1 | 120 yuan |
| SHT30 temperature and humidity sensor | Sensirion | 1 | 10 yuan |
| BMP280 pressure sensor | Bosch | 1 | 10 yuan |
| Tactile button | 12×12×7mm | 4 | 1.2 yuan |
| 10KΩ resistor | 1/4W | 4 | 0.2 yuan |
| Soft-pack polymer battery | 3.7V 6000mAh | 1 | 45 yuan |
| AXP192+Type-C power management module | Domestic brand | 1 | 25 yuan |
| BH1750 light sensor | Rohm | 1 | 5 yuan |
| Ribbon cable | 10cm, 10Pin | 2 | 10 yuan |
| Wooden enclosure | 7.5 inch | 1 | 80 yuan |
| Screw | M3×12 | 8 | 1.6 yuan |
| **Total** | | | 459 yuan |

## 12. Procurement Channels

| Channel | Advantage | Disadvantage | Recommendation Index |
|---------|-----------|--------------|---------------------|
| Taobao/Tmall | Wide range of products, low prices, convenient returns and exchanges | Quality varies, need to carefully select | ★★★★☆ |
| JD.com | Reliable quality, fast delivery, good after-sales service | Relatively higher prices | ★★★☆☆ |
| PDD | Lowest prices | High quality risk, poor after-sales service | ★★☆☆☆ |
| LCSC | Complete electronic components, reliable quality, BOM table one-click procurement | Minimum order quantity limit, relatively higher prices | ★★★★☆ |
| Mouser Electronics | Original imports, reliable quality | High prices, long delivery time | ★★☆☆☆ |
| Local electronic market | Can select on-site, immediate purchase | Relatively higher prices, limited variety | ★★★☆☆ |

## 13. Notes

1. **Quality Priority**: Choose well-known brands and reliable channels to ensure component quality
2. **Compatibility**: Ensure compatibility between components, especially interfaces and voltages
3. **Power Consumption Consideration**: Choose low-power components to extend battery life
4. **Size Matching**: Ensure component sizes match the enclosure for easy installation
5. **Expansion Reserved**: Reserve certain expansion space for future function expansion
6. **Cost Control**: Choose appropriate components according to budget, balance between quality and cost
7. **Procurement Quantity**: Purchase more consumables, such as resistors, capacitors, Dupont wires, etc.
8. **Datasheet Consultation**: Carefully consult component datasheets before procurement to ensure they meet requirements
9. **Environmental Consideration**: Choose environmentally friendly materials, avoid using toxic and harmful substances
10. **Safety Consideration**: Choose components that meet safety standards, especially power-related components

## 14. Summary

The hardware material selection for InkClock needs to consider multiple factors such as performance, power consumption, cost, and compatibility. Different configuration schemes can be selected according to different needs and budgets. During the selection process, it is recommended to prioritize well-known brands and reliable channels to ensure component quality and stability. At the same time, attention should be paid to the compatibility between components, and certain expansion space should be reserved for future function expansion.

## 14. Next Steps

1. Update the README.md file to ensure the compatible components list is consistent with the code implementation
2. Update the system secondary development documentation, add sensor expansion interface design
3. Align code implementation and documentation to ensure all sensor types have at least 5 replacement models
4. Complete the task using the finish tool after completion