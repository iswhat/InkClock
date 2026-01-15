# Home Network Intelligent E-Ink Calendar - Driver Documentation

[中文版本](驱动程序说明.md)

## 1. Driver Overview

Drivers are the bridge between the system and hardware devices, responsible for converting system instructions into signals that hardware devices can understand, while passing hardware device status and data to the system.

The drivers in this project adopt a modular design, mainly including the following types of drivers:

1. **Display Drivers**: Responsible for controlling the display and refresh of the e-ink screen
2. **Sensor Drivers**: Responsible for reading sensor data
3. **Communication Drivers**: Responsible for handling WiFi, Bluetooth and other communication functions
4. **Power Drivers**: Responsible for power management and battery monitoring

## 2. Display Drivers

### 2.1 Display Driver Interface

The display driver interface is defined in the `display_driver.h` file, mainly including the following methods:

```cpp
class IDisplayDriver {
public:
    virtual ~IDisplayDriver() = default;
    
    /**
     * @brief Initialize display driver
     * @return bool Whether initialization is successful
     */
    virtual bool init() = 0;
    
    /**
     * @brief Clear display content
     */
    virtual void clear() = 0;
    
    /**
     * @brief Draw pixel
     * @param x X coordinate
     * @param y Y coordinate
     * @param color Color (true for black, false for white)
     */
    virtual void drawPixel(int x, int y, bool color) = 0;
    
    /**
     * @brief Draw text
     * @param x X coordinate
     * @param y Y coordinate
     * @param text Text to draw
     * @param color Color (true for black, false for white)
     */
    virtual void drawText(int x, int y, const String& text, bool color = true) = 0;
    
    /**
     * @brief Refresh display
     */
    virtual void update() = 0;
    
    /**
     * @brief Partial refresh display
     * @param x X coordinate
     * @param y Y coordinate
     * @param width Width
     * @param height Height
     */
    virtual void updatePartial(int x, int y, int width, int height) = 0;
    
    /**
     * @brief Get display width
     * @return int Display width in pixels
     */
    virtual int getWidth() = 0;
    
    /**
     * @brief Get display height
     * @return int Display height in pixels
     */
    virtual int getHeight() = 0;
};
```

### 2.2 E-Ink Display Driver

The e-ink display driver implements the `IDisplayDriver` interface, responsible for controlling the display and refresh of the e-ink screen. It mainly supports the following e-ink screen sizes:

- 1.54 inch
- 2.13 inch
- 2.9 inch
- 4.2 inch
- 7.5 inch

Main features of the e-ink display driver:

1. **Support for multiple refresh modes**:
   - Full refresh: Refresh the entire screen, suitable for situations where display content changes significantly
   - Partial refresh: Only refresh part of the screen, suitable for situations where display content changes slightly, with lower power consumption

2. **Support for multiple drawing operations**:
   - Draw pixels
   - Draw lines
   - Draw rectangles
   - Draw circles
   - Draw text
   - Draw images

3. **Support for font settings**:
   - Support multiple font sizes
   - Support bold, italic and other font styles
   - Support custom fonts

### 2.3 E-Ink Display Driver Usage Example

```cpp
// Create e-ink display driver instance
IDisplayDriver* einkDriver = new EinkDriver();

// Initialize driver
if (einkDriver->init()) {
    // Clear screen
    einkDriver->clear();
    
    // Draw text
    einkDriver->drawText(10, 10, "Hello, InkClock!", true);
    
    // Draw line
    einkDriver->drawLine(10, 30, einkDriver->getWidth() - 10, 30, true);
    
    // Draw rectangle
    einkDriver->drawRect(10, 40, 50, 30, true);
    
    // Refresh display
    einkDriver->update();
}
```

## 3. Sensor Drivers

### 3.1 Sensor Driver Interface

The sensor driver interface is defined in the `sensor_driver.h` file, mainly including the following methods:

```cpp
class ISensorDriver {
public:
    virtual ~ISensorDriver() = default;
    
    /**
     * @brief Initialize sensor driver
     * @return bool Whether initialization is successful
     */
    virtual bool init() = 0;
    
    /**
     * @brief Read sensor data
     * @param value1 First data value (e.g., temperature)
     * @param value2 Second data value (e.g., humidity)
     * @return bool Whether reading is successful
     */
    virtual bool readData(float& value1, float& value2) = 0;
    
    /**
     * @brief Get sensor type
     * @return String Sensor type
     */
    virtual String getType() = 0;
};
```

### 3.2 Temperature and Humidity Sensor Driver

The temperature and humidity sensor driver implements the `ISensorDriver` interface, supporting the following temperature and humidity sensors:

- DHT22: Single-bus interface, temperature range -40℃~80℃, humidity range 0%~100%RH
- SHT30: I2C interface, temperature range -40℃~125℃, humidity range 0%~100%RH

Main features of the temperature and humidity sensor driver:

1. **Support for multiple sensor types**:
   - Support DHT22, SHT30 and other temperature and humidity sensors
   - Extensible to support other temperature and humidity sensors

2. **Support for data calibration**:
   - Support calibration of temperature and humidity data
   - Calibration can be done through configuration files or Web pages

3. **Support for data filtering**:
   - Support sliding average filtering
   - Configurable filtering window size

### 3.3 Temperature and Humidity Sensor Driver Usage Example

```cpp
// Create DHT22 sensor driver instance
ISensorDriver* dht22Driver = new DHT22Driver(DHT_PIN);

// Initialize driver
if (dht22Driver->init()) {
    float temperature, humidity;
    
    // Read sensor data
    if (dht22Driver->readData(temperature, humidity)) {
        // Print data
        DEBUG_PRINT("Temperature: ");
        DEBUG_PRINTLN(temperature);
        DEBUG_PRINT("Humidity: ");
        DEBUG_PRINTLN(humidity);
    }
}
```

## 4. Communication Drivers

### 4.1 WiFi Driver

The WiFi driver is responsible for handling WiFi connection and communication, with the following main features:

1. **Support for multiple WiFi modes**:
   - Station mode: Connect to existing WiFi network
   - AP mode: Create own WiFi network
   - AP+Station mode: Support both AP and Station modes simultaneously

2. **Support for WiFi scanning**:
   - Support scanning surrounding WiFi networks
   - Support displaying WiFi network signal strength and encryption methods

3. **Support for WiFi connection management**:
   - Support automatic reconnection
   - Support connection status monitoring
   - Support WiFi password encrypted storage

4. **Support for IPv6**:
   - Support IPv4 and IPv6 dual stack
   - Support IPv6 address automatic configuration

### 4.2 Bluetooth Driver

The Bluetooth driver is responsible for handling Bluetooth connection and communication, with the following main features:

1. **Support for Bluetooth Low Energy (BLE)**:
   - Support BLE server mode
   - Support BLE client mode
   - Support BLE broadcasting

2. **Support for multiple Bluetooth services**:
   - Support WiFi configuration service
   - Support sensor data service
   - Support device control service
   - Support OTA update service

3. **Support for Bluetooth connection management**:
   - Support connection status monitoring
   - Support connection encryption
   - Support connection restrictions

## 5. Power Drivers

### 5.1 Power Driver Functions

The power driver is responsible for power management and battery monitoring, with the following main features:

1. **Support for multiple power inputs**:
   - USB power input
   - DC power input
   - Lithium battery input

2. **Support for power conversion**:
   - Support DC-DC conversion
   - Support LDO conversion
   - Support boost and buck

3. **Support for battery management**:
   - Support battery charging management
   - Support battery level monitoring
   - Support battery overcharge, over discharge, short circuit protection

4. **Support for power consumption management**:
   - Support multiple power consumption modes
   - Support sleep and wake-up
   - Support power consumption monitoring

5. **Support for power stability checking**:
   - Implement power stability checking to ensure firmware updates are performed at safe voltage
   - Multiple checks (5 times) to ensure power stability
   - Provide power status feedback for firmware update decisions

### 5.2 Power Driver Usage Example

```cpp
// Create power driver instance
PowerDriver powerDriver;

// Initialize driver
if (powerDriver.init()) {
    // Get power status
    PowerStatus powerStatus = powerDriver.getPowerStatus();
    
    // Print power status
    DEBUG_PRINT("Power Type: ");
    if (powerStatus.powerType == POWER_TYPE_USB) {
        DEBUG_PRINTLN("USB Power");
    } else if (powerStatus.powerType == POWER_TYPE_BATTERY) {
        DEBUG_PRINTLN("Battery Power");
    }
    
    // Print battery level
    DEBUG_PRINT("Battery Level: ");
    DEBUG_PRINT(powerStatus.batteryLevel);
    DEBUG_PRINTLN("%");
    
    // Print charging status
    DEBUG_PRINT("Charging Status: ");
    if (powerStatus.charging) {
        DEBUG_PRINTLN("Charging");
    } else {
        DEBUG_PRINTLN("Not Charging");
    }
}
```

## 6. Firmware Management Driver

### 6.1 Firmware Management Driver Functions

The firmware management driver is responsible for handling various aspects of firmware updates, ensuring the security, reliability, and stability of firmware updates. Main features:

1. **Firmware Integrity Verification**:
   - Implement SHA-256 hash calculation and verification
   - Read expected hash value from firmware_info.json and verify
   - Ensure firmware files are not tampered with during transmission and storage

2. **Firmware Signature Verification Mechanism**:
   - Implement firmware signature verification framework
   - Obtain signatures and public keys from firmware information
   - Support encryption algorithms such as ECDSA/RSA
   - Prevent installation of malicious firmware

3. **Dual Partition Management**:
   - Implement partition management functionality: get current partition, switch partition, backup partition and restore partition
   - Support firmware rollback functionality, automatically detect rollback needs
   - Automatically backup current partition before update, can rollback to old partition if failed

4. **Watchdog Protection**:
   - Initialize watchdog timer (30-second timeout)
   - Regularly reset watchdog in key update steps
   - Prevent device bricking due to update process crashes
   - Disable watchdog after update completion

5. **Key Configuration Management**:
   - Automatically backup key configurations (WiFi configuration, device ID, etc.) before update
   - Check configuration validity during device startup, automatically restore if invalid
   - Ensure firmware updates do not lose important configuration data

6. **Memory Management**:
   - Add memory usage optimization functionality
   - Check available memory before update to ensure sufficient memory for update
   - Prevent update failures due to insufficient memory

7. **Update Status Reporting**:
   - Add detailed error code enumeration
   - Implement status callback mechanism to support real-time monitoring of update progress
   - Set unique error codes for each error situation
   - Provide public method to get last error code

8. **Update Authorization Mechanism**:
   - Add API key verification functionality
   - Support setting and getting API keys
   - Only authorized requests can perform remote OTA updates
   - Support no API key mode (convenient for local testing)

9. **OTA Update Retry Mechanism**:
   - Add retry logic for firmware download (up to 3 times)
   - Add retry logic for firmware update (up to 2 times)
   - Implement timeout handling to prevent infinite waiting
   - Intelligent retry mechanism, only retry in specific error situations
   - Improve OTA update success rate

## 7. Driver Design Principles

1. **Modular Design**:
   - Each driver is an independent module with clear interfaces
   - Drivers are independent of each other, easy to maintain and extend
   - Support hot plugging, can load and unload drivers at runtime

2. **Abstract Interface Design**:
   - Define unified interfaces for easy interaction between system and drivers
   - Support multiple hardware devices, as long as they implement the same interface, can be seamlessly switched
   - Easy to add new drivers, just implement the corresponding interface

3. **Error Handling Mechanism**:
   - Each driver has a complete error handling mechanism
   - Support error code return and error message recording
   - Support automatic recovery mechanism, can automatically try to recover when errors occur

4. **Performance Optimization**:
   - Optimize driver performance, reduce CPU and memory usage
   - Optimize IO operations, reduce latency
   - Support asynchronous operations, improve system concurrent processing capability

5. **Power Optimization**:
   - Optimize driver power consumption, extend battery life
   - Support low power mode, automatically reduce power consumption when not needed
   - Support power consumption monitoring, easy for system to perform power management

## 8. Driver Development Guide

### 8.1 Development Process

1. **Determine Driver Type**:
   - Determine the type of driver to develop (display driver, sensor driver, communication driver, etc.)
   - Determine the hardware devices that the driver should support

2. **Design Driver Interface**:
   - If it is a new type of driver, need to define a new driver interface
   - If it is an existing type of driver, just implement the existing driver interface

3. **Implement Driver**:
   - Implement all methods of the driver interface
   - Add necessary error handling and logging
   - Optimize driver performance and power consumption

4. **Test Driver**:
   - Write test code to test the driver's functionality
   - Test driver performance and power consumption
   - Test driver stability and reliability

5. **Integrate into System**:
   - Integrate the driver into the system
   - Write driver configuration and management code
   - Update system documentation

### 8.2 Development Notes

1. **Follow Coding Standards**:
   - Follow system coding standards
   - Use clear naming rules
   - Add necessary comments

2. **Pay Attention to Error Handling**:
   - Handle all possible error situations
   - Return clear error codes
   - Record detailed error information

3. **Pay Attention to Performance Optimization**:
   - Reduce CPU and memory usage
   - Optimize IO operations
   - Support asynchronous operations

4. **Pay Attention to Power Optimization**:
   - Reduce unnecessary power consumption
   - Support low power mode
   - Optimize power management

5. **Pay Attention to Compatibility**:
   - Support multiple hardware devices
   - Support different hardware versions
   - Support different system versions

## 9. Driver Configuration

Driver configuration is mainly done through the `config.h` file, with the following main configuration items:

1. **Display Driver Configuration**:
   - E-ink screen size
   - E-ink screen refresh mode
   - E-ink screen pin configuration

2. **Sensor Driver Configuration**:
   - Sensor type
   - Sensor pin configuration
   - Sensor sampling frequency
   - Sensor data filtering parameters

3. **Communication Driver Configuration**:
   - WiFi configuration (SSID, password, etc.)
   - Bluetooth configuration (device name, service UUID, etc.)
   - Communication parameters (timeout, retry times, etc.)

4. **Power Driver Configuration**:
   - Power type
   - Charging parameters (charging current, charging voltage, etc.)
   - Battery parameters (battery capacity, voltage range, etc.)
   - Power consumption mode

## 10. Driver Updates

Driver updates can be done through the following methods:

1. **Firmware Updates**:
   - Update firmware through OTA, including driver updates
   - Update firmware through USB

2. **Dynamic Driver Loading**:
   - Support dynamic loading and unloading of drivers at runtime
   - Support loading drivers from external storage
   - Support downloading drivers through network

## 11. Summary

Drivers are an important part of the system, responsible for the interaction between the system and hardware devices. The drivers in this project adopt a modular design, with clear interfaces and complete functions, easy to maintain and extend.

Main advantages of the drivers:

1. **Modular Design**: Easy to maintain and extend
2. **Abstract Interface**: Support multiple hardware devices
3. **Complete Error Handling**: Improve system reliability
4. **Performance Optimization**: Improve system performance
5. **Power Optimization**: Extend battery life
6. **Easy to Develop**: Provide clear development guide
7. **Easy to Configure**: Support multiple configuration methods
8. **Easy to Update**: Support multiple update methods

Through reasonable design and use of drivers, system performance, reliability and scalability can be improved, providing users with a better experience.