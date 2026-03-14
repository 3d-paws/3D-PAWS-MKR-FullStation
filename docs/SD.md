# SD Card Information
[←Top](../README.md)<BR>
## Supported Card Sizes
- **Size:** 32 GB or smaller  
  *Note: Larger SD cards will not be detected.*

---

## Formatting Tool
- **Recommended Tool:** [SD Memory Card Formatter by Tuxera](https://www.sdcard.org/downloads/formatter/)  
- **Format Type:** MS-DOS (FAT32)

## File and Directory Overview

| File/Directory    | Purpose                                                                                 |
|-------------------|-----------------------------------------------------------------------------------------|
| `/OBS/`           | Directory containing observation files.                                                 |
| `/OBS/20231024.LOG` | Daily observation file in JSON format (one file per day).                             |
| `/N2SOBS.TXT`     | "Need to Send" file storing unsent observations. Resets if larger than specified size.  |
| `/SIM.TXT`        | Support file for third-party SIM configurations.                                        |
| `/WIFI.TXT`       | Stores WiFi configuration information for Argon WiFi and Muon boards.                   |
| `/INFO.TXT`       | Station info file. Overwritten with every INFO call.                                    |
| `/ELEV.TXT`       | Station elevation. File contents is elevation in meters.                                |
| `/RTRO.TXT`       | Set Rain Total Rollover Offset from 0 UTC (hours).                                      |
|||
| *(no of the below)* | Sets transmit interval to 15 minutes with 1 Minute Observations *(DEFAULT)*           |
| `/TXI5M.TXT`      | Sets transmit interval to 5 minutes with 1 Minute Observations                          |
| `/TXI10M.TXT`     | Sets transmit interval to 10 minutes with 1 Minute Observations                         |
| `/OBI5M.TXT`      | Sets 5 Minute Observations, Transmit Interval to 5 Minutes                              |
| `/OBI10M.TXT`     | Sets 10 Minute Observations, Transmit Interval to 10 Minutes                            |
| `/OBI15M.TXT`     | Sets 15 Minute Observations, Transmit Interval to 15 Minutes                            |
|||
| `/NORAIN.TXT`     | Disable main rain gauge observations.                                                   |
| `/NOWIND.TXT`     | Disable wind observations.                                                              |
| `/OPTAQS.TXT`     | Enable Air Quality Station.                                                             |

---


## Hardware Pin Configuration

| Device           | OP1 Pin | OP2 Pin |
|------------------|---------|---------|
| Argon / Boron    | A4      | A5      |
| Muon             | A0      | A2      |

### Special Configuration Files for Pins

| File Name       | Description                                                 |
|-----------------|-------------------------------------------------------------|
| `/OP1DIST.TXT`  | Configures OP1 for a 10m distance gauge.                    |
| `/OP1D5M.TXT`   | Configures OP1 for a 5m sensor (requires OP1DIST.TXT).      |
| `/OP1RAIN.TXT`  | Configures OP1 for a second rain gauge.                     |
| `/OP1RAW.TXT`   | Configures OP1 for raw readings (avg. 5 samples spaced 10ms).|
| `/OP2RAW.TXT`   | Configures OP2 for raw readings (avg. 5 samples spaced 10ms).|


