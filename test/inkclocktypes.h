#ifndef INKCLOCKTYPES_H
#define INKCLOCKTYPES_H

#include <QDateTime>

// WiFi status enumeration
enum class WiFiStatus {
    Disconnected = 0,
    Connecting = 1,
    Connected = 2
};

// Display mode enumeration
enum class DisplayMode {
    ClockMode = 0,
    WeatherMode = 1,
    SensorMode = 2,
    LunarMode = 3,
    MessageMode = 4
};

// Display data structure
struct DisplayData {
    bool powerOn;              // Power status
    QDateTime time;            // Current time
    int temperature;           // Temperature in °C
    int humidity;              // Humidity in %
    int pressure;              // Pressure in hPa
    int lightLevel;            // Light level in lux
    int batteryLevel;          // Battery level in %
    WiFiStatus wifiStatus;     // WiFi connection status
    int brightness;            // Display brightness (0-100)
    bool motionDetected;       // Motion detection status
    int updateInterval;        // Display update interval in seconds
    DisplayMode mode;          // Current display mode
};

// Sensor data structure
struct SensorData {
    int temperature;           // Temperature in °C
    int humidity;              // Humidity in %
    int pressure;              // Pressure in hPa
    int lightLevel;            // Light level in lux
    bool motionDetected;       // Motion detection status
};

// Network data structure
struct NetworkData {
    WiFiStatus status;         // WiFi connection status
    QString ssid;              // Connected SSID
    int signalStrength;        // Signal strength (0-100)
    bool internetAccess;       // Internet access status
};

// Battery data structure
struct BatteryData {
    int level;                 // Battery level in %
    int voltage;               // Battery voltage in mV
    bool charging;             // Charging status
};

// Display settings structure
struct DisplaySettings {
    int brightness;            // Display brightness (0-100)
    int updateInterval;        // Display update interval in seconds
    DisplayMode defaultMode;   // Default display mode
    bool autoBrightness;       // Auto brightness adjustment
};

#endif // INKCLOCKTYPES_H