# 3D-PAWS-MKR-FullStation

## READMEs
### [Arduino Compile Notes](docs/CompileNotes.md)
### [Code Operation Notes](docs/CodeOperation.md)
### [Daily Reboot](docs/DailyReboot.md)
### [Modem Firmware Upgrading MKR NB 1500](docs/ModemFirmwareUpgrading.md)
### [Sensor Information](docs/Sensors.md)

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





