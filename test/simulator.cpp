#include "simulator.h"
#include <QGridLayout>
#include <QFormLayout>
#include <QDateTime>

Simulator::Simulator(QWidget *parent)
    : QMainWindow(parent),
      emulator(new InkClockEmulator(this)),
      updateTimer(new QTimer(this))
{
    // Set window properties
    setWindowTitle("InkClock Simulator v1.0.0");
    setMinimumSize(800, 600);
    
    // Create main widget and layout
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);
    
    // Create tab widget
    tabWidget = new QTabWidget(this);
    mainLayout->addWidget(tabWidget);
    
    // Create main tab (display area)
    setupMainTab();
    
    // Create control tab
    setupControlTab();
    
    // Create status tab
    setupStatusTab();
    
    // Connect display update signal
    connect(emulator, &InkClockEmulator::displayUpdated, this, &Simulator::updateDisplay);
    connect(emulator, &InkClockEmulator::logMessage, this, &Simulator::addLog);
    
    // Setup update timer
    connect(updateTimer, &QTimer::timeout, emulator, &InkClockEmulator::update);
    updateTimer->start(1000); // Update every 1 second
    
    // Add initial log message
    addLog("InkClock Simulator started");
    addLog("Device initialized in power-off state");
}

Simulator::~Simulator()
{
    delete updateTimer;
    delete emulator;
}

void Simulator::setupMainTab()
{
    mainTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(mainTab);
    
    // Create display widget
    displayWidget = new DisplayWidget(mainTab);
    displayWidget->setFixedSize(400, 300);
    
    // Center the display widget
    QHBoxLayout *displayLayout = new QHBoxLayout();
    displayLayout->addStretch();
    displayLayout->addWidget(displayWidget);
    displayLayout->addStretch();
    
    // Add display title
    QLabel *displayTitle = new QLabel("InkClock Display");
    displayTitle->setAlignment(Qt::AlignCenter);
    displayTitle->setStyleSheet("font-weight: bold; font-size: 16px;");
    
    layout->addWidget(displayTitle);
    layout->addLayout(displayLayout);
    
    // Add device status label
    deviceStatusLabel = new QLabel("Device Status: OFF");
    deviceStatusLabel->setAlignment(Qt::AlignCenter);
    deviceStatusLabel->setStyleSheet("color: red; font-weight: bold;");
    layout->addWidget(deviceStatusLabel);
    
    tabWidget->addTab(mainTab, "Display");
}

void Simulator::setupControlTab()
{
    controlTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(controlTab);
    
    // Create a scroll area for controls
    QScrollArea *scrollArea = new QScrollArea(this);
    QWidget *scrollContent = new QWidget(this);
    QVBoxLayout *scrollLayout = new QVBoxLayout(scrollContent);
    scrollContent->setLayout(scrollLayout);
    scrollArea->setWidget(scrollContent);
    scrollArea->setWidgetResizable(true);
    
    // Create power controls
    powerGroup = new QGroupBox("Power & Device Controls");
    QVBoxLayout *powerLayout = new QVBoxLayout(powerGroup);
    setupPowerControls(powerLayout);
    scrollLayout->addWidget(powerGroup);
    
    // Create sensor controls
    sensorsGroup = new QGroupBox("Sensor Controls");
    QVBoxLayout *sensorLayout = new QVBoxLayout(sensorsGroup);
    setupSensorControls(sensorLayout);
    scrollLayout->addWidget(sensorsGroup);
    
    // Create network controls
    networkGroup = new QGroupBox("Network Controls");
    QVBoxLayout *networkLayout = new QVBoxLayout(networkGroup);
    setupNetworkControls(networkLayout);
    scrollLayout->addWidget(networkGroup);
    
    // Create display controls
    displayGroup = new QGroupBox("Display Controls");
    QVBoxLayout *displayLayout = new QVBoxLayout(displayGroup);
    setupDisplayControls(displayLayout);
    scrollLayout->addWidget(displayGroup);
    
    layout->addWidget(scrollArea);
    tabWidget->addTab(controlTab, "Controls");
}

void Simulator::setupStatusTab()
{
    statusTab = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(statusTab);
    
    // Create status log
    QLabel *logLabel = new QLabel("Status Log:");
    logLabel->setStyleSheet("font-weight: bold;");
    layout->addWidget(logLabel);
    
    statusLog = new QTextEdit(this);
    statusLog->setReadOnly(true);
    statusLog->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    statusLog->setStyleSheet("font-family: monospace;");
    layout->addWidget(statusLog);
    
    tabWidget->addTab(statusTab, "Status");
}

void Simulator::setupPowerControls(QVBoxLayout *layout)
{
    // Power toggle button
    powerToggle = new QPushButton("Power ON");
    powerToggle->setCheckable(true);
    connect(powerToggle, &QPushButton::toggled, this, &Simulator::onPowerToggle);
    layout->addWidget(powerToggle);
    
    // Reset button
    resetButton = new QPushButton("Reset Device");
    connect(resetButton, &QPushButton::clicked, this, &Simulator::onResetButtonClicked);
    layout->addWidget(resetButton);
    
    // Mode button
    modeButton = new QPushButton("Change Mode");
    connect(modeButton, &QPushButton::clicked, this, &Simulator::onModeButtonClicked);
    layout->addWidget(modeButton);
    
    // Battery level control
    QFormLayout *batteryLayout = new QFormLayout();
    batterySpinBox = new QSpinBox(this);
    batterySpinBox->setRange(0, 100);
    batterySpinBox->setValue(80);
    connect(batterySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Simulator::onBatteryChanged);
    batteryLayout->addRow("Battery Level:", batterySpinBox);
    layout->addLayout(batteryLayout);
}

void Simulator::setupSensorControls(QVBoxLayout *layout)
{
    QFormLayout *sensorLayout = new QFormLayout();
    
    // Temperature control (-20 to 50°C)
    temperatureSpinBox = new QSpinBox(this);
    temperatureSpinBox->setRange(-20, 50);
    temperatureSpinBox->setValue(22);
    connect(temperatureSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Simulator::onTemperatureChanged);
    sensorLayout->addRow("Temperature (°C):", temperatureSpinBox);
    
    // Humidity control (0 to 100%)
    humiditySpinBox = new QSpinBox(this);
    humiditySpinBox->setRange(0, 100);
    humiditySpinBox->setValue(50);
    connect(humiditySpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Simulator::onHumidityChanged);
    sensorLayout->addRow("Humidity (%):", humiditySpinBox);
    
    // Pressure control (800 to 1200 hPa)
    pressureSpinBox = new QSpinBox(this);
    pressureSpinBox->setRange(800, 1200);
    pressureSpinBox->setValue(1013);
    connect(pressureSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Simulator::onPressureChanged);
    sensorLayout->addRow("Pressure (hPa):", pressureSpinBox);
    
    // Light level control (0 to 1000 lux)
    lightSpinBox = new QSpinBox(this);
    lightSpinBox->setRange(0, 1000);
    lightSpinBox->setValue(200);
    connect(lightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Simulator::onLightChanged);
    sensorLayout->addRow("Light Level (lux):", lightSpinBox);
    
    // Motion detection checkbox
    motionCheckBox = new QCheckBox("Motion Detected");
    connect(motionCheckBox, &QCheckBox::toggled, this, &Simulator::onMotionDetected);
    sensorLayout->addRow(motionCheckBox);
    
    layout->addLayout(sensorLayout);
}

void Simulator::setupNetworkControls(QVBoxLayout *layout)
{
    QFormLayout *networkLayout = new QFormLayout();
    
    // WiFi status combo box
    wifiStatusComboBox = new QComboBox(this);
    wifiStatusComboBox->addItem("Disconnected");
    wifiStatusComboBox->addItem("Connecting");
    wifiStatusComboBox->addItem("Connected");
    connect(wifiStatusComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &Simulator::onWiFiStatusChanged);
    networkLayout->addRow("WiFi Status:", wifiStatusComboBox);
    
    layout->addLayout(networkLayout);
}

void Simulator::setupDisplayControls(QVBoxLayout *layout)
{
    QFormLayout *displayLayout = new QFormLayout();
    
    // Brightness control (0 to 100%)
    brightnessSpinBox = new QSpinBox(this);
    brightnessSpinBox->setRange(0, 100);
    brightnessSpinBox->setValue(70);
    connect(brightnessSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Simulator::onBrightnessChanged);
    displayLayout->addRow("Brightness:", brightnessSpinBox);
    
    // Update interval control (1 to 60 seconds)
    updateIntervalSpinBox = new QSpinBox(this);
    updateIntervalSpinBox->setRange(1, 60);
    updateIntervalSpinBox->setValue(30);
    connect(updateIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &Simulator::onUpdateIntervalChanged);
    displayLayout->addRow("Update Interval (s):", updateIntervalSpinBox);
    
    layout->addLayout(displayLayout);
}

void Simulator::onPowerToggle(bool checked)
{
    if (checked) {
        emulator->powerOn();
        powerToggle->setText("Power OFF");
        deviceStatusLabel->setText("Device Status: ON");
        deviceStatusLabel->setStyleSheet("color: green; font-weight: bold;");
        addLog("Device powered ON");
    } else {
        emulator->powerOff();
        powerToggle->setText("Power ON");
        deviceStatusLabel->setText("Device Status: OFF");
        deviceStatusLabel->setStyleSheet("color: red; font-weight: bold;");
        addLog("Device powered OFF");
    }
}

void Simulator::onResetButtonClicked()
{
    emulator->reset();
    addLog("Device reset");
}

void Simulator::onModeButtonClicked()
{
    emulator->changeMode();
    addLog("Device mode changed");
}

void Simulator::onBrightnessChanged(int value)
{
    emulator->setBrightness(value);
    addLog(QString("Brightness set to %1%").arg(value));
}

void Simulator::onTemperatureChanged(int value)
{
    emulator->setTemperature(value);
}

void Simulator::onHumidityChanged(int value)
{
    emulator->setHumidity(value);
}

void Simulator::onPressureChanged(int value)
{
    emulator->setPressure(value);
}

void Simulator::onLightChanged(int value)
{
    emulator->setLightLevel(value);
}

void Simulator::onMotionDetected(bool detected)
{
    emulator->setMotionDetected(detected);
    if (detected) {
        addLog("Motion detected");
    } else {
        addLog("Motion cleared");
    }
}

void Simulator::onBatteryChanged(int value)
{
    emulator->setBatteryLevel(value);
    addLog(QString("Battery level set to %1%").arg(value));
}

void Simulator::onWiFiStatusChanged(int index)
{
    emulator->setWiFiStatus(static_cast<WiFiStatus>(index));
    addLog(QString("WiFi status changed to: %1").arg(wifiStatusComboBox->currentText()));
}

void Simulator::onUpdateIntervalChanged(int value)
{
    emulator->setUpdateInterval(value);
    addLog(QString("Update interval set to %1 seconds").arg(value));
}

void Simulator::updateDisplay()
{
    displayWidget->updateDisplay(emulator->getDisplayData());
}

void Simulator::addLog(const QString &message)
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    QString logEntry = QString("[%1] %2\n").arg(timestamp).arg(message);
    statusLog->append(logEntry);
    statusLog->verticalScrollBar()->setValue(statusLog->verticalScrollBar()->maximum());
}