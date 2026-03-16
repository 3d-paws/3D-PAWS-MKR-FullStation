/*
 * ======================================================================================================================
 *  info.cpp - Station Information Functions
 * ======================================================================================================================
 */
#include <Arduino.h>
#include <SdFat.h>
#include <RTClib.h>

#include "include/ssbits.h"
#include "include/eeprom.h"
#include "include/cf.h"
#include "include/mux.h"
#include "include/dsmux.h"
#include "include/sensors.h"
#include "include/wrda.h"
#include "include/sdcard.h"
#include "include/output.h"
#include "include/network.h"
#include "include/support.h"
#include "include/time.h"
#include "include/mkrboard.h"
#include "include/lora.h"
#include "include/obs.h"
#include "include/main.h"
#include "include/info.h"

/*
 * ======================================================================================================================
 * Variables and Data Structures 
 * =======================================================================================================================
 */
bool info_server_valid=false;       // If config file has valid Info Server configuration (WiFi) then this is true

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

 /*
 * ======================================================================================================================
 * INFO_Initialize() - Test if we have set the Info Server in the config file
 * =======================================================================================================================
 */
void INFO_Initialize() {
  if (cf_info_server  && cf_info_server[0]  != '\0' &&
      cf_info_urlpath && cf_info_urlpath[0] != '\0' &&
      cf_info_apikey  && cf_info_apikey[0]  != '\0' &&
      cf_info_server_port != 0) {

    info_server_valid = true;
    Output (F("INFO_Server:OK"));
  }
  else {
    info_server_valid = false;
    Output (F("INFO_Server:NOT SET"));
  }
}

/*
 * ======================================================================================================================
 * INFO_Perform() - Send System Information
 *   Build complete info message to be saved in INFO.TXT and SENT to the information server
 * =======================================================================================================================
 */
void INFO_Perform() {
  char msg[1024];   // Holds JSON information 
  const char *comma = "";

  memset(msg, 0, 1024);

  rtc_timestamp();
  
  sprintf (msg, "{\"MT\":\"INFO\""); // Message Type -> INFO

  // AFM0WiFi = Adafruit Feather M0 WiFi
  sprintf (msg+strlen(msg), ",\"at\":\"%s\",\"devid\":\"%s\",\"board\":\"AFM0WiFi\"", // Adafruit Feather M0 WiFi
    timestamp, DeviceID);

  int bcs = get_batterystate();
  sprintf (msg+strlen(msg), ",\"ver\":\"%s\",\"bcs\":%s,\"hth\":%d,\"elev\":%d,\"rtro\":%d",
    versioninfo, batterystate[bcs], SystemStatusBits, cf_elevation, cf_rtro);

  // Log Server Information and Chords Apikey and Id
  sprintf (msg+strlen(msg), ",\"ls\":\"%s\",\"lsp\":%d,\"lsurl\":\"%s\",\"lsapi\":\"%s\",\"lsid\":%d",
    cf_webserver, cf_webserver_port, cf_urlpath, cf_apikey, cf_instrument_id);

  // obs_period (1,5,6,10,15,20,30), 1 minute observation period is the default
  // daily_reboot Number of hours between daily reboots, A value of 0 disables this feature
  sprintf (msg+strlen(msg), ",\"obsi\":\"%dm\",\"t2nt\":\"%ds\",\"drbt\":\"%dm\"",
    cf_obs_period, (int)((millis()-time_to_next_obs())/1000), cf_daily_reboot);

  SD_NeedToSend_Status(Buffer32Bytes);
  sprintf (msg+strlen(msg), ",\"n2s\":%s", Buffer32Bytes);

  // Discovered Device List
  comma="";
  sprintf (msg+strlen(msg), ",\"devs\":\"");
  if (RTC_exists) {
    sprintf (msg+strlen(msg), "%srtc", comma);
    comma=",";
  }
  if (SD_exists) {
    sprintf (msg+strlen(msg), "%ssd", comma);
    comma=",";
  }
  if (eeprom_exists) {
    sprintf (msg+strlen(msg), "%seeprom", comma);
    comma=",";    
  }
  if (MUX_exists) {
    sprintf (msg+strlen(msg), "%smux", comma);
    comma=",";    
  }
  if (DSMUX_exists) {
    sprintf (msg+strlen(msg), "%sdsmux", comma);
    comma=",";    
  }
  if (LORA_exists) {
    sprintf (msg+strlen(msg), ",lora(%d,%d,%dMHz)", cf_lora_unitid, cf_lora_txpwr, cf_lora_freq);  
  }
   if (oled_type) {
    sprintf (msg+strlen(msg), ",oled(%s)", OLED32 ? "32" : "64");  
  }
  sprintf (msg+strlen(msg), "\""); 

  // SENSORS
  comma="";
  sprintf (msg+strlen(msg), ",\"sensors\":\"");

  if (BMX_1_exists) {
    sprintf (msg+strlen(msg), "%sBMX1(%s)", comma, bmxtype[BMX_1_type]);
    comma=",";
  }
  if (BMX_2_exists) {
    sprintf (msg+strlen(msg), "%sBMX2(%s)", comma, bmxtype[BMX_2_type]);
    comma=",";
  }
  if (MCP_1_exists) {
    sprintf (msg+strlen(msg), "%sMCP1", comma);
    comma=",";
  }
  if (MCP_2_exists) {
    sprintf (msg+strlen(msg), "%sMCP2", comma);
    comma=",";
  }
  if (MCP_3_exists) {
    sprintf (msg+strlen(msg), "%sMCP3/gt1", comma);
    comma=",";
  }
  if (MCP_4_exists) {
    sprintf (msg+strlen(msg), "%sMCP4/gt2", comma);
    comma=",";
  }
  if (SHT_1_exists) {
    sprintf (msg+strlen(msg), "%sSHT1", comma);
    comma=",";
  }
  if (SHT_2_exists) {
    sprintf (msg+strlen(msg), "%sSHT2", comma);
    comma=",";
  }
  if (HDC_1_exists) {
    sprintf (msg+strlen(msg), "%sHDC1", comma);
    comma=",";
  }
  if (HDC_2_exists) {
    sprintf (msg+strlen(msg), "%sHDC2", comma);
    comma=",";
  }
  if (LPS_1_exists) {
    sprintf (msg+strlen(msg), "%sLPS1", comma);
    comma=",";
  }
  if (LPS_2_exists) {
    sprintf (msg+strlen(msg), "%sLPS2", comma);
    comma=",";
  }
  if (HIH8_exists) {
    sprintf (msg+strlen(msg), "%sHIH8", comma);
    comma=",";
  }
  if (VEML7700_exists) {
    sprintf (msg+strlen(msg), "%sVEML", comma);
    comma=",";
  }
  if (BLX_exists) {
    sprintf (msg+strlen(msg), "%sBLX", comma);
    comma=",";
  }
  if (cf_nowind) {
    sprintf (msg+strlen(msg), "%s!WIND", comma);
    comma=",";
  }
  else {
    sprintf (msg+strlen(msg), "%sWIND", comma);
    comma=",";
    sprintf (msg+strlen(msg), "%sWS(%s)", comma, pinNames[ANEMOMETER_IRQ_PIN]);

    if (AS5600_exists) {
      sprintf (msg+strlen(msg), "%sAS5600", comma);
    }
    else {
      sprintf (msg+strlen(msg), "%s!AS5600", comma);
    }
  }
  if (TLW_exists) {
    sprintf (msg+strlen(msg), "%sTLW", comma);
    comma=",";
  } 
  if (TSM_exists) {
    sprintf (msg+strlen(msg), "%sTSM", comma);
    comma=",";
  }

  // MUX SENSORS  
  if (MUX_exists) {
    for (int c=0; c<MUX_CHANNELS; c++) {
      if (mux[c].inuse) {
        for (int s = 0; s < MAX_CHANNEL_SENSORS; s++) {
          if (mux[c].sensor[s].type == m_tsm) {
            sprintf (msg+strlen(msg), "%sTSM%d(%d.%d)", comma, mux[c].sensor[s].id, c, s);
            comma=",";
          }
        }
      }
    }
  }

  // DSMUX Temperature Sensors  
  if (DSMUX_exists) {
    sprintf (msg+strlen(msg), "%sDST(", comma);
    
    int count=0;
    const char *comma1="";
    for (int c=0; c<DS248X_CHANNELS; c++) {
      if (dsmux_sensor_exists[c]) {
        count++;
        sprintf (msg+strlen(msg), "%s%d", comma1, c);
        comma1=",";
      }
    }
    if (count) {
      sprintf (msg+strlen(msg), ")");
    }
    else {
      sprintf (msg+strlen(msg), "NF)");
    }
  }

  if (HI_exists) {
    sprintf (msg+strlen(msg), "%sHI", comma);
    comma=",";
  }
  if (WBT_exists) {
    sprintf (msg+strlen(msg), "%sWBT", comma);
    comma=",";
  }
  if (WBGT_exists) {
    if (MCP_3_exists) {
      sprintf (msg+strlen(msg), "%sWBGT W/GLOBE", comma);
    }
    else {
      sprintf (msg+strlen(msg), "%sWBGT WO/GLOBE", comma);
    }
    comma=",";
  }
  if (PM25AQI_exists) {
    sprintf (msg+strlen(msg), "%sPM25AQ", comma);
  }
  if (cf_rg1_enable) {
    sprintf (msg+strlen(msg), "%sRG1(%s)", comma, pinNames[RAINGAUGE1_IRQ_PIN]); 
    comma=",";
  } 
  if (cf_op1 == OP1_STATE_RAW) {
    sprintf (msg+strlen(msg), "%sOP1R(%s)", comma, pinNames[OP1_PIN]);
    comma=",";
  } 
  if (cf_op1 == OP1_STATE_RAIN) {
    sprintf (msg+strlen(msg), "%sRG2(%s)", comma, pinNames[RAINGAUGE2_IRQ_PIN]);
    comma=",";
  } 
  if (cf_op1 == OP1_STATE_DIST_5M) {
    sprintf (msg+strlen(msg), "%s5MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], cf_ds_baseline);
    comma=",";
  } 
  if (cf_op1 == OP1_STATE_DIST_10M) {
    sprintf (msg+strlen(msg), "%s10MDIST(%s,%d)", 
      comma, pinNames[DISTANCE_GAUGE_PIN], cf_ds_baseline);
    comma=",";
  }
  if (cf_op2 == OP2_STATE_RAW) {
    sprintf (msg+strlen(msg), "%sOP2R(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  }
  if (cf_op2 == OP2_STATE_VOLTAIC) {
    sprintf (msg+strlen(msg), "%sVBV(%s)", comma, pinNames[OP2_PIN]);
    comma=",";
  } 

   // Close off sensors
  sprintf (msg+strlen(msg), "\"");

  // Adding closing }
  sprintf (msg+strlen(msg), "}");

  Serial_writeln(msg); 

  if (Send_http(msg, cf_info_server, cf_info_server_port, cf_info_urlpath, METHOD_POST, cf_info_apikey)) {
    Output("INFO->PUB WiFi OK");
  }
  else {
    Output("INFO->PUB WiFi FAILED");

    // Save to the N2S File
    SD_NeedToSend_Add(msg);
  }

  // Update INFO.TXT file
  Serial_writeln(msg);
  SD_UpdateInfoFile(msg);
}

/*
 * ======================================================================================================================
 * INFO_Do() - Get and Send System Information
 * =======================================================================================================================
 */
void INFO_Do() {
  if (info_server_valid) {
    INFO_Perform();
  }
  else {
    Output ("INFO:INVLD INFO SVR");
  }
  nextinfo = millis() + INFO_TIME_INTERVAL;
}