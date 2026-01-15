# Home Network Intelligent E-Ink Calendar - DIY Instructions

[中文版本](DIY制作指南.md)

## 1. Project Overview

This project is an intelligent e-ink calendar based on ESP32, featuring low power consumption, high definition, and remote configuration capabilities. Through this guide, you can learn how to DIY this device.

## 2. Required Materials

| Material Name | Quantity | Description |
|--------------|----------|-------------|
| ESP32-S3 Development Board | 1 | Main control chip |
| E-Ink Display | 1 | Optional sizes: 1.54/2.13/2.66/2.7/2.9/3.12/4.2/4.37/5.4/5.83/6.0/7.5 inches |
| Temperature and Humidity Sensor | 1 | DHT22 or SHT30 (optional) |
| Soft Polymer Battery | 1 | 3.7V, capacity according to needs (recommended 2000mAh-6000mAh) |
| Type-C Charging Module | 1 | USB-Type-C interface, supporting 10W-20W charging power, with charging protection |
| AXP192+Type-C Power Management Module | 1 | Supporting power monitoring and charging management |
| Buttons | 4 | Tactile buttons |
| Resistors | 4 | 10KΩ, for button pull-up |
| Dupont Wires | Several | For connecting various modules |
| Case | 1 | Can be 3D printed or purchased |
| Screws | Several | For fixing the case |

## 3. Hardware Connection

### 1. E-Ink Display Connection

| E-Ink Display Pin | ESP32-S3 Pin |
|-------------------|--------------|
| VCC | 3.3V |
| GND | GND |
| SCK | GPIO18 |
| MOSI | GPIO23 |
| MISO | GPIO19 |
| CS | GPIO5 |
| DC | GPIO26 |
| RST | GPIO27 |
| BUSY | GPIO32 |

### 2. Temperature and Humidity Sensor Connection

**DHT22 Sensor:**
| DHT22 Pin | ESP32-S3 Pin |
|-----------|--------------|
| VCC | 3.3V |
| GND | GND |
| DATA | GPIO4 |

**SHT30 Sensor:**
| SHT30 Pin | ESP32-S3 Pin |
|-----------|--------------|
| VCC | 3.3V |
| GND | GND |
| SCL | GPIO22 |
| SDA | GPIO21 |

### 3. Button Connection

| Button Pin | ESP32-S3 Pin |
|------------|--------------|
| Button 1 | GPIO0 |
| Button 2 | GPIO1 |
| Button 3 | GPIO2 |
| Button 4 | GPIO3 |

Each button needs to be connected in series with a 10KΩ pull-up resistor.

### 4. Power Module Connection

#### 4.1 Type-C Charging Module Connection

| Type-C Charging Module Pin | ESP32-S3 Pin | Description |
|----------------------------|--------------|-------------|
| VCC | USB-Type-C Input | Connect to USB-Type-C charger |
| GND | GND | Ground |
| B+ | Soft Polymer Battery Positive | Connect to battery positive |
| B- | Soft Polymer Battery Negative | Connect to battery negative |
| OUT+ | VIN | Connect to ESP32-S3 VIN pin |
| OUT- | GND | Connect to ESP32-S3 GND pin |

#### 4.2 AXP192+Type-C Power Management Module Connection

| AXP192 Pin | ESP32-S3 Pin | Description |
|------------|--------------|-------------|
| VIN | USB-Type-C Input | Connect to USB-Type-C charger |
| GND | GND | Ground |
| VOUT2 | 3.3V | Connect to ESP32-S3 3.3V pin |
| SCL | GPIO22 | I2C clock pin |
| SDA | GPIO21 | I2C data pin |
| EXTEN | EN | Reset pin |

**Note: This device only supports USB-Type-C charging interface, not DC direct power supply**

## 4. Software Burning

### 1. Development Environment Setup

1. Download and install Visual Studio Code
2. Install PlatformIO plugin in VS Code
3. Clone project code: `git clone https://github.com/iswhat/InkClock.git`

### 2. Firmware Generation Tool

The system provides a command-line firmware generation tool `generate_firmware.py` for generating the simplest firmware according to actual needs. The tool supports two generation modes:

- **Full firmware mode**: Automatically select all functions and drivers, generate firmware with all functions
- **Custom simplified firmware mode**: Generate the simplest firmware with specific functions according to user selection

**Usage**:

```bash
# Windows
cd tool
generate_firmware.bat

# Linux/Mac
python3 generate_firmware.py
```

### 3. Configuration Modification

Open the `code/src/config.h` file and modify the following configuration items:

```cpp
// WiFi configuration
#define WIFI_SSID "your_wifi_ssid" // WiFi name
#define WIFI_PASSWORD "your_wifi_password" // WiFi password

// Geographic location configuration
#define AUTO_DETECT_LOCATION true // Whether to automatically detect geographic location
#define GEO_CITY_ID "your_city_id" // City ID (used when automatic detection fails)
#define GEO_CITY_NAME "your_city_name" // City name

// Weather API configuration
#define WEATHER_API_KEY "your_weather_api_key" // Weather API key
#define WEATHER_API_KEY_BACKUP "your_weatherapi_key" // Backup weather API key

// Low power configuration
#define LOW_POWER_MODE_ENABLED true // Whether to enable low power mode
#define NO_MOTION_TIMEOUT 30000 // No motion timeout (milliseconds)
#define NIGHT_LIGHT_THRESHOLD 100 // Night light threshold
```

### 4. Compilation and Upload

1. Open the project in PlatformIO
2. Select the corresponding development board model (ESP32-S3 Dev Module)
3. Click the "Upload" button to upload the firmware to the development board

## 5. First Configuration

1. After the device starts, it will create a WiFi hotspot named "InkClock-XXXX"
2. Connect to this hotspot with your phone or computer
3. Visit 192.168.4.1 in the browser
4. Configure WiFi information and basic settings
5. After saving the configuration, the device will automatically connect to the specified WiFi network

## 6. Function Testing

1. **Time Display**: Check if the time is accurate and automatically synchronized
2. **Weather Display**: Check if the current weather and weather forecast are correct
3. **Lunar Display**: Check if the lunar date and solar terms are correct
4. **Sensor Data**: Check if temperature and humidity data are normal
5. **Button Function**: Test if the buttons can respond normally
6. **Web Configuration**: Access the web page through the device IP address and check if the configuration function is normal

## 7. Case Making

### 1. 3D Printed Case

1. Download 3D model files (located in the `hardware/3d_models` directory)
2. Print the case using a 3D printer
3. Polish and assemble the case

### 2. Purchased Case

You can search for "ESP32 e-ink case" on platforms like Taobao and JD.com, and choose the appropriate size and style.

## 8. Notes

1. **Power Safety**: When using lithium batteries, pay attention to preventing overcharging and over-discharging
2. **ESD Protection**: During assembly, pay attention to electrostatic protection to avoid damaging electronic components
3. **Pin Connection**: Ensure correct pin connections to avoid short circuits
4. **Firmware Updates**: Update firmware regularly to get new features and fix bugs
5. **Network Security**: Set strong passwords and change passwords regularly
6. **Power Optimization**: Adjust update frequency according to needs to reduce power consumption

## 9. Common Problem Solving

1. **Device Cannot Start**:
   - Check if the power connection is correct
   - Check if the lithium battery has power
   - Check if the pin connections are correct

2. **Cannot Connect to WiFi**:
   - Check if the WiFi password is correct
   - Check WiFi signal strength
   - Try resetting the device

3. **Display Abnormality**:
   - Check if the e-ink display connection is correct
   - Try restarting the device
   - Check if the firmware version is compatible

4. **Weather Information Not Updating**:
   - Check if the weather API key is correct
   - Ensure the device can access the internet
   - Check if the city ID is correct

## 10. Resource Acquisition

1. Project code: https://github.com/iswhat/InkClock
2. 3D models: `hardware/3d_models` directory
3. Circuit diagram: `elec/电路设计.md`
4. Firmware updates: Through web page or OTA update

## 11. Disclaimer

This project is for learning and research purposes only, please do not use it for commercial purposes. The project author is not responsible for any losses caused by improper operation during the production and use process.