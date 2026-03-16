#define COPYRIGHT "Copyright [2026] [University Corporation for Atmospheric Research]"
#define VERSION_INFO "MKRFS-260312"  // MKR Full Station - Release Date

/*
 *======================================================================================================================
 * 3D-PAWS-MKR-FullStation
 *   Board Manager Package: Arduino SAMD Boards (32-bits ARM Cortex-M0+)
 *   Set Board Type to: Arduino MKR NB 1500
 *   Set Board Type to: Arduino MKR GSM 1400
 *   -*- mode: C++ -*-
 *
 * Author: Robert J Bubon
 * Version History
 *   2022-04-28 RJB Based on GSM_StreamGauge_V6
 *   2022-07-24 RJB All working but LoRa
 *   2022-08-01 RJB All working
 *   2022-08-13 RJB Added AlarmTimer to reset modem and reboot when doing HTTP connect calls
 *   2022-08-17 RJB Moved to MAX17043 fuel gauge requiring LoRa Pin changes for greating Wire1 
 *                  MAX17043 Library was modified to use Wire1 i2c pins
 *   2022-08-24 RJB LoRa Message handling modified to handle LoRa distance test messages and 
 *                  log to website
 *   2022-09-01 RJB Added code to reset modem and reboot after 60 loops with no network connection
 *                  Setting SystemStatusBits |= SSB_FROM_N2S when we add a entry to N2S file
 *                  Fixed skew on Observation time. Now 60s apart unless we have network issue and stuck
 *                    in connection handler calls.
 *   2022-09-13 RJB Removed cf_battery_size, not used anymore.
 *   2022-09-19 RJB Added checks on setting system time from network clock.
 *                  added timmers to cause a reset after 5m when calling conMan.connect
 *   2022-09-23 RJB Added support for 8 line OLEDs
 *   2022-09-26 RJB Added support for sim apn, username and password in ConfigurationSettings
 *   2022-09-29 RJB Went through the code and put conditional if (FG_exists) on fuel gauge before making calls
 *                  This is so we can run with out a FG connected. We just don't report Battery charge or powerdown
 *                  if LiPo is too low.
 *                  Added printing the Cell Provider info after a connect to the cell network
 *                  Limited where we do modem resets, other locations will do reboots
 *   2022-10-08 RJB Added configuration option "cf_enable_fuelgauge" to enable/disable the use of the fuel gauge
 *                  Can not trust the library to tell us if it is not connected to wire1 bus.
 *                  Reworked GetCellEpochTime() to do more checks and Ouput info, then reworked NetworkTimeManagement()
 *                    since GetCellEpochTime() is now doing the heavy lifting on the checks. Also reworked OBS_Send().
 *   2022-10-18 RJB Think SI1145 sensor was conflicting with on board ECC508 chip at i2c address 0x60. 
 *                    Moved SI1145 to Wire1 i2c bus.
 *   2022-10-25 RJB Set Float values to be 1/10 percision
 *   2022-11-07 RJB Can not use delay() in ISR so created a execution loop delay of 12s in the modem reset ISR
 *                  Added code to continue after 2 minutes when Wait For Serial Connection (W4SC set to true)
 *   2022-11-14 RJB Changed initiailize to initialize in function names
 *                  Set pinmode to output on LED_PIN in setup() 
 *   2022-11-22 RJB Switch to a Timed Relay that resets power to MKR and Sensors (aka ALL POWER)
 *                  Removed fuel gauge, disabled battery charger, stopped reporting these observations
 *                  Moved wind speed calc to use millis() to avoid delta time of 0 and divid by zero.
 *   2022-12-03 RJB Added Alarm arounf the get HTTP response after posting.
 *                  Modified the reconnect state to account for the 2 minutes spent in reconnect().
 *   2022-12-04 RJB In N2S fixed bug where we were not sending the right buffer.
 *                  Added Alarm Timer in function GetCellSignalStrength()
 *   2022-12-15 RJB Added NoNetworkLoopCycleCount++; When HTTP SEND fails
 *                  Added Alarm Handler around conMan.disconnect() when in loop() we are going to reboot
 *   2022-12-22 RJB Added a power pulse to turn modem on after reset in start()
 *   2022-12-27 RJB Added Timer around conMan.check() calls
 *                  Added reconnect from NetworkConnectionState::DISCONNECTED 
 *                  Alarm handler simpilified, It appears that REBOOT_PIN will only go high after we exit the handler.
 *   2023-02-05 RJB Added Support for reading and displaying the ECCX08 Crypto Chip Serial Number
 *   2023-02-08 RJB Added Support for HIH8, BMP390 and SHT31 sensors
 *   2023-02-21 RJB Changed polling timming waiting for http response from 100ms to 500ms checks for 3 minutes
 *   2023-03-14 RJB Added VEML7700 light sensor support, Removed SI1145, and 2nd i2c bus support
 *   2023-04-25 RJB Added (wind.sample_count>=60) support. If we don't have 60 new samples since last obs, we wait 
 *                        until we do before sending our next obs.
 *                  Added printing of Modem Firmware in Setup()
 *   2023-11-25 RJB Merge code base from Partice to support dynamic 4/8 line OLED displays
 *                  Added support for Air Quality Sensor PM25AQI
 *                  Fixed bug on status bit encoding SSB_PMIC = 0x4000, not 0x3000
 *   2024-03-25 RJB Removing support for cf_enable_fuelgauge.
 *                  Setting up so the CONFIG.TXT is the default
 *                  Shortened include file names
 *   2024-04-10 RJB Bug Fix with cf_sim_apn and cf_sim_username
 *   2024-07-15 RJB Added QC.h and modified OBS.h to use it
 *                  Added Derived observations
 *                  Added Copyright
 *   2024-07-28 RJB Added support of the MKR GSM 1400
 *                  Rebranded 3D-PAWS_MKRNB1500-FGLD to 3D-PAWS-MKR-FullStation
 *                  Moving to CONFIG.TXT resulted in modification of the Arduino_ConnectionHandler library
 *   2024-09-04 RJB Added time checks against TM_VALID_YEAR_START and TM_VALID_YEAR_END
 *   2024-10-07 RJB Improved hi_calculate() function.
 *   2024-11-05 RJB Discovered BMP390 first pressure reading is bad. Added read pressure to bmx_initialize()
 *                  Bug fixes for 2nd BMP sensor in bmx_initialize() using first sensor data structure
 *                  Now will only send humidity if bmx sensor supports it.
 *   2024-11-20 RJB Added SDU support. If UPDATE.bin exists on the SD card, Firmware is updated. File deleted.
 *
 *   2025-08-15 RJB Major Code Update to Match Particle FS Code
 *                  Added EP.h for EEPROM rain totals support library - Adafruit_FRAM_I2C
 *                  Data is now stored as JSON and converted to a GET URL for transmission to Chords.
 *                  Added clearing of EEPROM rain totals CRT.TXT. If file exists, rain totals cleared and file
 *                    removed.
 *   2025-09-07 RJB Maybe found the bug that hangs when sending observations. Reading the HTTP response
 *                  needed check for -1 and 5 second time out if no character read.
 *                  Fixed rain total eeprom initialization
 *   2025-09-11 RJB In OBS fixed casting bug on rain collection. Added (float)
 *                    (rain > (((float) rgds / 60) * QC_MAX_RG))
 *   2025-09-21 RJB Update all out of date libraries
 *
 *   2025-09-25 RJB Bug fix. Had obs tag names of length 6 bumped to 12.
 *
 *   2026-02-   RJB code cleanup
 *   2026-01-19 RJB Perplexity says to do a client.print(obs) not println; in OBS_Send()
 *   2026-02-26 RJB Removed the support for Tinovi MultiLevel Soil Moisture (4 Soil and 2 Temperature)
 *   2026-03-06 RJB added vbv and no wind support, keep reporting wind speed if direction as5600 fails
 *                  Adafruit_BusIO       1.17.3 -> 1.17.4
 *                  Adafruit_GFX_Library 1.12.2 -> 1.12.4
 *                  Adafruit_SSD1306     2.5.15 -> 2.5.16
 *                  ArduinoJson          7.4.2  -> 7.4.3
 *   2026-03-10 RJB Added DSMUX 1-Wire support for 8 temperature sensors dst0-7
 *   2026-03-12 RJB Added pinmode INPUT to wind rain OP1, and OP2
 *                  Added support to set rain total rollover
 * ======================================================================================================================
 */

/*
 *======================================================================================================================
 *  Note: The below 2 cases is where I have seen a reboot not resolving a modem problem.
 *        Resolution required removing of power (USB and Battery) to clean up the modem.
 *          We reset the modem after 5 minutes if we hang sending an observation.
 *          We reset the modem after 60 failed attempts to send an observation.
 *        A 15min Alarm to reboot (not reset) is placed is around conMan.connect() and conMan.check() functions.
 *
 * Terms
 *   Global System for Mobile Communication (GSM)
 *   General Packet Radio Service (GPRS)
 *   Access Point Name (APN)
 *   Data Logging Service (DLS)
 *
 * Arduino MKR NB 1500 Over the Telstra Narrowband Network (Cloud Solution)
 *   SEE https://create.arduino.cc/projecthub/diyode_magazine/arduino-mkr-nb-1500-over-the-telstra-narrowband-network-5b8853
 *   SEE https://docs.arduino.cc/hardware/mkr-nb-1500
 *   SEE https://forum.arduino.cc/t/mkr1500-no-longer-recognized/877901
 *
 * MKR NB1500 Board and Chips
 *   Arm® Cortex®-M0 32-bit SAMD21
 *      https://store-usa.arduino.cc/products/arduino-mkr-nb-1500?selectedStore=us
 *      https://content.arduino.cc/assets/mkr-microchip_samd21_family_full_datasheet-ds40001882d.pdf
 *   RTC Overview
 *        https://microchipdeveloper.com/32arm:samd21-rtc-overview
 *   uBlox SARA-R410M-02B module
 *       With the Arduino MKR NB 1500 and this library you can connect to the internet over a GSM network.
 *          GSM Library https://www.arduino.cc/en/Reference/MKRGSM
 *       The on board module operates in 4G, using LTE Cat M1 or NB1.
 *          https://www.arduino.cc/reference/en/libraries/mkrnb/
 *       Radio Access
 *          https://docs.arduino.cc/tutorials/mkr-nb-1500/setting-radio-access
 *          Tools > Manage libraries.., and search for MKRNB and install it.
 *       Download including AT Commands
 *          https://www.u-blox.com/en/product/sara-r4-series?legacy=Current
 *   ECC508 I2C Crypto Auth - I2C default value of 0xC0 But think it running at 0x60
 *     SEE https://github.com/arduino-libraries/ArduinoECCX08
 *     SEE https://www.arduino.cc/reference/en/libraries/arduinoeccx08/
 *   BQ24195L I2C Controlled 2.5-A /4.5-A SingleCell USB/AdapterCharger  I2C Address 0x6B
 *     SEE http://www.ti.com/lit/ds/symlink/bq24195.pdf
 *     SEE https://github.com/SmartTech/BQ24195 for the Arduino zip file library
 *     SEE https://www.arduino.cc/reference/en/libraries/arduino_bq24195/
 *   WatchDog Timer SAMD21have - We do not use this
 *     SEE https://github.com/gpb01/wdt_samd21
 *
 * MKR SD Proto Shield
 *   SEE https://store-usa.arduino.cc/products/mkr-sd-proto-shield
 *
 * RTC DS3231 I2C 0x68
 *   SEE https://www.adafruit.com/product/3013
 *
 * HiLetgo 2pcs DC 5V Trigger Time Delay Switch Relay Module Adjustable Time Delay
 *   SEE https://www.amazon.com/dp/B0832MW9L4
 *
 * NOYITO 1-Channel PC817 Optocoupler Isolation Module 3-5V
 *   SEE https://www.amazon.com/dp/B0B5373L4P
 * 
 * 1-Wire Bus Adapter1wire
 *   SEE https://github.com/adafruit/Adafruit_DS248x
 *       https://learn.adafruit.com/adafruit-ds2482s-800-8-channel-i2c-to-1-wire-bus-adapter
 * 
 * Twilio Super SIM
 * https://www.twilio.com/docs/iot/supersim/cellular-modem-knowledge-base/ublox-supersim#sara-r4-cat-m1-nb-iot
 *
 * EEPROM  https://www.adafruit.com/product/5146
 *
 * CHORDS Portal https://earthcubeprojects-chords.github.io/chords-docs/
 * ======================================================================================================================
 */

/*
 * ======================================================================================================================
 * Pin Definitions
 * 
 * Interrupt Pins 0, 1, 4, 5, 6, 7, 8, (16/A1), (17/A2)
 * A0   = Serial Console (Ground to Enable)
 * A1*  = Option Pin 1  
 * A2*  = Option Pin 2
 * A3   = LoRa SPI1 SPI1 MISO
 * A4   = LoRa Reset 
 * A5   = WatchDog Monitor/Relay Reset Trigger 
 * A6   = WatchDog Monitor Heartbeat
 * 
 * D0*  = Interrupt For Anemometer
 * D1*  = Interrupt For Rain gauge 1
 * D2   = LoRa SPI1 MOSI
 * D3   = LoRa SPI1 CLK
 * D4*  = SD Card Chip Select
 * D5*  = LoRa SS (Slave Select)
 * D6*  = On Board LED
 * D7*  = LoRa IRQ
 * D8*  = SPI0 MOSI - SD Card
 * D9   = SPI0 CLK  - SD Card
 * D10  = SPI0 MISO - SD Card
 * D11  = I2C SDA
 * D12  = I2C CLK
 * ======================================================================================================================
 */
                                  
/* 
 *=======================================================================================================================
 * Includes
 *=======================================================================================================================
 */
#include <SDU.h>  // Secure Digital Updater - UPDATE.bin Support
#include <Arduino_ConnectionHandler.h>  // Manage Cell Network Connection (Modified)

/* 
 *=======================================================================================================================
 * Local Includes
 *=======================================================================================================================
 */
#include "include/ssbits.h"         // System Status Bits
#include "include/qc.h"             // Quality Control Min and Max Sensor Values on Surface of the Earth
#include "include/mkrboard.h"       // MKR Related Board Functions and Definations
#include "include/support.h"        // Support Functions
#include "include/output.h"         // Serial and OLED Output Functions
#include "include/cf.h"             // Configuration File Variables
#include "include/eeprom.h"         // EEPROM Functions
#include "include/lora.h"           // LoRa Functions    
#include "include/sdcard.h"         // SD Card Functions
#include "include/time.h"           // Time Management Functions
#include "include/network.h"        // MKR modem network related functions
#include "include/wrda.h"           // Wind Rain Distance Air Functions
#include "include/mux.h"            // Mux Functions for mux connected sensors
#include "include/dsmux.h"          // Dallas One Wire Mux Functions 
#include "include/sensors.h"        // I2C Based Sensor Functions
#include "include/statmon.h"        // Station Monitor Functions
#include "include/obs.h"            // Observation Functions
#include "include/info.h"           // Info Functions
#include "include/main.h"

/*
 * ======================================================================================================================
 *  Globals
 * ======================================================================================================================
 */
bool JustPoweredOn = true;    // Used to clear SystemStatusBits set during power on device discovery
bool TurnLedOff = false;      // Set true in rain gauge interrupt

char versioninfo[sizeof(VERSION_INFO)];  // allocate enough space including null terminator
char msgbuf[MAX_MSGBUF_SIZE];   // Used to hold messages
char *msgp;                     // Pointer to message text
char Buffer32Bytes[32];         // General storage

unsigned long Time_of_obs = 0;         // unix time of observation
unsigned long Time_of_next_obs = 0;    // time of next observation in ms
unsigned long Time_of_last_hardreset=0;

// Local
int DailyRebootCountDownTimer;
unsigned long nextTimeRefresh=0;         // Time of Next Time refresh
unsigned long nextinfo=0;                // Time of Next INFO transmit

int DSM_countdown = 1800; // Exit Display Station Monitor screen when reaches 0 - protects against burnt out pin or forgotten jumper

/* 
 *=======================================================================================================================
 * obs_interval_initialize() - observation interval 1,2,5,6,10,15,20,30
 *=======================================================================================================================
 */
void obs_interval_initialize() {
  if ((cf_obs_period != 1) &&
      (cf_obs_period != 2) &&
      (cf_obs_period != 5) && 
      (cf_obs_period != 6) && 
      (cf_obs_period != 10) &&
      (cf_obs_period != 15) &&
      (cf_obs_period != 20) &&
      (cf_obs_period != 30)) {
    sprintf (Buffer32Bytes, "OBS Interval:%dm Now:1m", cf_obs_period);
    Output(Buffer32Bytes);
    cf_obs_period = 1; 
  }
  else {
    sprintf (Buffer32Bytes, "OBS Interval:%dm", cf_obs_period);
    Output(Buffer32Bytes);    
  }
}

/* 
 *=======================================================================================================================
 * time_to_next_obs() - This will return milliseconds to next observation
 *=======================================================================================================================
 */
unsigned long time_to_next_obs() {
  if (cf_obs_period == 1) {
    return (millis() + 60000); // Just go 60 seconds from now.
  }
  else {
    return ((cf_obs_period*60000) - (millis() % (cf_obs_period*60000))); // The mod operation gives us seconds passed
  }
}
/*
 * ======================================================================================================================
 * HeartBeat() - 
 * ======================================================================================================================
 */
void HeartBeat() {
  digitalWrite(HEARTBEAT_PIN, HIGH);
  delay(250);
  digitalWrite(HEARTBEAT_PIN, LOW);
}

/*
 * ======================================================================================================================
 * BackGroundWork() - Take Sensor Reading, Check LoRa for Messages, Delay 1 Second for use as timming delay            
 *                    Anything that needs sampling or to run every second add below.
 * ======================================================================================================================
 */
void BackGroundWork() {
  unsigned long OneSecondFromNow = millis() + 1000;
  
  ConnectionState = conMan->check();  
  NetworkTimeManagement();

  if ((cf_op1==OP1_STATE_DIST_5M)||(cf_op1==OP1_STATE_DIST_10M)) {
    DS_TakeReading();
  }

  if (!cf_nowind) {
    Wind_TakeReading();
  }

  if (PM25AQI_exists) {
    pm25aqi_TakeReading();
  }
  
  HeartBeat(); // Provides a 250ms delay
  
  if (LORA_exists) {
    lora_msg_poll(); // Provides a 750ms delay
  }

  unsigned long TimeRemaining = (OneSecondFromNow - millis());
  if ((TimeRemaining > 0) && (TimeRemaining < 1000)) {
    delay (TimeRemaining);
  }
  
  if (TurnLedOff) {   // Turned on by rain gauge interrupt handler
    digitalWrite(LED_PIN, LOW);  
    TurnLedOff = false;
  }
}

/*
 * ======================================================================================================================
 * setup()
 * ======================================================================================================================
 */
void setup() 
{
  pinMode (LED_PIN, OUTPUT);
  Output_Initialize();
  delay(2000); // prevents usb driver crash on startup, do not omit this

  analogReadResolution(12);  //Set all analog pins to 12bit resolution reads to match the SAMD21's ADC channels

  Serial_writeln(F(COPYRIGHT));
  strcpy(versioninfo, VERSION_INFO);
  Output (versioninfo);

  GetDeviceID();
  sprintf (msgbuf, "DevID:%s", DeviceID);
  Output (msgbuf);

  ECCX08_initialize();
  sprintf (Buffer32Bytes, "CryptoID:%s", CryptoID);
  Output(Buffer32Bytes);

  Output (F("REBOOTPN SET"));
  pinMode (REBOOT_PIN, OUTPUT); // By default all pins are LOW when board is first powered on. Setting OUTPUT keeps pin LOW.
  
  Output (F("HEARTBEAT SET"));
  pinMode (HEARTBEAT_PIN, OUTPUT);
  HeartBeat();

  // Initialize SD card if we have one.
  Output(F("SD:INIT"));
  SD_initialize();
  if (!SD_exists) {
    Output(F("!!!HALTED!!!"));
    while (true) {
      delay(1000);
    }
  }
  
  SD_ReadConfigFile();

  obs_interval_initialize();

  // INFO_Initialize();

  // Set Daily Reboot Timer
  DailyRebootCountDownTimer = cf_daily_reboot * 3600;

  // Refresh Time 
  nextTimeRefresh = millis() + (3600 * 4) * 1000; // 4 hours in the future

  // Connection Manager Initialize 
  CM_initialize();
  
  // Initialize System Time Clock
  Output(F("STC:INIT"));
  stc.begin(); 
  
  // Read RTC and set system clock if RTC clock valid
  Output(F("RTC:INIT"));
  rtc_initialize();

  EEPROM_initialize();

  obs_interval_initialize();

  INFO_Initialize();

  // NOTE: If no RTC or it is Invalid we have no system clock at this point

  // Power Management IC (bq24195)
  Output(F("PMIC:INIT"));
  pmic_initialize();

  // Optipolar Hall Effect Sensor SS451A - Rain1 Gauge
  if (cf_rg1_enable) {
    pinMode(RAINGAUGE1_IRQ_PIN, INPUT);
    raingauge1_interrupt_count = 0;
    raingauge1_interrupt_stime = millis();
    raingauge1_interrupt_ltime = 0;  // used to debounce the tip
    attachInterrupt(RAINGAUGE1_IRQ_PIN, raingauge1_interrupt_handler, FALLING);
    Output (F("RG1:ENABLED"));
  }
  else {
    Output (F("RG1:NOT ENABLED"));
  }

  // Optipolar Hall Effect Sensor SS451A - Rain2 Gauge
  if (cf_op1 == OP1_STATE_RAIN) {
    pinMode(RAINGAUGE2_IRQ_PIN, INPUT);
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = millis();
    raingauge2_interrupt_ltime = 0;  // used to debounce the tip
    attachInterrupt(RAINGAUGE2_IRQ_PIN, raingauge2_interrupt_handler, FALLING);
    Output (F("RG2:ENABLED"));
  }
  else {
    Output (F("RG2:NOT ENABLED"));
  }

  // I2C Sensors

  if (cf_nowind) {
    Output (F("WIND:DISABLED"));
  }
  else {
    Output (F("WIND:ENABLED"));
    as5600_initialize();
    // Optipolar Hall Effect Sensor SS451A - Wind Speed
    pinMode(ANEMOMETER_IRQ_PIN, INPUT);
    anemometer_interrupt_count = 0;
    anemometer_interrupt_stime = millis();
    attachInterrupt(ANEMOMETER_IRQ_PIN, anemometer_interrupt_handler, FALLING);
  }
  
  // Scan for i2c Devices and Sensors
  mux_initialize();
  if (!MUX_exists) {
    tsm_initialize(); // Check main bus
  }

  // Scan Dallas 1-Wire Mux for temperature sensors
  dsmux_initialize();

  bmx_initialize();
  htu21d_initialize();
  mcp9808_initialize();
  sht_initialize();
  hih8_initialize();
  lux_initialize();
  pm25aqi_initialize();
  hdc_initialize();
  lps_initialize();

  // Tinovi Leaf Mositure Sensor
  tlw_initialize();

  // Derived Observations
  wbt_initialize();
  hi_initialize();
  wbgt_initialize();
  mslp_initialize();

  Output(F("CM:CHECK"));
  // When not connected to a cellular network, conMan.check(); may hang or block for a long time because it internally 
  // waits for network registration or state changes that can take a significant timeout period on NB-IoT modems like 
  // the MKR NB 1500. This behavior has been reported by users experiencing long delays, sometimes many seconds or even 
  // minutes, during failed network attempts or no coverage situations.
  ConnectionState = conMan->check();
  Output(F("CM:CHECK AFTER"));
  WaitForNetworkConnection(2); // Let wait on the network to come online

  // Initialize RH_RF95 LoRa Module
  lora_initialize();
  lora_device_initialize();
  lora_msg_check();
 
  PrintModemIMEI();
  PrintModemFW();
  PrintCellSignalStrength();
  
  nextinfo = millis() + 60000; // Give Network some time to connect - ignore config setting here.

  Output (F("Start Main Loop"));
  Time_of_next_obs = millis() + 60000; // Give Network some time to connect - ignore config setting here.

  if (RTC_valid && !cf_nowind) {
    Wind_Distance_Air_Initialize(); // Will call HeartBeat()
  }
}

/*
 * ======================================================================================================================
 * loop() - This is now the rtos idle loop. No rtos blocking functions allowed!
 * 
 * This should be empty under freeRTOS. Under freeRTOS void loop() is used as a clean up object and runs when the task 
 * manager says there is nothing else to do.
 * ======================================================================================================================
 */
// int c = 0; // Low power testing
void loop() 
{
  BackGroundWork();
  
  // If Serial Console Pin LOW then Display Station Information
  // if (0 && DSM_countdown && digitalRead(SCE_PIN) == LOW) { //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< REMOVE 0 for production!!!!!!!!!
  if (DSM_countdown && digitalRead(SCE_PIN) == LOW) {
    StationMonitor();
    DSM_countdown--;
  }
  else { // Normal Operation - Main Work

    // This will be invalid if the RTC was bad at poweron and we have not connected to Cell network
    // Upon connection to cell network system time is set and this becomes valid.
    if (STC_valid) {

      // Delayed initialization. We need a valid clock before we can validate the EEPROM
      if (eeprom_exists && !eeprom_valid) {
        EEPROM_Validate();
        EEPROM_Dump();
        SD_ClearRainTotals(); 
      }

      // Send INFO
      if (millis() >= nextinfo) {      // Upon power on this will be true with nextinfo being zero
        INFO_Do();   // function will set nextinfo time for next call
      }
      
      if (millis() >= Time_of_next_obs) {
        Time_of_obs = stc.getEpoch(); // Update time we last sent (or attempted to send) observations.      
        OBS_Do();  // Here is why we are here 
        
        // Time since last Clock update, Countdown to daily reboot, Loop count with no network
        sprintf (Buffer32Bytes, "LOOP %u:%d:%d:%d", Time_of_obs-LastTimeUpdate, DailyRebootCountDownTimer, NoNetworkLoopCycleCount, OBS_PubFailCnt);
        Output (Buffer32Bytes);

        Time_of_next_obs = time_to_next_obs();
        JPO_ClearBits(); // Clear status bits from boot after we log our first observations
      }
    }

    // Check to see if we should reset after so many loops with out a network connection
    if (NoNetworkLoopCycleCount >= cf_no_network_reset_count) {
      Output(F("NW TimeOut:Reboot"));
      delay (5000);
      conMan->disconnect();  // Disconnect calls NB.shutdown() which calls send("AT+CPWROFF")       
      digitalWrite(REBOOT_PIN, HIGH);
      // Should not get here
      delay (1000);
      digitalWrite(REBOOT_PIN, LOW);
      NoNetworkLoopCycleCount = 0; // Reset count incase reboot fails
    }

    // Check to see if we should reset the modem after so many publish fails
    if (OBS_PubFailCnt >= PUB_FAILS_BEFORE_ACTION) {
      Output(F("Publish Fail - Resetting Modem!"));
      modem.hardReset();

      Output(F("Publish Fail - Waiting 12s"));
      delay(12000); // Give time for modem to reset

      Time_of_last_hardreset = millis();

      Output(F("Publish Fail - Calling Connect"));
      conMan->connect(); // Try some modem recovery, so next call to check() we will run init() and start connection process all over
      OBS_PubFailCnt = 0; // Reset count incase reboot fails
    }
    
    // Default = Reboot Boot Once a Day 60sec x 60min x 24hrs = 86400 seconds/day
    // Not using time but a loop counter.
    if (--DailyRebootCountDownTimer<=0) {
      // Lets not rip the rug out from the modem. Do a graceful shutdown.
      Output (F("Daily Reboot"));
      delay (5000);  
      conMan->disconnect();  // Disconnect calls NB.shutdown() which calls send("AT+CPWROFF")        
      digitalWrite(REBOOT_PIN, HIGH);
      // Should not get here
      delay (5000);
      Output (F("Rebooting"));
      digitalWrite(REBOOT_PIN, LOW);
      DailyRebootCountDownTimer = cf_daily_reboot * 3600; // Reset count incase reboot fails
    }
  }

  // Error State should of reconnected us - But try again below.
  if (ConnectionState == NetworkConnectionState::ERROR) {
    sprintf (Buffer32Bytes, "NWSE DR%d:NL%d:%s", DailyRebootCountDownTimer, NoNetworkLoopCycleCount, (STC_valid) ?"STC-OK" : "STC-!OK");
    
    NoNetworkLoopCycleCount += 2; // If we are stuck not connecting then lets speed up time to reboot
                                  // because the reconnect has a 2 minute delay before returning

    // If reboot fails then try again talking to the modem.
    conMan->connect();
  }
}
