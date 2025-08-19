# 3D-PAWS-MKR-FullStation

## Overview
3D-PAWS-MKR-FullStation is a low-cost, open-source weather station firmware designed to run on Arduino MKR NB 1500 and MKR GSM 1400 boards. It is part of the 3D-PAWS (3D-Printed Automatic Weather Station) initiative developed by UCAR and the U.S. National Weather Service International Activities Office to provide accessible, reliable weather monitoring solutions for underserved regions using local materials and 3D printing.

This project enables environmental data collection—including temperature, humidity, atmospheric pressure, wind speed/direction, precipitation, and light irradiance—using a suite of low-cost sensors integrated with the MKR boards. It supports cellular communication for remote data transmission and uses an SD card for local data logging.

## Features
- Supports Arduino MKR NB 1500 and MKR GSM 1400 boards
- Interfaces with a variety of environmental sensors:
  - Temperature and relative humidity sensors
  - Barometric pressure sensor
  - Wind speed and wind direction sensors (3-cup anemometer and wind vane)
  - Tipping bucket rain gauge
  - Light sensors (visible, infrared, UV)
- Data logging to SD card via SPI interface
- Cellular communication over NB-IoT or GSM for remote monitoring
- Real-Time Clock (RTC) integration for accurate data timestamping
- Modular firmware architecture for easy customization and expansion
- Open-source and designed for local assembly with 3D-printed parts and common materials (PVC piping for frame)
- Targeted for deployment in remote, low-resource regions for environmental monitoring and early warning systems

## Hardware Setup

### Pin Usage and Connections
The firmware uses typical Arduino MKR pin assignments for sensors and peripherals. Key pin configurations include:

| Component           | Connection Type | Arduino MKR Pins               | Description                            |
|---------------------|-----------------|-------------------------------|------------------------------------|
| Temperature & Humidity Sensor | Digital / I2C     | Configured via sensor libraries (Usually I2C: SDA/SCL pins) | Measures ambient temperature and relative humidity |
| Barometric Pressure Sensor    | I2C               | SDA (pin 11) / SCL (pin 12)     | Provides atmospheric pressure readings |
| Wind Speed Sensor (Anemometer) | Digital          | Interrupt-enabled digital pin (e.g., D2) | Counts rotations for wind speed measurement |
| Wind Direction Sensor (Wind Vane) | Analog          | Analog Input pin (e.g., A0)      | Reads analog voltage corresponding to wind direction |
| Rain Gauge (Tipping Bucket)    | Digital           | Interrupt digital pin (e.g., D3) | Counts bucket tips to calculate precipitation |
| Light Sensors (Lux, UV)        | Analog             | Analog pins (e.g., A1, A2)       | Measures sunlight and UV light intensity |
| SD Card Module                | SPI                | SCK (pin 13), MOSI (pin 11), MISO (pin 12), CS (configurable, e.g., pin 10) | Data logging storage |
| Cellular Modem (NB1500 / GSM 1400) | Serial / Modem pins | Dedicated modem interface pins on MKR board | Cellular network connectivity |

Note: Specific pin numbers can vary and are typically defined in header or configuration files within the firmware codebase.

### Power Supply
- The system runs off 5V input.
- Battery operation is supported, often combined with solar power for remote deployments.
- The Arduino MKR board provides onboard voltage regulation and optionally LiPo battery charging.

## Software Setup

### Configuration Files
- Sensor calibration and sampling intervals are set in configuration header files or code constants.
- Cellular network credentials (APN, username, password) are defined in designated configuration sections of the firmware.
- SD card file management is handled programmatically to create timestamped data files in CSV format.
- Real-time clock (RTC) initialization and time zone offsets are also configurable within the firmware.

### SD Card Data Logging
- Uses SPI interface for communication with the SD card module.
- Logs environmental sensor data every defined interval (e.g., 1 minute).
- Stores data in CSV files with a timestamped filename format for easy tracking.
- Directory and file structure is managed to prevent overwriting and ensure data integrity.

### Cellular Communication
- Supports NB-IoT or GSM cellular modules integrated with the MKR boards.
- Configured for remote data transmission to cloud platforms or central servers.
- Handles network registration, connection monitoring, and retransmission in case of communication failures.

## Installation and Usage

### Prerequisites
- Arduino MKR NB 1500 or Arduino MKR GSM 1400 board
- Arduino IDE with board and library support installed
- Necessary sensor modules and correct wiring per pin assignments
- SD card module connected via SPI
- Cellular modem compatible with MKR board
- USB cable for programming and debugging
- Optional power supply (battery, solar panel)

### Setup Steps
1. Clone the repository:

