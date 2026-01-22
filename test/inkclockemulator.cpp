#include "inkclockemulator.h"
#include <QDateTime>
#include <QDebug>

InkClockEmulator::InkClockEmulator(QObject *parent)
    : QObject(parent),
      isPoweredOn(false),
      currentMode(DisplayMode::ClockMode),
      hardware(new MockHardware(this)),
      lastUpdateTime(0)
{
    // Initialize display data
    displayData.powerOn = false;
    displayData.time = QDateTime::currentDateTime();
    displayData.temperature = 22;
    displayData.humidity = 50;
    displayData.pressure = 1013;
    displayData.lightLevel = 200;
    displayData.batteryLevel = 80;
    displayData.wifiStatus = WiFiStatus::Connected;
    displayData.brightness = 70;
    displayData.motionDetected = false;
    displayData.updateInterval = 30;
    displayData.mode = DisplayMode::ClockMode;
    
    // Set initial sensor data in hardware mock
    hardware->setTemperature(displayData.temperature);
    hardware->setHumidity(displayData.humidity);
    hardware->setPressure(displayData.pressure);
    hardware->setLightLevel(displayData.lightLevel);
    hardware->setMotionDetected(displayData.motionDetected);
    hardware->setBatteryLevel(displayData.batteryLevel);
    hardware->setWiFiStatus(displayData.wifiStatus);
    
    // Initialize last update time
    lastUpdateTime = QDateTime::currentMSecsSinceEpoch();
    
    emit logMessage("InkClock emulator initialized");
}

InkClockEmulator::~InkClockEmulator()
{
    delete hardware;
}

void InkClockEmulator::powerOn()
{
    if (!isPoweredOn) {
        isPoweredOn = true;
        displayData.powerOn = true;
        emit displayUpdated();
        emit logMessage("Device powered on");
    }
}

void InkClockEmulator::powerOff()
{
    if (isPoweredOn) {
        isPoweredOn = false;
        displayData.powerOn = false;
        emit displayUpdated();
        emit logMessage("Device powered off");
    }
}

void InkClockEmulator::reset()
{
    // Reset display mode
    currentMode = DisplayMode::ClockMode;
    displayData.mode = currentMode;
    
    // Reset brightness and update interval to defaults
    displayData.brightness = 70;
    displayData.updateInterval = 30;
    
    // Reset sensor data to defaults
    displayData.temperature = 22;
    displayData.humidity = 50;
    displayData.pressure = 1013;
    displayData.lightLevel = 200;
    displayData.motionDetected = false;
    
    // Update hardware mock
    hardware->setTemperature(displayData.temperature);
    hardware->setHumidity(displayData.humidity);
    hardware->setPressure(displayData.pressure);
    hardware->setLightLevel(displayData.lightLevel);
    hardware->setMotionDetected(displayData.motionDetected);
    
    emit displayUpdated();
    emit logMessage("Device reset to default settings");
}

void InkClockEmulator::changeMode()
{
    if (!isPoweredOn) {
        return;
    }
    
    // Cycle through display modes
    int currentModeInt = static_cast<int>(currentMode);
    currentModeInt++;
    
    // Check if we've reached the end of modes
    if (currentModeInt > static_cast<int>(DisplayMode::MessageMode)) {
        currentModeInt = static_cast<int>(DisplayMode::ClockMode);
    }
    
    currentMode = static_cast<DisplayMode>(currentModeInt);
    displayData.mode = currentMode;
    
    emit displayUpdated();
    emit logMessage(QString("Display mode changed to %1").arg(currentModeInt));
}

void InkClockEmulator::setTemperature(int temperature)
{
    displayData.temperature = temperature;
    hardware->setTemperature(temperature);
    emit logMessage(QString("Temperature set to %1°C").arg(temperature));
    emit displayUpdated();
}

void InkClockEmulator::setHumidity(int humidity)
{
    displayData.humidity = humidity;
    hardware->setHumidity(humidity);
    emit logMessage(QString("Humidity set to %1%").arg(humidity));
    emit displayUpdated();
}

void InkClockEmulator::setPressure(int pressure)
{
    displayData.pressure = pressure;
    hardware->setPressure(pressure);
    emit logMessage(QString("Pressure set to %1 hPa").arg(pressure));
    emit displayUpdated();
}

void InkClockEmulator::setLightLevel(int lightLevel)
{
    displayData.lightLevel = lightLevel;
    hardware->setLightLevel(lightLevel);
    emit logMessage(QString("Light level set to %1 lux").arg(lightLevel));
    emit displayUpdated();
}

void InkClockEmulator::setMotionDetected(bool detected)
{
    displayData.motionDetected = detected;
    hardware->setMotionDetected(detected);
    emit logMessage(detected ? "Motion detected" : "Motion cleared");
    emit displayUpdated();
}

void InkClockEmulator::setBatteryLevel(int level)
{
    displayData.batteryLevel = qBound(0, level, 100);
    hardware->setBatteryLevel(displayData.batteryLevel);
    emit logMessage(QString("Battery level set to %1%").arg(displayData.batteryLevel));
    emit displayUpdated();
}

void InkClockEmulator::setWiFiStatus(WiFiStatus status)
{
    displayData.wifiStatus = status;
    hardware->setWiFiStatus(status);
    
    QString statusString;
    switch (status) {
        case WiFiStatus::Connected:
            statusString = "Connected";
            break;
        case WiFiStatus::Connecting:
            statusString = "Connecting";
            break;
        case WiFiStatus::Disconnected:
        default:
            statusString = "Disconnected";
            break;
    }
    
    emit logMessage(QString("WiFi status changed to %1").arg(statusString));
    emit displayUpdated();
}

void InkClockEmulator::setBrightness(int brightness)
{
    displayData.brightness = qBound(0, brightness, 100);
    emit logMessage(QString("Brightness set to %1%").arg(displayData.brightness));
    emit displayUpdated();
}

void InkClockEmulator::setUpdateInterval(int interval)
{
    displayData.updateInterval = qBound(1, interval, 3600);
    emit logMessage(QString("Update interval set to %1 seconds").arg(displayData.updateInterval));
}

DisplayData InkClockEmulator::getDisplayData() const
{
    return displayData;
}

void InkClockEmulator::update()
{
    if (!isPoweredOn) {
        return;
    }
    
    // Get current time
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    
    // Calculate time elapsed since last update
    qint64 elapsedTime = currentTime - lastUpdateTime;
    
    // Update device time
    displayData.time = QDateTime::currentDateTime();
    
    // Process sensor data from hardware mock
    processSensorData();
    
    // Simulate battery drain (1% per hour when powered on)
    if (elapsedTime >= 3600000) { // 1 hour in milliseconds
        simulateBatteryDrain();
        lastUpdateTime = currentTime;
    }
    
    // Update display mode based on device behavior
    updateDisplayMode();
    
    // Emit display updated signal
    emit displayUpdated();
}

void InkClockEmulator::updateTime()
{
    displayData.time = QDateTime::currentDateTime();
}

void InkClockEmulator::updateDisplayMode()
{
    // In this simple implementation, we just keep the current mode
    // In a more sophisticated emulator, this would change based on device behavior
    // For example, auto-switching to weather mode when new weather data arrives
}

void InkClockEmulator::processSensorData()
{
    // In this simple implementation, we just use the sensor data set by the user
    // In a more sophisticated emulator, this would process real sensor data from the mock hardware
    // including noise, drift, and other sensor characteristics
}

void InkClockEmulator::simulateBatteryDrain()
{
    if (isPoweredOn) {
        displayData.batteryLevel = qMax(0, displayData.batteryLevel - 1);
        hardware->setBatteryLevel(displayData.batteryLevel);
        
        if (displayData.batteryLevel <= 10) {
            emit logMessage(QString("Low battery warning: %1%").arg(displayData.batteryLevel));
        }
        
        if (displayData.batteryLevel <= 0) {
            emit logMessage("Battery depleted, device shutting down");
            powerOff();
        }
    }
}