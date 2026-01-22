#ifndef INKCLOCKEMULATOR_H
#define INKCLOCKEMULATOR_H

#include <QObject>
#include <QTimer>
#include <QDateTime>
#include "inkclocktypes.h"
#include "mockhardware.h"

class InkClockEmulator : public QObject
{
    Q_OBJECT

public:
    explicit InkClockEmulator(QObject *parent = nullptr);
    ~InkClockEmulator();
    
    // Device control methods
    void powerOn();
    void powerOff();
    void reset();
    void changeMode();
    
    // Sensor control methods
    void setTemperature(int temperature);
    void setHumidity(int humidity);
    void setPressure(int pressure);
    void setLightLevel(int lightLevel);
    void setMotionDetected(bool detected);
    
    // Power and network control methods
    void setBatteryLevel(int level);
    void setWiFiStatus(WiFiStatus status);
    
    // Display control methods
    void setBrightness(int brightness);
    void setUpdateInterval(int interval);
    
    // Get display data for rendering
    DisplayData getDisplayData() const;
    
    // Update the emulator (called periodically)
    void update();

signals:
    // Signal emitted when display data changes
    void displayUpdated();
    
    // Signal emitted for logging
    void logMessage(const QString &message);

private:
    // Device state
    bool isPoweredOn;
    DisplayMode currentMode;
    
    // Hardware mock object
    MockHardware *hardware;
    
    // Display data
    DisplayData displayData;
    
    // Update timer
    QTimer *updateTimer;
    
    // Last update time
    qint64 lastUpdateTime;
    
    // Private methods
    void updateTime();
    void updateDisplayMode();
    void processSensorData();
    void simulateBatteryDrain();
};

#endif // INKCLOCKEMULATOR_H