#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QMainWindow>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QGroupBox>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QTimer>
#include "displaywidget.h"
#include "inkclockemulator.h"

class Simulator : public QMainWindow
{
    Q_OBJECT

public:
    Simulator(QWidget *parent = nullptr);
    ~Simulator();

private slots:
    // Device control slots
    void onPowerToggle(bool checked);
    void onResetButtonClicked();
    void onModeButtonClicked();
    void onBrightnessChanged(int value);
    void onTemperatureChanged(int value);
    void onHumidityChanged(int value);
    void onPressureChanged(int value);
    void onLightChanged(int value);
    void onMotionDetected(bool detected);
    void onBatteryChanged(int value);
    void onWiFiStatusChanged(int index);
    void onUpdateIntervalChanged(int value);
    
    // Display update slot
    void updateDisplay();
    
    // Log slot
    void addLog(const QString &message);

private:
    // UI components
    QTabWidget *tabWidget;
    QWidget *mainTab;
    QWidget *controlTab;
    QWidget *statusTab;
    
    // Display area
    DisplayWidget *displayWidget;
    
    // InkClock emulator
    InkClockEmulator *emulator;
    
    // Control panel widgets
    QGroupBox *powerGroup;
    QGroupBox *sensorsGroup;
    QGroupBox *networkGroup;
    QGroupBox *displayGroup;
    
    // Power controls
    QPushButton *powerToggle;
    QPushButton *resetButton;
    QPushButton *modeButton;
    QSpinBox *batterySpinBox;
    
    // Sensor controls
    QSpinBox *temperatureSpinBox;
    QSpinBox *humiditySpinBox;
    QSpinBox *pressureSpinBox;
    QSpinBox *lightSpinBox;
    QCheckBox *motionCheckBox;
    
    // Network controls
    QComboBox *wifiStatusComboBox;
    
    // Display controls
    QSpinBox *brightnessSpinBox;
    QSpinBox *updateIntervalSpinBox;
    
    // Status and log display
    QTextEdit *statusLog;
    QLabel *deviceStatusLabel;
    
    // Update timer
    QTimer *updateTimer;
    
    // UI setup functions
    void setupMainTab();
    void setupControlTab();
    void setupStatusTab();
    void setupPowerControls(QVBoxLayout *layout);
    void setupSensorControls(QVBoxLayout *layout);
    void setupNetworkControls(QVBoxLayout *layout);
    void setupDisplayControls(QVBoxLayout *layout);
};

#endif // SIMULATOR_H