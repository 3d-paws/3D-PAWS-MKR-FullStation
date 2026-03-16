/*
 * ======================================================================================================================
 * mkrboard.cpp - MKR Related Board Functions and Definations
 * ======================================================================================================================
 */
#include <ArduinoECCX08.h>      // Crypto Chip
#include <Arduino_PMIC.h>       // Arduino_BQ24195-master 

#include "include/mkrboard.h"
#include "include/output.h"
#include "include/main.h"

char DeviceID[25]; // A generated ID based on board's 128-bit serial number converted down to 96bits
char CryptoID[19]; // Crypto Chip ID, Exactly 9 bytes from ECCX08, AKA 18 bytes HEX + NULL
bool ECCX08_Exists=false;
bool PMIC_exists = false;


const char* pinNames[] = {
  "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "D8", "D9", "D10", "D11", "D12", "D13", "D14",
  "A0", "A1", "A2", "A3", "A4", "A5", "A6"
};

const char *batterystate[] = {"UNKN", "!CHARGING", "CHARGING", "CHARGED", "DISCHARGING", "FAULT", "MISSING"};

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/*
 * ======================================================================================================================
 * GetDeviceID() - 
 * 
 * This function reads the 128-bit (16-byte) unique serial number from the 
 * SAMD21 microcontroller's flash memory, compresses it to 12 bytes, and 
 * converts it to a 24-character hexadecimal string.
 * 
 * The compression is done by XORing the first 12 bytes with the last 12 bytes 
 * of the original 16-byte identifier, ensuring all 128 bits contribute to 
 * the final result.
 * 
 * SAM D21/DA1 Family Datas Sheet
 * 10.3.3 Serial Number
 * Each device has a unique 128-bit serial number which is a concatenation of four 32-bit words contained at the
 * following addresses:
 * Word 0: 0x0080A00C
 * Word 1: 0x0080A040
 * Word 2: 0x0080A044
 * Word 3: 0x0080A048
 * The uniqueness of the serial number is guaranteed only when using all 128 bits.
 * ======================================================================================================================
 */
void GetDeviceID() {
  // Pointer to the unique 128-bit serial number in flash
  uint32_t *uniqueID = (uint32_t *)0x0080A00C;
  
  // Temporary buffer to store 16 bytes (128 bits)
  uint8_t fullId[16];
  
  // Extract all 16 bytes from the 128-bit serial number
  for (int i = 0; i < 4; i++) {
    uint32_t val = uniqueID[i];
    fullId[i*4] = (val >> 24) & 0xFF;
    fullId[i*4 + 1] = (val >> 16) & 0xFF;
    fullId[i*4 + 2] = (val >> 8) & 0xFF;
    fullId[i*4 + 3] = val & 0xFF;
  }

  // Compress 16 bytes to 12 bytes using XOR
  uint8_t compressedId[12];
  for (int i = 0; i < 12; i++) {
    compressedId[i] = fullId[i] ^ fullId[i + 4];
  }

  memset (DeviceID, 0, 25);
  for (int i = 0; i < 12; i++) {
    sprintf (DeviceID+strlen(DeviceID), "%02x", compressedId[i]);
  }
  DeviceID[24] = '\0'; // Ensure null-termination
}

/*
 * ======================================================================================================================
 * ECCX08_initialize() - 
 * ======================================================================================================================
 */
void ECCX08_initialize() {
  if (ECCX08.begin()) {
    byte CID[9]; // Crypto Chip ID, Exactly 9 bytes from ECCX08
    if (ECCX08.serialNumber(CID)) {
      sprintf (CryptoID, "%02X%02X%02X%02X%02X%02X%02X%02X%02X", 
        CID[0],CID[1],CID[2],CID[3],CID[4],CID[5],CID[6],CID[7],CID[8]);
      ECCX08_Exists = true;
      return;
    }
  }
  sprintf (CryptoID, "NF");
  ECCX08_Exists = false;
}

/*
 * ======================================================================================================================
 *  PowerManagement LiPo Battery Monitor and PMIC 
 *  
 *  PMIC bq24195 is on board the MKR1500
 *  
 *  FROM: Arduino15/packages/arduino/hardware/samd/1.8.13/variants/mkrnb1500/
 *  FILE variant.h
 *  #define ADC_BATTERY  (32u)
 *  
 *  FILE: variant.cpp
 *  #if defined(USE_BQ24195L_PMIC)
 *  pinMode(ADC_BATTERY, OUTPUT);
 *  digitalWrite(ADC_BATTERY, LOW);
 *  delay(10);
 *  pinMode(ADC_BATTERY, INPUT);
 *  delay(100);
 *
 *  bool batteryPresent = analogRead(ADC_BATTERY) > 600;
 * if (batteryPresent) {
 *   enable_battery_charging();
 * }
 * disable_battery_fet(!batteryPresent);
 * set_voltage_current_thresholds();
 * #endif
 * ======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * get_batterystate()
 *=======================================================================================================================
 */
byte get_batterystate() {
  bool power_good;
  int  charge_status;
  byte chrg_stat;

  /* 
  8.5.1.9 System Status Register REG08 - Table 15. REG08 System Status Register Description
  Bit 7 VBUS_STAT[1] R 00 – Unknown (no input, or DPDM detection incomplete), 01 – USB host, 10 – Adapter
  Bit 6 VBUS_STAT[0] R port, 11 – OTG
  
  Bit 5 CHRG_STAT[1] R 00 – Not Charging, 01 – Pre-charge (<VBATLOWV), 10 – Fast Charging, 11 – Charge
  Bit 4 CHRG_STAT[0] R Termination Done
  
  Bit 3 DPM_STAT R 0 – Not DPM, 1 – VINDPM or IINDPM
  Bit 2 PG_STAT R 0 – Not Power Good, 1 – Power Good
  Bit 1 THERM_STAT R 0 – Normal, 1 – In Thermal Regulation
  Bit 0 VSYS_STAT R 0 – Not in VSYSMIN regulation (BAT > VSYSMIN), 1 – In VSYSMIN regulation (BAT <VSYSMIN)
   */

  charge_status = PMIC.chargeStatus();
  if (charge_status == -1) {
    return(BATTERY_STATE_UNKNOWN);
  }
  power_good = PMIC.isPowerGood();              // true if on USB
  chrg_stat = (charge_status & 0x30) >> 4;
  switch (chrg_stat) {
    case 0 : return(BATTERY_STATE_NOT_CHARGING); // Not Charging
    case 1 : return(BATTERY_STATE_CHARGING);     // Pre-charge (<VBATLOWV) - On USB Power
    case 2 : return(BATTERY_STATE_CHARGING);     // Fast Charging          - On USB Power
    case 3 :                                     // Charged
      if (power_good) {
        return(BATTERY_STATE_CHARGED); // On USB Power
      }
      else {
        return(BATTERY_STATE_DISCHARGING);  // Discharching so no usb power and we must be on battery
      }
  }
  return(BATTERY_STATE_UNKNOWN);
}

/* 
 *=======================================================================================================================
 * pmic_initialize() - BQ24195
 * SEE https://github.com/particle-iot/device-os  
 *            device-os-develop/wiring/src/spark_wiring_power.cpp
 *            
 * https://support.arduino.cc/hc/en-us/articles/360016119199-What-is-the-meaning-of-CHRG-LED-different-states-in-MKR-boards-          
 * LED STAT Pin State
 *   Charging in progress (including recharge) = LOW
 *   Charging complete                         = HIGH
 *   Sleep mode, charge disable                = HIGH
 *   Charge suspend (Input over-voltage, TS fault, timer fault, input or system over-voltage) =  blinking at 1Hz
 *=======================================================================================================================
 */
bool pmic_initialize() {
  if (!PMIC.begin()) {
    Output (F("PM:INIT FAILED"));
  }
  else {
    PMIC_exists = true;
    Output (F("PM:FOUND"));

    // No Battery connected so turn off the charger
    if (PMIC.disableCharge()) {
      Output (F("PM:CHRGR:DISABLED"));
    }
    else {
      Output (F("PM:CHRGR:NOTDISABLED"));
    }

    sprintf (msgbuf,"PM:MSV=%.2f", PMIC.getMinimumSystemVoltage());  // Factory Default is 3.5
    Output (msgbuf);
    // PMIC.setMinimumSystemVoltage(3.0);
    // sprintf (msgbuf,"PM:SMSV=%.2f", PMIC.getMinimumSystemVoltage());
    // Output (msgbuf);
    
    sprintf (msgbuf,"PM:BS=%d", get_batterystate());
    Output (msgbuf);
    sprintf (msgbuf,"PM:PS=%s", (PMIC.isPowerGood()) ? "GOOD" : "BAD");
    Output (msgbuf);
  }
  return (PMIC_exists);
}

