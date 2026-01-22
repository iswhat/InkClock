#ifndef MOCKHARDWARE_H
#define MOCKHARDWARE_H

#include <QObject>
#include "inkclocktypes.h"

class MockHardware : public QObject
{
    Q_OBJECT

public:
    explicit MockHardware(QObject *parent = nullptr);
    ~MockHardware();
    
    // Sensor methods
    int getTemperature() const;
    void setTemperature(int temperature);
    
    int getHumidity() const;
    void setHumidity(int humidity);
    
    int getPressure() const;
    void setPressure(int pressure);
    
    int getLightLevel() const;
    void setLightLevel(int lightLevel);
    
    bool isMotionDetected() const;
    void setMotionDetected(bool detected);
    
    // Battery methods
    int getBatteryLevel() const;
    void setBatteryLevel(int level);
    
    int getBatteryVoltage() const;
    bool isCharging() const;
    void setCharging(bool charging);
    
    // Network methods
    WiFiStatus getWiFiStatus() const;
    void setWiFiStatus(WiFiStatus status);
    
    QString getSSID() const;
    void setSSID(const QString &ssid);
    
    int getSignalStrength() const;
    void setSignalStrength(int strength);
    
    bool hasInternetAccess() const;
    void setInternetAccess(bool access);
    
    // Display methods
    int getDisplayBrightness() const;
    void setDisplayBrightness(int brightness);
    
    int getUpdateInterval() const;
    void setUpdateInterval(int interval);
    
    // Button methods
    bool isButtonPressed(int buttonId) const;
    void setButtonPressed(int buttonId, bool pressed);
    
    // LED methods
    bool isLedOn(int ledId) const;
    void setLedOn(int ledId, bool on);
    
    // Speaker methods
    bool isSpeakerEnabled() const;
    void setSpeakerEnabled(bool enabled);
    
private:
    // Sensor data
    int temperature;
    int humidity;
    int pressure;
    int lightLevel;
    bool motionDetected;
    
    // Battery data
    int batteryLevel;
    int batteryVoltage;
    bool charging;
    
    // Network data
    WiFiStatus wifiStatus;
    QString ssid;
    int signalStrength;
    bool internetAccess;
    
    // Display data
    int displayBrightness;
    int updateInterval;
    
    // Button data
    bool button1Pressed;
    bool button2Pressed;
    
    // LED data
    bool led1On;
    bool led2On;
    
    // Speaker data
    bool speakerEnabled;
};

#endif // MOCKHARDWARE_H