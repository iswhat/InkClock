# InkClock - Development Hardware Description

[中文版本](开发硬件说明.md)

## 1. Development Board Description

### 1.1 ESP32-S3 Development Board

ESP32-S3 is a low-power WiFi+Bluetooth dual-mode microcontroller launched by Espressif, with the following features:

| Parameter | Specification |
|-----------|---------------|
| Processor | Dual-core 32-bit Xtensa® LX7 processor, 240MHz |
| Memory | 512KB SRAM, supports external 8MB PSRAM |
| Storage | 16MB Flash (default), supports larger capacity |
| WiFi | 802.11 b/g/n, supports 2.4GHz band |
| Bluetooth | Bluetooth 5.0 LE |
| Peripherals | 45 GPIO pins, supports ADC, DAC, I2C, SPI, UART and other interfaces |
| Power | 3.3V operating voltage, supports 5V DC input |
| Power Consumption | <5μA in deep sleep mode |
| Temperature Range | -40℃ ~ 85℃ |

### 1.2 Recommended Development Boards

- **ESP32-S3-DevKitC-1**: Espressif official development board, all pins引出, convenient for development and debugging
- **ESP32-S3-Box**: Espressif official development kit, built-in screen, camera, microphone and other peripherals
- **ESP32-S3-LCD-EV-Board**: Suitable for e-paper applications, built-in LCD controller

## 2. E-Paper Display Description

### 2.1 E-Paper Display Features

E-paper (electronic paper) has the following features:

- Low power consumption: Only consumes power when refreshing, almost no power consumption during static display
- High definition: Clear display, high resolution
- Wide viewing angle: Can be clearly viewed from various angles
- Sunlight readable: Suitable for outdoor use
- No blue light: Eye-friendly
- Slow response speed: Refresh speed is generally hundreds of milliseconds to several seconds
- Limited colors: Usually black and white or black, white and red

### 2.2 Supported E-Paper Display Sizes

The system supports the following sizes of e-paper displays:

| Size | Resolution | Interface | Driver IC |
|------|------------|-----------|-----------|
| 1.54 inch | 200×200 | SPI | GDEW0154M09 |
| 2.13 inch | 212×104 | SPI | GDEW0213M09 |
| 2.66 inch | 152×296 | SPI | GDEW0266M09 |
| 2.7 inch | 264×176 | SPI | GDEW027W3 |
| 2.9 inch | 296×128 | SPI | GDEW029T5 |
| 3.12 inch | 250×300 | SPI | GDEW031T7 |
| 4.2 inch | 400×300 | SPI | GDEW042T2 |
| 4.37 inch | 544×376 | SPI | GDEW0437M13 |
| 5.4 inch | 600×448 | SPI | GDEW054Z09 |
| 5.83 inch | 648×480 | SPI | GDEW0583T7 |
| 6.0 inch | 800×600 | SPI | GDEW060T3 |
| 7.5 inch | 800×480 | SPI | GDEW075T7 |

### 2.3 E-Paper Display Connection Instructions

The e-paper display is connected to ESP32-S3 through SPI interface. Typical connection method is as follows:

| E-Paper Pin | ESP32-S3 Pin | Description |
|-------------|--------------|-------------|
| VCC | 3.3V | Power positive |
| GND | GND | Power negative |
| SCK | GPIO18 | SPI clock pin |
| MOSI | GPIO23 | SPI data output pin |
| MISO | GPIO19 | SPI data input pin (optional, not required for some e-paper displays) |
| CS | GPIO5 | Chip select pin |
| DC | GPIO26 | Data/command switching pin |
| RST | GPIO27 | Reset pin |
| BUSY | GPIO32 | Busy status pin (optional, used to detect refresh completion) |

## 3. Sensor Description

### 3.1 Temperature and Humidity Sensors

The system supports the following temperature and humidity sensors:

#### 3.1.1 DHT22 Sensor

| Parameter | Specification |
|-----------|---------------|
| Type | Digital temperature and humidity sensor |
| Measurement Range | Temperature: -40℃ ~ 80℃, Humidity: 0% ~ 100% RH |
| Accuracy | Temperature: ±0.5℃, Humidity: ±2% RH |
| Interface | Single bus |
| Power Supply | 3.3V ~ 5V |
| Response Time | 2s |

**Connection Method:**
| DHT22 Pin | ESP32-S3 Pin |
|-----------|--------------|
| VCC | 3.3V |
| GND | GND |
| DATA | GPIO4 |

#### 3.1.2 SHT30 Sensor

| Parameter | Specification |
|-----------|---------------|
| Type | Digital temperature and humidity sensor |
| Measurement Range | Temperature: -40℃ ~ 125℃, Humidity: 0% ~ 100% RH |
| Accuracy | Temperature: ±0.3℃, Humidity: ±2% RH |
| Interface | I2C |
| Power Supply | 2.4V ~ 5.5V |
| Response Time | <1s |
| I2C Address | 0x44 (default) or 0x45 |

**Connection Method:**
| SHT30 Pin | ESP32-S3 Pin |
|-----------|--------------|
| VCC | 3.3V |
| GND | GND |
| SCL | GPIO22 |
| SDA | GPIO21 |

#### 3.1.3 Other Temperature and Humidity Sensors

The system also supports the following temperature and humidity sensors:

- **DHT11**: Digital temperature and humidity sensor, single bus interface, lower accuracy but cheaper
- **DHT12**: Digital temperature and humidity sensor, I2C interface, suitable for space-limited applications
- **SHT21**: Digital temperature and humidity sensor, I2C interface, high accuracy
- **SHT40**: Digital temperature and humidity sensor, I2C interface, fast response speed
- **AM2302**: Waterproof version of DHT22, suitable for outdoor applications
- **HDC1080**: Digital temperature and humidity sensor, I2C interface, low power consumption
- **BME280**: Temperature, humidity and pressure sensor, I2C/SPI interface, suitable for weather prediction
- **BME680**: Temperature, humidity, pressure and gas sensor, I2C/SPI interface, rich features
- **HTU21D**: Digital temperature and humidity sensor, I2C interface, compatible with SHT21
- **SI7021**: Digital temperature and humidity sensor, I2C interface, compatible with SHT21

### 3.2 Human Presence Sensors

The system supports the following human presence sensors:

#### 3.2.1 HC-SR501 Sensor

| Parameter | Specification |
|-----------|---------------|
| Type | Passive infrared human presence sensor |
| Detection Distance | 3-7 meters |
| Detection Angle | 110° |
| Power Supply | 5V |
| Interface | Digital output |

**Connection Method:**
| HC-SR501 Pin | ESP32-S3 Pin |
|--------------|--------------|
| VCC | 5V |
| GND | GND |
| OUT | GPIO5 |

#### 3.2.2 Other Human Presence Sensors

- **HC-SR505**: Miniaturized human presence sensor, suitable for space-limited applications
- **RE200B**: High-sensitivity infrared sensor, suitable for long-distance detection
- **LD2410**: Millimeter wave radar human presence sensor, supports distance detection

### 3.3 Light Sensors

The system supports the following light sensors:

#### 3.3.1 BH1750 Sensor

| Parameter | Specification |
|-----------|---------------|
| Type | Digital light sensor |
| Measurement Range | 1-65535 lx |
| Accuracy | ±20% |
| Interface | I2C |
| Power Supply | 3.3V ~ 5V |
| I2C Address | 0x23 (default) or 0x5C |

**Connection Method:**
| BH1750 Pin | ESP32-S3 Pin |
|------------|--------------|
| VCC | 3.3V |
| GND | GND |
| SCL | GPIO22 |
| SDA | GPIO21 |

#### 3.3.2 Other Light Sensors

- **VEML6075**: UV light sensor, used to detect UV intensity
- **TSL2561**: Digital light sensor, supports wide dynamic range
- **GY30**: Digital light sensor, high accuracy, suitable for professional applications
- **SI1145**: Ambient light and UV sensor, rich features

### 3.4 Gas Sensors

The system supports the following gas sensors:

#### 3.4.1 MQ-135 Sensor

| Parameter | Specification |
|-----------|---------------|
| Type | Semiconductor gas sensor |
| Detected Gases | Formaldehyde, benzene, toluene, NH3, NOx and other harmful gases |
| Measurement Range | 10-1000 ppm |
| Interface | Analog output |
| Power Supply | 5V |

**Connection Method:**
| MQ-135 Pin | ESP32-S3 Pin |
|------------|--------------|
| VCC | 5V |
| GND | GND |
| A0 | GPIO34 |

#### 3.4.2 Other Gas Sensors

- **MQ-2**: Can detect smoke, gas, methane and other gases
- **MQ-5**: Can detect liquefied petroleum gas, methane, propane and other gases
- **MQ-7**: Can detect carbon monoxide gas
- **MQ-8**: Can detect hydrogen
- **TGS2600**: Can detect various volatile organic compounds

### 3.5 Flame Sensors

The system supports the following flame sensors:

#### 3.5.1 Infrared Flame Sensor

| Parameter | Specification |
|-----------|---------------|
| Type | Infrared flame sensor |
| Detection Distance | 0.1-1 meter (adjustable) |
| Detection Wavelength | 760-1100 nm |
| Interface | Digital output |
| Power Supply | 3.3V ~ 5V |

**Connection Method:**
| Flame Sensor Pin | ESP32-S3 Pin |
|------------------|--------------|
| VCC | 3.3V |
| GND | GND |
| OUT | GPIO35 |

#### 3.5.2 Other Flame Sensors

- **UV Flame Sensor**: Detects ultraviolet rays, suitable for detecting open flames
- **YG1006**: Infrared flame sensor, high sensitivity
- **MQ-2**: Can detect smoke and flames
- **TGS2600**: Can detect smoke generated by fire

## 4. Power Module Description

### 4.1 Lithium Battery

**Only supports soft-pack polymer batteries**, capacity selected according to needs:

- **Small size e-paper display (1.54-3.12 inches)**: 1000mAh ~ 3000mAh, soft-pack polymer battery
- **Medium size e-paper display (4.2-5.83 inches)**: 2000mAh ~ 5000mAh, soft-pack polymer battery
- **Large size e-paper display (6.0-7.5 inches)**: 4000mAh ~ 6000mAh, soft-pack polymer battery

### 4.2 Charging Module

**Only supports USB-Type-C charging**, recommended to use the following charging modules:

#### 4.2.1 Type-C Charging Module

| Parameter | Specification |
|-----------|---------------|
| Interface Type | USB-Type-C |
| Input Voltage | 5V DC |
| Charging Power | 10W-20W |
| Charging Current | 2A (adjustable) |
| Overcharge Protection | 4.2V ±1% |
| Overdischarge Protection | 3.0V |
| Short Circuit Protection | Supported |
| Reverse Connection Protection | Supported |

**Connection Method:**
| Type-C Charging Module Pin | Connection Object |
|----------------------------|-------------------|
| VCC | USB-Type-C input |
| GND | GND |
| B+ | Soft-pack polymer battery positive |
| B- | Soft-pack polymer battery negative |
| OUT+ | Output positive (connect to ESP32-S3's VIN or 3.3V) |
| OUT- | Output negative (connect to ESP32-S3's GND) |

#### 4.2.2 Other Recommended Charging Modules

- **TP4056+Type-C**: USB-Type-C interface, 10W charging, with charging protection
- **AXP192+Type-C**: USB-Type-C interface, 10W-15W charging, powerful functions

### 4.3 Power Management Module

Recommended to use AXP192+Type-C power management module, with the following features:

| Parameter | Specification |
|-----------|---------------|
| Interface Type | USB-Type-C |
| Input Voltage | 3.0V ~ 5.5V |
| Charging Power | 10W-20W |
| Output Channels | 6 DC-DC outputs, 2 LDO outputs |
| Maximum Output Current | 3A per channel |
| Power Consumption | <10mA in working mode, <10μA in deep sleep mode |
| Features | Supports power monitoring, charging management, power switch and other functions |
| Interface | I2C |

**Connection Method:**
| AXP192 Pin | ESP32-S3 Pin |
|------------|--------------|
| VIN | USB-Type-C input or soft-pack polymer battery positive |
| GND | GND |

### 4.4 Power Supply Instructions

- **Only supports USB-Type-C charging**, does not support DC direct plug-in power supply
- Charging power range: 10W-20W
- Working voltage: 3.3V
- Battery voltage: 3.3V-4.2V (soft-pack polymer battery)
| SCL | GPIO22 |
| SDA | GPIO21 |
| EXTEN | ESP32-S3's EN pin |
| VOUT2 | ESP32-S3's 3.3V pin |

## 5. Button Description

### 5.1 Button Type

Recommended to use tactile buttons, with the following features:

- Good operation feel
- Long life: Generally up to 100,000 times or more
- Small size: Easy to install
- Cheap price

### 5.2 Button Connection

Buttons use pull-up resistor connection method, connection diagram as follows:

```
ESP32-S3 GPIO --+-- [10KΩ resistor] --+ 3.3V
               |                  |
               +-- [tactile button] ----+ GND
```

**Connection Method:**
| Button Pin | ESP32-S3 Pin |
|------------|--------------|
| Button 1 | GPIO0 |
| Button 2 | GPIO1 |
| Button 3 | GPIO2 |
| Button 4 | GPIO3 |

Each button needs to be connected in series with a 10KΩ pull-up resistor.

## 6. Case Description

### 6.1 3D Printed Case

The system provides 3D printing model files, located in the `hardware/3d_models` directory, supporting the following sizes of e-paper displays:

- 1.54 inch
- 2.13 inch
- 2.9 inch
- 4.2 inch
- 7.5 inch

### 6.2 3D Printing Parameters

- **Material**: PLA or ABS
- **Layer Height**: 0.2mm
- **Infill Density**: 20% ~ 30%
- **Support**: Add according to model needs
- **Print Speed**: 50mm/s ~ 80mm/s

### 6.3 Purchase Finished Case

You can search for "ESP32 e-paper case" on platforms such as Taobao and JD.com, and choose the appropriate size and style.

## 7. Hardware Connection Example

### 7.1 Basic Connection Example

The following is a basic hardware connection example, using ESP32-S3-DevKitC-1 development board and 2.9 inch e-paper display:

| Component | Connection Relationship |
|-----------|--------------------------|
| ESP32-S3-DevKitC-1 | Main controller |
| 2.9 inch e-paper display | VCC→3.3V, GND→GND, SCK→GPIO18, MOSI→GPIO23, CS→GPIO5, DC→GPIO26, RST→GPIO27, BUSY→GPIO32 |
| DHT22 sensor | VCC→3.3V, GND→GND, DATA→GPIO4 |
| Lithium battery | Connected to ESP32-S3's VIN pin through TP4056 charging module |
| Buttons | 4 tactile buttons, connected to GPIO0, GPIO1, GPIO2, GPIO3 respectively, each button is connected in series with a 10KΩ pull-up resistor |

### 7.2 Extended Connection Example

The following is an extended hardware connection example, using ESP32-S3-Box development board and 7.5 inch e-paper display:

| Component | Connection Relationship |
|-----------|--------------------------|
| ESP32-S3-Box | Main controller, built-in screen, camera, microphone and other peripherals |
| 7.5 inch e-paper display | VCC→3.3V, GND→GND, SCK→GPIO18, MOSI→GPIO23, CS→GPIO5, DC→GPIO26, RST→GPIO27, BUSY→GPIO32 |
| SHT30 sensor | VCC→3.3V, GND→GND, SCL→GPIO22, SDA→GPIO21 |
| AXP192 power management module | VIN→lithium battery positive, GND→GND, SCL→GPIO22, SDA→GPIO21, VOUT2→ESP32-S3's 3.3V pin |
| Lithium battery | 5000mAh, connected to AXP192's VIN pin |
| BH1750 light sensor | VCC→3.3V, GND→GND, SCL→GPIO22, SDA→GPIO21 |

## 8. Hardware Testing

### 8.1 Power Testing

1. Use a multimeter to measure the voltage of the 3.3V and 5V pins of the development board to ensure stable voltage
2. Measure the voltage of the lithium battery to ensure sufficient battery power
3. Test the charging function to ensure the lithium battery can be charged normally

### 8.2 E-Paper Display Testing

1. Upload test code, test whether the e-paper display can display normally
2. Test the effect of different refresh modes (full refresh, partial refresh)
3. Test the display effect of different content (text, graphics, icons, etc.)

### 8.3 Sensor Testing

1. Upload test code, test whether the sensor can collect data normally
2. Test the accuracy and stability of the sensor
3. Test the performance of the sensor in different environments

### 8.4 Button Testing

1. Upload test code, test whether the button can respond normally
2. Test the sensitivity and reliability of the button
3. Test the debouncing effect of the button

## 9. Development Tools

### 9.1 Hardware Tools

- **USB Data Cable**: Used to connect development board and computer
- **Multimeter**: Used to measure voltage, current, resistance, etc.
- **Soldering Iron**: Used for soldering components
- **Hot Air Gun**: Used for soldering SMD components
- **Oscilloscope**: Used for debugging signals
- **Logic Analyzer**: Used for analyzing digital signals

### 9.2 Software Tools

- **Visual Studio Code**: Code editing and development environment
- **PlatformIO**: ESP32 development framework and toolchain
- **Arduino IDE**: ESP32 development environment
- **ESP-IDF**: Espressif official development framework
- **Serial Monitor**: Serial port debugging tool
- **Web Browser**: Used to access Web configuration page

## 10. Common Problem Solutions

### 10.1 Development Board Cannot Start

- Check whether the power connection is correct
- Check whether the lithium battery has power
- Check whether the reset pin is normal
- Check whether the firmware is uploaded correctly

### 10.2 E-Paper Display Cannot Display

- Check whether the e-paper display connection is correct
- Check whether the e-paper display driver is compatible
- Check whether the firmware supports this size of e-paper display
- Try restarting the device

### 10.3 Sensor Data Abnormal

- Check whether the sensor connection is correct
- Check whether the sensor power supply is normal
- Check whether the sensor driver is correct
- Try calibrating the sensor

### 10.4 Button No Response

- Check whether the button connection is correct
- Check whether the pull-up resistor is connected
- Check whether the button is damaged
- Check whether the button processing logic in the firmware is correct

### 10.5 Cannot Connect to WiFi

- Check whether the WiFi password is correct
- Check the WiFi signal strength
- Check whether the WiFi module is normal
- Try restarting the device

## 11. Resource Acquisition

1. ESP32-S3 datasheet：https://www.espressif.com.cn/en/products/socs/esp32-s3
2. E-paper display datasheet：Obtain from the manufacturer's website according to the specific model
3. 3D printing models：`hardware/3d_models` directory
4. Circuit diagram：`elec/电路设计.md`
5. Test code：`examples` directory

## 12. Notes

1. **ESD Protection**: When operating ESP32-S3 and e-paper display, pay attention to ESD protection to avoid damaging electronic components
2. **Power Safety**: When using lithium batteries, pay attention to prevent overcharging and overdischarging
3. **Pin Connection**: Ensure correct pin connection to avoid short circuits
4. **Soldering Quality**: Ensure good soldering quality to avoid false soldering and short circuits
5. **Heat Dissipation Design**: If the device has high power consumption, pay attention to heat dissipation design
6. **Waterproof Design**: If the device is used outdoors, pay attention to waterproof design
7. **EMC Design**: Pay attention to electromagnetic compatibility design to avoid interfering with other devices
8. **Reliability Design**: Pay attention to the reliability design of the device to improve the service life of the device