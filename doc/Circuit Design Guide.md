# InkClock - Circuit Design Guide

[中文版本](电路设计指南.md)

## 1. Circuit Design Overview

The circuit design of InkClock adopts a modular design, mainly including the following modules:

1. **Core Control Module**: Designed based on ESP32-S3, responsible for system control and data processing
2. **Display Module**: E-paper display driver circuit, responsible for refreshing and controlling display content
3. **Power Management Module**: Responsible for power input, output, charging and protection
4. **Sensor Module**: Interface circuits for temperature and humidity sensors and other peripherals
5. **Input/Output Module**: Interface circuits for buttons, indicators and other peripherals
6. **Communication Module**: WiFi and Bluetooth communication circuits

## 2. Core Control Module

### 2.1 ESP32-S3 Pin Allocation

| Function | ESP32-S3 Pin | Description |
|----------|--------------|-------------|
| Power | VIN/3.3V | Power input, supports 5V VIN or direct 3.3V input |
| Ground | GND | Ground pin |
| Reset | EN | Reset pin, low level reset |
| Serial Port | TXD0/RXD0 | Hardware serial port 0, used for debugging and communication |
| SPI Interface | GPIO18(SCK)/GPIO23(MOSI)/GPIO19(MISO) | Used to connect e-paper display and other SPI devices |
| I2C Interface | GPIO22(SCL)/GPIO21(SDA) | Used to connect I2C sensors and devices |
| E-paper Control | GPIO5(CS)/GPIO26(DC)/GPIO27(RST)/GPIO32(BUSY) | E-paper display control pins |
| Buttons | GPIO0/GPIO1/GPIO2/GPIO3 | Button input pins |
| Sensor | GPIO4 | DHT22 temperature and humidity sensor data pin |
| Indicator | GPIO25 | Status indicator pin |

### 2.2 ESP32-S3 Minimum System Circuit

The minimum system circuit of ESP32-S3 includes:

1. **Power Circuit**:
   - 3.3V voltage regulator circuit, using AMS1117-3.3 or XC6206 LDO
   - Power filter capacitors, 100nF ceramic capacitor and 10μF electrolytic capacitor
   - Power protection circuit, TVS tube and fuse

2. **Reset Circuit**:
   - 10KΩ pull-up resistor
   - 0.1μF filter capacitor
   - Reset button (optional)

3. **Crystal Oscillator Circuit**:
   - 40MHz main crystal oscillator
   - 32.768KHz RTC crystal oscillator
   - Load capacitors (usually 12pF)

4. **Antenna Circuit**:
   - 2.4GHz WiFi/Bluetooth antenna
   - Impedance matching circuit (50Ω)
   - Antenna switch (optional)

## 3. Display Module

### 3.1 E-Paper Display Interface Circuit

The e-paper display is connected to ESP32-S3 through SPI interface, typical circuit as follows:

```
ESP32-S3         E-paper Display
GPIO18(SCK)  ──── SCK
GPIO23(MOSI) ──── MOSI
GPIO19(MISO) ──── MISO (optional)
GPIO5(CS)    ──── CS
GPIO26(DC)   ──── DC
GPIO27(RST)  ──── RST
GPIO32(BUSY) ──── BUSY
3.3V         ──── VCC
GND          ──── GND
```

### 3.2 E-Paper Display Driver Circuit

The e-paper display driver circuit mainly includes:

1. **Power Filtering**: Parallel 100nF ceramic capacitor between VCC and GND of the e-paper display
2. **Signal Driving**: SPI signals can be directly connected to ESP32-S3 GPIO pins without additional driving circuit
3. **Level Conversion**: If the e-paper display uses 5V power, a level conversion circuit is needed to convert ESP32-S3's 3.3V signals to 5V
4. **BUSY Signal Processing**: The BUSY pin of the e-paper display is used to indicate refresh status, low level means refreshing, high level means refresh completed

## 4. Power Management Module

### 4.1 Power Topology Structure

The power management module adopts the following topology structure:

```
External Power (USB-Type-C) ──── Type-C Charging Module ──── Soft-pack Polymer Battery ──── Power Management IC (AXP192+Type-C) ──── Module Power
```

### 4.2 Charging Circuit Design

**Only supports USB-Type-C interface charging**, the charging circuit uses Type-C charging module, main parameters:

- Interface Type: USB-Type-C
- Input Voltage: 5V DC
- Charging Power: 10W-20W
- Charging Current: Maximum 2A, adjustable through resistor
- Charging Termination Voltage: 4.2V±1%
- Overcharge Protection: 4.2V±1%
- Overdischarge Protection: 3.0V
- Short Circuit Protection: Supported
- Reverse Connection Protection: Supported

Charging circuit design:

1. **Type-C Interface Design**:
   - Use USB-Type-C connector (such as CJ2101)
   - Built-in USB PD protocol support (optional, for fast charging)
   - CC pin detection and configuration

2. **Input Filtering**: 100nF ceramic capacitor and 10μF electrolytic capacitor to reduce input interference
3. **Charging Current Adjustment**: Adjusted through RPROG resistor, supporting 10W-20W charging power
4. **Status Indication**: CHRG pin (low level during charging, high level when fully charged), STDBY pin (high level when fully charged)
5. **Battery Protection**: Built-in overcharge, overdischarge, short circuit protection, suitable for soft-pack polymer batteries

### 4.3 Power Management Circuit

The power management circuit uses AXP192+Type-C power management IC, main parameters:

- Interface Type: USB-Type-C
- Input Voltage: 3.0V-5.5V
- Charging Power: 10W-20W
- Output Channels: 6 DC-DC outputs, 2 LDO outputs
- Maximum Output Current: 3A per channel
- Power Consumption: <10mA in working mode, <10μA in deep sleep mode
- Features: Supports power monitoring, charging management, power switch and other functions

Power management circuit design:

1. DC-DC1: 3.3V output, powers ESP32-S3
2. DC-DC2: 5V output, powers e-paper display and other 5V devices
3. LDO1: 3.3V output, powers sensors and other low-power devices
4. **Soft-pack Polymer Battery Management**:
   - Supports 3.3V-4.2V soft-pack polymer batteries
   - Power monitoring: Reads battery voltage and power through I2C interface
   - Charging current control: Supports 10W-20W charging power
5. Power switch: Controls system power through EXTEN pin
6. **Charging Protection**:
   - Overcharge Protection: 4.2V±1%
   - Overdischarge Protection: 3.0V
   - Short Circuit Protection: Supported
   - Overcurrent Protection: Supported

### 4.4 Power Supply Instructions

- **Only supports USB-Type-C charging**, does not support DC direct plug-in power supply
- Charging power range: 10W-20W
- Battery Type: 3.7V soft-pack polymer battery
- Working Voltage: 3.3V
- Battery Voltage Range: 3.3V-4.2V

## 5. Sensor Module

### 5.1 Temperature and Humidity Sensor Circuit

#### 5.1.1 DHT Series Sensor Circuit

Circuit design for DHT11, DHT22, DHT12 and other DHT series sensors:

- Power: 3.3V-5V
- Data Pin: GPIO4, pull-up 10KΩ resistor
- Filtering: Parallel 100nF ceramic capacitor on data line to reduce interference

**Circuit Connection**:
```
ESP32-S3  GPIO4 ──── 10KΩ ──── 3.3V
               └─── DHT DATA
DHT VCC ──── 3.3V
DHT GND ──── GND
```

#### 5.1.2 SHT Series Sensor Circuit

Circuit design for SHT30, SHT21, SHT40 and other SHT series sensors:

- Power: 2.4V-5.5V
- Interface: I2C, address optional 0x44 or 0x45 (SHT30/SHT40), 0x40 (SHT21)
- Filtering: Parallel 100nF ceramic capacitor between VCC and GND

**Circuit Connection**:
```
ESP32-S3  GPIO22(SCL) ──── SHT SCL
ESP32-S3  GPIO21(SDA) ──── SHT SDA
SHT VCC ──── 3.3V
SHT GND ──── GND
```

#### 5.1.3 Other Temperature and Humidity Sensor Circuits

- **BME280/BME680**: Temperature, humidity and pressure sensor, supports I2C/SPI interface
- **HDC1080**: Digital temperature and humidity sensor, I2C interface
- **HTU21D/SI7021**: Digital temperature and humidity sensor, I2C interface, compatible with SHT21

### 5.2 Human Presence Sensor Circuit

#### 5.2.1 HC-SR501 Sensor Circuit

Circuit design for HC-SR501 human presence sensor:

- Power: 5V
- Output Pin: GPIO5 (digital output)
- Sensitivity Adjustment: Adjustable through potentiometer
- Delay Adjustment: Adjustable through potentiometer

**Circuit Connection**:
```
ESP32-S3  GPIO5 ──── HC-SR501 OUT
HC-SR501 VCC ──── 5V
HC-SR501 GND ──── GND
```

#### 5.2.2 Other Human Presence Sensor Circuits

- **HC-SR505**: Miniaturized human presence sensor, circuit similar to HC-SR501
- **RE200B**: High-sensitivity infrared sensor, needs additional amplification circuit
- **LD2410**: Millimeter wave radar human presence sensor, UART interface

### 5.3 Light Sensor Circuit

#### 5.3.1 BH1750 Sensor Circuit

Circuit design for BH1750 light sensor:

- Power: 3.3V-5V
- Interface: I2C, address optional 0x23 or 0x5C
- Filtering: Parallel 100nF ceramic capacitor between VCC and GND

**Circuit Connection**:
```
ESP32-S3  GPIO22(SCL) ──── BH1750 SCL
ESP32-S3  GPIO21(SDA) ──── BH1750 SDA
BH1750 VCC ──── 3.3V
BH1750 GND ──── GND
```

#### 5.3.2 Other Light Sensor Circuits

- **VEML6075**: UV light sensor, I2C interface
- **TSL2561**: Digital light sensor, I2C interface
- **GY30**: Digital light sensor, I2C interface
- **SI1145**: Ambient light and UV sensor, I2C interface

### 5.4 Gas Sensor Circuit

#### 5.4.1 MQ Series Sensor Circuit

Circuit design for MQ-2, MQ-5, MQ-7, MQ-8, MQ-135 and other MQ series gas sensors:

- Power: 5V
- Preheating Time: Recommended 2-10 minutes
- Interface: Analog output (AO) or digital output (DO)
- Load Resistor: Choose appropriate load resistor according to sensor model (usually 1-10KΩ)

**Circuit Connection**:
```
ESP32-S3  GPIO34 ──── MQ Sensor AO
ESP32-S3  GPIOx  ──── MQ Sensor DO (optional)
MQ Sensor VCC ──── 5V
MQ Sensor GND ──── GND
```

#### 5.4.2 TGS2600 Sensor Circuit

Circuit design for TGS2600 gas sensor:

- Power: 5V
- Interface: Analog output
- Load Resistor: Approximately 10KΩ

### 5.5 Flame Sensor Circuit

#### 5.5.1 Infrared Flame Sensor Circuit

Circuit design for infrared flame sensor:

- Power: 3.3V-5V
- Interface: Digital output
- Sensitivity Adjustment: Adjustable through potentiometer

**Circuit Connection**:
```
ESP32-S3  GPIO35 ──── Flame Sensor OUT
Flame Sensor VCC ──── 3.3V
Flame Sensor GND ──── GND
```

#### 5.5.2 UV Flame Sensor Circuit

Circuit design for UV flame sensor:

- Power: 3.3V-5V
- Interface: Digital output or analog output
- Filtering: Parallel 100nF ceramic capacitor on signal line

### 5.6 Sensor Expansion Interface Design

To facilitate expanding more sensors, the system has designed standard sensor expansion interfaces:

- **I2C Interface**: Provides standard I2C interface (SCL, SDA, VCC, GND)
- **SPI Interface**: Provides standard SPI interface (SCK, MOSI, MISO, CS, VCC, GND)
- **Analog Interface**: Provides multiple analog inputs (AO, VCC, GND)
- **Digital Interface**: Provides multiple digital inputs/outputs (IO, VCC, GND)

**Expansion Interface Pin Allocation**:
| Expansion Interface Pin | ESP32-S3 Pin |
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

## 6. Input/Output Module

### 6.1 Button Circuit Design

The button circuit adopts pull-up resistor design to prevent button bounce:

- Number of Buttons: 4
- Pins: GPIO0/GPIO1/GPIO2/GPIO3
- Pull-up Resistor: 10KΩ
- Debounce: Hardware debounce (RC filter) or software debounce (delay detection)

**Circuit Connection**:
```
ESP32-S3  GPIOx ──── 10KΩ ──── 3.3V
               └─── Button ──── GND
```

### 6.2 Indicator Circuit Design

Indicator circuit design:

- Type: LED indicator
- Color: Red (power), Green (WiFi connection), Blue (Bluetooth connection)
- Current Limiting Resistor: 220Ω-1KΩ
- Control Method: GPIO output, low level or high level to light up

**Circuit Connection**:
```
ESP32-S3  GPIOx ──── 220Ω ──── LED Anode
LED Cathode ──── GND
```

### 6.3 Buzzer Circuit Design

Buzzer circuit design:

- Type: Active buzzer or passive buzzer
- Active Buzzer: Can sound directly when connected to power, controlled by GPIO switch
- Passive Buzzer: Needs PWM signal to drive, uses GPIO to output PWM signal
- Current Limiting Resistor: 100Ω-1KΩ
- Transistor Driver: If the buzzer current is large, a transistor driver is needed

**Active Buzzer Circuit**:
```
ESP32-S3  GPIOx ──── 100Ω ──── Buzzer Anode
Buzzer Cathode ──── GND
```

**Passive Buzzer Circuit**:
```
ESP32-S3  GPIOx ──── 100Ω ──── Transistor Base
Transistor Collector ──── Buzzer Anode
Buzzer Cathode ──── GND
Transistor Emitter ──── GND
10KΩ Resistor ──── Transistor Base ──── GND (pull-down resistor)
```

## 7. Communication Module

### 7.1 WiFi Antenna Design

The following points need to be noted in WiFi antenna design:

1. **Antenna Type**:
   - External Antenna: High gain, good signal, but needs additional space
   - Built-in PCB Antenna: Small size, easy to install, but lower gain
   - Ceramic Antenna: Stable performance, suitable for high-precision applications

2. **Impedance Matching**:
   - Antenna impedance is 50Ω, need to design 50Ω transmission line
   - Use impedance matching network to ensure impedance matching between antenna and ESP32-S3

3. **Antenna Layout**:
   - Keep clear area around the antenna, avoid metal objects interference
   - Keep certain distance between antenna and ground to improve radiation efficiency
   - Avoid antenna close to power supply and other interference sources

### 7.2 Bluetooth Antenna Design

Bluetooth antenna design is similar to WiFi antenna, main points to note:

- Bluetooth and WiFi use the same 2.4GHz frequency band, can share antenna
- Use antenna switch to switch between WiFi and Bluetooth modes
- Ensure antenna bandwidth covers 2.4GHz-2.5GHz frequency band

## 8. Circuit Protection Design

### 8.1 Power Protection

Power protection circuit includes:

1. **Overvoltage Protection**: Use TVS tube or varistor to prevent input voltage from being too high
2. **Overcurrent Protection**: Use fuse or PTC self-recovery fuse to prevent excessive current
3. **Short Circuit Protection**: Power management IC has built-in short circuit protection, or use external short circuit protection circuit
4. **Reverse Connection Protection**: Use diode or MOS tube to prevent power reverse connection

### 8.2 Signal Protection

Signal protection circuit includes:

1. **ESD Protection**: Use ESD protection diode to prevent static damage
2. **Surge Protection**: Use TVS tube to prevent signal surge
3. **Overvoltage Protection**: Use Zener diode to prevent signal voltage from being too high

### 8.3 Electromagnetic Compatibility Design

Electromagnetic compatibility design includes:

1. **Grounding Design**:
   - Single point grounding or multiple point grounding
   - Separate digital ground and analog ground, finally converge at power supply
   - Ground loop of high-frequency signal should be as short as possible

2. **Filtering Design**:
   - Add EMI filter at power inlet
   - Add filter capacitors at each module power supply
   - Add filtering components on signal lines, such as magnetic beads, inductors, capacitors, etc.

3. **Shielding Design**:
   - Shield sensitive circuits
   - Use shielded cables to transmit sensitive signals
   - Ground shield case

## 9. PCB Design

### 9.1 PCB Layout

The following points need to be noted in PCB layout:

1. **Modular Layout**: Separate layout of different functional modules, such as core control module, display module, power module, etc.
2. **Signal Flow**: Layout according to signal flow, from input to output, from left to right or top to bottom
3. **Power Layout**: Power part close to input, separate digital power and analog power
4. **Ground Layout**: Large area ground plane to reduce ground resistance and inductance
5. **Sensitive Component Layout**: Place sensitive components (such as crystal oscillator, sensor) away from interference sources
6. **Connector Layout**: Place connectors on PCB edge for easy connection to external devices

### 9.2 PCB Routing

The following points need to be noted in PCB routing:

1. **Line Width**:
   - Line width on power line is determined by current size, generally current density on power line does not exceed 1A/mm
   - Line width of signal line is generally 0.2mm-0.4mm

2. **Line Spacing**:
   - Line spacing on power line is determined by voltage, meeting creepage distance requirements
   - Line spacing of signal line is generally 0.1mm-0.2mm

3. **Differential Line**:
   - High-speed signals use differential lines, such as USB, Ethernet, etc.
   - Length matching of differential lines, error no more than 5mil
   - Keep the spacing of differential lines consistent, impedance matching 50Ω or 100Ω

4. **Via**:
   - Avoid drilling holes on high-frequency signal lines
   - Via size on power line is determined by current, generally 0.6mm-0.8mm via is used
   - Via size on signal line is generally 0.3mm-0.4mm via

5. **Copper Pouring**:
   - Large area copper pouring to reduce ground resistance and inductance
   - Connect copper pouring with ground, use multiple vias for connection
   - Avoid creating islands in copper pouring

## 10. Circuit Schematic

### 10.1 Main Circuit Schematic

The main circuit schematic includes core circuits such as ESP32-S3 minimum system, power management circuit, e-paper display interface circuit, etc.

### 10.2 Module Circuit Schematic

Module circuit schematic includes detailed circuit design of each functional module:

1. **ESP32-S3 Minimum System Circuit**
2. **Power Management Circuit**
3. **E-paper Display Driver Circuit**
4. **Sensor Interface Circuit**
5. **Button and Indicator Circuit**
6. **Communication Circuit**

## 11. Circuit Debugging

### 11.1 Power Debugging

Power debugging steps:

1. Check if power input is normal
2. Check if power output of each module is normal
3. Check if charging function is normal
4. Check if power protection function is normal
5. Check if power consumption meets requirements

### 11.2 Core Module Debugging

Core module debugging steps:

1. Check if ESP32-S3 can start normally
2. Check if serial communication is normal
3. Check if WiFi and Bluetooth functions are normal
4. Check if I2C and SPI interfaces are normal

### 11.3 Display Module Debugging

Display module debugging steps:

1. Check if e-paper display can display normally
2. Check if full refresh and partial refresh functions are normal
3. Check if display content is correct
4. Check if refresh speed meets requirements

### 11.4 Sensor Module Debugging

Sensor module debugging steps:

1. Check if sensor can communicate normally
2. Check if sensor data is correct
3. Check if sensor accuracy and stability
4. Check if sensor performance in different environments

### 11.5 Input/Output Module Debugging

Input/output module debugging steps:

1. Check if buttons can respond normally
2. Check if indicators can light up normally
3. Check if buzzer can sound normally

## 12. Circuit Optimization

### 12.1 Power Consumption Optimization

Power consumption optimization measures:

1. Choose low-power components
2. Optimize power management, use DC-DC converter instead of LDO
3. Reasonably set sleep mode to reduce unnecessary power consumption
4. Reduce refresh frequency, reduce e-paper display refresh times
5. Turn off unnecessary modules and functions

### 12.2 Reliability Optimization

Reliability optimization measures:

1. Use high-quality components
2. Increase redundancy design, such as backup power supply, backup sensor, etc.
3. Strengthen circuit protection, such as overvoltage, overcurrent, short circuit protection, etc.
4. Optimize PCB design to improve electromagnetic compatibility
5. Conduct reliability tests, such as temperature cycle, humidity cycle, vibration test, etc.

### 12.3 Cost Optimization

Cost optimization measures:

1. Choose cost-effective components
2. Optimize circuit design to reduce component count
3. Optimize PCB design to reduce PCB area
4. Choose appropriate packaging, such as SMT packaging instead of through-hole packaging
5. Bulk procurement to reduce procurement cost

## 13. Extended Function Design

### 13.1 Expandable Sensors

The system supports the following expandable sensors:

1. **Light Sensor**: BH1750, used to detect ambient light intensity
2. **Human Infrared Sensor**: HC-SR501, used to detect human activities
3. **Noise Sensor**: LM393, used to detect ambient noise
4. **Air Quality Sensor**: MQ series, used to detect air quality
5. **Pressure Sensor**: BMP280, used to detect pressure and altitude

### 13.2 Expandable Interfaces

The system provides the following expandable interfaces:

1. **I2C Interface**: Used to connect I2C devices
2. **SPI Interface**: Used to connect SPI devices
3. **UART Interface**: Used to connect serial devices
4. **GPIO Interface**: Used to connect digital and analog devices
5. **ADC Interface**: Used to connect analog sensors

### 13.3 Expandable Functions

The system supports the following expandable functions:

1. **Camera Function**: Add camera module to realize image capture and recognition
2. **Audio Function**: Add audio module to realize voice playback and recording
3. **NFC Function**: Add NFC module to realize NFC tag reading and writing
4. **RFID Function**: Add RFID module to realize RFID tag reading
5. **ZigBee Function**: Add ZigBee module to realize ZigBee network connection
6. **LoRa Function**: Add LoRa module to realize long-distance communication

## 14. Notes

1. **Power Safety**:
   - Ensure power voltage and current meet requirements
   - Avoid power short circuit and reverse connection
   - Use power adapters that meet safety standards

2. **Circuit Safety**:
   - Ensure circuit meets electrical safety standards
   - Avoid high voltage and large current circuits
   - Strengthen circuit protection to prevent electric shock and fire

3. **Electromagnetic Compatibility**:
   - Ensure circuit meets electromagnetic compatibility standards
   - Reduce electromagnetic interference to avoid affecting other devices
   - Prevent external electromagnetic interference from affecting system normal operation

4. **Heat Dissipation Design**:
   - Ensure good heat dissipation of circuit to avoid overheating
   - Add heat sink to components with large power consumption
   - Optimize PCB layout to improve heat dissipation efficiency

5. **Installation and Debugging**:
   - Check circuit correctness before installation
   - Use appropriate tools and methods during debugging
   - Follow safety operating procedures to prevent electric shock and equipment damage

## 15. Summary

The circuit design of InkClock adopts modular design with the following features:

- Uses ESP32-S3 as core controller, powerful performance, low power consumption
- Supports multiple sizes of e-paper displays, clear display, low power consumption
- Power management module supports charging, power monitoring, power protection and other functions
- Supports multiple sensors, rich expandable functions
- Adopts modular design, easy to maintain and upgrade
- Circuit design considers reliability, safety, electromagnetic compatibility and other factors
- Provides detailed debugging and optimization suggestions

Through reasonable circuit design, system performance, reliability and stability can be improved, power consumption and cost can be reduced, and subsequent maintenance and upgrade can be facilitated.