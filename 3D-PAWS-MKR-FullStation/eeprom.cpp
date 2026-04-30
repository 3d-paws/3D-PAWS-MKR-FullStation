/*
 * ======================================================================================================================
 *  eeprom.cpp - EEPROM Functions
 * ======================================================================================================================
 */
#include <Adafruit_EEPROM_I2C.h>
#include <RTClib.h>

#include "include/ssbits.h"
#include "include/qc.h"
#include "include/cf.h"
#include "include/main.h"
#include "include/output.h"
#include "include/sdcard.h"
#include "include/wrda.h"
#include "include/time.h"
#include "include/eeprom.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
EEPROM_NVM eeprom;
uint8_t *eeprom_ptr;

int  eeprom_address = 0x00;
bool eeprom_valid = false;
bool eeprom_exists = false;

Adafruit_EEPROM_I2C eeprom_i2c;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * EEPROM_ChecksumCompute()
 *=======================================================================================================================
 */
unsigned long EEPROM_ChecksumCompute() {
  unsigned long checksum=0;

  checksum += (unsigned long) eeprom.rgt1;
  checksum += (unsigned long) eeprom.rgp1;
  checksum += (unsigned long) eeprom.rgt2;
  checksum += (unsigned long) eeprom.rgp2;
  checksum += (unsigned long) eeprom.rgts;
  checksum += (unsigned long) eeprom.n2sfp;
  return (checksum);
}

/* 
 *=======================================================================================================================
 * EEPROM_ChecksumUpdate()
 *=======================================================================================================================
 */
void EEPROM_ChecksumUpdate() {
  eeprom.checksum = EEPROM_ChecksumCompute();
}

/* 
 *=======================================================================================================================
 * EEPROM_Valid()
 *=======================================================================================================================
 */
bool EEPROM_Valid() {
  unsigned long checksum = EEPROM_ChecksumCompute();

  if (checksum == eeprom.checksum) {
    if ((eeprom.rgt1 < 0.0) ||
        (eeprom.rgp1 < 0.0) ||
        (eeprom.rgt2 < 0.0) ||
        (eeprom.rgp2 < 0.0) ||
        (eeprom.n2sfp > (SD_n2s_max_filesz + 1000) )) {
      return (false);    
    }
    else {
      return (true);
    }
  }
  else {
    return (false);
  }
}

/* 
 *=======================================================================================================================
 * EEPROM_ClearRainTotals() - Reset to default values
 *                           Requires system clock to be valid
 *=======================================================================================================================
 */
void EEPROM_ClearRainTotals(uint32_t current_time) {
  if (STC_valid) {
    eeprom.rgt1 = 0.0;
    eeprom.rgp1 = 0.0;
    eeprom.rgt2 = 0.0;
    eeprom.rgp2 = 0.0;
    eeprom.rgts = current_time;
    eeprom.n2sfp = 0;
    EEPROM_ChecksumUpdate();
    eeprom_i2c.write(eeprom_address, eeprom_ptr, sizeof(eeprom));
  }
  else {
    Output(F("EEPROM CRT ERROR"));
  }
}

/* 
 *=======================================================================================================================
 * EEPROM_TimeToRollOver() - 
 *=======================================================================================================================
 */
bool EEPROM_TimeToRollOver() {
  if (RainEnabled() && eeprom_valid) {
    uint32_t current_time        = stc.getEpoch();
    uint32_t seconds_today       = current_time % 86400;
    uint32_t seconds_at_0000     = current_time - seconds_today;
    uint32_t seconds_at_rollover = seconds_at_0000 + (cf_rtro_hour * 3600) + (cf_rtro_minute * 60);

    // If no rain in 24 hours. Then rgts will be time of last rollover. 
    // Or rgts will be the eeprom initialized time.
    // if rgts is before rollover then we need to move today's totals to prior day
    // Work with in memory copy of eeprom rain gauge time stamp
    if ((current_time > seconds_at_rollover) && (eeprom.rgts <= seconds_at_rollover)) {
      Output ("RollOverTime");
      return (true);
    }
  }
  return (false);
}

/* 
 *=======================================================================================================================
 * EEPROM_Validate() - Check status of EEPROM information and determine status
 *                       Requires system clock to be valid
 *=======================================================================================================================
 */
void EEPROM_Validate() {

  if (!STC_valid) {
    return;
  }
  
  uint32_t current_time = stc.getEpoch();

  eeprom_i2c.read(eeprom_address, eeprom_ptr, sizeof(eeprom));

  if (!EEPROM_Valid()) {
    EEPROM_ClearRainTotals(current_time);
    if (SerialConsoleEnabled) {
      Output(F("EEPROM CLEARED:SCE")); // Serial Console Enabled
    }
    else {
      Output(F("EEPROM CLEARED:CSE")); // Checksum error
    }
  }
  else {
    uint32_t seconds_today                 = current_time % 86400;
    uint32_t seconds_at_0000               = current_time - seconds_today;
    uint32_t seconds_at_rollover           = seconds_at_0000 + (cf_rtro_hour * 3600) + (cf_rtro_minute * 60);
    uint32_t seconds_yesterday_at_rollover = seconds_at_rollover - 86400;

    // RT = Rain Total
    if ((current_time > seconds_at_rollover) && (eeprom.rgts > seconds_at_rollover)) {
      // If current time is after 6am and RT time is after 6am  - update RT time.
      Output("T>RO, RT>RO - OK");
      eeprom.rgts = current_time;
      EEPROM_ChecksumUpdate();
      eeprom_i2c.write(eeprom_address, eeprom_ptr, sizeof(eeprom));          
    }
    else if ((current_time > seconds_at_rollover) && (eeprom.rgts <= seconds_at_rollover) && (eeprom.rgts > seconds_yesterday_at_rollover)){
      // if current time is after 6am and RT time is before 6am and after yesterday at 6am -  move today's totals to yesterday
      if (eeprom.rgts > seconds_yesterday_at_rollover) {
        Output("T>RO, RT<=RO &&  RT>YRO- Move");   
        eeprom.rgp1 = eeprom.rgt1;
        eeprom.rgt1 = 0.0;
        eeprom.rgp2 = eeprom.rgt2;
        eeprom.rgt2 = 0.0;
        eeprom.rgts = current_time;
        EEPROM_ChecksumUpdate();
        eeprom_i2c.write(eeprom_address, eeprom_ptr, sizeof(eeprom));
      }
      else {
        // if current time is after 6am and RT time is before 6am and before yesterday at 6am - EEPROM has no valid data - clear EEPROM
        Output("T>RO, RT<YRO - Clear");
        EEPROM_ClearRainTotals(current_time);
      }
    }
    else { // current time is before 6AM
      // if current time is before 6am and RT time is before 6am and after yesterday at 6am - update RT time
      if (eeprom.rgts > seconds_yesterday_at_rollover) {
        Output("T<RO, RT<RO & RT>YRO - OK");
        eeprom.rgts = current_time;
        EEPROM_ChecksumUpdate();
        eeprom_i2c.write(eeprom_address, eeprom_ptr, sizeof(eeprom));          
      }
      else if (eeprom.rgts > (seconds_yesterday_at_rollover - 84600)) { 
        // if current time is before 6am and RT time after 6am 2 days ago - move current total to yesterday
        Output("T<RO, RT<RO && RT>RO-2d - Move");  
        eeprom.rgp1 = eeprom.rgt1;
        eeprom.rgt1 = 0.0;
        eeprom.rgp2 = eeprom.rgt2;
        eeprom.rgt2 = 0.0;
        eeprom.rgts = current_time;
        EEPROM_ChecksumUpdate();
        eeprom_i2c.write(eeprom_address, eeprom_ptr, sizeof(eeprom));
      }
      else {
        // if current time is before 6am and RT time before 6am 2 days ago - EEPROM has no valid data - clear EEPROM
        Output("T<RO, RT<RO && RT<=RO-2d - Clear");  
        EEPROM_ClearRainTotals(current_time);
      }
    }
  }
  eeprom_valid = true;
}


/* 
 *=======================================================================================================================
 * EEPROM_UpdateRainTotals() - 
 *=======================================================================================================================
 */
void EEPROM_UpdateRainTotals(float rgt1, float rgt2) {
  if (eeprom_valid) {
    bool update=false;

    uint32_t current_time        = stc.getEpoch();
    uint32_t seconds_today       = current_time % 86400;
    uint32_t seconds_at_0000     = current_time - seconds_today;
    uint32_t seconds_at_rollover = seconds_at_0000 + (cf_rtro_hour * 3600) + (cf_rtro_minute * 60);

    // If no rain in 24 hours. Then rgts will be time of last rollover. 
    // Or rgts will be the eeprom initialized time.
    // if rgts is before rollover then we need to move today's totals to prior day
    if ((current_time > seconds_at_rollover) && (eeprom.rgts <= seconds_at_rollover)) {
      eeprom.rgp1 = eeprom.rgt1;
      eeprom.rgt1 = 0;

      eeprom.rgp2 = eeprom.rgt2;
      eeprom.rgt2 = 0;
      update=true;
    }

    // We might of rolled rain total over in the above code.
    // If there is rain in the below code. Technically the rain could have occured in the prior day.
    // The rain has not been reported/transmitted yet. And when we do transmit it, the time will be after the 
    // rain total rollover time. So it has to go into the NEW daily total

    // {"at":"2026-03-24T23:14:16","rg":0.0,"rgt":7.6,"rgp":1.6
    // RollOverTime
    // {"at":"2026-03-24T23:15:02","rg":0.2,"rgt":0.2,"rgp":7.6

    // Add rain to the total if there is some.
    if (rgt1>0.0) {
      eeprom.rgt1 += rgt1;
      update=true;
    }
    if (rgt2>0.0) {
      eeprom.rgt2 += rgt2;
      update=true;
    }

    if (update) {
      eeprom.rgts = current_time;
      EEPROM_ChecksumUpdate();
      eeprom_i2c.write(eeprom_address, eeprom_ptr, sizeof(eeprom));
      Output(F("EEPROM RT UPDATED"));
    }
  }
}

/* 
 *=======================================================================================================================
 * EEPROM_SaveUnreportedRain() - 
 *=======================================================================================================================
 */
void EEPROM_SaveUnreportedRain() {
  if (raingauge1_interrupt_count || raingauge2_interrupt_count) {
    unsigned long rgds;     // rain gauge delta seconds, seconds since last rain gauge observation logged

    float rain1 = raingauge1_interrupt_count * 0.2;
    rgds = (millis()-raingauge1_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rain1 = (isnan(rain1) || (rain1 < QC_MIN_RG) || (rain1 > (((float)rgds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rain1;

    float rain2 = raingauge2_interrupt_count * 0.2;
    rgds = (millis()-raingauge2_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rain2 = (isnan(rain2) || (rain2 < QC_MIN_RG) || (rain2 > (((float)rgds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rain2;

    EEPROM_UpdateRainTotals(rain1, rain2);
  }
}

/* 
 *=======================================================================================================================
 * EEPROM_Update() - Check status of EEPROM information and determine status
 *=======================================================================================================================
 */
void EEPROM_Update() {
  if (eeprom_valid) {
    eeprom.rgts = stc.getEpoch();
    EEPROM_ChecksumUpdate();
    eeprom_i2c.write(eeprom_address, eeprom_ptr, sizeof(eeprom));
    Output(F("EEPROM UPDATED"));
  }
}

/* 
 *=======================================================================================================================
 * EEPROM_Dump() - 
 *=======================================================================================================================
 */
void EEPROM_Dump() {
  eeprom_i2c.read(eeprom_address, eeprom_ptr, sizeof(eeprom));

  unsigned long checksum = EEPROM_ChecksumCompute();

  Output(F("EEPROM DUMP"));

  sprintf (Buffer32Bytes, " RT1:%.2f", eeprom.rgt1); 
  Output (Buffer32Bytes);

  sprintf (Buffer32Bytes, " RP1:%.2f", eeprom.rgp1); 
  Output (Buffer32Bytes);

  sprintf (Buffer32Bytes, " RT2:%.2f", eeprom.rgt2); 
  Output (Buffer32Bytes);

  sprintf (Buffer32Bytes, " RP2:%.2f", eeprom.rgp2); 
  Output (Buffer32Bytes);

  sprintf (Buffer32Bytes, " RGTS:%lu", eeprom.rgts);
  Output (Buffer32Bytes);

  sprintf (Buffer32Bytes, " N2SFP:%lu", eeprom.n2sfp);
  Output (Buffer32Bytes);

  sprintf (Buffer32Bytes, " CS:%lu", eeprom.checksum);
  Output (Buffer32Bytes);

  sprintf (Buffer32Bytes, " CSC:%lu", checksum);
  Output (Buffer32Bytes);
}

/* 
 *=======================================================================================================================
 * EEPROM_initialize() - 
 *=======================================================================================================================
 */
void EEPROM_initialize() {
  eeprom_ptr = (uint8_t *) &eeprom;

  if (eeprom_i2c.begin(EEPROM_I2C_ADDR)) {
    Output(F("EEPROM OK"));
    eeprom_exists = true;
    // EEPROM_Validate(); // Have to do this later when we have a valid System Clock
    EEPROM_Dump();
  } else {
    Output(F("EEPROM NF"));
  }
}
