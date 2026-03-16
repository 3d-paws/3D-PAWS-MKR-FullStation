/*
 * ======================================================================================================================
 *  cf.cpp - Configuration File Functions
 * ======================================================================================================================
 */
#include <SdFat.h>

#include "include/ssbits.h"
#include "include/qc.h"
#include "include/mkrboard.h"
#include "include/support.h"
#include "include/output.h"
#include "include/sdcard.h"
#include "include/wrda.h"
#include "include/main.h"
#include "include/cf.h"

/*
 * ======================================================================================================================
 *  Define Global Configuration File Variables
 * ======================================================================================================================
 */
// Web Server
char *cf_webserver=NULL;
int  cf_webserver_port=80;
char *cf_urlpath=NULL;
char *cf_apikey=NULL;
int cf_instrument_id=0;
// Info Server
char *cf_info_server=NULL;
int  cf_info_server_port=80;
char *cf_info_urlpath=NULL;
char *cf_info_apikey=NULL; 
// SIM
char *cf_sim_pin=NULL;
char *cf_sim_apn=NULL;
char *cf_sim_username=NULL;
char *cf_sim_password=NULL;
// LoRa AES
char *cf_aes_pkey=NULL;
long cf_aes_myiv=0;
// LoRa
int cf_lora_unitid=1;
int cf_lora_txpwr=5;
int cf_lora_freq=915;
// Instruments
int cf_nowind=0;
int cf_rg1_enable=0;
int cf_op1=OP1_STATE_NULL;
int cf_op2=OP2_STATE_NULL;
int cf_ds_baseline=0;
int cf_elevation=0;
// System Timing
int cf_obs_period=0;
int cf_daily_reboot=22;
int cf_no_network_reset_count=60;
int cf_rtro=0;

/*
 * ======================================================================================================================
 * Fuction Definations
 * =======================================================================================================================
 */

/* 
 *=======================================================================================================================
 * Support functions for Config file
 * 
 *  https://arduinogetstarted.com/tutorials/arduino-read-config-from-sd-card
 *  
 *  myInt_1    = SD_findInt(F("myInt_1"));
 *  myFloat_1  = SD_findFloat(F("myFloat_1"));
 *  myString_1 = SD_findString(F("myString_1"));
 *  
 *  CONFIG.TXT content example
 *  myString_1=Hello
 *  myInt_1=2
 *  myFloat_1=0.74
 *=======================================================================================================================
 */

int SD_findKey(const __FlashStringHelper * key, char * value) {
  File configFile = SD.open(CF_NAME);

  if (!configFile) {
    Serial.print(F("SD Card: error on opening file "));
    Serial.println(CF_NAME);
    return(0);
  }

  char key_string[KEY_MAX_LENGTH];
  char SD_buffer[KEY_MAX_LENGTH + VALUE_MAX_LENGTH + 1]; // 1 is = character
  int key_length = 0;
  int value_length = 0;

  // Flash string to string
  PGM_P keyPoiter;
  keyPoiter = reinterpret_cast<PGM_P>(key);
  byte ch;
  do {
    ch = pgm_read_byte(keyPoiter++);
    if (ch != 0)
      key_string[key_length++] = ch;
  } while (ch != 0);

  // check line by line
  while (configFile.available()) {
    int buffer_length = configFile.readBytesUntil('\n', SD_buffer, LINE_MAX_LENGTH);
    if (SD_buffer[buffer_length - 1] == '\r')
      buffer_length--; // trim the \r

    if (buffer_length > (key_length + 1)) { // 1 is = character
      if (memcmp(SD_buffer, key_string, key_length) == 0) { // equal
        if (SD_buffer[key_length] == '=') {
          value_length = buffer_length - key_length - 1;
          memcpy(value, SD_buffer + key_length + 1, value_length);
          break;
        }
      }
    }
  }

  configFile.close();  // close the file
  return value_length;
}

int HELPER_ascii2Int(char *ascii, int length) {
  int sign = 1;
  int number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

long HELPER_ascii2Long(char *ascii, int length) {
  int sign = 1;
  long number = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c >= '0' && c <= '9')
        number = number * 10 + (c - '0');
    }
  }

  return number * sign;
}

float HELPER_ascii2Float(char *ascii, int length) {
  int sign = 1;
  int decimalPlace = 0;
  float number  = 0;
  float decimal = 0;

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    if (i == 0 && c == '-')
      sign = -1;
    else {
      if (c == '.')
        decimalPlace = 1;
      else if (c >= '0' && c <= '9') {
        if (!decimalPlace)
          number = number * 10 + (c - '0');
        else {
          decimal += ((float)(c - '0') / pow(10.0, decimalPlace));
          decimalPlace++;
        }
      }
    }
  }

  return (number + decimal) * sign;
}

String HELPER_ascii2String(char *ascii, int length) {
  String str;
  str.reserve(length);
  str = "";

  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str += String(c);
  }
  return str;
}

char* HELPER_ascii2CharStr(char *ascii, int length) {
  char *str;
  int i = 0;
  str = (char *) malloc (length+1);
  str[0] = 0;
  for (int i = 0; i < length; i++) {
    char c = *(ascii + i);
    str[i] = c;
    str[i+1] = 0;
  }
  return str;
}

bool SD_available(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return value_length > 0;
}

int SD_findInt(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Int(value_string, value_length);
}

float SD_findFloat(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Float(value_string, value_length);
}

String SD_findString(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2String(value_string, value_length);
}

char* SD_findCharStr(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2CharStr(value_string, value_length);
}

long SD_findLong(const __FlashStringHelper * key) {
  char value_string[VALUE_MAX_LENGTH];
  int value_length = SD_findKey(key, value_string);
  return HELPER_ascii2Long(value_string, value_length);
}

/* 
 *=======================================================================================================================
 * SD_ReadConfigFile()
 *=======================================================================================================================
 */
void SD_ReadConfigFile() {
  // Web Server
  cf_webserver      = SD_findCharStr(F("webserver"));
  sprintf(msgbuf, "CF:%s=[%s]", F("webserver"), cf_webserver); Output (msgbuf);
  
  cf_webserver_port = SD_findInt(F("webserver_port"));
  if (cf_webserver_port == 0) {
    cf_webserver_port = 80;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("webserver_port"), cf_webserver_port); Output (msgbuf);

  cf_urlpath = SD_findCharStr(F("urlpath"));
  sprintf(msgbuf, "CF:%s=[%s]", F("urlpath"), cf_urlpath); Output (msgbuf);
  
  cf_apikey         = SD_findCharStr(F("apikey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("apikey"), cf_apikey); Output (msgbuf);
  
  cf_instrument_id  = SD_findInt(F("instrument_id"));
  sprintf(msgbuf, "CF:%s=[%d]", F("instrument_id"), cf_instrument_id); Output (msgbuf);

  // Info Server
  cf_info_server  = SD_findCharStr(F("info_server"));
  sprintf(msgbuf, "%s=[%s]", F("CF:info_server"), cf_info_server); Output (msgbuf);

  cf_info_server_port = SD_findInt(F("info_server_port"));
  sprintf(msgbuf, "%s=[%d]", F("CF:info_server_port"), cf_info_server_port); Output (msgbuf);

  cf_info_urlpath = SD_findCharStr(F("info_urlpath"));
  sprintf(msgbuf, "%s=[%s]", F("CF:info_urlpath"), cf_info_urlpath); Output (msgbuf);

  cf_info_apikey  = SD_findCharStr(F("info_apikey"));
  sprintf(msgbuf, "%s=[%s]", F("CF:info_apikey"), cf_info_apikey); Output (msgbuf);
  
  // SIM
  cf_sim_apn        = SD_findCharStr(F("sim_apn"));
  sprintf(msgbuf, "CF:%s=[%s]", F("sim_apn"), cf_sim_apn); Output (msgbuf);
  
  cf_sim_pin   = SD_findCharStr(F("sim_pin"));
  sprintf(msgbuf, "CF:%s=[%s]", F("sim_pin"), cf_sim_pin); Output (msgbuf);
  
  cf_sim_username   = SD_findCharStr(F("sim_username"));
  sprintf(msgbuf, "CF:%s=[%s]", F("sim_username"), cf_sim_username); Output (msgbuf);
  
  cf_sim_password   = SD_findCharStr(F("sim_password"));
  sprintf(msgbuf, "CF:%s=[%s]", F("sim_password"), cf_sim_password); Output (msgbuf);
  
  // LoRa AES
  cf_aes_pkey   = SD_findCharStr(F("aes_pkey"));
  sprintf(msgbuf, "CF:%s=[%s]", F("aes_pkey"), cf_aes_pkey); Output (msgbuf);
  
  cf_aes_myiv    = SD_findLong(F("aes_myiv"));
  sprintf(msgbuf, "CF:%s=[%lu]", F("aes_myiv"), cf_aes_myiv); Output (msgbuf);

  // LoRa
  cf_lora_unitid     = SD_findInt(F("lora_unitid"));
  if (cf_lora_unitid == 0) {
    cf_lora_unitid = 1;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("lora_unitid"), cf_lora_unitid); Output (msgbuf);
  
  cf_lora_txpwr     = SD_findInt(F("lora_txpwr"));
  if (cf_lora_txpwr == 0) {
    cf_lora_txpwr = 5;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("lora_txpwr"), cf_lora_txpwr); Output (msgbuf);
  
  cf_lora_freq      = SD_findInt(F("lora_freq"));
  if (cf_lora_freq == 0) {
    cf_lora_freq = 915;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("lora_freq"), cf_lora_freq); Output (msgbuf);

  // No Wind = 1
  cf_nowind      = SD_findInt(F("nowind"));
  sprintf(msgbuf, "CF:%s=[%d]", F("nowind"), cf_nowind); Output (msgbuf);

  // Rain Gauge 1 D1
  cf_rg1_enable   = SD_findInt(F("rg1_enable"));
  sprintf(msgbuf, "CF:%s=[%d]", F("rg1_enable"), cf_rg1_enable); Output (msgbuf);

  // Option Pin 1 A1
  cf_op1   = SD_findInt(F("op1"));
  sprintf(msgbuf, "%s=[%d]", F("CF:op1"), cf_op1); Output (msgbuf);
  if ((cf_op1 != OP1_STATE_NULL) && 
      (cf_op1 != OP1_STATE_RAW) && 
      (cf_op1 != OP1_STATE_RAIN) && 
      (cf_op1 != OP1_STATE_DIST_5M) && 
      (cf_op1 != OP1_STATE_DIST_10M)) {
    cf_op1 = OP1_STATE_NULL;
    Output (" OP1 Invalid");
  }
  else if (cf_op1 == OP1_STATE_DIST_5M) {
    dg_resolution_adjust = 5;
    Output (" DIST5M Set");
    pinMode(OP1_PIN, INPUT);
  }
  else if (cf_op1 == OP1_STATE_DIST_10M) {
    dg_resolution_adjust = 10;
    Output (" DIST10M Set");
    pinMode(OP1_PIN, INPUT);
  }
  else if ((cf_op1 == OP1_STATE_RAW) || 
           (cf_op1 == OP1_STATE_RAIN)) {
    pinMode(OP2_PIN, INPUT);
  }

  // Option Pin 2 A2
  cf_op2    = SD_findInt(F("op2"));
  sprintf(msgbuf, "%s=[%d]", F("CF:op2"), cf_op2); Output (msgbuf);
  if ((cf_op2 != OP2_STATE_NULL) && 
      (cf_op2 != OP2_STATE_RAW) && 
      (cf_op2 != OP2_STATE_VOLTAIC)) {
    cf_op2 = OP2_STATE_NULL;
    Output (" OP2 Invalid");
  }
  if ((cf_op2 == OP2_STATE_RAW) || (cf_op2 == OP2_STATE_VOLTAIC)) {
     pinMode(OP2_PIN, INPUT);
  }

  cf_ds_baseline = SD_findInt(F("ds_baseline"));
  sprintf(msgbuf, "CF:%s=[%d]", F("ds_baseline"), cf_ds_baseline); Output (msgbuf);

  // System Timing
  cf_obs_period   = SD_findInt(F("obs_period"));
  if (cf_obs_period == 0) {
    cf_obs_period = 1;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("obs_period"), cf_obs_period); Output (msgbuf);

  int cf_rtro = SD_findInt(F("rtro"));
  if ((cf_rtro < 0) || (cf_rtro > 23)){
    cf_rtro = 0;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("cf_rtro"), cf_rtro); Output (msgbuf);

  int cf_rain_total_rollover;

  // Misc
  cf_daily_reboot = SD_findInt(F("daily_reboot"));
  sprintf(msgbuf, "%s=[%d]", F("CF:daily_reboot"), cf_daily_reboot); Output (msgbuf);
  
  cf_no_network_reset_count = SD_findInt(F("no_network_reset_count"));
  if (cf_no_network_reset_count == 0) {
    cf_no_network_reset_count = 60;
  }
  sprintf(msgbuf, "CF:%s=[%d]", F("no_network_reset_count"), cf_no_network_reset_count); Output (msgbuf);
}