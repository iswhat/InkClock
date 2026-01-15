# Home Network Intelligent E-Ink Calendar - System Secondary Development Guide

[中文版本](系统二次开发说明.md)

## 1. System Architecture

### 1. Overall Architecture

The system adopts a modular design based on the ESP32 platform (supporting ESP32-S3, ESP32-C3, ESP32-C6, ESP32-S2 series, etc.), mainly including the following layers:

| Layer | Main Function | Module Examples |
|-------|--------------|----------------|
| Hardware Driver Layer | Directly control hardware devices | EinkDriver, SensorDriver |
| Driver Management Layer | Manage and schedule hardware drivers | DriverRegistry |
| Core Service Layer | Provide basic services and management | WiFiManager, TimeManager, APIManager |
| Functional Module Layer | Implement specific business functions | WeatherManager, LunarManager, SensorManager |
| Application Layer | Handle user interaction and display | DisplayManager, WebServerManager |
| Extension Layer | Support plugin extensions | PluginManager |

### 2. Driver Registry Mechanism

The system introduces a Driver Registry mechanism for unified management and scheduling of all hardware drivers. This mechanism is implemented using the singleton pattern, supporting automatic detection and dynamic loading of drivers, which improves system scalability and compatibility.

**Main functions of the Driver Registry:**
- Unified management of all hardware drivers
- Support for automatic hardware device detection
- Support for dynamic registration and loading of drivers
- Provide driver lookup and acquisition interfaces
- Support multiple communication protocols (I2C, SPI, single-bus, etc.)
- Support hardware support verification before firmware writing to ensure firmware matches hardware

**Design features of the Driver Registry:**
- Adopt singleton pattern to ensure global unique instance
- Support template metaprogramming to simplify driver registration
- Support multiple sensor types and display drivers
- Provide automatic detection mechanism without manual configuration
- Support driver priority and fault tolerance mechanism
- Support automatic hardware driver adaptation to avoid hardware damage

**Usage flow of the Driver Registry:**
1. Driver registration: Register all available drivers through template functions during system startup
2. Automatic detection: After system startup, automatically detect connected hardware devices
3. Driver initialization: Initialize appropriate drivers based on detection results
4. Driver usage: Obtain driver instances through the driver registry and call corresponding methods
5. Driver management: Support dynamic addition, deletion and updating of drivers
6. Hardware verification: Check if the firmware supports current hardware before firmware update

### 3. Module Relationships

```
┌─────────────────────────────────────────────────────────┐
│                     Application Layer                   │
├─────────────────────┬─────────────────────────────────────┤
│  DisplayManager     │  WebServerManager                  │
└─────────────────────┴─────────────────────────────────────┘
                        │
┌─────────────────────┬─────────────────────────────────────┐
│   Functional Module Layer                                │
├────────┬────────────┼─────────┬───────────┬───────────────┤
│Weather │  Lunar     │ Sensor  │  Message  │  Stock        │
│Manager │  Manager   │ Manager │  Manager  │  Manager      │
└────────┴────────────┴─────────┴───────────┴───────────────┘
                        │
┌─────────────────────┬─────────────────────────────────────┐
│   Core Service Layer                                     │
├────────┬────────────┼─────────┬───────────┬───────────────┤
│ WiFi   │   Time     │  API    │  Geo      │  Power        │
│Manager │  Manager   │ Manager │  Manager  │  Manager      │
└────────┴────────────┴─────────┴───────────┴───────────────┘
                        │
┌─────────────────────┬─────────────────────────────────────┐
│   Driver Management Layer                                │
├─────────────────────┤                                     │
│  DriverRegistry     │                                     │
└─────────────────────┘                                     │
                        │
┌─────────────────────┬─────────────────────────────────────┐
│   Hardware Driver Layer                                  │
├────────┬────────────┼─────────┬───────────┬───────────────┤
│ Eink   │  Sensor    │  Audio  │  Button   │  Camera       │
│Driver  │  Driver    │  Driver │  Driver   │  Driver       │
└────────┴────────────┴─────────┴───────────┴───────────────┘
                        │
┌─────────────────────┬─────────────────────────────────────┐
│   Hardware Layer                                          │
├────────┬────────────┼─────────┬───────────┬───────────────┤
│ Ink    │  ESP32     │  Sensor │  Button   │  Camera       │
│Screen  │  S3        │         │           │               │
└────────┴────────────┴─────────┴───────────┴───────────────┘
```

## 2. Core Module Design

### 1. WiFiManager

**Function**: Manage WiFi connections, including WiFi scanning, connection, reconnection and other functions.

**Main methods**:
- `init()`: Initialize WiFi module
- `connect()`: Connect to specified WiFi network
- `isConnected()`: Check if WiFi is connected
- `getIPAddress()`: Get device IP address
- `loop()`: WiFi status monitoring and reconnection

### 2. TimeManager

**Function**: Manage time synchronization, support NTP time synchronization and backup servers.

**Main methods**:
- `init()`: Initialize time management module
- `update()`: Update time
- `getCurrentTime()`: Get current time
- `getCurrentDate()`: Get current date
- `loop()`: Time synchronization monitoring

### 3. APIManager

**Function**: Unified management of API requests, support caching, retry, backup API and other functions.

**Main methods**:
- `init()`: Initialize API manager
- `get()`: Send GET request
- `post()`: Send POST request
- `cleanupCache()`: Clean up cache

### 4. GeoManager

**Function**: Manage geographic location information, support automatic detection and manual configuration.

**Main methods**:
- `init()`: Initialize geographic location manager
- `update()`: Update geographic location information
- `getLocation()`: Get current geographic location
- `setLocation()`: Set geographic location
- `setAutoDetect()`: Set auto-detection switch
- `loop()`: Regularly update geographic location

### 5. SceneManager

**Function**: Manage scene modes, support scene switching and configuration.

**Main methods**:
- `init()`: Initialize scene manager
- `applySceneConfig()`: Apply scene configuration
- `switchScene()`: Switch scene
- `recordUserActivity()`: Record user activity
- `saveScenes()`: Save scene configuration
- `loadScenes()`: Load scene configuration
- `loop()`: Scene status monitoring

### 6. StorageManager

**Function**: Manage storage resources, support data backup and compression.

**Main methods**:
- `init()`: Initialize storage manager
- `backupData()`: Backup data to other storage media
- `compressData()`: Compress data
- `decompressData()`: Decompress data
- `getStorageInfo()`: Get storage information

### 7. HardwareDetector

**Function**: Detect hardware resource status.

**Main methods**:
- `init()`: Initialize hardware detector
- `detectCpu()`: Detect CPU status
- `detectStorage()`: Detect storage status
- `detectNetwork()`: Detect network status
- `detectPower()`: Detect power status
- `getHardwareInfo()`: Get hardware information

### 8. PerformanceMonitor

**Function**: Monitor system performance, publish performance data and alerts.

**Main methods**:
- `init()`: Initialize performance monitor
- `collectPerformanceData()`: Collect performance data
- `publishPerformanceDataEvent()`: Publish performance data event
- `publishAlertEvent()`: Publish alert event
- `loop()`: Performance monitoring loop

## 3. Functional Module Design

### 1. WeatherManager

**Function**: Obtain and manage weather data, support main API and backup API.

**Main methods**:
- `init()`: Initialize weather manager
- `update()`: Update weather data
- `getCurrentWeather()`: Get current weather
- `getForecastData()`: Get weather forecast
- `loop()`: Regularly update weather data

### 2. LunarManager

**Function**: Manage lunar calendar and holiday information.

**Main methods**:
- `init()`: Initialize lunar manager
- `update()`: Update lunar data
- `getLunarDate()`: Get lunar date
- `getSolarTerm()`: Get solar term
- `getHoliday()`: Get holiday
- `loop()`: Regularly update lunar data

### 3. DisplayManager

**Function**: Manage display content and refresh strategies.

**Main methods**:
- `init()`: Initialize display manager
- `setDisplayDriver()`: Set display driver
- `updateDisplay()`: Update entire display
- `updateDisplayPartial()`: Partial update display
- `showMessage()`: Show message
- `showSplashScreen()`: Show splash screen

### 4. WebServerManager

**Function**: Provide Web configuration page and API interfaces.

**Main methods**:
- `init()`: Initialize Web server
- `loop()`: Handle Web requests
- `handleRoot()`: Handle root path request
- `handleSettings()`: Handle settings page request
- `handleUpdateSettings()`: Handle settings update request

## 4. Secondary Development Guide

### 1. Environment Setup

1. Install Visual Studio Code
2. Install PlatformIO plugin
3. Clone project code
4. Install dependency libraries

### 2. Module Extension

#### 2.1 Adding New Plugins

1. Define plugin type in `plugin_manager.h`:
   ```cpp
enum PluginType {
    PLUGIN_TYPE_URL_JSON,
    PLUGIN_TYPE_URL_XML,
    PLUGIN_TYPE_URL_JS,
    PLUGIN_TYPE_CUSTOM // Add custom plugin type
};
   ```

2. Implement plugin class, inheriting from `Plugin` base class:
   ```cpp
class CustomPlugin : public Plugin {
public:
    CustomPlugin() : Plugin("CustomPlugin", "1.0", "Custom Plugin") {}
    
    bool init() override {
        // Initialize plugin
        return true;
    }
    
    bool update() override {
        // Update plugin data
        return true;
    }
    
    String getDisplayContent() override {
        // Return display content
        return "Custom Plugin Content";
    }
};
   ```

3. Register plugin in `plugin_manager.cpp`:
   ```cpp
void PluginManager::init() {
    // Register built-in plugins
    registerPlugin(new CustomPlugin());
}
   ```

#### 2.2 Adding New Sensors

1. Add new sensor type to `SensorType` enum in `sensor_driver.h`:
   ```cpp
enum SensorType {
    // Existing sensor types...
    SENSOR_TYPE_CUSTOM // Add custom sensor type
};
   ```

2. Implement sensor driver class, inheriting from `ISensorDriver` interface:
   ```cpp
class CustomSensorDriver : public ISensorDriver {
private:
    SensorConfig _config;
    
public:
    CustomSensorDriver() {}
    
    bool init(const SensorConfig& config) override {
        _config = config;
        // Initialize sensor, use config.pin or config.address
        return true;
    }
    
    bool readData(SensorData& data) override {
        // Read sensor data and fill into data structure
        data.valid = true;
        data.timestamp = millis();
        data.temperature = 25.0; // Example data
        data.humidity = 50.0;    // Example data
        return true;
    }
    
    void calibrate(float tempOffset, float humOffset) override {
        // Calibrate sensor
        _config.tempOffset = tempOffset;
        _config.humOffset = humOffset;
    }
    
    String getTypeName() const override {
        return "CustomSensor";
    }
    
    SensorType getType() const override {
        return SENSOR_TYPE_CUSTOM;
    }
    
    void setConfig(const SensorConfig& config) override {
        _config = config;
    }
    
    SensorConfig getConfig() const override {
        return _config;
    }
};
   ```

3. Register sensor driver to driver registry in `main.cpp`:
   ```cpp
void setup() {
    // Existing initialization code...
    
    // Register custom sensor driver
    registerSensorDriver<CustomSensorDriver>();
    
    // Existing code...
}
   ```

4. Add default configuration to `commonConfigs` array in `driver_registry.h` (optional):
   ```cpp
SensorDefaultConfig commonConfigs[] = {
    // Existing configurations...
    {SENSOR_TYPE_CUSTOM, CUSTOM_PIN, 0x40, {0x00, 0x00, 0x00}} // Custom sensor default configuration
};
   ```

#### 2.3 Adding New Display Drivers

1. Define display driver interface in `display_driver.h`:
   ```cpp
   class IDisplayDriver {
   public:
       virtual ~IDisplayDriver() = default;
       virtual bool init() = 0;
       virtual void clear() = 0;
       virtual void drawPixel(int x, int y, bool color) = 0;
       virtual void drawText(int x, int y, const String& text, bool color = true) = 0;
       virtual void update() = 0;
       virtual void updatePartial(int x, int y, int width, int height) = 0;
       virtual int getWidth() = 0;
       virtual int getHeight() = 0;
   };
   ```

2. Implement display driver class:
   ```cpp
   class CustomDisplayDriver : public IDisplayDriver {
   public:
       bool init() override {
           // Initialize display driver
           return true;
       }
       
       void clear() override {
           // Clear display
       }
       
       void drawPixel(int x, int y, bool color) override {
           // Draw pixel
       }
       
       void drawText(int x, int y, const String& text, bool color) override {
           // Draw text
       }
       
       void update() override {
           // Update display
       }
       
       void updatePartial(int x, int y, int width, int height) override {
           // Partial update display
       }
       
       int getWidth() override {
           return 200;
       }
       
       int getHeight() override {
           return 200;
       }
   };
   ```

3. Use new display driver in `display_manager.cpp`:
   ```cpp
   void DisplayManager::init() {
       // Use custom display driver
       IDisplayDriver* customDriver = new CustomDisplayDriver();
       setDisplayDriver(customDriver);
   }
   ```

### 2.4 Sensor Extension Interface Design

To facilitate the extension of more sensors, the system has designed standard sensor extension interfaces:

- **I2C Interface**: Provide standard I2C interface (SCL, SDA, VCC, GND)
- **SPI Interface**: Provide standard SPI interface (SCK, MOSI, MISO, CS, VCC, GND)
- **Analog Interface**: Provide multiple analog inputs (AO, VCC, GND)
- **Digital Interface**: Provide multiple digital input/outputs (IO, VCC, GND)

**Extension Interface Pin Assignment:**
| Extension Interface Pin | ESP32-S3 Pin |
|-------------------------|--------------|
| I2C_SCL                 | GPIO22       |
| I2C_SDA                 | GPIO21       |
| SPI_SCK                 | GPIO18       |
| SPI_MOSI                | GPIO23       |
| SPI_MISO                | GPIO19       |
| SPI_CS                  | GPIO5        |
| AO1                     | GPIO34       |
| AO2                     | GPIO35       |
| DO1                     | GPIO6        |
| DO2                     | GPIO7        |
| VCC_3V3                 | 3.3V         |
| VCC_5V                  | 5V           |
| GND                     | GND          |

### 2.5 Supported Sensor Types

The system supports the following sensor types, each with no less than 5 replacement models:

- **Temperature and Humidity Sensors**: DHT11, DHT22, DHT12, SHT30, SHT21, SHT40, AM2302, HDC1080, BME280, BME680, HTU21D, SI7021
- **Human Presence Sensors**: HC-SR501, HC-SR505, RE200B, LD2410, BH1750
- **Light Sensors**: BH1750, VEML6075, TSL2561, GY30, SI1145
- **Gas Sensors**: MQ2, MQ5, MQ7, MQ8, MQ135, TGS2600
- **Flame Sensors**: IR flame sensor, UV flame sensor, YG1006, MQ2, TGS2600
- **Barometric Pressure Sensors**: BMP280, BMP388, LPS25HB
- **Soil Moisture Sensors**: FC-28, Capacitive soil moisture sensor
- **Water Level Sensors**: Water level sensor module, Float level switch
- **Vibration Sensors**: SW-420, Vibration sensor module
- **Sound Sensors**: Sound sensor module, MAX9814 microphone
- **Acceleration Sensors**: MPU6050, LSM6DS3
- **Distance Sensors**: HC-SR04, VL53L0X, TF-Luna
- **Heart Rate Sensors**: MAX30102, pulseSensor
- **UV Sensors**: GUVA-S12SD, VEML6075
- **Magnetic Field Sensors**: HMC5883L, QMC5883L
- **Gas Flow Sensors**: YF-S201, G1/2" gas flow sensor
- **Raindrop Sensors**: Raindrop sensor module, Leaf humidity sensor
- **Carbon Monoxide Sensors**: MQ-7, TGS5042
- **Formaldehyde Sensors**: MQ-138, ZE08-CH2O
- **Oxygen Sensors**: MQ-131, ME2-O2
- **Smoke Sensors**: MQ-2, GP2Y1010AU0F, MQ-135
- **Alcohol Sensors**: MQ-3, MQ-8
- **Radiation Sensors**: GM Tube Module, AS7341
- **Attitude Sensors**: MPU9250, BNO055
- **Current Sensors**: ACS712-05B, ACS758LCB-050B
- **Voltage Sensors**: Voltage sensor module, INA219
- **Power Sensors**: INA219, INA3221
- **Weight Sensors**: HX711+Load Cell, Pressure sensor
- **Touch Sensors**: TTP223, TTP229, CAP1188
- **Color Sensors**: TCS3200, TCS34725, AS7341
- **Laser Sensors**: Laser diode module, Laser ranging module
- **Infrared Receivers**: TSOP38238, VS1838B
- **Infrared Transmitters**: Infrared LED, Infrared emission module
- **Camera Modules**: OV7670, OV2640, GC0308, GC2145
- **Microphone Modules**: MAX9814, WM8960, INMP441
- **Speaker Modules**: Buzzer module, Speaker module, MAX98357A
- **LED Modules**: LED module, RGB LED module, WS2812B
- **Relay Modules**: 1-channel relay module, 2-channel relay module, 4-channel relay module
- **Motor Driver Modules**: L298N, DRV8833, TB6612FNG
- **Stepper Motor Driver Modules**: ULN2003, A4988, DRV8825
- **Servo Modules**: SG90, MG996R, DS3218
- **Encoder Modules**: Rotary encoder module, Photoelectric encoder

### 3. API Development

#### 3.1 Adding New API Interfaces

1. Add new route handling in `web_server.cpp`:
   ```cpp
void WebServerManager::init() {
    // Add new API route
    server.on("/api/custom", HTTP_GET, std::bind(&WebServerManager::handleCustomApi, this));
}
   ```

2. Implement API handling function:
   ```cpp
void WebServerManager::handleCustomApi() {
    // Handle custom API request
    DynamicJsonDocument doc(1024);
    doc["status"] = "success";
    doc["message"] = "Custom API response";
    
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    
    server.send(200, "application/json", jsonResponse);
}
   ```

3. Declare new handling function in `web_server.h`:
   ```cpp
class WebServerManager {
private:
    // Existing handling functions
    void handleCustomApi();
};
   ```

### 4. Configuration Extension

1. Add new configuration items in `config.h`:
   ```cpp
// Custom configuration
#define CUSTOM_CONFIG_ITEM 1 // Custom configuration item
   ```

2. Add new configuration items in Web configuration page:
   ```html
<div class="form-group">
    <label for="custom_config">Custom Configuration:</label>
    <input type="checkbox" id="custom_config" name="custom_config" %CUSTOM_CONFIG%>
    <small>Custom configuration item description</small>
</div>
   ```

3. Handle new configuration items in `web_server.cpp`:
   ```cpp
void WebServerManager::handleSettings() {
    // Replace custom configuration variable
    String customConfigChecked = CUSTOM_CONFIG_ITEM ? "checked" : "";
    html.replace("%CUSTOM_CONFIG%", customConfigChecked);
}

void WebServerManager::handleUpdateSettings() {
    // Handle custom configuration
    bool customConfig = server.hasArg("custom_config");
    // Save configuration
}
   ```

## 5. Development Specifications

### 1. Code Style

- Adopt C++11 and above standards
- Class names use PascalCase (e.g., DisplayManager)
- Function names use camelCase (e.g., updateDisplay)
- Variable names use camelCase (e.g., currentWeather)
- Constant names use ALL_CAPS with underscores (e.g., WEATHER_API_KEY)
- Indentation uses 4 spaces
- Code line length does not exceed 120 characters

### 2. Error Handling

- Use try-catch blocks to catch exceptions
- Each module's loop function should be wrapped in try-catch
- Error messages should include module name and specific error
- Key operations should have error checking and logging

### 3. Logging

- Use DEBUG_PRINT and DEBUG_PRINTLN macros for logging
- Logs should include timestamps and module names
- Log levels are divided into: debug, info, warning, error
- Debug logs should be turned off in production environment

### 4. Memory Management

- Avoid memory leaks, timely release dynamically allocated memory
- Use smart pointers to manage dynamic objects
- Avoid using too many global variables
- Set cache size reasonably to avoid memory overflow

### 5. Power Optimization

- Reduce unnecessary refreshes and updates
- Use partial refresh instead of full screen refresh
- Set update frequency reasonably
- Turn off unnecessary modules and functions

## 6. Debugging and Testing

### 1. Debugging Methods

- Use serial debugging output
- Use Web page to view device status
- Use ESP32 built-in debugging tools
- Use remote logging

### 2. Testing Strategies

- Unit testing: Test individual functions or classes
- Integration testing: Test interactions between modules
- System testing: Test the functionality of the entire system
- Performance testing: Test system performance and power consumption
- Stability testing: Test system stability under long-term operation

## 7. Firmware Updates

### 1. Firmware Generation Tools

The system provides a command-line firmware generation tool `generate_firmware.py` for generating the simplest firmware according to actual needs. The tool supports two generation modes:

- **Full firmware mode**: Automatically select all functions and drivers, generate firmware with all functions
- **Custom simplified firmware mode**: Generate the simplest firmware with specific functions according to user selection

**Main functions**:
- Automatically detect running environment
- Support multiple hardware model selection
- Support functional module customization
- Automatically generate conditional compilation macros
- Hardware support verification before firmware writing

**Usage**:
```bash
# Windows
cd tool
generate_firmware.bat

# Linux/Mac
python3 generate_firmware.py
```

### 2. OTA Updates

The system supports OTA (Over-The-Air) firmware updates, which can be performed through the following methods:

1. Web page update: Upload new firmware files in the Web configuration page
2. API update: Upload firmware through API interface
3. Cloud update: Push firmware updates through cloud server

### 3. Firmware Update Process

1. Check firmware version
2. Download new firmware
3. **Firmware integrity verification**: Use SHA-256 hash value to verify firmware integrity
4. **Firmware signature verification**: Verify firmware signature to prevent malicious firmware
5. **Power stability check**: Ensure updates are performed at safe voltage
6. **Memory availability check**: Ensure sufficient memory for updates
7. **Hardware support verification**: Check if firmware supports current hardware
8. **Initialize watchdog**: Start 30-second timeout watchdog timer
9. **Backup key configurations**: Automatically backup WiFi configuration, device ID and other key data
10. **Backup current partition**: Create a backup of the current firmware partition
11. Install new firmware to backup partition
12. Switch to new partition
13. Restart device
14. Verify update results
15. Automatically detect and handle update failure situations, roll back to old partition if necessary

### 4. Factory Reset

The system supports two factory reset methods:

- **Soft reset**: Triggered through API or Web page
- **Hardware hard reset**: Triggered by long-pressing the button for more than 5 seconds

**Reset process**:
1. Clear all configuration data
2. Reset all module states
3. Restart device
4. Enter initial configuration mode

## 8. Power Management

### 1. Power Management Strategy

The system adopts a multi-level power management strategy to ensure the device maintains low power consumption in various scenarios:

**Core System Layer**:
- Dynamic CPU frequency adjustment (80MHz-240MHz)
- Multiple low power modes (light sleep, deep sleep)
- Intelligent power management

**Application Service Layer**:
- Low power strategy based on human presence detection
- Night energy saving mode based on light sensing
- Adaptive display refresh interval

**Hardware Driver Layer**:
- Dynamic adjustment of sensor sampling frequency
- Alarm-related sensors maintain normal sampling in low power mode
- Reduce sampling frequency for non-critical sensors

### 2. Low Power Mode

The system supports automatic entry into low power mode, triggered by conditions including:
- Night mode (insufficient light)
- Long time no human activity
- Low battery level

**Optimizations in low power mode**:
- Reduce CPU frequency
- Turn off WiFi and Bluetooth
- Turn off unnecessary peripherals
- Reduce non-critical sensor sampling frequency
- Extend display refresh interval

**Recovery conditions**:
- Detect human activity
- Light returns to normal
- Battery level sufficient

## 9. Resource Acquisition

1. Project code: https://github.com/iswhat/InkClock
2. Development documentation: This file and other documents
3. API documentation: Access `/api/docs` through Web page
4. Example code: `examples` directory
5. Technical support: https://github.com/iswhat/InkClock/issues

## 10. Version Management

1. Version number format: Major version.Minor version.Revision number (e.g., 1.0.0)
2. Major version number: Major function changes or architecture adjustments
3. Minor version number: New features or modules
4. Revision number: Fix bugs or minor improvements
5. Version history record: Recorded in README.md file

### Version History

| Version | Date       | Description |
|---------|------------|-------------|
| v1.5.0  | 2026-01-13 | Scene management system optimization, storage management system enhancement, hardware detector optimization, performance monitoring system enhancement |
| v1.4.0  | 2025-12-31 | Firmware management system optimization, enhanced Bluetooth network configuration and hotspot WiFi configuration, remote control and data synchronization functionality enhancement |
| v1.3.0  | 2025-12-31 | Cross-platform compatibility optimization, low power optimization enhancement, remote control and data synchronization functionality |
| v1.2.0  | 2025-12-29 | Implemented three-layer architecture design, improved event bus and driver registry |
| v1.1.0  | 2025-12-26 | Added geographic location automatic identification function, optimized API management module |
| v1.0.0  | 2025-12-01 | Initial version |

## 11. Notes

1. Please carefully read existing code and documentation before development
2. Follow development specifications to maintain consistent code style
3. Test fully to ensure new features do not break existing features
4. Update documentation in time to keep documentation consistent with code
5. Please conduct code review before submitting code
6. Back up code regularly to avoid data loss