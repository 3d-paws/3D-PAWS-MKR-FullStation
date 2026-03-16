/*
 * ======================================================================================================================
 *  obs.cpp - Observation Reporting Functions
 * ======================================================================================================================
 */

#include "include/qc.h"
#include "include/ssbits.h"
#include "include/mkrboard.h"
#include "include/eeprom.h"
#include "include/network.h"
#include "include/lora.h"
#include "include/wrda.h"
#include "include/cf.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/mux.h"
#include "include/dsmux.h"
#include "include/support.h"
#include "include/time.h"
#include "include/sensors.h"
#include "include/main.h"
#include "include/obs.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures
 * =======================================================================================================================
 */
OBSERVATION_STR obs;
char obsbuf[MAX_OBS_SIZE];      // Url that holds observations for HTTP GET
char *obsp;                     // Pointer to obsbuf

int OBS_PubFailCnt = 0;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 * OBS_Clear() - Set OBS to not in use
 * ======================================================================================================================
 */
void OBS_Clear() {
  obs.inuse =false;
  for (int s=0; s<MAX_SENSORS; s++) {
    obs.sensor[s].inuse = false;
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_Add() - Save OBS to N2S file
 * ======================================================================================================================
 */
void OBS_N2S_Add() {
  if (obs.inuse) {     // Sanity check
    char ts[32];
   
    memset(obsbuf, 0, sizeof(obsbuf));

    sprintf (obsbuf, "{");
    sprintf (obsbuf+strlen(obsbuf), "\"key\":\"%s\"", cf_apikey); 
    sprintf (obsbuf+strlen(obsbuf), ",\"instrument_id\":%d", cf_instrument_id);

    tm *dt = gmtime(&obs.ts);

    sprintf (obsbuf+strlen(obsbuf), ",\"at\":\"%d-%02d-%02dT%02d%:%02d%:%02d\"",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);
      
    sprintf (obsbuf+strlen(obsbuf), ",\"css\":%d", obs.css);
    obs.hth |= SSB_FROM_N2S; // Turn On Bit - Modify System Status and Set From Need to Send file bit
    sprintf (obsbuf+strlen(obsbuf), ",\"hth\":%d", obs.hth);

    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output (F("WhyAmIHere?"));
            break;
        }
      }
    }
    sprintf (obsbuf+strlen(obsbuf), "}");

    Serial_writeln (obsbuf);
    SD_NeedToSend_Add(obsbuf); // Save to N2F File
    Output(F("OBS-> N2S"));
  }
  else {
    Output(F("OBS->N2S OBS:Empty"));
  }
}

/*
 * ======================================================================================================================
 * OBS_Build_JSON() - Create observation in obsbuf for sending to Chords 
 * 
 * Example {"key":1234,"instrument_id":53,"at":"2022-05-17T17:40:04","hth":8770}
 * ======================================================================================================================
 */
bool OBS_Build_JSON() {   
  if (obs.inuse) {     // Sanity check  
    memset(obsbuf, 0, sizeof(obsbuf));

    // Save the Observation in JSON format
    
    sprintf (obsbuf, "{");
    sprintf (obsbuf+strlen(obsbuf), "\"key\":\"%s\"", cf_apikey); 
    sprintf (obsbuf+strlen(obsbuf),",\"devid\":\"%s\"", DeviceID);
    sprintf (obsbuf+strlen(obsbuf), ",\"instrument_id\":%d", cf_instrument_id);

    tm *dt = gmtime(&obs.ts); 
    
    sprintf (obsbuf+strlen(obsbuf), ",\"at\":\"%d-%02d-%02dT%02d%:%02d%:%02d\"",
      dt->tm_year+1900, dt->tm_mon+1,  dt->tm_mday, dt->tm_hour, dt->tm_min, dt->tm_sec);

    sprintf (obsbuf+strlen(obsbuf), ",\"css\":%d", obs.css);
    sprintf (obsbuf+strlen(obsbuf), ",\"hth\":%d", obs.hth);
    
    for (int s=0; s<MAX_SENSORS; s++) {
      if (obs.sensor[s].inuse) {
        switch (obs.sensor[s].type) {
          case F_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%.1f", obs.sensor[s].id, obs.sensor[s].f_obs);
            break;
          case I_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%d", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          case U_OBS :
            sprintf (obsbuf+strlen(obsbuf), ",\"%s\":%u", obs.sensor[s].id, obs.sensor[s].i_obs);
            break;
          default : // Should never happen
            Output (F("WhyAmIHere?"));
            break;
        }
      }
    }
    sprintf (obsbuf+strlen(obsbuf), "}");

    Output(F("OBS->URL"));
    Serial_writeln (obsbuf);
    return (true);
  }
  else {
    Output(F("OBS->JSON OBS:Empty"));
    return (false);
  }
}

/*
 * ======================================================================================================================
 * OBS_N2S_Save() - Save Observations to Need2Send File
 * ======================================================================================================================
 */
void OBS_N2S_Save() {

  // Save Station Observations to N2S file
  OBS_N2S_Add();
  OBS_Clear();

  // Save Rain and Soil LoRa Observations to N2S file
  while (lora_relay_need2log()) {
    lora_relay_build_JSON(); // Copy JSON observation to obsbuf, remove from relay structure
    SD_NeedToSend_Add(obsbuf); // Save to N2F File
    Output(F("LR->N2S"));
    Serial_write (obsbuf); 
  }
}

/*
 * ======================================================================================================================
 * OBS_Take() - Take Observations - Should be called once a minute - fill data structure
 * ======================================================================================================================
 */
void OBS_Take() {
  int sidx = 0;;
  float rg1 = 0.0;
  float rg2 = 0.0;
  unsigned long rg1ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  unsigned long rg2ds;   // rain gauge delta seconds, seconds since last rain gauge observation logged
  float ws = 0.0;
  int wd = 0;
  float mcp3_temp = 0.0;  // globe temperature
  float wetbulb_temp = 0.0;
  float sht1_humid = 0.0;
  float sht1_temp = 0.0;
  float heat_index = 0.0;
  float bmx_1_pressure = 0.0;

  // Safty Check for Vaild Time
  if (!STC_valid) {
    Output (F("OBS_Take: Time NV"));
    return;
  }
  
  OBS_Clear(); // Just do it again as a safty check

  Wind_GustUpdate(); // Update Gust and Gust Direction readings

  obs.inuse = true;
  obs.ts = Time_of_obs;     // Set in main loop no need to call stc.getEpoch();
  obs.css = GetCellSignalStrength();
  obs.hth = SystemStatusBits;

  // Rain Gauge 1 - Each tip is 0.2mm of rain
  if (cf_rg1_enable) {
    rg1ds = (millis()-raingauge1_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rg1 = raingauge1_interrupt_count * 0.2;
    raingauge1_interrupt_count = 0;
    raingauge1_interrupt_stime = millis();
    raingauge1_interrupt_ltime = 0; // used to debounce the tip
    // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
    rg1 = (isnan(rg1) || (rg1 < QC_MIN_RG) || (rg1 > (((float)rg1ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg1;
  }

  // Rain Gauge 2 - Each tip is 0.2mm of rain
  if (cf_op1 == OP1_STATE_RAIN) {
    rg2ds = (millis()-raingauge2_interrupt_stime)/1000;  // seconds since last rain gauge observation logged
    rg2 = raingauge2_interrupt_count * 0.2;
    raingauge2_interrupt_count = 0;
    raingauge2_interrupt_stime = millis();
    raingauge2_interrupt_ltime = 0; // used to debounce the tip
    // QC Check - Max Rain for period is (Observations Seconds / 60s) *  Max Rain for 60 Seconds
    rg2 = (isnan(rg2) || (rg2 < QC_MIN_RG) || (rg2 > (((float)rg2ds / 60) * QC_MAX_RG)) ) ? QC_ERR_RG : rg2;
  }

  if (cf_rg1_enable || (cf_op1 == OP1_STATE_RAIN)) {
    EEPROM_UpdateRainTotals(rg1, rg2);
  }
  
  // Rain Gauge 1
  if (cf_rg1_enable) {
    strcpy (obs.sensor[sidx].id, "rg1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = rg1;
    obs.sensor[sidx++].inuse = true;
  
    strcpy (obs.sensor[sidx].id, "rgt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = eeprom.rgt1;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "rgp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = eeprom.rgp1;
    obs.sensor[sidx++].inuse = true;
  }

  // Rain Gauge 2
  if ((cf_op1 == OP1_STATE_RAIN)) {
    strcpy (obs.sensor[sidx].id, "rg2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = rg2;
    obs.sensor[sidx++].inuse = true;
  
    strcpy (obs.sensor[sidx].id, "rgt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = eeprom.rgt2;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "rgp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = eeprom.rgp2;
    obs.sensor[sidx++].inuse = true;
  }

  if (cf_op1 == OP1_STATE_RAW) {
    // OP1 Raw
    strcpy (obs.sensor[sidx].id, "op1r");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = Pin_ReadAvg(OP1_PIN);
    obs.sensor[sidx++].inuse = true;
  }

  if ((cf_op1 == OP1_STATE_DIST_5M) || (cf_op1 == OP1_STATE_DIST_10M)) {
    float ds_median, ds_median_raw;
    
    ds_median = ds_median_raw = DS_Median();
    if (cf_ds_baseline > 0) {
      ds_median = cf_ds_baseline - ds_median_raw;
    }

    strcpy (obs.sensor[sidx].id, "ds");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ds_median;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "dsr");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ds_median_raw;
    obs.sensor[sidx++].inuse = true;
  }

  if (cf_op2 == OP2_STATE_RAW) {
    // OP2 Raw
    strcpy (obs.sensor[sidx].id, "op2r");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = Pin_ReadAvg(OP2_PIN);
    obs.sensor[sidx++].inuse = true;
  }

  if (cf_op2 == OP2_STATE_VOLTAIC) {
    // OP2 Voltaic Battery Voltage
    float vbv = VoltaicVoltage(OP2_PIN);
    strcpy (obs.sensor[sidx].id, "vbv");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = vbv;
    obs.sensor[sidx++].inuse = true;

    strcpy (obs.sensor[sidx].id, "vpc");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = VoltaicPercent(vbv);
    obs.sensor[sidx++].inuse = true;
  }
  
  if (!cf_nowind) {
    // Wind Speed (Global)
    ws = Wind_SpeedAverage();
    ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
    strcpy (obs.sensor[sidx].id, "ws");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ws;
    obs.sensor[sidx++].inuse = true;

    // Wind Direction
    wd = Wind_DirectionVector();
    wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
    strcpy (obs.sensor[sidx].id, "wd");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = wd;
    obs.sensor[sidx++].inuse = true;

    // Wind Gust (Global)
    ws = Wind_Gust();
    ws = (isnan(ws) || (ws < QC_MIN_WS) || (ws > QC_MAX_WS)) ? QC_ERR_WS : ws;
    strcpy (obs.sensor[sidx].id, "wg");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = ws;
    obs.sensor[sidx++].inuse = true;

    // Wind Gust Direction (Global)
    wd = Wind_GustDirection();
    wd = (isnan(wd) || (wd < QC_MIN_WD) || (wd > QC_MAX_WD)) ? QC_ERR_WD : wd;
    strcpy (obs.sensor[sidx].id, "wgd");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = wd;
    obs.sensor[sidx++].inuse = true;
  }

  //
  // Add I2C Sensors
  //
  if (BMX_1_exists) {
    float p = 0.0;
    float t = 0.0;
    float h = 0.0;

    if (BMX_1_chip_id == BMP280_CHIP_ID) {
      p = bmp1.readPressure()/100.0F;       // bp1 hPa
      t = bmp1.readTemperature();           // bt1
    }
    else if (BMX_1_chip_id == BME280_BMP390_CHIP_ID) {
      if (BMX_1_type == BMX_TYPE_BME280) {
        p = bme1.readPressure()/100.0F;     // bp1 hPa
        t = bme1.readTemperature();         // bt1
        h = bme1.readHumidity();            // bh1
      }
      if (BMX_1_type == BMX_TYPE_BMP390) {
        p = bm31.readPressure()/100.0F;     // bp1 hPa
        t = bm31.readTemperature();         // bt1       
      }
    }
    else { // BMP388
      p = bm31.readPressure()/100.0F;       // bp1 hPa
      t = bm31.readTemperature();           // bt1
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    
    // BMX1 Preasure
    strcpy (obs.sensor[sidx].id, "bp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = p;
    obs.sensor[sidx++].inuse = true;

    // BMX1 Temperature
    strcpy (obs.sensor[sidx].id, "bt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // BMX1 Humidity
    if (BMX_1_type == BMX_TYPE_BME280) {
      strcpy (obs.sensor[sidx].id, "bh1");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = h;
      obs.sensor[sidx++].inuse = true;
    }

    bmx_1_pressure = p; // Used later for mslp calc
  }

  if (BMX_2_exists) {
    float p = 0.0;
    float t = 0.0;
    float h = 0.0;

    if (BMX_2_chip_id == BMP280_CHIP_ID) {
      p = bmp2.readPressure()/100.0F;       // bp2 hPa
      t = bmp2.readTemperature();           // bt2
    }
    else if (BMX_2_chip_id == BME280_BMP390_CHIP_ID) {
      if (BMX_2_type == BMX_TYPE_BME280) {
        p = bme2.readPressure()/100.0F;     // bp2 hPa
        t = bme2.readTemperature();         // bt2
        h = bme2.readHumidity();            // bh2 
      }
      if (BMX_2_type == BMX_TYPE_BMP390) {
        p = bm32.readPressure()/100.0F;       // bp2 hPa
        t = bm32.readTemperature();           // bt2
      }      
    }
    else { // BMP388
      p = bm32.readPressure()/100.0F;       // bp2 hPa
      t = bm32.readTemperature();           // bt2
    }
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

    // BMX2 Preasure
    strcpy (obs.sensor[sidx].id, "bp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = p;
    obs.sensor[sidx++].inuse = true;

    // BMX2 Temperature
    strcpy (obs.sensor[sidx].id, "bt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // BMX2 Humidity
    if (BMX_2_type == BMX_TYPE_BME280) {
      strcpy (obs.sensor[sidx].id, "bh2");
      obs.sensor[sidx].type = F_OBS;
      obs.sensor[sidx].f_obs = h;
      obs.sensor[sidx++].inuse = true;
    }
  }

  if (HTU21DF_exists) {
    float t = 0.0;
    float h = 0.0;
    
    // HTU Humidity
    strcpy (obs.sensor[sidx].id, "hh1");
    obs.sensor[sidx].type = F_OBS;
    h = htu.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;

    // HTU Temperature
    strcpy (obs.sensor[sidx].id, "ht1");
    obs.sensor[sidx].type = F_OBS;
    t = htu.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (SHT_1_exists) {
    float t = 0.0;
    float h = 0.0;

    // SHT1 Temperature
    strcpy (obs.sensor[sidx].id, "st1");
    obs.sensor[sidx].type = F_OBS;
    t = sht1.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // SHT1 Humidity   
    strcpy (obs.sensor[sidx].id, "sh1");
    obs.sensor[sidx].type = F_OBS;
    h = sht1.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;

    sht1_humid = h; // save for derived observations
  }

  if (SHT_2_exists) {
    float t = 0.0;
    float h = 0.0;

    // SHT2 Temperature
    strcpy (obs.sensor[sidx].id, "st2");
    obs.sensor[sidx].type = F_OBS;
    t = sht2.readTemperature();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
    
    // SHT2 Humidity   
    strcpy (obs.sensor[sidx].id, "sh2");
    obs.sensor[sidx].type = F_OBS;
    h = sht2.readHumidity();
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }

  if (HDC_1_exists) {
    double t = -999.9;
    double h = -999.9;

    if (hdc1.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0)) {
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
      h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    }
    else {
      Output (F("ERR:HDC1 Read"));
    }

    // HDC1 Temperature
    strcpy (obs.sensor[sidx].id, "hdt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // HDC1 Humidity
    strcpy (obs.sensor[sidx].id, "hdh1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) h;
    obs.sensor[sidx++].inuse = true;
  }

  if (HDC_2_exists) {
    double t = -999.9;
    double h = -999.9;

    if (hdc2.readTemperatureHumidityOnDemand(t, h, TRIGGERMODE_LP0)) {
      t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
      h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;
    }
    else {
      Output (F("ERR:HDC1 Read"));
    }

    // HDC2 Temperature
    strcpy (obs.sensor[sidx].id, "hdt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // HDC2 Humidity
    strcpy (obs.sensor[sidx].id, "hdh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) h;
    obs.sensor[sidx++].inuse = true;
  }

  if (LPS_1_exists) {
    float t = lps1.readTemperature();
    float p = lps1.readPressure();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;

    // LPS1 Temperature
    strcpy (obs.sensor[sidx].id, "lpt1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // LPS1 Pressure
    strcpy (obs.sensor[sidx].id, "lpp1");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) p;
    obs.sensor[sidx++].inuse = true;
  }

  if (LPS_2_exists) {
    float t = lps2.readTemperature();
    float p = lps2.readPressure();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    p = (isnan(p) || (p < QC_MIN_P)  || (p > QC_MAX_P))  ? QC_ERR_P  : p;

    // LPS1 Temperature
    strcpy (obs.sensor[sidx].id, "lpt2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;

    // LPS1 Pressure
    strcpy (obs.sensor[sidx].id, "lpp2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) p;
    obs.sensor[sidx++].inuse = true;
  }
  
  if (HIH8_exists) {
    float t = 0.0;
    float h = 0.0;
    bool status = hih8_getTempHumid(&t, &h);
    if (!status) {
      t = -999.99;
      h = 0.0;
    }
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    h = (isnan(h) || (h < QC_MIN_RH) || (h > QC_MAX_RH)) ? QC_ERR_RH : h;

    // HIH8 Temperature
    strcpy (obs.sensor[sidx].id, "ht2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    // HIH8 Humidity
    strcpy (obs.sensor[sidx].id, "hh2");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = h;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_1_exists) {
    float t = 0.0;
   
    // MCP1 Temperature
    strcpy (obs.sensor[sidx].id, "mt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp1.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_2_exists) {
    float t = 0.0;
    
    // MCP2 Temperature
    strcpy (obs.sensor[sidx].id, "mt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp2.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (MCP_3_exists) {
    float t = 0.0;

    // MCP3 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt1");
    obs.sensor[sidx].type = F_OBS;
    t = mcp3.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;

    mcp3_temp = t; // globe temperature
  }

  if (MCP_4_exists) {
    float t = 0.0;

    // MCP4 Globe Temperature
    strcpy (obs.sensor[sidx].id, "gt2");
    obs.sensor[sidx].type = F_OBS;
    t = mcp4.readTempC();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;
    obs.sensor[sidx].f_obs = t;
    obs.sensor[sidx++].inuse = true;
  }

  if (VEML7700_exists) {
    float lux = veml.readLux(VEML_LUX_AUTO);
    lux = (isnan(lux) || (lux < QC_MIN_VLX)  || (lux > QC_MAX_VLX))  ? QC_ERR_VLX  : lux;

    // VEML7700 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "vlx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
    obs.sensor[sidx++].inuse = true;
  }

  if (BLX_exists) {
    float lux=blx_takereading();
    lux = (isnan(lux) || (lux < QC_MIN_BLX)  || (lux > QC_MAX_BLX))  ? QC_ERR_BLX  : lux;

    // DFR BLUX30 Auto Lux Value
    strcpy (obs.sensor[sidx].id, "blx");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = lux;
    obs.sensor[sidx++].inuse = true;
  }

  if (PM25AQI_exists) {
    // Atmospheric Environmental PM1.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e10");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.e10;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM2.5 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e25");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.e25;
    obs.sensor[sidx++].inuse = true;

    // Atmospheric Environmental PM10.0 concentration unit µg m3
    strcpy (obs.sensor[sidx].id, "pm1e100");
    obs.sensor[sidx].type = I_OBS;
    obs.sensor[sidx].i_obs = pm25aqi_obs.e100;
    obs.sensor[sidx++].inuse = true;

    // Clear readings
    pm25aqi_clear();
  }
  
  // Heat Index Temperature
  if (HI_exists) {
    heat_index = hi_calculate(sht1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "hi");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) heat_index;
    obs.sensor[sidx++].inuse = true;    
  } 
  
  // Wet Bulb Temperature
  if (WBT_exists) {
    wetbulb_temp = wbt_calculate(sht1_temp, sht1_humid);
    strcpy (obs.sensor[sidx].id, "wbt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wetbulb_temp;
    obs.sensor[sidx++].inuse = true;  
  }

  // Wet Bulb Globe Temperature
  if (WBGT_exists) {
    float wbgt = 0.0;
    if (MCP_3_exists) {
      wbgt = wbgt_using_wbt(sht1_temp, mcp3_temp, wetbulb_temp); // TempAir, TempGlobe, TempWetBulb
    }
    else {
      wbgt = wbgt_using_hi(heat_index);
    }
    strcpy (obs.sensor[sidx].id, "wbgt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) wbgt;
    obs.sensor[sidx++].inuse = true;    
  }

  if (MSLP_exists) {
    float mslp = (float) mslp_calculate(sht1_temp, sht1_humid, bmx_1_pressure, cf_elevation);
    strcpy (obs.sensor[sidx].id, "mslp");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) mslp;
    obs.sensor[sidx++].inuse = true;  
  }

  // Tinovi Leaf Wetness
  if (TLW_exists) {
    tlw.newReading();
    delay(100);
    float w = tlw.getWet();
    float t = tlw.getTemp();
    t = (isnan(t) || (t < QC_MIN_T)  || (t > QC_MAX_T))  ? QC_ERR_T  : t;

    strcpy (obs.sensor[sidx].id, "tlww");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) w;
    obs.sensor[sidx++].inuse = true; 

    strcpy (obs.sensor[sidx].id, "tlwt");
    obs.sensor[sidx].type = F_OBS;
    obs.sensor[sidx].f_obs = (float) t;
    obs.sensor[sidx++].inuse = true;
  }

  // Tinovi Soil Moisture
  mux_obs_do(sidx);

  // Dallas Sensors Temperature on mux
  dsmux_obs_do(sidx);
}

/*
 * ======================================================================================================================
 * OBS_Do() - Do Observation Processing
 * ======================================================================================================================
 */
void OBS_Do() {
  bool OK2Send = false;
  
  Output(F("OBS_DO()"));
  Output(F("OBS_TAKE()"));
  OBS_Take();          // Take a observation
  
  Output(F("OBS_BUILD()"));
  
  if (OBS_Build_JSON()) { // This will also print the JSON out - obsbuf has the json
    // At this point, the obs data structure has been filled in with observation data

    Output(F("OBS->SD"));
    // Serial_writeln (obsbuf);
    SD_LogObservation(obsbuf); // Saves Main observations to Log file. LoRa observations are not saved. LoRa devices have their own SD card

    Output(F("OBS_SEND()"));
  
    if (!Send_http(obsbuf, cf_webserver, cf_webserver_port, cf_urlpath, METHOD_GET, cf_apikey)) {  
      Output(F("FS->PUB FAILED"));
      OBS_N2S_Save(); // Saves Main observations and Lora observations
      
      OBS_PubFailCnt++; // This is catching if network is not connected and if we get bad http responses when connected
    }
    else {
      OK2Send = true; 
      Output(F("FS->PUB OK"));
      OBS_PubFailCnt=0;
    }
  }
  else {
    OK2Send = false;
  }

  // Publish LoRa Relay Observations   
  if (LORA_exists) {
      
    // We want to transmit all the LoRa msgs or save them to N2S file if we can not transmit them.
    while (lora_relay_need2log()) {      
      lora_relay_build_JSON(); // Copy JSON observation to obsbuf, remove from relay structure
      if (OK2Send) {
         OK2Send = Send_http(obsbuf, cf_webserver, cf_webserver_port, cf_urlpath, METHOD_GET, cf_apikey);
         // Note a new LoRa RS msgs could be received as we are sending    
      }
      if (!OK2Send) {
        SD_NeedToSend_Add(obsbuf); // Save to N2F File
        Output(F("LR->N2S"));
        Serial_write (msgbuf); 
      }
    }
  }

  // Check if we have any N2S only if we have not added to the file while trying to send OBS
  if (OK2Send) {
    SD_N2S_Publish(); 
  }
}

