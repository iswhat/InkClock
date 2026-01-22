#include "mockhardware.h"

MockHardware::MockHardware(QObject *parent)
    : QObject(parent),
      temperature(22),
      humidity(50),
      pressure(1013),
      lightLevel(200),
      motionDetected(false),
      batteryLevel(80),
      batteryVoltage(3800),
      charging(false),
      wifiStatus(WiFiStatus::Connected),
      ssid("InkClockTest"),
      signalStrength(80),
      internetAccess(true),
      displayBrightness(70),
      updateInterval(30),
      button1Pressed(false),
      button2Pressed(false),
      led1On(false),
      led2On(false),
      speakerEnabled(false)
{
}

MockHardware::~MockHardware()
{
}

// Sensor methods
int MockHardware::getTemperature() const
{
    return temperature;
}

void MockHardware::setTemperature(int temperature)
{
    this->temperature = temperature;
}

int MockHardware::getHumidity() const
{
    return humidity;
}

void MockHardware::setHumidity(int humidity)
{
    this->humidity = humidity;
}

int MockHardware::getPressure() const
{
    return pressure;
}

void MockHardware::setPressure(int pressure)
{
    this->pressure = pressure;
}

int MockHardware::getLightLevel() const
{
    return lightLevel;
}

void MockHardware::setLightLevel(int lightLevel)
{
    this->lightLevel = lightLevel;
}

bool MockHardware::isMotionDetected() const
{
    return motionDetected;
}

void MockHardware::setMotionDetected(bool detected)
{
    this->motionDetected = detected;
}

// Battery methods
int MockHardware::getBatteryLevel() const
{
    return batteryLevel;
}

void MockHardware::setBatteryLevel(int level)
{
    this->batteryLevel = level;
    
    // Calculate battery voltage based on level (simplified)
    // Assuming 3.2V (0%) to 4.2V (100%)
    this->batteryVoltage = 3200 + (level * 10);
}

int MockHardware::getBatteryVoltage() const
{
    return batteryVoltage;
}

bool MockHardware::isCharging() const
{
    return charging;
}

void MockHardware::setCharging(bool charging)
{
    this->charging = charging;
}

// Network methods
WiFiStatus MockHardware::getWiFiStatus() const
{
    return wifiStatus;
}

void MockHardware::setWiFiStatus(WiFiStatus status)
{
    this->wifiStatus = status;
    
    // Update internet access based on WiFi status
    if (status == WiFiStatus::Connected) {
        internetAccess = true;
    } else {
        internetAccess = false;
    }
}

QString MockHardware::getSSID() const
{
    return ssid;
}

void MockHardware::setSSID(const QString &ssid)
{
    this->ssid = ssid;
}

int MockHardware::getSignalStrength() const
{
    return signalStrength;
}

void MockHardware::setSignalStrength(int strength)
{
    this->signalStrength = qBound(0, strength, 100);
}

bool MockHardware::hasInternetAccess() const
{
    return internetAccess;
}

void MockHardware::setInternetAccess(bool access)
{
    this->internetAccess = access;
}

// Display methods
int MockHardware::getDisplayBrightness() const
{
    return displayBrightness;
}

void MockHardware::setDisplayBrightness(int brightness)
{
    this->displayBrightness = qBound(0, brightness, 100);
}

int MockHardware::getUpdateInterval() const
{
    return updateInterval;
}

void MockHardware::setUpdateInterval(int interval)
{
    this->updateInterval = qBound(1, interval, 3600);
}

// Button methods
bool MockHardware::isButtonPressed(int buttonId) const
{
    switch (buttonId) {
        case 1:
            return button1Pressed;
        case 2:
            return button2Pressed;
        default:
            return false;
    }
}

void MockHardware::setButtonPressed(int buttonId, bool pressed)
{
    switch (buttonId) {
        case 1:
            button1Pressed = pressed;
            break;
        case 2:
            button2Pressed = pressed;
            break;
        default:
            break;
    }
}

// LED methods
bool MockHardware::isLedOn(int ledId) const
{
    switch (ledId) {
        case 1:
            return led1On;
        case 2:
            return led2On;
        default:
            return false;
    }
}

void MockHardware::setLedOn(int ledId, bool on)
{
    switch (ledId) {
        case 1:
            led1On = on;
            break;
        case 2:
            led2On = on;
            break;
        default:
            break;
    }
}

// Speaker methods
bool MockHardware::isSpeakerEnabled() const
{
    return speakerEnabled;
}

void MockHardware::setSpeakerEnabled(bool enabled)
{
    this->speakerEnabled = enabled;
}